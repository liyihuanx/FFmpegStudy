//
// Created by leeyh on 2022/9/27.
//

#include <SLES/OpenSLES_Android.h>
#include "AudioRender.h"

void AudioRender::onCreate() {
    LOGD("AudioRender::onCreate()")

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
    LOGD("open_sles create fail, result = %d", result)
}

void AudioRender::onDestroy() {

}

void AudioRender::renderAudioFrame(uint8_t *pData, int dataSize) {
//    LOGD("AudioRender::renderAudioFrame")
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

int AudioRender::createEngine() {
    LOGD("AudioRender::createEngine()")
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

int AudioRender::createOutputMix() {
    LOGD("AudioRender::createOutputMix()")

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

void AudioRender::audioPlayerCallback(SLAndroidSimpleBufferQueueItf bufferQueue, void *context) {
//    LOGD("AudioRender::audioPlayerCallback()")
    auto *openSlRender = static_cast<AudioRender *>(context);
    openSlRender->handleFrame();
}

int AudioRender::createAudioPlayer() {
    LOGD("AudioRender::createAudioPlayer()")
    int result = SL_RESULT_SUCCESS;
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                            2};

    // 看做是一个bean类，存放的配置信息
    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,              // PCM 数据格式
            (SLuint32) 2,      // 声道数，双声道
            SL_SAMPLINGRATE_44_1,          // 采样率（每秒44100个点）
            SL_PCMSAMPLEFORMAT_FIXED_16,   // 每秒采样样本 存放大小 16bit
            SL_PCMSAMPLEFORMAT_FIXED_16,   // 每个样本位数 16bit
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, // 前左声道  前右声道
            SL_BYTEORDER_LITTLEENDIAN      // 字节序(小端)
    };
    SLDataSource slDataSource = {&android_queue, &pcm};

    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink slDataSink = {&outputMix, nullptr};

    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND, SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};


    do {

        result = (*engineInterface)->CreateAudioPlayer(engineInterface,
                                                       &bqPlayerObject, // 参数2：播放器
                                                       &slDataSource,   // 参数3：音频配置信息
                                                       &slDataSink,     // 参数4：混音器
                                                       3,               // 参数5：开放的参数的个数
                                                       ids,             // 参数6：代表我们需要 Buff
                                                       req              // 参数7：代表我们上面的Buff 需要开放出去
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
                                                          audioPlayerCallback,  // 回调函数
                                                          this                  // 给回调函数的参数
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

void AudioRender::startSLESRender() {
    LOGD("AudioRender::startSLESRender")

    while (getAudioFrameQueueSize() < MAX_QUEUE_BUFFER_SIZE && !isExit) {
        std::unique_lock<std::mutex> lock(audio_mutex);
        audio_cond.wait_for(lock, std::chrono::milliseconds(10));
        lock.unlock();
    }

    (*bqPlayerPlayInterface)->SetPlayState(bqPlayerPlayInterface, SL_PLAYSTATE_PLAYING);
    audioPlayerCallback(bqPlayerBufferQueue, this);
}

void AudioRender::handleFrame() {
//    LOGD("AudioRender::handleFrame")

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

int AudioRender::getAudioFrameQueueSize() {
    std::unique_lock<mutex> lock(audio_mutex);
    return audioFrameQueue.size();
}

void AudioRender::createSLWaitingThread(AudioRender *openSlRender) {
    openSlRender->startSLESRender();
}


