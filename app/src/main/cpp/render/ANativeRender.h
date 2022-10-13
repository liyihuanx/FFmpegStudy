//
// Created by leeyh on 2022/9/27.
//

#ifndef FFMPEGSTUDY_ANATIVERENDER_H
#define FFMPEGSTUDY_ANATIVERENDER_H


#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <jni.h>
#include "BaseVideoRender.h"

class ANativeRender : public BaseVideoRender {
public:
    ANativeRender(JNIEnv *env, jobject surface);

    virtual ~ANativeRender();

    virtual void onCreate(int videoWidth, int videoHeight, int *dstSize);

    virtual void renderVideoFrame(NativeImage *pImage);

    virtual void onDestroy();

private:
    ANativeWindow_Buffer nativeWindowBuffer;
    ANativeWindow *nativeWindow = nullptr;
    int dst_width;
    int dst_height;
};


#endif //FFMPEGSTUDY_ANATIVERENDER_H
