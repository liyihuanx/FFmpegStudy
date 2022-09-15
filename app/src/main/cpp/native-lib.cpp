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
    LOGD("native_1playVideo")
    FFmpegPlayer *ffmpeg_player = new FFmpegPlayer();
    LOGD("native_1playVideo - ffmpeg_player")

    ANativeWindow *window = ANativeWindow_fromSurface(env,surface_view);
    LOGD("native_1playVideo - ANativeWindow_fromSurface")

    ffmpeg_player->setWindow(window);
    LOGD("native_1playVideo - setWindow")

    char *dataSource = const_cast<char *>(env->GetStringUTFChars(url, nullptr));
    ffmpeg_player->prepare(dataSource);

    // 释放掉
    env->ReleaseStringUTFChars(url, dataSource);

}

