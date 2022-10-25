//
// Created by k1 on 2022/9/8.
//

#include "FFmpegPlayer.h"
#include "decoder/VideoDecoder.h"
#include "render/video/ANativeRender.h"
#include "decoder/AudioDecoder.h"
#include "render/audio/AudioSLRender.h"
#include "render/video/VideoOpenGLRender.h"

#include "util/GLUtils.h"


FFmpegPlayer::FFmpegPlayer() {

}

FFmpegPlayer::~FFmpegPlayer() {

}

void FFmpegPlayer::init(JNIEnv *jniEnv, jobject obj, char *url, int videoRenderType, jobject surface) {
    LOGD("FFmpegPlayer::init");
    jniEnv->GetJavaVM(&m_JavaVM);
    m_JavaObj = jniEnv->NewGlobalRef(obj);
    // 视频
    videoDecoder = new VideoDecoder(url);
    // 视频回调
    videoDecoder->SetMessageCallback(this, PostMessage);
    // 视频渲染
    if (videoRenderType == VIDEO_RENDER_OPENGL) {
        videoDecoder->setVideoRender(VideoOpenGLRender::getInstance());
    } else if (videoRenderType == VIDEO_RENDER_ANWINDOW) {
        videoRender = new ANativeRender(jniEnv, surface);
        videoDecoder->setVideoRender(videoRender);
    }

    // 音频
    audioDecoder = new AudioDecoder(url);
    audioRender = new AudioSLRender();
    // 音频渲染
    audioDecoder->setAudioRender(audioRender);
    // 视频回调
    audioDecoder->SetMessageCallback(this, PostMessage);

}

void FFmpegPlayer::play() {
    if (videoDecoder) videoDecoder->start();
    if (audioDecoder) audioDecoder->start();
}


void FFmpegPlayer::PostMessage(void *context, int msgType, float msgCode) {
    if (context != nullptr) {
        FFmpegPlayer *player = static_cast<FFmpegPlayer *>(context);
        bool isAttach = false;
        JNIEnv *env = player->GetJNIEnv(&isAttach);
//        LOGD("FFMediaPlayer::PostMessage env=%p", env);
        if (env == nullptr)
            return;
        jobject javaObj = player->GetJavaObj();
        jmethodID mid = env->GetMethodID(env->GetObjectClass(javaObj),
                                         JAVA_PLAYER_EVENT_CALLBACK_API_NAME, "(IF)V");
        env->CallVoidMethod(javaObj, mid, msgType, msgCode);
        if (isAttach)
            player->GetJavaVM()->DetachCurrentThread();

    }
}


JNIEnv *FFmpegPlayer::GetJNIEnv(bool *isAttach) {
    JNIEnv *env;
    int status;
    if (nullptr == m_JavaVM) {
        LOGD("FFMediaPlayer::GetJNIEnv m_JavaVM == nullptr");
        return nullptr;
    }
    *isAttach = false;
    status = m_JavaVM->GetEnv((void **) &env, JNI_VERSION_1_4);
    if (status != JNI_OK) {
        status = m_JavaVM->AttachCurrentThread(&env, nullptr);
        if (status != JNI_OK) {
            LOGD("FFMediaPlayer::GetJNIEnv failed to attach current thread");
            return nullptr;
        }
        *isAttach = true;
    }
    return env;
}

jobject FFmpegPlayer::GetJavaObj() {
    return m_JavaObj;
}

JavaVM *FFmpegPlayer::GetJavaVM() {
    return m_JavaVM;
}




