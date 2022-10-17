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

    void startSLESRender();
    void handleFrame();

    int getAudioFrameQueueSize();

    static void audioPlayerCallback(SLAndroidSimpleBufferQueueItf bufferQueue, void *context);

    static void createSLWaitingThread(AudioRender *openSlRender);


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

    queue<AudioFrame *> audioFrameQueue;
    thread *audio_thread = nullptr;
    mutex   audio_mutex;
    condition_variable audio_cond;
    volatile bool isExit = false;
};


#endif //FFMPEGSTUDY_AUDIORENDER_H
