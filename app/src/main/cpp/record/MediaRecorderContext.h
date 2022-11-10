//
// Created by leeyh on 2022/11/7.
//

#ifndef FFMPEGSTUDY_MEDIARECORDERCONTEXT_H
#define FFMPEGSTUDY_MEDIARECORDERCONTEXT_H

#define RECORDER_TYPE_SINGLE_VIDEO  0 //仅录制视频
#define RECORDER_TYPE_SINGLE_AUDIO  1 //仅录制音频
#define RECORDER_TYPE_AV            2 //同时录制音频和视频,打包成 MP4 文件

#include <jni.h>
#include "GLCameraRender.h"
#include "../render/video/VideoOpenGLRender.h"
#include "../render/video/ANativeRender.h"
#include "SingleVideoRecorder.h"

class MediaRecorderContext {
public:
    MediaRecorderContext();

    ~MediaRecorderContext();

    static void CreateContext(JNIEnv *env, jobject instance);

    static void StoreContext(JNIEnv *env, jobject instance, MediaRecorderContext *pContext);

    static void DeleteContext(JNIEnv *env, jobject instance);

    static MediaRecorderContext* GetContext(JNIEnv *env, jobject instance);

    void SetTransformMatrix(float translateX, float translateY, float scaleX, float scaleY, int degree, int mirror);

    static void OnGLRenderFrame(void *ctx, NativeImage * pImage);

    int Init();

    int UnInit();

    void OnPreviewFrame(int format, uint8_t *pBuffer, int width, int height);

    //OpenGL callback
    void OnSurfaceCreated();

    void OnSurfaceChanged(int width, int height);

    void OnDrawFrame();

    void startRecord(int recorderType, const char* outUrl, int frameWidth, int frameHeight, long videoBitRate, int fps);

    void stopRecord();

private:
    static jfieldID contextHandle;
    TransformMatrix m_transformMatrix;
    SingleVideoRecorder *videoRecorder = nullptr;
    mutex record_mutex;
};


#endif //FFMPEGSTUDY_MEDIARECORDERCONTEXT_H
