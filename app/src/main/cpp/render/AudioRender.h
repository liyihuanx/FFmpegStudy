//
// Created by leeyh on 2022/9/27.
//

#ifndef FFMPEGSTUDY_AUDIORENDER_H
#define FFMPEGSTUDY_AUDIORENDER_H


#include <cstdint>
#include <SLES/OpenSLES.h>
#include "BaseAudioRender.h"

class AudioRender : public BaseAudioRender {

public:
    virtual void onCreate();

    virtual void onDestroy();

    virtual void renderAudioFrame(uint8_t *pData, int dataSize);

private:
    int createEngine();
    int createOutputMix();
    int createAudioPlayer();

    void openSLESRender();
    static void audioPlayerCallback(SLAndroidSimpleBufferQueueItf bufferQueue, void *context);

private:
    // 创建引擎用的
    SLObjectItf engineObject = nullptr;
    SLEngineItf engineInterface = nullptr;
    // 创建混音器用的
    SLObjectItf outputMixObject = nullptr;
    // 创建播放器用的
    SLObjectItf bqPlayerObject = nullptr;
    SLPlayItf bqPlayerPlayInterface = nullptr;
    SLVolumeItf m_AudioPlayerVolume = nullptr;
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;


};


#endif //FFMPEGSTUDY_AUDIORENDER_H
