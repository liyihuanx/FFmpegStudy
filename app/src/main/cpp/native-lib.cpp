#include <jni.h>
#include <string>
#include "log4c.h"
#include "util.h"


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


static const JNINativeMethod dynamicMethods[] = {
        {"native_helloFFmpeg", "()Ljava/lang/String;", (jstring *) helloFFmpeg},
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

