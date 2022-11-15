//
// Created by leeyh on 2022/11/14.
//

#include "SingleAudioRecorder.h"

SingleAudioRecorder::SingleAudioRecorder(const char *outUrl, int sampleRate, int channelLayout,
                                         int sampleFormat) {

    out_url = new char[strlen(outUrl) + 1];
    strcpy(out_url, outUrl);

    sample_rate = sampleRate;
    channel_layout = channelLayout;
    sample_format = sampleFormat;

}

SingleAudioRecorder::~SingleAudioRecorder() {
    if (out_url) {
        delete out_url;
        out_url = nullptr;
    }
}

int SingleAudioRecorder::StartRecord() {
    int result = -1;
    do {

        // 1.初始化AVFormatContext结构体,根据文件名获取到合适的封装格式
        result = avformat_alloc_output_context2(&avFormatContext, nullptr, nullptr, out_url);
        if (result < 0) {
            LOGD("SingleVideoRecorder::StartRecord avformat_alloc_output_context2 ret=%d", result);
            break;
        }

        // 2.打开文件IO流
        result = avio_open(&avFormatContext->pb, out_url, AVIO_FLAG_WRITE);
        if (result < 0) {
            LOGD("SingleVideoRecorder::StartRecord avio_open ret=%d", result);
            break;
        }

        // 3.初始化视频码流
        avStream = avformat_new_stream(avFormatContext, nullptr);
        if (avStream == nullptr) {
            result = -1;
            LOGD("SingleVideoRecorder::StartRecord avformat_new_stream ret=%d", result);
            break;
        }

        // 4.找到编码器
        AVOutputFormat *avOutputFormat = avFormatContext->oformat;
        avCodec = avcodec_find_encoder(avOutputFormat->audio_codec);
        if (avCodec == nullptr) {
            result = -1;
            LOGD("SingleVideoRecorder::StartRecord avcodec_find_encoder ret=%d", result);
            break;
        }

        // 5.编码器上下文，设置编码信息
        avCodecContext = avcodec_alloc_context3(avCodec);
        if (avCodecContext == nullptr) {
            result = -1;
            LOGD("SingleVideoRecorder::StartRecord avcodec_alloc_context3 ret=%d", result);
            break;
        }

//        avCodecContext = avStream->codec;
        avCodecContext->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
        LOGD("SingleAudioRecorder::StartRecord avOutputFormat->audio_codec=%d",
             avOutputFormat->audio_codec);
        avCodecContext->codec_id = AV_CODEC_ID_AAC;
        avCodecContext->codec_type = AVMEDIA_TYPE_AUDIO;
        avCodecContext->sample_fmt = AV_SAMPLE_FMT_FLTP;//float, planar, 4 字节
        avCodecContext->sample_rate = DEFAULT_SAMPLE_RATE;
        avCodecContext->channel_layout = DEFAULT_CHANNEL_LAYOUT;
        avCodecContext->channels = av_get_channel_layout_nb_channels(
                avCodecContext->channel_layout);
        avCodecContext->bit_rate = 96000;

        // 6.
        result = avcodec_parameters_from_context(avStream->codecpar, avCodecContext);
        if (result < 0) {
            LOGD("SingleVideoRecorder::StartRecord avcodec_parameters_from_context ret=%d", result);
            break;
        }

        // 7.打开编码器
        result = avcodec_open2(avCodecContext, avCodec, nullptr);
        if (result < 0) {
            LOGD("SingleVideoRecorder::StartRecord avcodec_open2 ret=%d", result);
            break;
        }

        av_dump_format(avFormatContext, 0, out_url, 1);

        // 8.创建Frame
        frame = av_frame_alloc();
        frame->nb_samples = avCodecContext->frame_size;
        frame->format = avCodecContext->sample_fmt;

        // 计算
        frame_buffer_size = av_samples_get_buffer_size(nullptr, avCodecContext->channels,
                                                       avCodecContext->frame_size,
                                                       avCodecContext->sample_fmt, 1);

        // 创建该buffer
        frame_buf = static_cast<uint8_t *>(av_malloc(frame_buffer_size));

        avcodec_fill_audio_frame(frame, avCodecContext->channels, avCodecContext->sample_fmt,
                                 frame_buf, frame_buffer_size, 1);

        // 10.写入头文件
        avformat_write_header(avFormatContext, nullptr);
        av_new_packet(&packet, frame_buffer_size);


        // 音频转码器
        swrContext = swr_alloc();
        av_opt_set_channel_layout(swrContext, "in_channel_layout", channel_layout, 0);
        av_opt_set_channel_layout(swrContext, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
        av_opt_set_int(swrContext, "in_sample_rate", sample_rate, 0);
        av_opt_set_int(swrContext, "out_sample_rate", DEFAULT_SAMPLE_RATE, 0);
        av_opt_set_sample_fmt(swrContext, "in_sample_fmt", AVSampleFormat(sample_format), 0);
        av_opt_set_sample_fmt(swrContext, "out_sample_fmt", AV_SAMPLE_FMT_FLTP, 0);
        swr_init(swrContext);


    } while (false);

    if (result >= 0) {
        encodeThread = new thread(StartAACEncoderThread, this);
    }

    return 0;
}

int SingleAudioRecorder::StopRecord() {
    LOGD("SingleVideoRecorder::StopRecord()")
    isExit = 1;
    if (encodeThread != nullptr) {
        encodeThread->join();
        delete encodeThread;
        encodeThread = nullptr;

        int result = EncodeFrame(nullptr);
        if (result >= 0) {
            av_write_trailer(avFormatContext);
        }
    }

    while (!frameQueue.Empty()) {
        AudioFrame *audioframe = frameQueue.Pop();
        delete audioframe;
    }

    if (avCodecContext != nullptr) {
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = nullptr;
    }
    if (frame != nullptr) {
        av_frame_free(&frame);
        frame = nullptr;
    }
    if (frame_buf != nullptr) {
        av_free(frame_buf);
        frame_buf = nullptr;
    }
    if (avFormatContext != nullptr) {
        avio_close(avFormatContext->pb);
//        avformat_free_context(avFormatContext);
        avFormatContext = nullptr;
    }
    if (swrContext != nullptr) {
        swr_free(&swrContext);
        swrContext = nullptr;
    }


    return 0;
}

int SingleAudioRecorder::DispatchRecordFrame(AudioFrame *inputFrame) {
    if (isExit) return 0;
    AudioFrame *pAudioFrame = new AudioFrame(inputFrame->data, inputFrame->dataSize);
    frameQueue.Push(pAudioFrame);
    return 0;
}

void SingleAudioRecorder::StartAACEncoderThread(SingleAudioRecorder *recorder) {
    LOGD("SingleAudioRecorder::StartAACEncoderThread start");
    while (!recorder->isExit || !recorder->frameQueue.Empty()) {
        if (recorder->frameQueue.Empty()) {
            //队列为空，休眠等待
            usleep(10 * 1000);
            continue;
        }

        AudioFrame *audioFrame = recorder->frameQueue.Pop();
        AVFrame *pFrame = recorder->frame;
        int result = swr_convert(recorder->swrContext, pFrame->data, pFrame->nb_samples,
                                 (const uint8_t **) &(audioFrame->data), audioFrame->dataSize / 4);
        LOGD("SingleAudioRecorder::StartAACEncoderThread result=%d", result);
        if (result >= 0) {
            pFrame->pts = recorder->frame_index++;
            recorder->EncodeFrame(pFrame);
        }
        delete audioFrame;
    }

    LOGD("SingleAudioRecorder::StartAACEncoderThread end");
}

int SingleAudioRecorder::EncodeFrame(AVFrame *pFrame) {
    LOGD("SingleAudioRecorder::EncodeFrame pFrame->nb_samples=%d",
         pFrame != nullptr ? pFrame->nb_samples : 0);
    int result = 0;
    result = avcodec_send_frame(avCodecContext, pFrame);
    if (result < 0) {
        LOGD("SingleAudioRecorder::EncodeFrame avcodec_send_frame fail. ret=%d, error:%s", result,
             av_err2str(result));
        return result;
    }
    while (!result) {
        result = avcodec_receive_packet(avCodecContext, &packet);
        if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
            return 0;
        } else if (result < 0) {
            LOGD("SingleAudioRecorder::EncodeFrame avcodec_receive_packet fail. ret=%d", result);
            return result;
        }
        LOGD("SingleAudioRecorder::EncodeFrame frame pts=%ld, size=%d", packet.pts, packet.size);
        packet.stream_index = avStream->index;
        av_interleaved_write_frame(avFormatContext, &packet);
        av_packet_unref(&packet);
    }
    return 0;
}


