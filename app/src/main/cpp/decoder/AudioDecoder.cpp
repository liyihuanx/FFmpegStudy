//
// Created by leeyh on 2022/9/27.
//

#include "AudioDecoder.h"


void AudioDecoder::initDecoderEnvironment() {
    LOGD("AudioDecoder::initDecoderEnvironment()")
    if (audioRender != nullptr) {
        audioSwrCtx = swr_alloc();

        AVCodecContext *tempCodecCtx = getCodecContext();
//        swr_ctx = swr_alloc_set_opts(
//                nullptr,
//                AV_CH_LAYOUT_STEREO,  // 声道布局类型 双声道
//                AV_SAMPLE_FMT_S16,  // 采样大小 16bit
//                out_sample_rate, // 采样率  44100
//                audio_codec_ctx->channel_layout, // 声道布局类型
//                audio_codec_ctx->sample_fmt, // 采样大小
//                audio_codec_ctx->sample_rate,  // 采样率
//                0,
//                0
//        );

        av_opt_set_int(audioSwrCtx, "in_channel_layout", tempCodecCtx->channel_layout, 0);
        av_opt_set_int(audioSwrCtx, "out_channel_layout", AUDIO_DST_CHANNEL_LAYOUT, 0);

        av_opt_set_int(audioSwrCtx, "in_sample_rate", tempCodecCtx->sample_rate, 0);
        av_opt_set_int(audioSwrCtx, "out_sample_rate", AUDIO_DST_SAMPLE_RATE, 0);

        av_opt_set_sample_fmt(audioSwrCtx, "in_sample_fmt", tempCodecCtx->sample_fmt, 0);
        av_opt_set_sample_fmt(audioSwrCtx, "out_sample_fmt", DST_SAMPLE_FORMAT, 0);

        LOGD("AudioDecoder::initDecoderEnvironment audio metadata sample rate: %d, channel: %d, format: %d, frame_size: %d, layout: %lu",
             tempCodecCtx->sample_rate, tempCodecCtx->channels,
             tempCodecCtx->sample_fmt, tempCodecCtx->frame_size,
             tempCodecCtx->channel_layout);

        swr_init(audioSwrCtx);

        // 重采样
        dst_nb_samples = (int) av_rescale_rnd(
                ACC_NB_SAMPLES,
                AUDIO_DST_SAMPLE_RATE,       // 输出采样率 44100
                tempCodecCtx->sample_rate,  // 输入采样率
                AV_ROUND_UP            // 向上取
        );

        // 获取给定音频参数所需的缓冲区大小
        dst_frame_datasize = av_samples_get_buffer_size(NULL, AUDIO_DST_CHANNEL_COUNTS,
                                                        dst_nb_samples, DST_SAMPLE_FORMAT, 1);

        audioOutBuffer = (uint8_t *) malloc(dst_frame_datasize);

        audioRender->onCreate();
    }
}

void AudioDecoder::onFrameAvailable(AVFrame *frame) {
//    LOGD("AudioDecoder::onFrameAvailable=%p", frame)
    if (frame != nullptr && audioRender != nullptr) {
        int result = swr_convert(audioSwrCtx,
                                 &audioOutBuffer,                   // 重采样后的buff
                                 dst_frame_datasize / 2,  //
                                 (const uint8_t **) frame->data,
                                 frame->nb_samples);
        if (result > 0) {

            audioRender->renderAudioFrame(audioOutBuffer, dst_frame_datasize);
        }
    }


}

void AudioDecoder::releaseDecoder() {
    if(audioRender)
        audioRender->onDestroy();

    if(audioOutBuffer) {
        free(audioOutBuffer);
        audioOutBuffer = nullptr;
    }

    if(audioSwrCtx) {
        swr_free(&audioSwrCtx);
        audioSwrCtx = nullptr;
    }
}
