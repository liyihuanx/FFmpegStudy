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
        if (avStream == nullptr) {
            result = -1;
            LOGD("SingleVideoRecorder::StartRecord avformat_new_stream ret=%d", result);
            break;
        }

        // 4.找到编码器
        avCodec = avcodec_find_encoder(AV_CODEC_ID_MPEG4);
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
                             avCodecContext->height, 1);

        AVDictionary *opt = 0;
        if (avCodecContext->codec_id == AV_CODEC_ID_H264) {
            av_dict_set_int(&opt, "video_track_timescale", 25, 0);
            av_dict_set(&opt, "preset", "slow", 0);
            av_dict_set(&opt, "tune", "zerolatency", 0);
        }

        // 10.写入头文件
        avformat_write_header(avFormatContext, nullptr);
        av_new_packet(&packet, buff_size * 3);


    } while (false);

    if (result >= 0) {
        encodeThread = new thread(StartH264EncoderThread, this);
    }


    return 0;
}

// 停止录制
int SingleVideoRecorder::StopRecord() {
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
        NativeImage *pImage = frameQueue.Pop();
        NativeImageUtil::FreeNativeImage(pImage);
        delete pImage;
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
        avformat_free_context(avFormatContext);
        avFormatContext = nullptr;
    }

    if (swsContext != nullptr) {
        sws_freeContext(swsContext);
        swsContext = nullptr;
    }

    return 0;
}

// 启动线程，准备编码工作
void SingleVideoRecorder::StartH264EncoderThread(SingleVideoRecorder *recorder) {
    // 停止编码且队列为空时退出循环
    while (!recorder->isExit || !recorder->frameQueue.Empty()) {
        if (recorder->frameQueue.Empty()) {
            //队列为空，休眠等待
            usleep(10 * 1000);
            continue;
        }

        //从队列中取一帧预览帧
        NativeImage *pImage = recorder->frameQueue.Pop();
        AVFrame *pFrame = recorder->frame;
        AVPixelFormat srcPixFmt = AV_PIX_FMT_YUV420P;

        switch (pImage->format) {
            case IMAGE_FORMAT_RGBA:
                srcPixFmt = AV_PIX_FMT_RGBA;
                break;
            case IMAGE_FORMAT_NV21:
                srcPixFmt = AV_PIX_FMT_NV21;
                break;
            case IMAGE_FORMAT_NV12:
                srcPixFmt = AV_PIX_FMT_NV12;
                break;
            case IMAGE_FORMAT_I420:
                srcPixFmt = AV_PIX_FMT_YUV420P;
                break;
            default:
                LOGD("SingleVideoRecorder::StartH264EncoderThread unsupport format pImage->format=%d",
                     pImage->format);
                break;
        }

        if (recorder->swsContext == nullptr) {
            recorder->swsContext = sws_getContext(
                    pImage->width, pImage->height, srcPixFmt,
                    recorder->frame_width, recorder->frame_height, AV_PIX_FMT_YUV420P,
                    SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
        }

        if (recorder->swsContext != nullptr) {
            sws_scale(recorder->swsContext, pImage->ppPlane, pImage->pLineSize,
                      0, recorder->frame_height,
                      pFrame->data, pFrame->linesize);
        }

        //设置 pts
        pFrame->pts = recorder->frame_index++;
        recorder->EncodeFrame(pFrame);
        NativeImageUtil::FreeNativeImage(pImage);
        delete pImage;

    }

}


// 编码
int SingleVideoRecorder::EncodeFrame(AVFrame *pFrame) {
    int result = 0;
    result = avcodec_send_frame(avCodecContext, pFrame);
    if (result < 0) {
        LOGD("SingleVideoRecorder::EncodeFrame avcodec_send_frame fail. ret=%d", result);
        return result;
    }
    while (!result) {
        result = avcodec_receive_packet(avCodecContext, &packet);
        if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
            return 0;
        } else if (result < 0) {
            LOGD("SingleVideoRecorder::EncodeFrame avcodec_receive_packet fail. ret=%d", result);
            return result;
        }
        LOGD("SingleVideoRecorder::EncodeFrame frame pts=%ld, size=%d", packet.pts, packet.size);
        packet.stream_index = avStream->index;
        av_packet_rescale_ts(&packet, avCodecContext->time_base, avStream->time_base);
        packet.pos = -1;
        av_interleaved_write_frame(avFormatContext, &packet);
        av_packet_unref(&packet);
    }
    return 0;
}

// 录制时传入的frame
int SingleVideoRecorder::DispatchRecordFrame(NativeImage *inputFrame) {
    if (isExit) return 0;
//    LOGD("SingleVideoRecorder::recordFrame [w,h,format]=[%d,%d,%d]", inputFrame->width,
//         inputFrame->height, inputFrame->format);
    NativeImage *pImage = new NativeImage();
    pImage->width = inputFrame->width;
    pImage->height = inputFrame->height;
    pImage->format = inputFrame->format;
    NativeImageUtil::AllocNativeImage(pImage);
    NativeImageUtil::CopyNativeImage(inputFrame, pImage);
    //NativeImageUtil::DumpNativeImage(pImage, "/sdcard", "camera");
    frameQueue.Push(pImage);
    return 0;
}
