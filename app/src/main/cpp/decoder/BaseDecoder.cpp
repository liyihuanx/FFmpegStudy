//
// Created by leeyh on 2022/9/27.
//

#include "BaseDecoder.h"
#include "../util/util.h"

void BaseDecoder::onCreate(char *url, AVMediaType mediaType) {
    // 深拷贝，免得变量释放后导致错误，长度+1 ==> 处理 \0
    data_source = new char[strlen(url) + 1];
    // 把源Copy给成员
    strcpy(data_source, url);

    // 记录media类型
    avMediaType = mediaType;
}

void BaseDecoder::onDestroy() {
    if (data_source) {
        delete data_source;
        data_source = nullptr;
    }

    if (frame != nullptr) {
        av_frame_free(&frame);
        frame = nullptr;
    }

    if (packet != nullptr) {
        av_packet_free(&packet);
        packet = nullptr;
    }

    if (avCodecContext != nullptr) {
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = nullptr;
        avCodec = nullptr;
    }

    if (avFormatContext != nullptr) {
        avformat_close_input(&avFormatContext);
        avformat_free_context(avFormatContext);
        avFormatContext = nullptr;
    }

}

void BaseDecoder::start() {
    LOGD("BaseDecoder::start()")
    decode_thread = new thread(startDecodeTread, this);
}


void BaseDecoder::startDecodeTread(BaseDecoder *decoder) {
    LOGD("BaseDecoder::startDecodeTread");
    // 初始化上下文环境
    if (decoder->initFFmpegEnvironment() != 0) {
        LOGD("decoder->initFFmpegEnvironment() error")
        return;
    }

    // 初始化解码环境
    decoder->initDecoderEnvironment();

    // 开始循环解码
    decoder->startDecoder();

    // 解码完释放
    decoder->releaseDecoder();
}

int BaseDecoder::initFFmpegEnvironment() {
    int ret = -1;
    // 1.
    avFormatContext = avformat_alloc_context();

    LOGD("data_source: %s", data_source)
    // 2.
    ret = avformat_open_input(&avFormatContext, data_source, nullptr, nullptr);
    if (ret != 0) {
        LOGD("avformat_open_input error=%s", av_err2str(ret));
        goto end;
    }

    // 3.
    ret = avformat_find_stream_info(avFormatContext, nullptr);
    if (ret < 0) {
        LOGD("avformat_find_stream_info error=%s", av_err2str(ret));
        goto end;
    }

    // 4. 在onCreate记录了mediatype
    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
        AVMediaType codec_type = avFormatContext->streams[i]->codecpar->codec_type;
        if (codec_type == avMediaType) {
            stream_index = i;
            break;
        }
    }

    // 5.打开解码器
    avCodec = avcodec_find_decoder(avFormatContext->streams[stream_index]->codecpar->codec_id);
    if (avCodec == nullptr) {
        LOGD("avcodec_find_decoder error")
        goto end;
    }

    // 6.
    avCodecContext = avcodec_alloc_context3(avCodec);
    if (avCodecContext == nullptr) {
        LOGD("avcodec_alloc_context3 error")
        goto end;
    }

    ret = avcodec_parameters_to_context(avCodecContext,
                                        avFormatContext->streams[stream_index]->codecpar);
    if (ret < 0) {
        LOGD("avcodec_parameters_to_context error=%s", av_err2str(ret));
        goto end;
    }

    // 7.
    ret = avcodec_open2(avCodecContext, avCodec, nullptr);
    if (ret < 0) {
        LOGD("avcodec_open2 ret=%d, error=%s", ret, av_err2str(ret));
        goto end;
    }

    frame = av_frame_alloc();

    packet = av_packet_alloc();

    // 顺利走到这则说明ok
    ret = 0;

    end:
    return ret;
}


// 开始解码工作
void BaseDecoder::startDecoder() {
    LOGD("startDecoder MediaType=%d", avMediaType);
    {
        std::unique_lock<std::mutex> lock(decode_mutex);
        decoderState = STATE_DECODING;
        lock.unlock();
    }

    for (;;) {
        while (decoderState == STATE_PAUSE) {
            std::unique_lock<std::mutex> lock(decode_mutex);
            LOGD("DecoderBase::DecodingLoop waiting, avMediaType=%d", avMediaType);
            decode_cond.wait_for(lock, std::chrono::milliseconds(10));
            startPlayTime = getSysCurrentTime() - curPlayTime;
        }

        if (decoderState == STATE_STOP) {
            break;
        }

        if (startPlayTime == -1) {
            startPlayTime = getSysCurrentTime();
        }

        if (decodeOnePacket() != 0) {
            //解码结束，暂停解码器
            std::unique_lock<std::mutex> lock(decode_mutex);
            decoderState = STATE_PAUSE;
        }
    }
    LOGD("BaseDecoder::startDecoder end");


}

int BaseDecoder::decodeOnePacket() {
    LOGD("decodeOnePacket");

    int result = -1;
    //
    while (av_read_frame(avFormatContext, packet) == 0) {
        if (packet->stream_index == stream_index) {
            if (avcodec_send_packet(avCodecContext, packet) == AVERROR_EOF) {
                //解码结束
                goto end;
            }

//            int frameCount = 0;
            // 获取到YUV帧
            while (avcodec_receive_frame(avCodecContext, frame) >= 0) {
                // 更新时间戳
                updateTimeStamp();
                // 同步
                syncAV();
                // 渲染
                onFrameAvailable(frame);
//                frameCount++;
            }

            //判断一个 packet 是否解码完成 ???
//            if (frameCount > 0) {
//                result = 0;
//                goto end;
//            }
        }
        av_packet_unref(packet);
    }


    end:
    av_packet_unref(packet);
    return result;
}


AVCodecContext *BaseDecoder::getCodecContext() {
    return avCodecContext;
}

// 改变播放状态
void BaseDecoder::changeMediaStatus(int status) {

}

void BaseDecoder::updateTimeStamp() {
//    LOGD("BaseDecoder::updateTimeStamp");
    std::unique_lock<std::mutex> lock(decode_mutex);
    // DTS:解码时间戳，用来告诉解码器Packet的解码顺序
    // PTS:显示时间戳，表示从packet解码出来的的原始数据frame的显示顺序
    // 时间基:看做时间刻度（如一把尺子有10格，我占据8份，如果刻度为1m，则为8m，刻度为1cm则为8cm）
    if (frame->pkt_dts != AV_NOPTS_VALUE) {
        curPlayTime = frame->pkt_dts;
    } else if (frame->pts != AV_NOPTS_VALUE) {
        curPlayTime = frame->pts;
    } else {
        curPlayTime = 0;
    }

    // 通过与时间基的计算,计算出该帧显示的正常时间
    curPlayTime = (int64_t) (
            (curPlayTime * av_q2d(avFormatContext->streams[stream_index]->time_base)) * 1000);
}

long BaseDecoder::syncAV() {
//    LOGD("DecoderBase::AVSync");
    // 当前的系统时间
    long curSysTime = getSysCurrentTime();
    // 基于系统时钟计算从开始播放流逝的时间
    //

    long elapsedTime = curSysTime - startPlayTime;

    long delay = 0;

    // 向系统时钟同步, 音频或视频播放时间戳大于系统时钟时，解码线程进行休眠，直到时间戳与系统时钟对齐
    if (curPlayTime > elapsedTime) {
        // 休眠时间
        auto sleepTime = static_cast<unsigned int>(curPlayTime - elapsedTime);//ms
        // 限制休眠时间不能过长
        sleepTime = sleepTime > DELAY_THRESHOLD ? DELAY_THRESHOLD : sleepTime;
        av_usleep(sleepTime * 1000);
    }
    delay = elapsedTime - curPlayTime;

    return delay;
}

void BaseDecoder::resume() {
    unique_lock<mutex> lock(decode_mutex);
    decoderState = STATE_DECODING;
    decode_cond.notify_all();
}

void BaseDecoder::pause() {
    unique_lock<mutex> lock(decode_mutex);
    decoderState = STATE_PAUSE;
}

void BaseDecoder::stop() {
    unique_lock<mutex> lock(decode_mutex);
    decoderState = STATE_STOP;
    decode_cond.notify_all();
}


BaseDecoder::BaseDecoder() = default;

BaseDecoder::~BaseDecoder() = default;
