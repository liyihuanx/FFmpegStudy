//
// Created by leeyh on 2022/9/27.
//

#include <SLES/OpenSLES_Android.h>
#include "AudioSLRender.h"

void AudioSLRender::onCreate() {
    LOGD("AudioSLRender::onCreate()")

    int result = 0;
    result = createEngine();
    if (result != SL_RESULT_SUCCESS) {
        goto end;
    }

    result = createOutputMix();
    if (result != SL_RESULT_SUCCESS) {
        goto end;
    }
    result = createAudioPlayer();
    if (result != SL_RESULT_SUCCESS) {
        goto end;
    }

    audio_thread = new thread(createSLWaitingThread, this);

end:
    if (result != 0) {
        LOGD("open_sles create fail, result = %d", result)
    }
}

void AudioSLRender::onDestroy() {

}

void AudioSLRender::renderAudioFrame(uint8_t *pData, int dataSize) {
//    LOGD("AudioSLRender::renderAudioFrame")
    if (bqPlayerPlayInterface) {
        if (pData != nullptr && dataSize > 0) {

            //temp resolution, when queue size is too big.
            while (getAudioFrameQueueSize() >= MAX_QUEUE_BUFFER_SIZE && !isExit) {
                std::this_thread::sleep_for(std::chrono::milliseconds(15));
            }

            unique_lock<mutex> lock(audio_mutex);
            auto *audioFrame = new AudioFrame(pData, dataSize);
            audioFrameQueue.push(audioFrame);
            audio_cond.notify_all();
            lock.unlock();
        }
    }

}

int AudioSLRender::createEngine() {
    LOGD("AudioSLRender::createEngine()")
    int result = SL_RESULT_SUCCESS;
    do {
        result = slCreateEngine(&engineObject, 0, nullptr, 0, nullptr, nullptr);
        if (result != SL_RESULT_SUCCESS) {
            LOGD("OpenSLRender::CreateEngine slCreateEngine fail. result=%d", result);
            break;
        }

        result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
        if (result != SL_RESULT_SUCCESS) {
            LOGD("OpenSLRender::CreateEngine Realize fail. result=%d", result);
            break;
        }

        result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineInterface);
        if (result != SL_RESULT_SUCCESS) {
            LOGD("OpenSLRender::CreateEngine GetInterface fail. result=%d", result);
            break;
        }

    } while (false);

    return result;
}

int AudioSLRender::createOutputMix() {
    LOGD("AudioSLRender::createOutputMix()")

    int result = SL_RESULT_SUCCESS;
    do {
        const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
        const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};

        result = (*engineInterface)->CreateOutputMix(engineInterface, &outputMixObject, 1, mids,
                                                     mreq);
        if (result != SL_RESULT_SUCCESS) {
            LOGD("OpenSLRender::CreateOutputMixer CreateOutputMix fail. result=%d", result);
            break;
        }

        result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
        if (result != SL_RESULT_SUCCESS) {
            LOGD("OpenSLRender::CreateOutputMixer CreateOutputMix fail. result=%d", result);
            break;
        }

    } while (false);


    return result;
}

void AudioSLRender::audioPlayerCallback(SLAndroidSimpleBufferQueueItf bufferQueue, void *context) {
//    LOGD("AudioSLRender::audioPlayerCallback()")
    auto *openSlRender = static_cast<AudioSLRender *>(context);
    openSlRender->handleFrame();
}

int AudioSLRender::createAudioPlayer() {
    LOGD("AudioSLRender::createAudioPlayer()")
    int result = SL_RESULT_SUCCESS;
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                            2};

    // ???????????????bean???????????????????????????
    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,              // PCM ????????????
            (SLuint32) 2,      // ?????????????????????
            SL_SAMPLINGRATE_44_1,          // ??????????????????44100?????????
            SL_PCMSAMPLEFORMAT_FIXED_16,   // ?????????????????? ???????????? 16bit
            SL_PCMSAMPLEFORMAT_FIXED_16,   // ?????????????????? 16bit
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, // ????????????  ????????????
            SL_BYTEORDER_LITTLEENDIAN      // ?????????(??????)
    };
    SLDataSource slDataSource = {&android_queue, &pcm};

    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink slDataSink = {&outputMix, nullptr};

    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND, SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};


    do {

        result = (*engineInterface)->CreateAudioPlayer(engineInterface,
                                                       &bqPlayerObject, // ??????2????????????
                                                       &slDataSource,   // ??????3?????????????????????
                                                       &slDataSink,     // ??????4????????????
                                                       3,               // ??????5???????????????????????????
                                                       ids,             // ??????6????????????????????? Buff
                                                       req              // ??????7????????????????????????Buff ??????????????????
        );

        if (result != SL_RESULT_SUCCESS) {
            LOGD("OpenSLRender::CreateAudioPlayer CreateAudioPlayer fail. result=%d", result);
            break;
        }

        result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
        if (result != SL_RESULT_SUCCESS) {
            LOGD("OpenSLRender::CreateAudioPlayer Realize fail. result=%d", result);
            break;
        }

        result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY,
                                                 &bqPlayerPlayInterface);
        if (result != SL_RESULT_SUCCESS) {
            LOGD("OpenSLRender::CreateAudioPlayer GetInterface fail. result=%d", result);
            break;
        }

        result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
                                                 &bqPlayerBufferQueue);
        if (result != SL_RESULT_SUCCESS) {
            LOGD("OpenSLRender::CreateAudioPlayer GetInterface fail. result=%d", result);
            break;
        }

        result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue,
                                                          audioPlayerCallback,  // ????????????
                                                          this                  // ????????????????????????
        );
        if (result != SL_RESULT_SUCCESS) {
            LOGD("OpenSLRender::CreateAudioPlayer RegisterCallback fail. result=%d", result);
            break;
        }

        result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_VOLUME,
                                                 &m_AudioPlayerVolume);
        if (result != SL_RESULT_SUCCESS) {
            LOGD("OpenSLRender::CreateAudioPlayer GetInterface fail. result=%d", result);
            break;
        }

    } while (false);


    return result;
}

void AudioSLRender::startSLESRender() {
    LOGD("AudioSLRender::startSLESRender")

    while (getAudioFrameQueueSize() < MAX_QUEUE_BUFFER_SIZE && !isExit) {
        std::unique_lock<std::mutex> lock(audio_mutex);
        audio_cond.wait_for(lock, std::chrono::milliseconds(10));
        lock.unlock();
    }

    (*bqPlayerPlayInterface)->SetPlayState(bqPlayerPlayInterface, SL_PLAYSTATE_PLAYING);
    audioPlayerCallback(bqPlayerBufferQueue, this);
}

void AudioSLRender::handleFrame() {
//    LOGD("AudioSLRender::handleFrame")

    while (getAudioFrameQueueSize() < MAX_QUEUE_BUFFER_SIZE && !isExit) {
        std::unique_lock<std::mutex> lock(audio_mutex);
        audio_cond.wait_for(lock, std::chrono::milliseconds(10));
    }

    std::unique_lock<std::mutex> lock(audio_mutex);
    AudioFrame *audioFrame = audioFrameQueue.front();
    if (nullptr != audioFrame && bqPlayerPlayInterface) {
        SLresult result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, audioFrame->data,
                                                          (SLuint32) audioFrame->dataSize);
        if (result == SL_RESULT_SUCCESS) {
            audioFrameQueue.pop();
            delete audioFrame;
        }

    }
    lock.unlock();
}

int AudioSLRender::getAudioFrameQueueSize() {
    std::unique_lock<mutex> lock(audio_mutex);
    return audioFrameQueue.size();
}

void AudioSLRender::createSLWaitingThread(AudioSLRender *openSlRender) {
    openSlRender->startSLESRender();
}


