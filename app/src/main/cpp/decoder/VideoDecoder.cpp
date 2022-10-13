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
    }


    // 初始化要转换的frame帧
    frame_rgb = av_frame_alloc();

    // 根据宽高，格式，获取要填充的buffer-size
    int bufferSize = av_image_get_buffer_size(AV_PIX_FMT_RGBA, video_width, video_height, 1);

    // 申请framebuffer空间
    frame_rgb_buffer = static_cast<uint8_t *>(av_malloc(bufferSize * sizeof(uint8_t)));

    av_image_fill_arrays(frame_rgb->data, frame_rgb->linesize,
                         frame_rgb_buffer, AV_PIX_FMT_RGBA,
                         video_width, video_height, 1);

    videoSwsCtx = sws_getContext(
            // src 源
            getCodecContext()->width, getCodecContext()->height,
            getCodecContext()->pix_fmt,
            // dst 目标
            getCodecContext()->width, getCodecContext()->height,
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
            nativeImage.width = video_width;
            nativeImage.height = video_height;
            //
            nativeImage.ppPlane[0] = frame_rgb->data[0];
            // lineSize = 宽度 * 4字节
            nativeImage.pLineSize[0] = nativeImage.width * 4;

            videoRender->renderVideoFrame(&nativeImage);

        }
    }

    // 1.转换格式
    // 2.给render渲染

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
