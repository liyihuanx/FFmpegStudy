//
// Created by leeyh on 2022/11/8.
//

#ifndef FFMPEGSTUDY_SINGLEVIDEORECORDER_H
#define FFMPEGSTUDY_SINGLEVIDEORECORDER_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

#include "../log4c.h"
#include "thread"

using namespace std;

class SingleVideoRecorder {
public:
    SingleVideoRecorder(const char *outUrl, int frameWidth, int frameHeight, long bitRate, int fps);

    ~SingleVideoRecorder();

    int StartRecord();

    int StopRecord();

private:
    static void StartH264EncoderThread(SingleVideoRecorder *context);

    int EncodeFrame(AVFrame *pFrame);

private:
    char *out_url = nullptr;
    thread *encodeThread = nullptr;


    AVFormatContext *avFormatContext = nullptr;
    AVStream *avStream = nullptr;
    AVCodec *avCodec = nullptr;
    AVCodecContext *avCodecContext = nullptr;

    AVFrame *frame = nullptr;
    AVPacket *packet = nullptr;
    uint8_t *frame_buf = nullptr;

    // 帧的信息
    int frame_width = -1;
    int frame_height = -1;
    long bit_rate = -1;
    int fps = -1;

};


#endif //FFMPEGSTUDY_SINGLEVIDEORECORDER_H
