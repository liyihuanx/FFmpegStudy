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
#include "../util/ImageDef.h"
#include "../ThreadSafeQueue.h"

using namespace std;

class SingleVideoRecorder {
public:
    SingleVideoRecorder(const char *outUrl, int frameWidth, int frameHeight, long bitRate, int fps);

    ~SingleVideoRecorder();

    int StartRecord();

    int StopRecord();

    int DispatchRecordFrame(NativeImage *inputFrame);

private:
    static void StartH264EncoderThread(SingleVideoRecorder *recorder);

    int EncodeFrame(AVFrame *pFrame);

private:
    char *out_url = nullptr;
    thread *encodeThread = nullptr;
    ThreadSafeQueue<NativeImage *> frameQueue;
    volatile int isExit = 0;


    AVFormatContext *avFormatContext = nullptr;
    AVStream *avStream = nullptr;
    AVCodec *avCodec = nullptr;
    AVCodecContext *avCodecContext = nullptr;
    SwsContext *swsContext = nullptr;


    AVFrame *frame = nullptr;
    AVPacket packet;
    uint8_t *frame_buf = nullptr;

    // 帧的信息
    int frame_width = -1;
    int frame_height = -1;
    long bit_rate = -1;
    int fps = -1;
    int frame_index = 0;

};


#endif //FFMPEGSTUDY_SINGLEVIDEORECORDER_H
