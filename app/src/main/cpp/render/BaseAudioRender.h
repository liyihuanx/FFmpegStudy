//
// Created by leeyh on 2022/10/13.
//

#ifndef FFMPEGSTUDY_BASEAUDIORENDER_H
#define FFMPEGSTUDY_BASEAUDIORENDER_H

#include "../log4c.h"

class BaseAudioRender {
public:
    BaseAudioRender();

    virtual ~BaseAudioRender();

    virtual void onCreate() = 0;

    virtual void onDestroy() = 0;

    virtual void renderAudioFrame(uint8_t *pData, int dataSize) = 0;
};


#endif //FFMPEGSTUDY_BASEAUDIORENDER_H
