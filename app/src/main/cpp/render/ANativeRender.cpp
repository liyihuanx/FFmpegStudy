//
// Created by leeyh on 2022/9/27.
//

#include "ANativeRender.h"

ANativeRender::ANativeRender(JNIEnv *env, jobject surface) : BaseVideoRender(VIDEO_RENDER_ANWINDOW) {
    nativeWindow = ANativeWindow_fromSurface(env, surface);

}

ANativeRender::~ANativeRender() {

    if (nativeWindow) {
        ANativeWindow_release(nativeWindow);
    }
}

void ANativeRender::onCreate(int videoWidth, int videoHeight, int *dstSize) {
    ANativeWindow_setBuffersGeometry(nativeWindow, videoWidth,
                                     videoHeight, WINDOW_FORMAT_RGBA_8888);
}

void ANativeRender::renderVideoFrame(NativeImage *pImage) {
//    LOGD("ANativeRender::renderVideoFrame")
    ANativeWindow_lock(nativeWindow, &nativeWindowBuffer, nullptr);

    uint8_t *dst = static_cast<uint8_t *>(nativeWindowBuffer.bits);
    // 6.3 输入图的步长，一行像素有多少个字节
    int dstLineSize = nativeWindowBuffer.stride * 4;

    // 6.4 目标
    uint8_t *src = pImage->ppPlane[0];
    int srcLineSize = pImage->pLineSize[0];

    // 6.5 一行行拷贝
    for (int i = 0; i < pImage->height; ++i) {
        // 源数据 ---> 目标数据（字节对齐）
        // 第一个像素开始，每次复制srcLineSize这么多
        memcpy(dst + i * dstLineSize,
               src + i * srcLineSize,
               srcLineSize);
    }

    ANativeWindow_unlockAndPost(nativeWindow);
}

void ANativeRender::onDestroy() {

}