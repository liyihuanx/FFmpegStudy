//
// Created by leeyh on 2022/11/7.
//

#include "GLCameraRender.h"


GLCameraRender *GLCameraRender::instance = nullptr;
std::mutex GLCameraRender::camera_gl_mutex;


GLCameraRender::GLCameraRender(): BaseVideoRender(VIDEO_RENDER_OPENGL) {

}

GLCameraRender::~GLCameraRender() {
    NativeImageUtil::FreeNativeImage(rend)
}


GLCameraRender *GLCameraRender::getInstance() {
    if (instance == nullptr) {
        std::lock_guard<std::mutex> lock(camera_gl_mutex);
        if (instance == nullptr) {
            instance = new GLCameraRender();
        }
    }
    return instance;
}

void GLCameraRender::releaseInstance() {
    if (instance != nullptr) {
        std::lock_guard<std::mutex> lock(camera_gl_mutex);
        if (instance != nullptr) {
            delete instance;
            instance = nullptr;
        }

    }
}

void GLCameraRender::onCreate(int videoWidth, int videoHeight, int *dstSize) {

}

void GLCameraRender::renderVideoFrame(NativeImage *pImage) {
    LOGD("GLCameraRender::renderVideoFrame")
}

void GLCameraRender::onDestroy() {

}

void GLCameraRender::onSurfaceCreated() {

}

void GLCameraRender::onSurfaceChanged(int w, int h) {

}

void GLCameraRender::onDrawFrame() {

}


