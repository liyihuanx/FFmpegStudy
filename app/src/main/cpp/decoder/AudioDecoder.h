//
// Created by leeyh on 2022/9/27.
//

#ifndef FFMPEGSTUDY_AUDIODECODER_H
#define FFMPEGSTUDY_AUDIODECODER_H


#include "BaseDecoder.h"
#include "../render/BaseAudioRender.h"

// 音频编码采样率
static const int AUDIO_DST_SAMPLE_RATE = 44100;
// 音频编码通道数
static const int AUDIO_DST_CHANNEL_COUNTS = 2;
// 音频编码声道格式
static const uint64_t AUDIO_DST_CHANNEL_LAYOUT = AV_CH_LAYOUT_STEREO;
// 音频编码比特率
static const int AUDIO_DST_BIT_RATE = 64000;
// ACC音频一帧采样数
static const int ACC_NB_SAMPLES = 1024;


class AudioDecoder : public BaseDecoder {
public:
    AudioDecoder(char *url) {
        LOGD("AudioDecoder")
        onCreate(url, AVMEDIA_TYPE_AUDIO);
    }

    virtual ~AudioDecoder() {
        onDestroy();
    }

    void setVideoRender(BaseAudioRender *render) {
        audioRender = render;
    }

private:

    virtual void initDecoderEnvironment();

    virtual void onFrameAvailable(AVFrame *frame);

    virtual void releaseDecoder();

private:
    const AVSampleFormat DST_SAMPLE_FORMAT = AV_SAMPLE_FMT_S16;

    SwrContext *audioSwrCtx = nullptr;
    BaseAudioRender *audioRender = nullptr;

    int dst_nb_samples = -1;
    int dst_frame_datasize = -1;

    uint8_t *audioOutBuffer = nullptr;




};


#endif //FFMPEGSTUDY_AUDIODECODER_H
