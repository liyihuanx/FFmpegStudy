//
// Created by leeyh on 2022/9/27.
//

#include "VideoDecoder.h"

// 初始化，
void VideoDecoder::initDecoderEnvironment() {
    LOGD("VideoDecoder::initDecoderEnvironment()")
    // 获取视频宽高
    video_height = getCodecContext()->height;
    video_width = getCodecContext()->width;

    // 初始化渲染器
    if (videoRender != nullptr) {
        int dstSize[2] = {0};
        videoRender->onCreate(video_width, video_height, dstSize);
        render_width = dstSize[0];
        render_height = dstSize[1];
    }


    // 初始化要转换的frame帧
    frame_rgb = av_frame_alloc();

    // 根据宽高，格式，获取要填充的buffer-size
    int bufferSize = av_image_get_buffer_size(AV_PIX_FMT_RGBA, render_width, render_height,1);

    // 申请framebuffer空间
    frame_rgb_buffer = static_cast<uint8_t *>(av_malloc(bufferSize * sizeof(uint8_t)));

    av_image_fill_arrays(frame_rgb->data, frame_rgb->linesize,
                               frame_rgb_buffer, AV_PIX_FMT_RGBA,
                               render_width, render_height, 1);

    videoSwsCtx = sws_getContext(
            // src 源
            video_height, video_width,
            getCodecContext()->pix_fmt,
            // dst 目标
            render_width, render_height,
            AV_PIX_FMT_RGBA,
            SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
}

// 做格式转换
void VideoDecoder::onFrameAvailable(AVFrame *frame) {
    // 从packet解码出来的frame(YUV原始帧)
//    LOGD("VideoDecoder::onFrameAvailable frame=%p", frame);

    if (frame != nullptr && videoRender != nullptr) {
        NativeImage nativeImage;
        if (videoRender->getRenderType() == VIDEO_RENDER_ANWINDOW) {
            sws_scale(videoSwsCtx, frame->data, frame->linesize,
                      0, video_height,
                      frame_rgb->data, frame_rgb->linesize);

            nativeImage.format = IMAGE_FORMAT_RGBA;
            nativeImage.width = render_width;
            nativeImage.height = render_height;
            nativeImage.ppPlane[0] = frame_rgb->data[0];
            // lineSize = 宽度 * 4字节
            nativeImage.pLineSize[0] = nativeImage.width * 4;
        } else if (getCodecContext()->pix_fmt == AV_PIX_FMT_YUV420P ||
                   getCodecContext()->pix_fmt == AV_PIX_FMT_YUVJ420P) {
            nativeImage.format = IMAGE_FORMAT_I420;
            nativeImage.width = frame->width;
            nativeImage.height = frame->height;
            nativeImage.pLineSize[0] = frame->linesize[0];
            nativeImage.pLineSize[1] = frame->linesize[1];
            nativeImage.pLineSize[2] = frame->linesize[2];
            nativeImage.ppPlane[0] = frame->data[0];
            nativeImage.ppPlane[1] = frame->data[1];
            nativeImage.ppPlane[2] = frame->data[2];
            if (frame->data[0] && frame->data[1] && !frame->data[2] &&
                frame->linesize[0] == frame->linesize[1] && frame->linesize[2] == 0) {
                // on some android device, output of h264 mediacodec decoder is NV12 兼容某些设备可能出现的格式不匹配问题
                nativeImage.format = IMAGE_FORMAT_NV12;
            }
        } else if (getCodecContext()->pix_fmt == AV_PIX_FMT_NV12) {
            nativeImage.format = IMAGE_FORMAT_NV12;
            nativeImage.width = frame->width;
            nativeImage.height = frame->height;
            nativeImage.pLineSize[0] = frame->linesize[0];
            nativeImage.pLineSize[1] = frame->linesize[1];
            nativeImage.ppPlane[0] = frame->data[0];
            nativeImage.ppPlane[1] = frame->data[1];
        } else if (getCodecContext()->pix_fmt == AV_PIX_FMT_NV21) {
            nativeImage.format = IMAGE_FORMAT_NV21;
            nativeImage.width = frame->width;
            nativeImage.height = frame->height;
            nativeImage.pLineSize[0] = frame->linesize[0];
            nativeImage.pLineSize[1] = frame->linesize[1];
            nativeImage.ppPlane[0] = frame->data[0];
            nativeImage.ppPlane[1] = frame->data[1];
        } else if (getCodecContext()->pix_fmt == AV_PIX_FMT_RGBA) {
            nativeImage.format = IMAGE_FORMAT_RGBA;
            nativeImage.width = frame->width;
            nativeImage.height = frame->height;
            nativeImage.pLineSize[0] = frame->linesize[0];
            nativeImage.ppPlane[0] = frame->data[0];
        } else {
            sws_scale(videoSwsCtx, frame->data, frame->linesize,
                      0, video_height,
                      frame_rgb->data, frame_rgb->linesize);
            nativeImage.format = IMAGE_FORMAT_RGBA;
            nativeImage.width = render_width;
            nativeImage.height = render_height;
            nativeImage.ppPlane[0] = frame_rgb->data[0];
            nativeImage.pLineSize[0] = nativeImage.width * 4;
        }
        videoRender->renderVideoFrame(&nativeImage);

    }

    if(m_MsgContext && m_MsgCallback) {
//        LOGD("m_MsgCallback - MSG_REQUEST_RENDER")
        m_MsgCallback(m_MsgContext, MSG_REQUEST_RENDER, 0);
    }


}

// 释放解码相关变量
void VideoDecoder::releaseDecoder() {
    LOGD("releaseDecoder");
    if (videoRender)
        videoRender->onDestroy();

    if (frame_rgb != nullptr) {
        av_frame_free(&frame_rgb);
        frame_rgb = nullptr;
    }

    if (frame_rgb_buffer != nullptr) {
        free(frame_rgb_buffer);
        frame_rgb_buffer = nullptr;
    }

    if (videoSwsCtx != nullptr) {
        sws_freeContext(videoSwsCtx);
        videoSwsCtx = nullptr;
    }


}
