//
// Created by leeyh on 2022/10/13.
//

#ifndef FFMPEGSTUDY_BASEAUDIORENDER_H
#define FFMPEGSTUDY_BASEAUDIORENDER_H

#include "../../log4c.h"
#include "mutex"
#include "thread"
#include "condition_variable"
#include "AudioFrame.h"
#include "queue"

#define MAX_QUEUE_BUFFER_SIZE 3

using namespace std;

class BaseAudioRender {
public:
    BaseAudioRender();

    virtual ~BaseAudioRender();

    virtual void onCreate() = 0;

    virtual void onDestroy() = 0;

    virtual void renderAudioFrame(uint8_t *pData, int dataSize) = 0;
};


#endif //FFMPEGSTUDY_BASEAUDIORENDER_H
