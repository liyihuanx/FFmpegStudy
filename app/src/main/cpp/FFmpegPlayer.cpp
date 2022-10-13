//
// Created by k1 on 2022/9/8.
//

#include "FFmpegPlayer.h"
#include "decoder/VideoDecoder.h"
#include "render/ANativeRender.h"
#include "decoder/AudioDecoder.h"
#include "render/AudioRender.h"

FFmpegPlayer::FFmpegPlayer() {

}

FFmpegPlayer::~FFmpegPlayer() {

}

void FFmpegPlayer::init(JNIEnv *jniEnv, jobject obj, char *url, jobject surface) {
    LOGD("FFmpegPlayer::init");
    auto *videoDecoder = new VideoDecoder(url);
    auto *aNativeRender = new ANativeRender(jniEnv, surface);
    videoDecoder->setVideoRender(aNativeRender);
    videoDecoder->start();

    auto *audioDecoder = new AudioDecoder(url);
    auto *audioRender = new AudioRender();
    audioDecoder->setVideoRender(audioRender);
    audioDecoder->start();


}



