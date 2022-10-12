//
// Created by leeyh on 2022/9/27.
//

#ifndef FFMPEGSTUDY_BASERENDER_H
#define FFMPEGSTUDY_BASERENDER_H

#define VIDEO_RENDER_OPENGL             0
#define VIDEO_RENDER_ANWINDOW           1
#define VIDEO_RENDER_3D_VR              2

#include "../util/ImageDef.h"

class BaseRender {
public:
    BaseRender(int type);

    virtual ~BaseRender();

    virtual void onCreate(int videoWidth, int videoHeight, int *dstSize) = 0;

    virtual void renderVideoFrame(NativeImage *pImage) = 0;

    virtual void onDestroy() = 0;

    int getRenderType();

private:
    int render_type = VIDEO_RENDER_ANWINDOW;
};


#endif //FFMPEGSTUDY_BASERENDER_H
