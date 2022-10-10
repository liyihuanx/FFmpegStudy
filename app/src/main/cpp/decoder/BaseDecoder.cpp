//
// Created by leeyh on 2022/9/27.
//

#include "BaseDecoder.h"

void BaseDecoder::onCreate(char *url, AVMediaType mediaType) {
    // 深拷贝，免得变量释放后导致错误，长度+1 ==> 处理 \0
    data_source = new char[strlen(url) + 1];
    // 把源Copy给成员
    strcpy(data_source, url);

    // 记录media类型
    avMediaType = mediaType;
}

void BaseDecoder::onDestroy() {
    if (data_source) {
        delete data_source;
        data_source = nullptr;
    }
}

void BaseDecoder::start() {

    new thread(startDecodeTread, this);
}


void BaseDecoder::startDecodeTread(BaseDecoder *decoder) {
    // 初始化上下文环境
    if (decoder->initFFmpegEnvironment() < 0) {
        return;
    }

    // 初始化解码环境
    decoder->initDecoder();

    // 开始循环解码
    decoder->startDecoder();

    // 解码完释放
    decoder->releaseDecoder();
}

int BaseDecoder::initFFmpegEnvironment() {
    int ret = 0;
    // 1.
    avFormatContext = avformat_alloc_context();

    // 2.
    ret = avformat_open_input(&avFormatContext, data_source, nullptr, nullptr);
    if (ret != 0) {
        LOGD("avformat_open_input error=%s", av_err2str(ret));
        goto end;
    }

    // 3.
    ret = avformat_find_stream_info(avFormatContext, nullptr);
    if (ret < 0) {
        LOGD("avformat_find_stream_info error=%s", av_err2str(ret));
        goto end;
    }

    // 4. 在onCreate记录了mediatype
    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
        AVMediaType codec_type = avFormatContext->streams[i]->codecpar->codec_type;
        if (avFormatContext->streams[i]->codecpar->codec_type == avMediaType) {
            stream_index = i;
            break;
        }
    }

    // 5.打开解码器
    avCodec = avcodec_find_decoder(avFormatContext->streams[stream_index]->codecpar->codec_id);
    if (avCodec == nullptr) {
        LOGD("avcodec_find_decoder error")
        goto end;
    }

    // 6.
    avCodecContext = avcodec_alloc_context3(avCodec);
    if (avCodecContext == nullptr) {
        LOGD("avcodec_alloc_context3 error")
        goto end;
    }

    ret = avcodec_parameters_from_context(avFormatContext->streams[stream_index]->codecpar,
                                          avCodecContext);
    if (ret < 0) {
        LOGD("avcodec_parameters_to_context error=%s", av_err2str(ret));
        goto end;
    }

    // 7.
    ret = avcodec_open2(avCodecContext, avCodec, nullptr);
    if (ret != 0) {
        LOGD("avcodec_open2 ret=%d, error=%s", ret, av_err2str(ret));
        goto end;
    }

    frame = av_frame_alloc();

    packet = av_packet_alloc();

    end:
    return -1;
}


int BaseDecoder::startDecoder() {
    LOGD("releaseDecoder");

    end:
    return -1;
}

int BaseDecoder::releaseDecoder() {
    LOGD("releaseDecoder");
    return 0;
}


BaseDecoder::BaseDecoder() = default;

BaseDecoder::~BaseDecoder() = default;
