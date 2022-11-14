//
// Created by leeyh on 2022/11/14.
//


#ifndef FFMPEGSTUDY_SINGLEAUDIORECORDER_H
#define FFMPEGSTUDY_SINGLEAUDIORECORDER_H

#define DEFAULT_SAMPLE_RATE    44100
#define DEFAULT_CHANNEL_LAYOUT AV_CH_LAYOUT_STEREO

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
}

#include <libavutil/frame.h>
#include "../log4c.h"
#include "thread"
#include "../util/ImageDef.h"
#include "../ThreadSafeQueue.h"
#include "../render/audio/AudioFrame.h"

using namespace std;


class SingleAudioRecorder {
public:
    SingleAudioRecorder(const char *outUrl, int sampleRate, int channelLayout, int sampleFormat);

    ~SingleAudioRecorder();

    int StartRecord();

    int StopRecord();

    int DispatchRecordFrame(AudioFrame *inputFrame);

private:
    static void StartAACEncoderThread(SingleAudioRecorder *recorder);

    int EncodeFrame(AVFrame *pFrame);

private:
    char *out_url = nullptr;
    thread *encodeThread = nullptr;
    ThreadSafeQueue<AudioFrame *> frameQueue;

    volatile int isExit = 0;

    AVFormatContext *avFormatContext = nullptr;
    AVStream *avStream = nullptr;
    AVCodec *avCodec = nullptr;
    AVCodecContext *avCodecContext = nullptr;

    SwrContext *swrContext = nullptr;


    AVFrame *frame = nullptr;
    AVPacket packet;
    uint8_t *frame_buf = nullptr;

    // 帧的信息
    int frame_index = 0;
    int sample_rate;
    int channel_layout;
    int sample_format;

    int frame_buffer_size;

};


#endif //FFMPEGSTUDY_SINGLEAUDIORECORDER_H
