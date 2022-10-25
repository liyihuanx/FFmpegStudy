//
// Created by leeyh on 2022/10/19.
//


#include "VideoOpenGLRender.h"

VideoOpenGLRender *VideoOpenGLRender::instance = nullptr;
std::mutex VideoOpenGLRender::video_gl_mutex;

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

// opengl坐标系，屏幕中点为(0,0)
GLfloat verticesCoords[] = {
        -1.0f, 1.0f, 0.0f,  // Position 0
        -1.0f, -1.0f, 0.0f,  // Position 1
        1.0f, -1.0f, 0.0f,  // Position 2
        1.0f, 1.0f, 0.0f,  // Position 3
};

// 纹理坐标系，和android坐标系0.5y轴对称
GLfloat textureCoords[] = {
        0.0f, 0.0f,        // TexCoord 0
        0.0f, 1.0f,        // TexCoord 1
        1.0f, 1.0f,        // TexCoord 2
        1.0f, 0.0f         // TexCoord 3
};

GLushort indices[] = {
        0, 1, 2,  // first triangle
        0, 2, 3   // second triangle
};

VideoOpenGLRender::VideoOpenGLRender() : BaseVideoRender(VIDEO_RENDER_OPENGL) {

}

VideoOpenGLRender::~VideoOpenGLRender() {
    // 释放缓存图像
    NativeImageUtil::FreeNativeImage(&renderImage);
}

void VideoOpenGLRender::onCreate(int videoWidth, int videoHeight, int *dstSize) {
    // [544,960]
    LOGD("OpenGLRender::onCreate video[w, h]=[%d, %d]", videoWidth, videoHeight);
    if (dstSize != nullptr) {
        dstSize[0] = videoWidth;
        dstSize[1] = videoHeight;
    }
    frameIndex = 0;
}

void VideoOpenGLRender::renderVideoFrame(NativeImage *pImage) {
    if (pImage == nullptr || pImage->ppPlane[0] == nullptr)
        return;
    // 加互斥锁，解码线程和渲染线程是 2 个不同的线程，避免数据访问冲突
    std::unique_lock<std::mutex> lock(video_gl_mutex);
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

void VideoOpenGLRender::onDestroy() {

}

// 对应GLSurfaceView的三个方法
void VideoOpenGLRender::onSurfaceCreated() {
    LOGD("VideoGLRender::OnSurfaceCreated");

    m_ProgramObj = GLUtils::CreateProgram(vShaderStr, fShaderStr);
    if (!m_ProgramObj) {
        LOGD("VideoGLRender::OnSurfaceCreated create program fail");
        return;
    }
    // 创建纹理
    glGenTextures(TEXTURE_NUM, m_TextureIds);
    for (int i = 0; i < TEXTURE_NUM; ++i) {
        // 激活纹理单元
        glActiveTexture(GL_TEXTURE0 + i);
        // 绑定纹理
        glBindTexture(GL_TEXTURE_2D, m_TextureIds[i]);
        // 纹理配置：GL_TEXTURE_WRAP_S（横坐标）,GL_TEXTURE_WRAP_T（纵坐标）
        // GL_CLAMP_TO_EDGE：采样纹理边缘，即剩余部分显示纹理临近的边缘颜色值
        // GL_REPEAT：重复纹理
        // GL_MIRRORED_REPEAT:镜像重复
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // 纹理过滤配置
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // 解绑
        glBindTexture(GL_TEXTURE_2D, GL_NONE);
    }

    // 创建VBO专门缓冲顶点数据，将创建好的vbo的id存放在VBOs数组中
    glGenBuffers(3, m_VboIds);
    // 此时上下文绑定VBOs[0]对应的vbo缓冲
    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
    // 将顶点数据存入vbo的缓冲区中
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCoords), verticesCoords, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoords), textureCoords, GL_STATIC_DRAW);

    // 绑定EBO缓冲对象,专门缓冲顶点索引数据
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[2]);
    // 给EBO缓冲对象传入索引数据
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 创建VAO,将VBO或者EBO等缓冲的配置操作缓存起来
    glGenVertexArrays(1, &m_VaoId);
    // 绑定VAO,接下来对VBO的操作都会缓存到这
    glBindVertexArray(m_VaoId);

    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
    // 打开着色器中layout为0的输入变量
    glEnableVertexAttribArray(0);
    // 指定如何解析顶点属性数组，注意这里最后一个参数传的不是原数组地址，而是数据再vbo缓冲区中的相对地址
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (const void *) 0);
    // 解绑
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (const void *) 0);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[2]);
    glBindVertexArray(GL_NONE);

    m_TouchXY = vec2(0.5f, 0.5f);
}

void VideoOpenGLRender::onSurfaceChanged(int w, int h) {
    LOGD("OpenGLRender::OnSurfaceChanged [w, h]=[%d, %d]", w, h);
    m_ScreenSize.x = w;
    m_ScreenSize.y = h;
    glViewport(0, 0, w, h);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

}

void VideoOpenGLRender::onDrawFrame() {
    glClear(GL_COLOR_BUFFER_BIT);
    if (m_ProgramObj == GL_NONE || renderImage.ppPlane[0] == nullptr) {
        LOGD("VideoOpenGLRender::OnDrawFrame return!")
        return;
    }
    LOGD("VideoOpenGLRender::OnDrawFrame [w, h]=[%d, %d], format=%d", renderImage.width,
         renderImage.height, renderImage.format);
    frameIndex++;

    // upload image data
    std::unique_lock<std::mutex> lock(video_gl_mutex);
    switch (renderImage.format) {
        case IMAGE_FORMAT_RGBA:
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_TextureIds[0]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, renderImage.width, renderImage.height, 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, renderImage.ppPlane[0]);
            glBindTexture(GL_TEXTURE_2D, GL_NONE);
            break;
        case IMAGE_FORMAT_NV21:
        case IMAGE_FORMAT_NV12:
            //upload Y plane data
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_TextureIds[0]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, renderImage.width,
                         renderImage.height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                         renderImage.ppPlane[0]);
            glBindTexture(GL_TEXTURE_2D, GL_NONE);

            //update UV plane data
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, m_TextureIds[1]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, renderImage.width >> 1,
                         renderImage.height >> 1, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE,
                         renderImage.ppPlane[1]);
            glBindTexture(GL_TEXTURE_2D, GL_NONE);
            break;
        case IMAGE_FORMAT_I420:
            //upload Y plane data
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_TextureIds[0]);
            glTexImage2D(GL_TEXTURE_2D,
                         0,             // 指定要Mipmap的等级
                         GL_LUMINANCE,       // gpu内部格式，告诉OpenGL内部用什么格式存储和使用这个纹理数据
                         renderImage.width,  // 加载的纹理宽度。最好为2的次幂
                         renderImage.height, // 加载的纹理高度。最好为2的次幂
                         0,           // 纹理边框
                         GL_LUMINANCE,       // 数据的像素格式 亮度，灰度图
                         GL_UNSIGNED_BYTE,   // 一个像素点存储的数据类型
                         renderImage.ppPlane[0] // 纹理的数据
            );
            glBindTexture(GL_TEXTURE_2D, GL_NONE);

            //update U plane data
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, m_TextureIds[1]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, renderImage.width >> 1,
                         renderImage.height >> 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                         renderImage.ppPlane[1]);
            glBindTexture(GL_TEXTURE_2D, GL_NONE);

            //update V plane data
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, m_TextureIds[2]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, renderImage.width >> 1,
                         renderImage.height >> 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                         renderImage.ppPlane[2]);
            glBindTexture(GL_TEXTURE_2D, GL_NONE);
            break;
        default:
            break;
    }
    lock.unlock();


    // Use the program object
    glUseProgram(m_ProgramObj);

    glBindVertexArray(m_VaoId);

//    GLUtils::setMat4(m_ProgramObj, "u_MVPMatrix", m_MVPMatrix);

    for (int i = 0; i < TEXTURE_NUM; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_TextureIds[i]);
        char samplerName[64] = {0};
        sprintf(samplerName, "s_texture%d", i);
        GLUtils::setInt(m_ProgramObj, samplerName, i);
    }


    float offset = (sin(frameIndex * MATH_PI / 40) + 1.0f) / 2.0f;
    GLUtils::setFloat(m_ProgramObj, "u_Offset", offset);
    GLUtils::setVec2(m_ProgramObj, "u_TexSize", vec2(renderImage.width, renderImage.height));
    GLUtils::setInt(m_ProgramObj, "u_nImgType", renderImage.format);

    // 从indices中按顺序取出索引对应6个顶点依次进行绘制，图元类型为GL_TRIANGLES
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *) 0);

}

VideoOpenGLRender *VideoOpenGLRender::getInstance() {
    if (instance == nullptr) {
        std::lock_guard<std::mutex> lock(video_gl_mutex);
        if (instance == nullptr) {
            instance = new VideoOpenGLRender();
        }
    }
    return instance;
}

void VideoOpenGLRender::releaseInstance() {
    if (instance != nullptr) {
        std::lock_guard<std::mutex> lock(video_gl_mutex);
        if (instance != nullptr) {
            delete instance;
            instance = nullptr;
        }

    }
}
