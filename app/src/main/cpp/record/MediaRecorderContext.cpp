//
// Created by leeyh on 2022/11/7.
//


#include "MediaRecorderContext.h"

jfieldID MediaRecorderContext::contextHandle = 0L;

MediaRecorderContext::MediaRecorderContext() {
    VideoOpenGLRender::getInstance();
}

MediaRecorderContext::~MediaRecorderContext() {
    VideoOpenGLRender::releaseInstance();
}

void MediaRecorderContext::CreateContext(JNIEnv *env, jobject instance) {
    // instance == FFMediaRecord.java
    LOGD("MediaRecorderContext::CreateContext");
    auto *pContext = new MediaRecorderContext();
    StoreContext(env, instance, pContext);
}

void
MediaRecorderContext::StoreContext(JNIEnv *env, jobject instance, MediaRecorderContext *pContext) {
    // 拿到Class对象
    jclass cls = env->GetObjectClass(instance);
    if (cls == nullptr) {
        LOGD("MediaRecorderContext::StoreContext cls == null")
        return;
    }
    // 拿到变量ID
    contextHandle = env->GetFieldID(cls, "mNativeContextHandle", "J");

    // 给java层变量赋值pContext = new MediaRecorderContext()
    env->SetLongField(instance, contextHandle, reinterpret_cast<jlong>(pContext));


}

void MediaRecorderContext::DeleteContext(JNIEnv *env, jobject instance) {

    auto *pContext = reinterpret_cast<MediaRecorderContext *>(env->GetLongField(instance,
                                                                                contextHandle));

    if (pContext != nullptr) {
        delete pContext;
    }

    // 赋空值
    env->SetLongField(instance, contextHandle, 0L);

}

MediaRecorderContext *MediaRecorderContext::GetContext(JNIEnv *env, jobject instance) {
    if (contextHandle != nullptr) {
        auto *pContext = reinterpret_cast<MediaRecorderContext *>(env->GetLongField(instance,
                                                                                    contextHandle));
        return pContext;
    }

    return nullptr;
}

int MediaRecorderContext::Init() {
    VideoOpenGLRender::getInstance()->onCreate(0, 0, nullptr);

    return 0;
}

int MediaRecorderContext::UnInit() {
    VideoOpenGLRender::getInstance()->onDestroy();
    return 0;
}

// 相当于ffmpeg拿到原始数据frame帧后给渲染器渲染
void MediaRecorderContext::OnPreviewFrame(int format, uint8_t *pBuffer, int width, int height) {
    NativeImage nativeImage;
    nativeImage.format = format;
    nativeImage.width = width;
    nativeImage.height = height;
    // 数据的开头
    nativeImage.ppPlane[0] = pBuffer;

    switch (format) {
        case IMAGE_FORMAT_NV12:
        case IMAGE_FORMAT_NV21:
            // ppPlane[1]存放UV，偏移width * height(Y的容量)
            nativeImage.ppPlane[1] = nativeImage.ppPlane[0] + width * height;
            nativeImage.pLineSize[0] = width;
            nativeImage.pLineSize[1] = width;
            break;
        case IMAGE_FORMAT_I420:
            nativeImage.ppPlane[1] = nativeImage.ppPlane[0] + width * height;
            nativeImage.ppPlane[2] = nativeImage.ppPlane[1] + width * height / 4;
            nativeImage.pLineSize[0] = width;
            nativeImage.pLineSize[1] = width / 2;
            nativeImage.pLineSize[2] = width / 2;
            break;
        default:
            break;
    }

    VideoOpenGLRender::getInstance()->renderVideoFrame(&nativeImage);
}

void MediaRecorderContext::OnSurfaceCreated() {
    VideoOpenGLRender::getInstance()->onSurfaceCreated();
}

void MediaRecorderContext::OnSurfaceChanged(int width, int height) {
    VideoOpenGLRender::getInstance()->onSurfaceChanged(width, height);
}

void MediaRecorderContext::OnDrawFrame() {
    VideoOpenGLRender::getInstance()->onDrawFrame();
}
