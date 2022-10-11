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
void VideoDecoder::OnFrameAvailable(AVFrame *frame) {
    LOGD("VideoDecoder::OnFrameAvailable()");
    // 从packet解码出来的frame(YUV原始帧)

    // 1.转换格式
    // 2.给render渲染

}
