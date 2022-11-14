//
// Created by leeyh on 2022/11/7.
//


#include "MediaRecorderContext.h"

jfieldID MediaRecorderContext::contextHandle = 0L;

MediaRecorderContext::MediaRecorderContext() {
    GLCameraRender::getInstance();
}

MediaRecorderContext::~MediaRecorderContext() {
    GLCameraRender::releaseInstance();
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

// 回调函数，从FBO拿到渲染数据后，回调到此处
void MediaRecorderContext::OnGLRenderFrame(void *ctx, NativeImage *pImage) {
//    LOGD("MediaRecorderContext::OnGLRenderFrame ctx=%p, pImage=%p", ctx, pImage);
    MediaRecorderContext *context = static_cast<MediaRecorderContext *>(ctx);
    std::unique_lock<std::mutex> lock(context->record_mutex);
    if (context->videoRecorder != nullptr)
        context->videoRecorder->DispatchRecordFrame(pImage);

}


int MediaRecorderContext::Init() {
    GLCameraRender::getInstance()->onCreate(0, 0, nullptr);
    GLCameraRender::getInstance()->setRenderCallback(this, OnGLRenderFrame);
    return 0;
}

int MediaRecorderContext::UnInit() {
    GLCameraRender::getInstance()->onDestroy();
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

    GLCameraRender::getInstance()->renderVideoFrame(&nativeImage);
}

void MediaRecorderContext::OnSurfaceCreated() {
    GLCameraRender::getInstance()->onSurfaceCreated();
}

void MediaRecorderContext::OnSurfaceChanged(int width, int height) {
    GLCameraRender::getInstance()->onSurfaceChanged(width, height);
}

void MediaRecorderContext::OnDrawFrame() {
    GLCameraRender::getInstance()->onDrawFrame();
}

void MediaRecorderContext::SetTransformMatrix(float translateX, float translateY, float scaleX,
                                              float scaleY, int degree, int mirror) {
    m_transformMatrix.translateX = translateX;
    m_transformMatrix.translateY = translateY;
    m_transformMatrix.scaleX = scaleX;
    m_transformMatrix.scaleY = scaleY;
    m_transformMatrix.degree = degree;
    m_transformMatrix.mirror = mirror;
    GLCameraRender::getInstance()->UpdateMVPMatrix(&m_transformMatrix);
}

void MediaRecorderContext::startRecord(int recorderType, const char *outUrl, int frameWidth,
                                       int frameHeight, long videoBitRate, int fps) {
    LOGD("MediaRecorderContext::StartRecord recorderType=%d, outUrl=%s, [w,h]=[%d,%d], videoBitRate=%ld, fps=%d",
         recorderType, outUrl, frameWidth, frameHeight, videoBitRate, fps);
    std::unique_lock<std::mutex> lock(record_mutex);
    switch (recorderType) {
        case RECORDER_TYPE_SINGLE_VIDEO:
            if (videoRecorder == nullptr) {
                videoRecorder = new SingleVideoRecorder(outUrl, frameHeight, frameWidth,
                                                        videoBitRate, fps);
                videoRecorder->StartRecord();
            }
            break;
        case RECORDER_TYPE_SINGLE_AUDIO:
            if (audioRecorder == nullptr) {
                audioRecorder = new SingleAudioRecorder(outUrl, DEFAULT_SAMPLE_RATE,
                                                        AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16);
                audioRecorder->StartRecord();
            }
            break;

        default:
            break;
    }

}

void MediaRecorderContext::stopRecord() {
    std::unique_lock<std::mutex> lock(record_mutex);
    if (videoRecorder != nullptr) {
        videoRecorder->StopRecord();
        delete videoRecorder;
        videoRecorder = nullptr;
    }

    if (audioRecorder != nullptr) {
        audioRecorder->StopRecord();
        delete audioRecorder;
        audioRecorder = nullptr;
    }
}

void MediaRecorderContext::OnAudioData(uint8_t *pData, int size) {
    LOGD("MediaRecorderContext::OnAudioData pData=%p, dataSize=%d", pData, size);
    AudioFrame audioFrame(pData, size, false);
    if (audioRecorder != nullptr)
        audioRecorder->DispatchRecordFrame(&audioFrame);
}


