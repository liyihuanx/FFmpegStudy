//
// Created by k1 on 2022/9/8.
//

#include "FFmpegPlayer.h"

int get_pcm_size();

FFmpegPlayer::FFmpegPlayer() {

}

FFmpegPlayer::~FFmpegPlayer() {

}


void FFmpegPlayer::start() {

}

void FFmpegPlayer::release() {

}

void FFmpegPlayer::setWindow(ANativeWindow *native_window) {
    this->native_window = native_window;
}

int get_pcm_size() {
    int pcm_data_size = 0;


    // 采样率 和 样本数的关系？
    // 样本数 = 采样率 * 声道数 * 位声


    return pcm_data_size;
}


void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *args) {
    auto *player = static_cast<FFmpegPlayer *>(args);

    // 原始包
    // 重采样（将音频PCM数据转换成播放器可识别的格式）
    // 重采样的一些配置
    // 假设：来源：10个48000   ---->  目标: 11个44100（10个不够，就向上取）
    int dst_nb_samples = av_rescale_rnd(
            // 获取下一个输入样本相对于下一个输出样本将经历的延迟
            swr_get_delay(player->swr_ctx, player->frame->sample_rate) + player->frame->nb_samples,
            player->out_sample_rate, // 输出采样率（44100）
            player->frame->sample_rate, // 输入采样率（音频的输入采样率，假设48000）
            AV_ROUND_UP // 先上取 取去11个才能容纳的上
    );

    // 重采样工作函数
    // samples_per_channel 每个通道输出的样本数
    int samples_per_channel = swr_convert(
            player->swr_ctx,
            &(player->out_buffers),  // 重采样后的buff
            dst_nb_samples,
            (const uint8_t **) player->frame->data, // 队列的AVFrame * 那的  PCM数据 未重采样的
            player->frame->nb_samples // 此帧描述的音频样本数（每个通道
    );

    int pcm_data_size = samples_per_channel * player->out_sample_size *
                        player->out_channels; // 941通道样本数  *  2样本格式字节数  *  2声道数  =3764


    (*bq)->Enqueue(
            bq,
            // PCM数据
            player->frame->data,
            pcm_data_size); // PCM数据对应的大小
}


void FFmpegPlayer::initOpenSLES() {

    // openSLES 专门接收返回值的类型
    SLresult result;

    // TODO 1.创建引擎对象并获取【引擎接口】
    // 1.1 创建对象
    result = slCreateEngine(&engineObject, 0, 0, 0, 0, 0);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("创建引擎 slCreateEngine error");
        return;
    }
    // 1.2 初始化 SL_BOOLEAN_FALSE:延时等待你创建成功
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("初始化引擎失败");
        return;
    }
    // 1.3 有了引擎对象后，获取引擎接口，拿到接口才能调用引擎方法
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineInterface);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("创建引擎接口失败");
        return;
    }

    // TODO 2.设置混音器

    // 2.1 创建
    result = (*engineInterface)->CreateOutputMix(engineInterface, &outputMixObject, 0, nullptr,
                                                 nullptr); // 环境特效，混响特效，.. 都不需要
    if (SL_RESULT_SUCCESS != result) {
        LOGD("初始化混音器 CreateOutputMix failed");
        return;
    }
    // 2.2 初始化
    result = (*outputMixObject)->Realize(outputMixObject,
                                         SL_BOOLEAN_FALSE); // SL_BOOLEAN_FALSE:延时等待你创建成功
    if (SL_RESULT_SUCCESS != result) {
        LOGD("初始化混音器 (*outputMixObject)->Realize failed");
        return;
    }
    // 2.3 获取混音器接口，就可设置声音效果

    // TODO 3.创建播放器
    // 3.1 播放器配置信息
    // 创建buffer缓存类型的队列
    SLDataLocator_AndroidSimpleBufferQueue loc_buf_queue = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 10};

    // 看做是一个bean类，存放的配置信息
    SLDataFormat_PCM format_pcm = {
            SL_DATAFORMAT_PCM, // PCM 数据格式
            2, // 声道数，双声道
            SL_SAMPLINGRATE_44_1, // 采样率（每秒44100个点）
            SL_PCMSAMPLEFORMAT_FIXED_16, // 每秒采样样本 存放大小 16bit
            SL_PCMSAMPLEFORMAT_FIXED_16, // 每个样本位数 16bit
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, // 前左声道  前右声道
            SL_BYTEORDER_LITTLEENDIAN // 字节序(小端)
    };

    // 看做bean类。存放的是缓存队列和配置信息
    SLDataSource audioSrc = {&loc_buf_queue, &format_pcm};

    // 3.2 配置音轨（输出）
    // SL_DATALOCATOR_OUTPUTMIX:输出混音器类型
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    // outmix最终混音器的成果，给后面代码使用
    SLDataSink audioSnk = {&loc_outmix, NULL};
    // 需要的接口 操作队列的接口
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};

    // 3.3 创建播放器
    result = (*engineInterface)->CreateAudioPlayer(engineInterface,
                                                   &bqPlayerObject, // 参数2：播放器
                                                   &audioSrc, // 参数3：音频配置信息
                                                   &audioSnk, // 参数4：混音器
                                                   1, // 参数5：开放的参数的个数
                                                   ids,  // 参数6：代表我们需要 Buff
                                                   req // 参数7：代表我们上面的Buff 需要开放出去
    );

    if (SL_RESULT_SUCCESS != result) {
        LOGD("创建播放器 CreateAudioPlayer failed!");
        return;
    }

    // 3.4 初始化
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        LOGD("实例化播放器 CreateAudioPlayer failed!");
        return;
    }
    LOGD("创建播放器 CreateAudioPlayer success!");


    // 3.5 获取播放器接口
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY,
                                             &bqPlayerPlayInterface); // SL_IID_PLAY:播放接口 == iplayer
    if (SL_RESULT_SUCCESS != result) {
        LOGD("获取播放接口 GetInterface SL_IID_PLAY failed!");
        return;
    }
    LOGI("3、创建播放器 Success");

    // TODO 4.设置回调函数
    // 获取播放器队列接口
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
                                             &bqPlayerBufferQueue);
    if (result != SL_RESULT_SUCCESS) {
        LOGD("获取播放队列 GetInterface SL_IID_BUFFERQUEUE failed!");
        return;
    }

    (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue,  // 传入刚刚设置好的队列
                                             bqPlayerCallback,  // 回调函数
                                             this); // 给回调函数的参数

}


void FFmpegPlayer::prepare(char *data) {
    // 深拷贝，免得变量释放后导致错误，长度+1 ==> 处理 \0
    data_source = new char[strlen(data) + 1];
    // 把源Copy给成员
    strcpy(data_source, data);



    // 准备阶段
    // 1.创建上下文
    av_format_ctx = avformat_alloc_context();

    LOGD("data_source : %s", data_source);

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
    LOGD("av_format_ctx->nb_streams : %d", av_format_ctx->nb_streams);
    // 4.遍历流信息
    for (int i = 0; i < av_format_ctx->nb_streams; ++i) {
        AVMediaType codec_type = av_format_ctx->streams[i]->codecpar->codec_type;
        if (codec_type == AVMEDIA_TYPE_VIDEO) {
            video_index = i;
        } else if (codec_type == AVMEDIA_TYPE_AUDIO) {
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
    // 视频
    AVCodecID av_codec_id = av_format_ctx->streams[video_index]->codecpar->codec_id;
    av_codec = avcodec_find_decoder(av_codec_id);
    if (av_codec == nullptr) {
        LOGD("avcodec_find_decoder error");
        return;
    }


    // 6.获取解码器上下文
    // 视频
    av_codec_ctx = avcodec_alloc_context3(av_codec);
    if (av_codec_ctx == nullptr) {
        LOGD("avcodec_alloc_context3 error");
        return;
    }

    // 7.将流的参数给上下文
    // 视频
    AVCodecParameters *codecpar = av_format_ctx->streams[video_index]->codecpar;
    ret = avcodec_parameters_to_context(av_codec_ctx, codecpar);
    if (ret < 0) {
        LOGD("avcodec_parameters_to_context error=%s", av_err2str(ret));
        return;
    }

    // 8.打开解码器
    // 视频
    ret = avcodec_open2(av_codec_ctx, av_codec, nullptr);
    if (ret != 0) {
        LOGD("avcodec_open2 ret=%d, error=%s", ret, av_err2str(ret));
        return;
    }



    // 音频
    AVCodecID audio_codec_id = av_format_ctx->streams[audio_index]->codecpar->codec_id;
    audio_codec = avcodec_find_decoder(audio_codec_id);
    if (audio_codec == nullptr) {
        LOGD("audio_avcodec_find_decoder error");
        return;
    }


    // 6.获取解码器上下文
    // 视频
    audio_codec_ctx = avcodec_alloc_context3(audio_codec);
    if (audio_codec_ctx == nullptr) {
        LOGD("audio_avcodec_alloc_context3 error");
        return;
    }



    // 7.将流的参数给上下文
    // 视频
    AVCodecParameters *audio_codecpar = av_format_ctx->streams[audio_index]->codecpar;
    ret = avcodec_parameters_to_context(audio_codec_ctx, audio_codecpar);
    if (ret < 0) {
        LOGD("audio_avcodec_parameters_to_context error=%s", av_err2str(ret));
        return;
    }

    // 8.打开解码器
    ret = avcodec_open2(audio_codec_ctx, audio_codec, nullptr);
    if (ret != 0) {
        LOGD("avcodec_open2 ret=%d, error=%s", ret, av_err2str(ret));
    }


    LOGD("准备转换")
    // 准备转换
    // 1.获取Video长宽
    video_width = av_codec_ctx->width;
    video_height = av_codec_ctx->height;
    // 2.新建rgb的帧
    frame_rgb = av_frame_alloc();
    // 3.获取，填充rgb buffer
    int bufferSize = av_image_get_buffer_size(AV_PIX_FMT_RGBA, video_width, video_height, 1);
    frame_rgb_buffer = static_cast<uint8_t *>(av_malloc(bufferSize * sizeof(uint8_t)));
    // 4. 怎么解释呢
    av_image_fill_arrays(frame_rgb->data, frame_rgb->linesize,
                         frame_rgb_buffer, AV_PIX_FMT_RGBA,
                         video_width, video_height, 1);

    // 5. 获取转换上下文
    SwsContext *sws_context = sws_getContext(av_codec_ctx->width, av_codec_ctx->height,
                                             av_codec_ctx->pix_fmt,
                                             av_codec_ctx->width, av_codec_ctx->height,
                                             AV_PIX_FMT_RGBA,
                                             SWS_FAST_BILINEAR, NULL, NULL, NULL
    );
    LOGD("渲染准备")

    // audio

    // 音频三要素
    /*
     * 1.采样率 44100 48000
     * 2.位声/采用格式大小  16bit == 2字节
     * 3.声道数 2  --- 人类就是两个耳朵
     */

    // 声道布局频道数
    out_channels = av_get_channel_layout_nb_channels(
            AV_CH_LAYOUT_STEREO); // STEREO:双声道类型 == 获取 声道数 2
    // 返回每个样本的字节数
    out_sample_size = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16); // 每个sample是16 bit == 2字节

    out_sample_rate = 44100; // 采样率

    // out_buffers_size = 176,400
    // 缓冲区大小？
    out_buffers_size = out_sample_rate * out_sample_size * out_channels; // 44100 * 2 * 2 = 176,400
    // 堆区开辟缓冲区
    out_buffers = static_cast<uint8_t *>(malloc(out_buffers_size));

    // codecContext 解码器
    swr_ctx = swr_alloc_set_opts(
            nullptr,
            AV_CH_LAYOUT_STEREO,  // 声道布局类型 双声道
            AV_SAMPLE_FMT_S16,  // 采样大小 16bit
            out_sample_rate, // 采样率  44100
            audio_codec_ctx->channel_layout, // 声道布局类型
            audio_codec_ctx->sample_fmt, // 采样大小
            audio_codec_ctx->sample_rate,  // 采样率
            0,
            0
    );
    // 初始化 重采样上下文
    swr_init(swr_ctx);


    // 初始话OpenSL ES
    initOpenSLES();


    // 渲染准备
    // 1.设置渲染区域和输入格式
    ANativeWindow_setBuffersGeometry(native_window, video_width, video_height,
                                     WINDOW_FORMAT_RGBA_8888);
    // 2.准备一个渲染buffer
    ANativeWindow_Buffer windowBuffer;
    LOGD("开始工作")
    // 开始工作
    // 1. 创建 存放解码后的数据frame 和 存放编码数据packet
    frame = av_frame_alloc();
    packet = av_packet_alloc();
    // 2. 读取数据信息到packet中,注意是av_format_ctx
    while (av_read_frame(av_format_ctx, packet) >= 0) {
        LOGD("读取数据")
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
                sws_scale(sws_context,
                          frame->data, frame->linesize,
                          0, video_height,
                          frame_rgb->data, frame_rgb->linesize);

                LOGD("渲染开始")
                // 6.渲染开始
                // 6.1 锁定Windows
                ANativeWindow_lock(native_window, &windowBuffer, nullptr);

                // 6.2 源头的bit像素
                uint8_t *dst = static_cast<uint8_t *>(windowBuffer.bits);
                // 6.3 输入图的步长，一行像素有多少个字节
                int dstLineSize = windowBuffer.stride * 4;

                // 6.4 目标
                uint8_t *src = frame_rgb->data[0];
                int srcLineSize = frame_rgb->linesize[0];

                // 6.5 一行行拷贝
                for (int i = 0; i < video_height; ++i) {
                    // 源数据 ---> 目标数据（字节对齐）
                    // 第一个像素开始，每次复制srcLineSize这么多
                    memcpy(dst + i * dstLineSize,
                           src + i * srcLineSize,
                           srcLineSize);
                }

                // 6.6 释放刷新
                ANativeWindow_unlockAndPost(native_window);

            }
        } else if (packet->stream_index == audio_index) {

            LOGD("音频数据")

            // 音频解析
            ret = avcodec_send_packet(audio_codec_ctx, packet);
            if (ret < 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                LOGD("avcodec_send_packet audio error=%s", av_err2str(ret));
                return;
            }

            while (avcodec_receive_frame(audio_codec_ctx, frame) >= 0) {
                // 使用OpenSL ES
                // TODO 5.设置播放器状态为播放状态
                (*bqPlayerPlayInterface)->SetPlayState(bqPlayerPlayInterface, SL_PLAYSTATE_PLAYING);
                LOGI("5、设置播放器状态为播放状态 Success");


                // TODO 6.手动激活回调函数
                bqPlayerCallback(bqPlayerBufferQueue, this);
                LOGI("6、手动激活回调函数 Success");

            }
        }


    }


    LOGD("收尾工作")
    // 收尾工作
    // 1.释放各种
    if (frame != nullptr) {
        av_frame_free(&frame);
        frame = nullptr;
    }

    if (packet != nullptr) {
        av_packet_unref(packet);
        av_packet_free(&packet);
        packet = nullptr;
    }

    if (av_codec_ctx != nullptr) {
        avcodec_close(av_codec_ctx);
        avcodec_free_context(&av_codec_ctx);
        av_codec_ctx = nullptr;
        av_codec = nullptr;
    }

    if (av_format_ctx != nullptr) {
        avformat_close_input(&av_format_ctx);
        avformat_free_context(av_format_ctx);
        av_format_ctx = nullptr;
    }

    if (audio_codec_ctx != nullptr) {
        avcodec_close(audio_codec_ctx);
        avcodec_free_context(&audio_codec_ctx);
        audio_codec_ctx = nullptr;
        audio_codec = nullptr;
    }

}


