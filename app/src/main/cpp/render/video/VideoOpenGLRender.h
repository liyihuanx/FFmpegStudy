//
// Created by leeyh on 2022/10/19.
//

#ifndef FFMPEGSTUDY_VIDEOOPENGLRENDER_H
#define FFMPEGSTUDY_VIDEOOPENGLRENDER_H


#include "BaseVideoRender.h"
#include "mutex"
#include "condition_variable"
#include <GLES3/gl3.h>
#include "../../util/GLUtils.h"
#include <glm/vec2.hpp>

#define MATH_PI 3.1415926535897932384626433832802
#define TEXTURE_NUM 3
using namespace glm;

class VideoOpenGLRender : public BaseVideoRender {
public:
    VideoOpenGLRender();

    virtual ~VideoOpenGLRender();

private:
    static VideoOpenGLRender* instance;
    static std::mutex video_gl_mutex;
    NativeImage renderImage;
    int frameIndex;


    // 需要修改的参数
    GLuint m_ProgramObj = GL_NONE;
    GLuint m_TextureIds[TEXTURE_NUM];
    GLuint m_TextureId;
    GLuint m_VaoId;
    GLuint m_VboIds[3];

    vec2 m_TouchXY;
    vec2 m_ScreenSize;

public:
    static VideoOpenGLRender *getInstance();
    static void releaseInstance();
private:
    virtual void onCreate(int videoWidth, int videoHeight, int *dstSize);

    virtual void renderVideoFrame(NativeImage *pImage);

    virtual void onDestroy();

public:
    virtual void onSurfaceCreated();
    virtual void onSurfaceChanged(int w, int h);
    virtual void onDrawFrame();



};


#endif //FFMPEGSTUDY_VIDEOOPENGLRENDER_H
