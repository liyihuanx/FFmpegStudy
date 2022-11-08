//
// Created by leeyh on 2022/11/7.
//

#include "GLCameraRender.h"


static char vShaderStr[] =
        "#version 300 es\n"
        "layout(location = 0) in vec4 a_position;\n"
        "layout(location = 1) in vec2 a_texCoord;\n"
        "out vec2 v_texCoord;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = a_position;\n"
        "    v_texCoord = a_texCoord;\n"
        "}";

static char fShaderStr[] =
        "#version 300 es\n"
        "precision highp float;\n"
        "in vec2 v_texCoord;\n"
        "layout(location = 0) out vec4 outColor;\n"
        "uniform sampler2D s_texture0;\n"
        "uniform sampler2D s_texture1;\n"
        "uniform sampler2D s_texture2;\n"
        "uniform int u_nImgType;// 1:RGBA, 2:NV21, 3:NV12, 4:I420\n"
        "\n"
        "void main()\n"
        "{\n"
        "\n"
        "    if(u_nImgType == 1) //RGBA\n"
        "    {\n"
        "        outColor = texture(s_texture0, v_texCoord);\n"
        "    }\n"
        "    else if(u_nImgType == 2) //NV21\n"
        "    {\n"
        "        vec3 yuv;\n"
        "        yuv.x = texture(s_texture0, v_texCoord).r;\n"
        "        yuv.y = texture(s_texture1, v_texCoord).a - 0.5;\n"
        "        yuv.z = texture(s_texture1, v_texCoord).r - 0.5;\n"
        "        highp vec3 rgb = mat3(1.0,       1.0,     1.0,\n"
        "        0.0, \t-0.344, \t1.770,\n"
        "        1.403,  -0.714,     0.0) * yuv;\n"
        "        outColor = vec4(rgb, 1.0);\n"
        "\n"
        "    }\n"
        "    else if(u_nImgType == 3) //NV12\n"
        "    {\n"
        "        vec3 yuv;\n"
        "        yuv.x = texture(s_texture0, v_texCoord).r;\n"
        "        yuv.y = texture(s_texture1, v_texCoord).r - 0.5;\n"
        "        yuv.z = texture(s_texture1, v_texCoord).a - 0.5;\n"
        "        highp vec3 rgb = mat3(1.0,       1.0,     1.0,\n"
        "        0.0, \t-0.344, \t1.770,\n"
        "        1.403,  -0.714,     0.0) * yuv;\n"
        "        outColor = vec4(rgb, 1.0);\n"
        "    }\n"
        "    else if(u_nImgType == 4) //I420\n"
        "    {\n"
        "        vec3 yuv;\n"
        "        yuv.x = texture(s_texture0, v_texCoord).r;\n"
        "        yuv.y = texture(s_texture1, v_texCoord).r - 0.5;\n"
        "        yuv.z = texture(s_texture2, v_texCoord).r - 0.5;\n"
        "        highp vec3 rgb = mat3(1.0,       1.0,     1.0,\n"
        "                              0.0, \t-0.344, \t1.770,\n"
        "                              1.403,  -0.714,     0.0) * yuv;\n"
        "        outColor = vec4(rgb, 1.0);\n"
        "    }\n"
        "    else\n"
        "    {\n"
        "        outColor = vec4(1.0);\n"
        "    }\n"
        "}";

static GLfloat verticesCoords[] = {
        -1.0f, 1.0f, 0.0f,  // Position 0
        -1.0f, -1.0f, 0.0f,  // Position 1
        1.0f, -1.0f, 0.0f,  // Position 2
        1.0f, 1.0f, 0.0f,  // Position 3
};

//static GLfloat textureCoords[] = {
//        0.0f,  1.0f,        // TexCoord 0
//        0.0f,  0.0f,        // TexCoord 1
//        1.0f,  0.0f,        // TexCoord 2
//        1.0f,  1.0f         // TexCoord 3
//};
static GLfloat textureCoords[] = {
        0.0f, 0.0f,        // TexCoord 0
        0.0f, 1.0f,        // TexCoord 1
        1.0f, 1.0f,        // TexCoord 2
        1.0f, 0.0f         // TexCoord 3
};

static GLushort indices[] = { 0, 1, 2, 0, 2, 3 };


GLCameraRender *GLCameraRender::instance = nullptr;
std::mutex GLCameraRender::camera_gl_mutex;


GLCameraRender::GLCameraRender() : BaseVideoRender(VIDEO_RENDER_OPENGL) {

}

GLCameraRender::~GLCameraRender() {
    NativeImageUtil::FreeNativeImage(&renderImage)
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
    if (dstSize != nullptr) {
        dstSize[0] = videoWidth;
        dstSize[1] = videoHeight;
    }
    frameIndex = 0;
}

void GLCameraRender::onDestroy() {

}

void GLCameraRender::renderVideoFrame(NativeImage *pImage) {
    if (pImage == nullptr || pImage->ppPlane[0] == nullptr)
        return;
    // 加互斥锁，解码线程和渲染线程是 2 个不同的线程，避免数据访问冲突
    std::unique_lock<std::mutex> lock(camera_gl_mutex);
    // 解码到第一帧，初始化renderImage
    if (pImage->width != renderImage.width || pImage->height != renderImage.height) {
        if (renderImage.ppPlane[0] != nullptr) {
            NativeImageUtil::FreeNativeImage(&renderImage);
        }
        memset(&renderImage, 0, sizeof(NativeImage));
        // 赋值frame的一些常规变量
        renderImage.format = pImage->format;
        renderImage.width = pImage->width;
        renderImage.height = pImage->height;
        // 根据format类型，创建存放容器
        NativeImageUtil::AllocNativeImage(&renderImage);
    }
    // 将数据复制到容器中
    NativeImageUtil::CopyNativeImage(pImage, &renderImage);
}


void GLCameraRender::onSurfaceCreated() {

    m_ProgramObj = GLUtils::CreateProgram(vShaderStr, fShaderStr);
    m_FboProgramObj = GLUtils::CreateProgram(vShaderStr, fShaderStr);
    if (!m_ProgramObj || !m_FboProgramObj) {
        LOGD("GLCameraRender::OnSurfaceCreated create program fail");
        return;
    }

    // 创建纹理
    glGenTextures(TEXTURE_NUM, m_TextureIds);
    for (int i = 0; i < TEXTURE_NUM; ++i) {
        // 激活纹理
        glActiveTexture(GL_TEXTURE0 + i);
        // 绑定
        glBindTexture(GL_TEXTURE_2D, m_TextureIds[i]);

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // 解除绑定
        glBindTexture(GL_TEXTURE_2D, GL_NONE);

    }

    // VBO
    glGenBuffers(3, m_VboIds);
    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCoords), verticesCoords, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCoords), verticesCoords, GL_STATIC_DRAW);

    // e
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // vao

}

void GLCameraRender::onSurfaceChanged(int w, int h) {

}

void GLCameraRender::onDrawFrame() {

}


