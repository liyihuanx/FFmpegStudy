//
// Created by leeyh on 2022/9/27.
//

#include "BaseDecoder.h"

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
}

void BaseDecoder::start() {
    LOGD("BaseDecoder::start()")
    new thread(startDecodeTread, this);
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

    if (decodeOnePacket() != 0) {
        // 解码结束

    }

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

                // 同步

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


// 释放解码相关变量
void BaseDecoder::releaseDecoder() {
    LOGD("releaseDecoder");

}

AVCodecContext *BaseDecoder::getCodecContext() {
    return avCodecContext;
}

// 改变播放状态
void BaseDecoder::changeMediaStatus(int status) {

}


BaseDecoder::BaseDecoder() = default;

BaseDecoder::~BaseDecoder() = default;
