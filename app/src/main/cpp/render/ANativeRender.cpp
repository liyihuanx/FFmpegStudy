//
// Created by leeyh on 2022/9/27.
//

#include "ANativeRender.h"

ANativeRender::ANativeRender(JNIEnv *env, jobject surface) : BaseRender(VIDEO_RENDER_ANWINDOW) {

}

ANativeRender::~ANativeRender() {

}

void ANativeRender::onCreate(int videoWidth, int videoHeight, int *dstSize) {

}

void ANativeRender::renderVideoFrame(NativeImage *pImage) {

}

void ANativeRender::onDestroy() {

}
