//
// Created by leeyh on 2022/9/27.
//

#ifndef FFMPEGSTUDY_BASEDECODER_H
#define FFMPEGSTUDY_BASEDECODER_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include <libswresample/swresample.h>
#include <libavutil/samplefmt.h>
#include <libavutil/opt.h>
#include <libavutil/audio_fifo.h>
#include <libavutil/time.h>
};

#include "../log4c.h"
#include <thread>

#define MAX_PATH   2048
#define DELAY_THRESHOLD 100 //100ms

using namespace std;

class BaseDecoder {

private:
    // ffmpeg上下文
    AVFormatContext *avFormatContext = nullptr;
    // 解码器上下文
    AVCodecContext *avCodecContext = nullptr;
    // 解码器
    AVCodec *avCodec = nullptr;
    // 解码后的原始帧
    AVFrame *frame = nullptr;
    // 解码前的数据帧
    AVPacket *packet = nullptr;


    // 音视频数据
    char *data_source = nullptr;
    // 数据流的类型
    AVMediaType avMediaType = AVMEDIA_TYPE_UNKNOWN;
    // 音视频流的下标
    int stream_index = -1;
    // 当前播放时间
    long curPlayTime = 0;
    // 播放的起始时间
    long startPlayTime = -1;
    // 总时长 ms
    long duration = 0;

    // 锁和条件变量
    mutex decode_mutex;
    condition_variable decode_cond;
    thread *decode_thread = nullptr;

public:
    BaseDecoder();

    virtual ~BaseDecoder();

    // 给player调用
    void start();

    // 获取codec上下文
    AVCodecContext *getCodecContext();

protected:
    // 初始化Decoder（audio,video）
    void onCreate(char *url, AVMediaType mediaType);

    // 释放Decoder以及参数
    void onDestroy();

private:

    // 初始化ffmpeg上下文
    int initFFmpegEnvironment();

    // 初始化video/audio解码器要使用的配置
    virtual void initDecoderEnvironment() = 0;

    // 开始解码
    void startDecoder();

    // 释放解放相关变量
    virtual void releaseDecoder() = 0;

    // 更改播放状态
    void changeMediaStatus(int status);

    // 解码packet
    int decodeOnePacket();

    //更新显示时间戳
    void updateTimeStamp();

    //音视频同步
    long syncAV();

    // 线程函数
    static void startDecodeTread(BaseDecoder *decoder);

    // 向外提供解码后的frame帧
    virtual void onFrameAvailable(AVFrame *frame) = 0;

};


#endif //FFMPEGSTUDY_BASEDECODER_H
