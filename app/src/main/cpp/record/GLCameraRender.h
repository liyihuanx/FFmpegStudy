//
// Created by leeyh on 2022/11/7.
//

#ifndef FFMPEGSTUDY_GLCAMERARENDER_H
#define FFMPEGSTUDY_GLCAMERARENDER_H

#define TEXTURE_NUM 3
#define MATH_PI 3.1415926535897932384626433832802

#include "../render/video/BaseVideoRender.h"
#include "mutex"
#include "condition_variable"
#include <GLES3/gl3.h>
#include "../util/GLUtils.h"
#include <glm/vec2.hpp>
#include <glm/detail/type_mat.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;
using namespace std;

struct TransformMatrix {
    int degree;
    int mirror;
    float translateX;
    float translateY;
    float scaleX;
    float scaleY;
    int angleX;
    int angleY;

    TransformMatrix():
            translateX(0),
            translateY(0),
            scaleX(1.0),
            scaleY(1.0),
            degree(0),
            mirror(0),
            angleX(0),
            angleY(0)
    {

    }
    void Reset()
    {
        translateX = 0;
        translateY = 0;
        scaleX = 1.0;
        scaleY = 1.0;
        degree = 0;
        mirror = 0;

    }
};

typedef void (*OnRenderFrameCallback)(void*, NativeImage*);

class GLCameraRender : public BaseVideoRender {

public:
    static GLCameraRender *getInstance();
    static void releaseInstance();

    GLCameraRender();
    ~GLCameraRender();

    virtual void onCreate(int videoWidth, int videoHeight, int *dstSize);

    virtual void renderVideoFrame(NativeImage *pImage);

    virtual void onDestroy();

    virtual void onSurfaceCreated();
    virtual void onSurfaceChanged(int w, int h);
    virtual void onDrawFrame();

    // 更新变换矩阵，Camera预览帧需要进行旋转
    virtual void UpdateMVPMatrix(int angleX, int angleY, float scaleX, float scaleY);

    virtual void UpdateMVPMatrix(TransformMatrix * pTransformMatrix);

    bool CreateFrameBufferObj();

    void GetRenderFrameFromFBO();

    OnRenderFrameCallback m_RenderFrameCallback = nullptr;
    void *m_CallbackContext = nullptr;

    virtual void SetTouchLoc(float touchX, float touchY) {
        m_TouchXY.x = touchX / m_ScreenSize.x;
        m_TouchXY.y = touchY / m_ScreenSize.y;
    }

    //添加好滤镜之后，视频帧的回调，然后将带有滤镜的视频帧放入编码队列
    void SetRenderCallback(void *ctx, OnRenderFrameCallback callback) {
        m_CallbackContext = ctx;
        m_RenderFrameCallback = callback;
    }

public:
    static GLCameraRender* instance;
    static std::mutex camera_gl_mutex;
    NativeImage renderImage;
    int frameIndex;

    // opengl
    GLuint m_ProgramObj = GL_NONE;
    GLuint m_FboProgramObj = GL_NONE;

    GLuint m_TextureIds[TEXTURE_NUM];
    GLuint m_VaoId;
    GLuint m_VboIds[3];

    vec2 m_TouchXY;
    vec2 m_ScreenSize;

    GLuint m_SrcFboTextureId = GL_NONE;
    GLuint m_SrcFboId = GL_NONE;
    GLuint m_DstFboTextureId = GL_NONE;
    GLuint m_DstFboId = GL_NONE;

    glm::mat4 m_MVPMatrix;
    TransformMatrix m_transformMatrix;
};


#endif //FFMPEGSTUDY_GLCAMERARENDER_H
