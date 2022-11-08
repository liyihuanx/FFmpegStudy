//
// Created by leeyh on 2022/11/8.
//

#include "SingleVideoRecorder.h"

SingleVideoRecorder::SingleVideoRecorder(const char *outUrl, int frameWidth, int frameHeight,
                                         long bitRate, int fps) {
    out_url = new char[strlen(outUrl) + 1];
    strcpy(out_url, outUrl);

    frame_width = frameWidth;
    frame_height = frameHeight;
    bit_rate = bitRate;
    this->fps = fps;
}

SingleVideoRecorder::~SingleVideoRecorder() {
    if (out_url) {
        delete out_url;
        out_url = nullptr;
    }
}


int SingleVideoRecorder::StartRecord() {

    int result = 0;
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
        if (avStream) {
            result = -1;
            LOGD("SingleVideoRecorder::StartRecord avformat_new_stream ret=%d", result);
            break;
        }

        // 4.找到编码器
        avCodec = avcodec_find_encoder(AV_CODEC_ID_MPEG4);
        if (avCodec) {
            result = -1;
            LOGD("SingleVideoRecorder::StartRecord avcodec_find_encoder ret=%d", result);
            break;
        }

        // 5.编码器上下文，设置编码信息
        avCodecContext = avcodec_alloc_context3(avCodec);
        if (avCodecContext) {
            result = -1;
            LOGD("SingleVideoRecorder::StartRecord avcodec_alloc_context3 ret=%d", result);
            break;
        }

        avCodecContext->codec_id = avCodec->id;
        avCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
        avCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
        avCodecContext->width = frame_width;
        avCodecContext->height = frame_height;
        avCodecContext->time_base.num = 1;
        avCodecContext->time_base.den = fps;
        avCodecContext->bit_rate = bit_rate;
        avCodecContext->gop_size = 15;

        // 6.
        result = avcodec_parameters_from_context(avStream->codecpar, avCodecContext);
        if (result < 0) {
            LOGD("SingleVideoRecorder::StartRecord avcodec_parameters_from_context ret=%d", result);
            break;
        }

        // 设置流的实时帧率，供ffmpeg参考
        av_stream_set_r_frame_rate(avStream, {1, fps});


        // 7.打开编码器
        result = avcodec_open2(avCodecContext, avCodec, nullptr);
        if (result < 0) {
            LOGD("SingleVideoRecorder::StartRecord avcodec_open2 ret=%d", result);
            break;
        }

        // 8.创建Frame
        frame = av_frame_alloc();
        frame->width = avCodecContext->width;
        frame->height = avCodecContext->height;
        frame->format = avCodecContext->pix_fmt;

        // 获取填充对应像素格式的buf_size
        int buff_size = av_image_get_buffer_size(avCodecContext->pix_fmt,
                                                 avCodecContext->width,
                                                 avCodecContext->height, 1);
        // 创建该buffer
        frame_buf = static_cast<uint8_t *>(av_malloc(buff_size));

        av_image_fill_arrays(frame->data,
                             frame->linesize,
                             frame_buf,
                             avCodecContext->pix_fmt,
                             avCodecContext->width,
                             avCodecContext->height, 16);

        // 10.写入头文件
        avformat_write_header(avFormatContext, nullptr);
        av_new_packet(packet, buff_size * 3);


    } while (false);

    if (result >= 0) {
        encodeThread = new thread(StartH264EncoderThread, this);
    }


    return 0;
}

int SingleVideoRecorder::StopRecord() {
    return 0;
}

void SingleVideoRecorder::StartH264EncoderThread(SingleVideoRecorder *context) {

}

int SingleVideoRecorder::EncodeFrame(AVFrame *pFrame) {
    return 0;
}
