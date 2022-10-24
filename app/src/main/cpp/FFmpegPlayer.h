//
// Created by k1 on 2022/9/8.
//

#ifndef FFMPEGSTUDY_FFMPEGPLAYER_H
#define FFMPEGSTUDY_FFMPEGPLAYER_H

#define JAVA_PLAYER_EVENT_CALLBACK_API_NAME "playerEventCallback"

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include <libswresample/swresample.h>
};

#include "log4c.h"
#include "decoder/AudioDecoder.h"
#include "decoder/VideoDecoder.h"
#include <android/native_window_jni.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

class FFmpegPlayer {
private:
    VideoDecoder *videoDecoder = nullptr;
    AudioDecoder *audioDecoder = nullptr;

    BaseVideoRender *videoRender = nullptr;
    BaseAudioRender *audioRender = nullptr;
public:
    JavaVM *m_JavaVM = nullptr;
    jobject m_JavaObj = nullptr;

public:
    FFmpegPlayer();

    ~FFmpegPlayer();

    void init(JNIEnv *jniEnv, jobject obj, char *url, int videoRenderType, jobject surface);

    void play();

    static void PostMessage(void *context, int msgType, float msgCode);

    virtual JNIEnv *GetJNIEnv(bool *isAttach);

    virtual jobject GetJavaObj();

    virtual JavaVM *GetJavaVM();

};


#endif //FFMPEGSTUDY_FFMPEGPLAYER_H
