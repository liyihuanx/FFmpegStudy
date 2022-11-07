//
// Created by leeyh on 2022/11/7.
//

#ifndef FFMPEGSTUDY_GLCAMERARENDER_H
#define FFMPEGSTUDY_GLCAMERARENDER_H

#include "../render/video/BaseVideoRender.h"
#include "mutex"
#include "condition_variable"
#include <GLES3/gl3.h>
#include "../util/GLUtils.h"
#include <glm/vec2.hpp>

using namespace glm;
using namespace std;

class GLCameraRender : public BaseVideoRender {

public:
    static GLCameraRender *getInstance();
    static void releaseInstance();

    GLCameraRender();
    ~GLCameraRender();

    virtual void onCreate(int videoWidth, int videoHeight, int *dstSize);

    virtual void renderVideoFrame(NativeImage *pImage);

    virtual void onDestroy();

    virtual void onSurfaceCreated();
    virtual void onSurfaceChanged(int w, int h);
    virtual void onDrawFrame();

public:
    static GLCameraRender* instance;
    static std::mutex camera_gl_mutex;
    NativeImage renderImage;
    int frameIndex;
};


#endif //FFMPEGSTUDY_GLCAMERARENDER_H
