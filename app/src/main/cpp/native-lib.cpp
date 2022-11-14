#include <jni.h>
#include <string>
#include "log4c.h"
#include "util/util.h"
#include "FFmpegPlayer.h"
#include "render/video/VideoOpenGLRender.h"
#include "record/MediaRecorderContext.h"

extern "C" {
#include <libavutil/avutil.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
}

using namespace std;


JavaVM *vm = nullptr;
FFmpegPlayer *ffmpegPlayer = nullptr;

jstring helloFFmpeg(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = av_version_info();

    return env->NewStringUTF(hello.c_str());
}


static const JNINativeMethod dynamicMethods[] = {
        {"native_helloFFmpeg", "()Ljava/lang/String;", (jstring *) helloFFmpeg},
};

// 动态注册
static int registerNativeMethods(JNIEnv *env, const char *className) {
    jclass clazz = env->FindClass(className);
    if (clazz == nullptr) {
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, dynamicMethods, NELEM(dynamicMethods)) < 0) {
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

jint JNI_OnLoad(JavaVM *jvm, void *) {
    JNIEnv *env = NULL;
    if (jvm->GetEnv((void **) &env, JNI_VERSION_1_6)) {
        return JNI_ERR;
    }

    if (registerNativeMethods(env, "com/example/ffmpegstudy/FFmpegPlayer") != 1) {
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegstudy_FFmpegPlayer_native_1OnSurfaceCreated(JNIEnv *env, jclass clazz,
                                                                   jint render_type) {
    VideoOpenGLRender::getInstance()->onSurfaceCreated();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegstudy_FFmpegPlayer_native_1OnSurfaceChanged(JNIEnv *env, jclass clazz,
                                                                   jint render_type, jint width,
                                                                   jint height) {
    VideoOpenGLRender::getInstance()->onSurfaceChanged(width, height);

}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegstudy_FFmpegPlayer_native_1OnDrawFrame(JNIEnv *env, jclass clazz,
                                                              jint render_type) {
    VideoOpenGLRender::getInstance()->onDrawFrame();

}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegstudy_FFmpegPlayer_native_1play(JNIEnv *env, jobject thiz) {
    ffmpegPlayer->play();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegstudy_FFmpegPlayer_native_1init(JNIEnv *env, jobject thiz, jint media_type,
                                                       jstring url, jobject surface_view) {
    LOGD("native_init")
    char *dataSource = const_cast<char *>(env->GetStringUTFChars(url, nullptr));

    ffmpegPlayer = new FFmpegPlayer();
    ffmpegPlayer->init(env, thiz, dataSource, media_type, surface_view);

    // 释放掉
    env->ReleaseStringUTFChars(url, dataSource);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegstudy_FFmpegPlayer_native_1uninit(JNIEnv *env, jobject thiz) {
    LOGD("native_uninit")
    ffmpegPlayer->uninit();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegstudy_FFmpegPlayer_native_1resume(JNIEnv *env, jobject thiz) {
    LOGD("native_resume")
    ffmpegPlayer->resume();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegstudy_FFmpegPlayer_native_1pause(JNIEnv *env, jobject thiz) {
    LOGD("native_pause")
    ffmpegPlayer->pause();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegstudy_FFmpegPlayer_native_1stop(JNIEnv *env, jobject thiz) {
    LOGD("native_stop")
    ffmpegPlayer->stop();
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegstudy_camera_FFMediaRecord_native_1CreateContext(JNIEnv *env, jobject thiz) {
    MediaRecorderContext::CreateContext(env, thiz);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegstudy_camera_FFMediaRecord_native_1DestroyContext(JNIEnv *env,
                                                                         jobject thiz) {
    MediaRecorderContext::DeleteContext(env, thiz);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_example_ffmpegstudy_camera_FFMediaRecord_native_1Init(JNIEnv *env, jobject thiz) {
    MediaRecorderContext *pContext = MediaRecorderContext::GetContext(env, thiz);
    if (pContext) return pContext->Init();
    return 0;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_example_ffmpegstudy_camera_FFMediaRecord_native_1UnInit(JNIEnv *env, jobject thiz) {
    MediaRecorderContext *pContext = MediaRecorderContext::GetContext(env, thiz);
    if (pContext) return pContext->UnInit();
    return 0;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegstudy_camera_FFMediaRecord_native_1OnPreviewFrame(JNIEnv *env, jobject thiz,
                                                                         jint format,
                                                                         jbyteArray data,
                                                                         jint width, jint height) {
    int len = env->GetArrayLength(data);
    auto *buf = new unsigned char[len];
    env->GetByteArrayRegion(data, 0, len, reinterpret_cast<jbyte *>(buf));

    MediaRecorderContext *pContext = MediaRecorderContext::GetContext(env, thiz);
    if (pContext) pContext->OnPreviewFrame(format, buf, width, height);
    delete[] buf;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegstudy_camera_FFMediaRecord_native_1OnSurfaceCreated(JNIEnv *env,
                                                                           jobject thiz) {
    MediaRecorderContext *pContext = MediaRecorderContext::GetContext(env, thiz);
    if (pContext) pContext->OnSurfaceCreated();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegstudy_camera_FFMediaRecord_native_1OnSurfaceChanged(JNIEnv *env,
                                                                           jobject thiz, jint width,
                                                                           jint height) {
    MediaRecorderContext *pContext = MediaRecorderContext::GetContext(env, thiz);
    if (pContext) pContext->OnSurfaceChanged(width, height);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegstudy_camera_FFMediaRecord_native_1OnDrawFrame(JNIEnv *env, jobject thiz) {
    MediaRecorderContext *pContext = MediaRecorderContext::GetContext(env, thiz);
    if (pContext) pContext->OnDrawFrame();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegstudy_camera_FFMediaRecord_native_1SetTransformMatrix(JNIEnv *env,
                                                                             jobject thiz,
                                                                             jfloat translate_x,
                                                                             jfloat translate_y,
                                                                             jfloat scale_x,
                                                                             jfloat scale_y,
                                                                             jint degree,
                                                                             jint mirror) {
    MediaRecorderContext *pContext = MediaRecorderContext::GetContext(env, thiz);
    if (pContext)
        pContext->SetTransformMatrix(translate_x, translate_y, scale_x, scale_y, degree, mirror);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_example_ffmpegstudy_camera_FFMediaRecord_native_1StartRecord(JNIEnv *env, jobject thiz,
                                                                      jint recorder_type,
                                                                      jstring out_url,
                                                                      jint frame_width,
                                                                      jint frame_height,
                                                                      jlong video_bit_rate,
                                                                      jint fps) {

    const char *url = env->GetStringUTFChars(out_url, nullptr);
    MediaRecorderContext *pContext = MediaRecorderContext::GetContext(env, thiz);
    env->ReleaseStringUTFChars(out_url, url);
    if (pContext)
        pContext->startRecord(recorder_type, url, frame_width, frame_height, video_bit_rate, fps);

    return 0;

}
extern "C"
JNIEXPORT jint JNICALL
Java_com_example_ffmpegstudy_camera_FFMediaRecord_native_1StopRecord(JNIEnv *env, jobject thiz) {
    MediaRecorderContext *pContext = MediaRecorderContext::GetContext(env, thiz);
    if (pContext) pContext->stopRecord();

    return 0;

}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegstudy_camera_FFMediaRecord_native_1onAudioData(JNIEnv *env, jobject thiz,
                                                                      jbyteArray data,
                                                                      jint data_size) {


    int len = env->GetArrayLength (data);
    auto* buf = new unsigned char[len];
    env->GetByteArrayRegion(data, 0, len, reinterpret_cast<jbyte*>(buf));
    MediaRecorderContext *pContext = MediaRecorderContext::GetContext(env, thiz);
    if(pContext) pContext->OnAudioData(buf, len);
    delete[] buf;


}