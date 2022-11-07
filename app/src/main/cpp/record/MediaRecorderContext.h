//
// Created by leeyh on 2022/11/7.
//

#ifndef FFMPEGSTUDY_MEDIARECORDERCONTEXT_H
#define FFMPEGSTUDY_MEDIARECORDERCONTEXT_H

#include <jni.h>
#include "GLCameraRender.h"
#include "../render/video/VideoOpenGLRender.h"

class MediaRecorderContext {
public:
    MediaRecorderContext();

    ~MediaRecorderContext();

    static void CreateContext(JNIEnv *env, jobject instance);

    static void StoreContext(JNIEnv *env, jobject instance, MediaRecorderContext *pContext);

    static void DeleteContext(JNIEnv *env, jobject instance);

    static MediaRecorderContext* GetContext(JNIEnv *env, jobject instance);


    int Init();

    int UnInit();

    void OnPreviewFrame(int format, uint8_t *pBuffer, int width, int height);

    //OpenGL callback
    void OnSurfaceCreated();

    void OnSurfaceChanged(int width, int height);

    void OnDrawFrame();

private:
    static jfieldID contextHandle;

};


#endif //FFMPEGSTUDY_MEDIARECORDERCONTEXT_H
