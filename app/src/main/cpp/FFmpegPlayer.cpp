//
// Created by k1 on 2022/9/8.
//

#include "FFmpegPlayer.h"
#include "decoder/VideoDecoder.h"

FFmpegPlayer::FFmpegPlayer() {

}

FFmpegPlayer::~FFmpegPlayer() {

}

void FFmpegPlayer::init(JNIEnv *jniEnv, jobject obj, char *url, jobject surface) {
    LOGD("FFmpegPlayer::init");
    auto *videoDecoder = new VideoDecoder(url);

    videoDecoder->start();
}



