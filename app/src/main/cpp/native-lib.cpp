#include <jni.h>
#include <string>
#include "log4c.h"
#include "util.h"
#include "FFmpegPlayer.h"

extern "C" {
#include <libavutil/avutil.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
}



using namespace std;


JavaVM *vm = nullptr;


jstring helloFFmpeg(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = av_version_info();

    return env->NewStringUTF(hello.c_str());
}

void playVideo(
        JNIEnv *env,
        jobject /* this */,jstring data,jobject surface) {


}

jint Mp4toYuv2(JNIEnv *env, jobject /* this */, jstring filename_in, jstring filename_out) {
    char *input_url = const_cast<char *>(env->GetStringUTFChars(filename_in, nullptr));
    char *output_url = const_cast<char *>(env->GetStringUTFChars(filename_out, nullptr));

    // 1.
    AVFormatContext *avFormatContext;
    // 2.
    AVCodecContext *avCodecContext;
    // 3.
    AVCodec *avCodec;
    // 4.
    AVPacket *avPacket = nullptr;
    // 5.
    AVFrame *avFrame = nullptr;
    AVFrame *avFrameYUV = nullptr;

    // 6.
    SwsContext *swsContext;

    FILE *fileYuv;


    int ret = -1;

    int videoindex = -1;

    // 1.注册编解码器
    av_register_all();


    // 2.
    avFormatContext = avformat_alloc_context();
    // 3.
    ret = avformat_open_input(&avFormatContext, input_url, nullptr, nullptr);
    if (ret != 0) {
        return -1;
    }

    // 3.
    ret = avformat_find_stream_info(avFormatContext, nullptr);
    if (ret < 0) {
        return -1;
    }


    // 4.
    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
//        if (avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
//            videoIndex = i;
//            break;
//        }
        AVStream *stream = avFormatContext->streams[i];
        AVCodecParameters *parameters = stream->codecpar;
        AVMediaType mediaType = parameters->codec_type;
        if (mediaType == AVMEDIA_TYPE_VIDEO) {

            videoindex = i;

            avCodec = avcodec_find_decoder(parameters->codec_id);

            // 5.
            avCodecContext = avcodec_alloc_context3(avCodec);
            ret = avcodec_parameters_to_context(avCodecContext, parameters);
            if (ret < 0) {
                return -1;
            }
            break;
        }
    }

    // 7.
    ret = avcodec_open2(avCodecContext, avCodec, nullptr);
    if (ret != 0) {
        LOGD("打开解码器失败！");
        return -1;
    } else {
        LOGD("打开成功！");
    }

    // 8.
    avPacket = av_packet_alloc();
    avFrame = av_frame_alloc();
    avFrameYUV = av_frame_alloc();


    unsigned char *out_buffer = (unsigned char *) av_malloc(
            av_image_get_buffer_size(AV_PIX_FMT_YUV420P, avCodecContext->width,
                                     avCodecContext->height, 1));


    av_image_fill_arrays(avFrameYUV->data,
                         avFrameYUV->linesize,
                         out_buffer,
                         AV_PIX_FMT_YUV420P,
                         avCodecContext->width,
                         avCodecContext->height, 1);

    swsContext = sws_getContext(avCodecContext->width, avCodecContext->height,
                                avCodecContext->pix_fmt,
                                avCodecContext->width, avCodecContext->height, AV_PIX_FMT_YUV420P,
                                SWS_BICUBIC, nullptr, nullptr, nullptr);

    fileYuv = fopen(output_url, "wb+");


    // 9.
    while (av_read_frame(avFormatContext, avPacket) >= 0) {
        if (avPacket->stream_index == videoindex) {
            // 10.
            ret = avcodec_send_packet(avCodecContext, avPacket);
            if (ret < 0) {
                break;
            }

            while (true) {
                // 11.
                ret = avcodec_receive_frame(avCodecContext, avFrame);
                if (ret) {
                    break;
                }
                sws_scale(swsContext,
                          avFrame->data, avFrame->linesize,
                          0, avCodecContext->height,
                          avFrameYUV->data, avFrameYUV->linesize);


                int y_size = avCodecContext->width * avCodecContext->height;
                fwrite(avFrameYUV->data[0], 1, y_size, fileYuv);    //Y
                fwrite(avFrameYUV->data[1], 1, y_size / 4, fileYuv);  //U
                fwrite(avFrameYUV->data[2], 1, y_size / 4, fileYuv);  //V
            }

        }


    }

    sws_freeContext(swsContext);

    fclose(fileYuv);

    av_frame_free(&avFrame);
    av_frame_free(&avFrameYUV);
    avcodec_close(avCodecContext);
    avformat_close_input(&avFormatContext);


    return 0;
}



static const JNINativeMethod dynamicMethods[] = {
        {"native_helloFFmpeg", "()Ljava/lang/String;", (jstring *) helloFFmpeg},
        {"native_Mp4toYuv",        "(Ljava/lang/String;Ljava/lang/String;)I", (jint *) Mp4toYuv2},

};

// 动态注册
static int registerNativeMethods(JNIEnv *env, const char *className) {
    jclass clazz = env->FindClass(className);
    if (clazz == nullptr) {
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, dynamicMethods, NELEM(dynamicMethods)) < 0) {
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

jint JNI_OnLoad(JavaVM *jvm, void *) {
    JNIEnv *env = NULL;
    if (jvm->GetEnv((void **) &env, JNI_VERSION_1_6)) {
        return JNI_ERR;
    }

    if (registerNativeMethods(env, "com/example/ffmpegstudy/MainActivity") != 1) {
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegstudy_MainActivity_native_1playVideo(JNIEnv *env, jobject thiz, jstring url,
                                                            jobject surface_view) {
    FFmpegPlayer *ffmpeg_player = new FFmpegPlayer();
    ANativeWindow *window = ANativeWindow_fromSurface(env,surface_view);
    ffmpeg_player->setWindow(window);
    char *dataSource = const_cast<char *>(env->GetStringUTFChars(url, nullptr));
    ffmpeg_player->prepare(dataSource);

    // 释放掉
    env->ReleaseStringUTFChars(url, dataSource);

}


extern "C"
JNIEXPORT jint JNICALL
Java_com_example_ffmpegstudy_MainActivity_native_1play(JNIEnv *env, jobject thiz,jobject surface_view) {
    char * file_name = "/data/data/com.example.ffmpegstudy/cache/testvideo1.mp4";

    av_register_all();

    AVFormatContext * pFormatCtx = avformat_alloc_context();

    // Open video file
    if(avformat_open_input(&pFormatCtx, file_name, NULL, NULL)!=0) {

        LOGE("Couldn't open file:%s\n", file_name);
        return -1; // Couldn't open file
    }

    // Retrieve stream information
    if(avformat_find_stream_info(pFormatCtx, NULL)<0) {
        LOGE("Couldn't find stream information.");
        return -1;
    }

    // Find the first video stream
    int videoStream = -1, i;
    for (i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO
            && videoStream < 0) {
            videoStream = i;
        }
    }
    if(videoStream==-1) {
        LOGE("Didn't find a video stream.");
        return -1; // Didn't find a video stream
    }

    // Get a pointer to the codec context for the video stream
    AVCodecContext  * pCodecCtx = pFormatCtx->streams[videoStream]->codec;

    // Find the decoder for the video stream
    AVCodec * pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec==NULL) {
        LOGE("Codec not found.");
        return -1; // Codec not found
    }

    if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        LOGE("Could not open codec.");
        return -1; // Could not open codec
    }

    // 获取native window
    ANativeWindow* nativeWindow = ANativeWindow_fromSurface(env, surface_view);

    // 获取视频宽高
    int videoWidth = pCodecCtx->width;
    int videoHeight = pCodecCtx->height;

    // 设置native window的buffer大小,可自动拉伸
    ANativeWindow_setBuffersGeometry(nativeWindow,  videoWidth, videoHeight, WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer windowBuffer;

    if(avcodec_open2(pCodecCtx, pCodec, NULL)<0) {
        LOGE("Could not open codec.");
        return -1; // Could not open codec
    }

    // Allocate video frame
    AVFrame * pFrame = av_frame_alloc();

    // 用于渲染
    AVFrame * pFrameRGBA = av_frame_alloc();
    if(pFrameRGBA == NULL || pFrame == NULL) {
        LOGE("Could not allocate video frame.");
        return -1;
    }

    // Determine required buffer size and allocate buffer
    int numBytes=av_image_get_buffer_size(AV_PIX_FMT_RGBA, pCodecCtx->width, pCodecCtx->height, 1);
    uint8_t * buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
    av_image_fill_arrays(pFrameRGBA->data, pFrameRGBA->linesize, buffer, AV_PIX_FMT_RGBA,
                         pCodecCtx->width, pCodecCtx->height, 1);

    // 由于解码出来的帧格式不是RGBA的,在渲染之前需要进行格式转换
    struct SwsContext *sws_ctx = sws_getContext(pCodecCtx->width,
                                                pCodecCtx->height,
                                                pCodecCtx->pix_fmt,
                                                pCodecCtx->width,
                                                pCodecCtx->height,
                                                AV_PIX_FMT_RGBA,
                                                SWS_BILINEAR,
                                                NULL,
                                                NULL,
                                                NULL);

    int frameFinished;
    AVPacket packet;
    while(av_read_frame(pFormatCtx, &packet)>=0) {
        // Is this a packet from the video stream?
        if(packet.stream_index==videoStream) {

            // Decode video frame
            avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);

            // 并不是decode一次就可解码出一帧
            if (frameFinished) {

                // lock native window buffer
                ANativeWindow_lock(nativeWindow, &windowBuffer, 0);

                // 格式转换
                sws_scale(sws_ctx, (uint8_t const * const *)pFrame->data,
                          pFrame->linesize, 0, pCodecCtx->height,
                          pFrameRGBA->data, pFrameRGBA->linesize);

                // 获取stride
                uint8_t * dst = (uint8_t*)windowBuffer.bits;
                int dstStride = windowBuffer.stride * 4;
                uint8_t * src = (uint8_t*) (pFrameRGBA->data[0]);
                int srcStride = pFrameRGBA->linesize[0];

                // 由于window的stride和帧的stride不同,因此需要逐行复制
                int h;
                for (h = 0; h < videoHeight; h++) {
                    memcpy(dst + h * dstStride,
                           src + h * srcStride,
                           srcStride);
                }

                ANativeWindow_unlockAndPost(nativeWindow);
            }

        }
        av_packet_unref(&packet);
    }

    av_free(buffer);
    av_free(pFrameRGBA);

    // Free the YUV frame
    av_free(pFrame);

    // Close the codecs
    avcodec_close(pCodecCtx);

    // Close the video file
    avformat_close_input(&pFormatCtx);


    return -1;
}