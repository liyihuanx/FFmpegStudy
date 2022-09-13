//
// Created by k1 on 2022/9/8.
//

#include "FFmpegPlayer.h"

FFmpegPlayer::FFmpegPlayer() {

}

FFmpegPlayer::~FFmpegPlayer() {

}

void FFmpegPlayer::prepare(char *data) {
    // 深拷贝，免得变量释放后导致错误，长度+1 ==> 处理 \0
    data_source = new char[strlen(data) + 1];
    // 把源Copy给成员
    strcpy(data_source, data);

    // 准备阶段
    // 1.创建上下文
    av_format_ctx = avformat_alloc_context();


    int ret = 0;
    // 2. 打开输入流
    ret = avformat_open_input(&av_format_ctx, this->data_source, nullptr, nullptr);
    if (ret != 0) {
        LOGD("avformat_open_input error=%s", av_err2str(ret));
        return;
    }

    // 3.再打开文件中的流信息
    ret = avformat_find_stream_info(av_format_ctx, nullptr);
    if (ret < 0) {
        LOGD("avformat_find_stream_info error=%s", av_err2str(ret));
        return;
    }

    // 4.遍历流信息
    for (int i = 0; i < av_format_ctx->nb_streams; ++i) {
        AVMediaType codec_type = av_format_ctx->streams[i]->codecpar->codec_type;
        if (codec_type == AVMEDIA_TYPE_VIDEO) {
            video_index = i;
            break;
        } else if (codec_type == AVMEDIA_TYPE_VIDEO) {
            audio_index = i;
        }
        // 拆解如下
//        // 4.1 拿到流信息
//        AVStream *av_stream = av_format_ctx->streams[i];
//        // 4.2 流的参数
//        AVCodecParameters *codecpar = av_stream->codecpar;
//        // 4.3 ID
//        AVCodecID av_codec_id = codecpar->codec_id;
//        // 4.4 类型
//        AVMediaType codec_type = codecpar->codec_type;
//        if (codec_type == AVMEDIA_TYPE_AUDIO) {
//            video_index = i;
//        } else if (codec_type == AVMEDIA_TYPE_VIDEO) {
//            audio_index = i;
//        }
    }

    // 5.打开解码器
    AVCodecID av_codec_id = av_format_ctx->streams[video_index]->codecpar->codec_id;
    av_codec = avcodec_find_encoder(av_codec_id);

    // 6.获取解码器上下文
    av_codec_ctx = avcodec_alloc_context3(av_codec);

    // 7.将流的参数给上下文
    AVCodecParameters *codecpar = av_format_ctx->streams[video_index]->codecpar;
    ret = avcodec_parameters_to_context(av_codec_ctx, codecpar);
    if (ret < 0) {
        LOGD("avcodec_parameters_to_context error=%s", av_err2str(ret));
        return;
    }

    // 8.打开解码器
    ret = avcodec_open2(av_codec_ctx, av_codec, nullptr);
    if (ret != 0) {
        LOGD("avcodec_open2 error=%s", av_err2str(ret));
        return;
    }


    // 开始工作
    // 1. 创建 存放解码后的数据frame 和 存放编码数据packet
    frame = av_frame_alloc();
    packet = av_packet_alloc();

    // 2. 读取数据信息到packet中,注意是av_format_ctx
    while (av_read_frame(av_format_ctx, packet) >= 0) {
        // 3. 判断是video还是audio，各自处理
        if (packet->stream_index == video_index) {
            // 4. 将读取到的数据发送给解码器解码
            ret = avcodec_send_packet(av_codec_ctx, packet);
            if (ret != 0) {
                LOGD("avcodec_send_packet error=%s", av_err2str(ret));
                return;
            }

            // 4. 从解码器拿解码后的数据保存在frame
            while (avcodec_receive_frame(av_codec_ctx, frame) >= 0) {
                // 5. 进行格式转换并且渲染 YUV ——> RGB
            }
        }


    }

    // 收尾工作
    // 1.释放各种
    if(frame != nullptr) {
        av_frame_free(&frame);
        frame = nullptr;
    }

    if(packet != nullptr) {
        av_packet_unref(packet);
        av_packet_free(&packet);
        packet = nullptr;
    }

    if(av_codec_ctx != nullptr) {
        avcodec_close(av_codec_ctx);
        avcodec_free_context(&av_codec_ctx);
        av_codec_ctx = nullptr;
        av_codec = nullptr;
    }

    if(av_format_ctx != nullptr) {
        avformat_close_input(&av_format_ctx);
        avformat_free_context(av_format_ctx);
        av_format_ctx = nullptr;
    }
}


void FFmpegPlayer::start() {

}

void FFmpegPlayer::release() {

}




