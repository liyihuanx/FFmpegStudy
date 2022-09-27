//
// Created by leeyh on 2022/9/27.
//

#include "BaseDecoder.h"

void BaseDecoder::onCreate(char *url) {
    // 深拷贝，免得变量释放后导致错误，长度+1 ==> 处理 \0
    data_source = new char[strlen(url) + 1];
    // 把源Copy给成员
    strcpy(data_source, url);
}

void BaseDecoder::onDestroy() {
    if (data_source) {
        delete data_source;
        data_source = nullptr;
    }
}

int BaseDecoder::initFFmpegCtx() {
    int ret = 0;
    // 1.
    avFormatContext = avformat_alloc_context();
    // 2.
    ret = avformat_open_input(&avFormatContext,data_source, nullptr, nullptr);
    if (ret != 0) {
        LOGD("avformat_open_input error=%s", av_err2str(ret));
        goto end;
    }

    ret = avformat_find_stream_info(avFormatContext, nullptr);
    if (ret < 0) {
        LOGD("avformat_find_stream_info error=%s", av_err2str(ret));
        goto end;
    }


end:
    return -1;
}

int BaseDecoder::initDecoder() {
    return 0;
}

int BaseDecoder::startDecoder() {
    return 0;
}

BaseDecoder::BaseDecoder() = default;

BaseDecoder::~BaseDecoder() = default;
