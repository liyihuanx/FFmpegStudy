//
// Created by k1 on 2022/9/8.
//

#ifndef FFMPEGSTUDY_FFMPEGPLAYER_H
#define FFMPEGSTUDY_FFMPEGPLAYER_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include <libswresample/swresample.h>
};

#include "log4c.h"
#include <android/native_window_jni.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

class FFmpegPlayer {
private:




public:



public:
    FFmpegPlayer();

    ~FFmpegPlayer();

    void init(JNIEnv *jniEnv, jobject obj, char *url,jobject surface);
};


#endif //FFMPEGSTUDY_FFMPEGPLAYER_H
