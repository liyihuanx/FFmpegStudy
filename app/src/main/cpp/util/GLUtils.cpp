#include "GLUtils.h"


GLuint GLUtils::LoadShader(GLenum shaderType, const char *pSource) {
    GLuint shader = 0;
    FUN_BEGIN_TIME("GLUtils::LoadShader")
    // 创建一个新shader
    shader = glCreateShader(shaderType);
    if (shader) {
        // 加载shader源代码
        glShaderSource(shader, 1, &pSource, NULL);
        // 编译shader
        glCompileShader(shader);
        // 存放编译结果的变量
        GLint compiled = 0;
        // 获取shader编译的结果
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        // 0=false
        if (!compiled) {
            GLint infoLen = 0;
            // 获取错误日志长度？
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char *buf = (char *) malloc((size_t) infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    LOGD("GLUtils::LoadShader Could not compile shader %d:\n%s\n", shaderType, buf);
                    free(buf);
                }
                // 出错则删除
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    FUN_END_TIME("GLUtils::LoadShader")
    return shader;
}

GLuint GLUtils::CreateProgram(const char *pVertexShaderSource, const char *pFragShaderSource,
                              GLuint &vertexShaderHandle, GLuint &fragShaderHandle) {
    GLuint program = 0;
    FUN_BEGIN_TIME("GLUtils::CreateProgram")
    // 加载顶点着色器
    vertexShaderHandle = LoadShader(GL_VERTEX_SHADER, pVertexShaderSource);
    if (!vertexShaderHandle) return program;
    // 加载片元着色器
    fragShaderHandle = LoadShader(GL_FRAGMENT_SHADER, pFragShaderSource);
    if (!fragShaderHandle) return program;

    // 新建一个程序
    program = glCreateProgram();
    if (program) {
        // 若程序创建成功则向程序中加入顶点着色器与片元着色器
        glAttachShader(program, vertexShaderHandle);
        CheckGLError("glAttachShader");
        glAttachShader(program, fragShaderHandle);
        CheckGLError("glAttachShader");

        // 链接程序
        glLinkProgram(program);

        // 获得链接情况
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

        // 从程序移除着色器
        glDetachShader(program, vertexShaderHandle);
        // 删除shader
        glDeleteShader(vertexShaderHandle);
        vertexShaderHandle = 0;

        glDetachShader(program, fragShaderHandle);
        glDeleteShader(fragShaderHandle);
        fragShaderHandle = 0;

        // 链接出错的处理
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char *buf = (char *) malloc((size_t) bufLength);
                if (buf) {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    LOGD("GLUtils::CreateProgram Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    FUN_END_TIME("GLUtils::CreateProgram")
    LOGD("GLUtils::CreateProgram program = %d", program);
    return program;
}

GLuint
GLUtils::CreateProgramWithFeedback(const char *pVertexShaderSource, const char *pFragShaderSource,
                                   GLuint &vertexShaderHandle, GLuint &fragShaderHandle,
                                   GLchar const **varying, int varyingCount) {
    GLuint program = 0;
    FUN_BEGIN_TIME("GLUtils::CreateProgramWithFeedback")
        vertexShaderHandle = LoadShader(GL_VERTEX_SHADER, pVertexShaderSource);
        if (!vertexShaderHandle) return program;

        fragShaderHandle = LoadShader(GL_FRAGMENT_SHADER, pFragShaderSource);
        if (!fragShaderHandle) return program;

        program = glCreateProgram();
        if (program) {
            glAttachShader(program, vertexShaderHandle);
            CheckGLError("glAttachShader");
            glAttachShader(program, fragShaderHandle);
            CheckGLError("glAttachShader");

            //transform feedback
            glTransformFeedbackVaryings(program, varyingCount, varying, GL_INTERLEAVED_ATTRIBS);
            LOGD("CHECK_GL_ERROR %s glGetError = %d, line = %d, ", __FUNCTION__, glGetError(),
                 __LINE__)
            glLinkProgram(program);
            GLint linkStatus = GL_FALSE;
            glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

            glDetachShader(program, vertexShaderHandle);
            glDeleteShader(vertexShaderHandle);
            vertexShaderHandle = 0;
            glDetachShader(program, fragShaderHandle);
            glDeleteShader(fragShaderHandle);
            fragShaderHandle = 0;
            if (linkStatus != GL_TRUE) {
                GLint bufLength = 0;
                glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
                if (bufLength) {
                    char *buf = (char *) malloc((size_t) bufLength);
                    if (buf) {
                        glGetProgramInfoLog(program, bufLength, NULL, buf);
                        LOGD("GLUtils::CreateProgramWithFeedback Could not link program:\n%s\n",
                             buf);
                        free(buf);
                    }
                }
                glDeleteProgram(program);
                program = 0;
            }
        }
    FUN_END_TIME("GLUtils::CreateProgramWithFeedback")
    LOGD("GLUtils::CreateProgramWithFeedback program = %d", program);
    return program;
}

void GLUtils::DeleteProgram(GLuint &program) {
    LOGD("GLUtils::DeleteProgram");
    if (program) {
        glUseProgram(0);
        glDeleteProgram(program);
        program = 0;
    }
}

void GLUtils::CheckGLError(const char *pGLOperation) {
    for (GLint error = glGetError(); error; error = glGetError()) {
        LOGD("GLUtils::CheckGLError GL Operation %s() glError (0x%x)\n", pGLOperation, error);
    }

}

GLuint GLUtils::CreateProgram(const char *pVertexShaderSource, const char *pFragShaderSource) {
    GLuint vertexShaderHandle, fragShaderHandle;
    return CreateProgram(pVertexShaderSource, pFragShaderSource, vertexShaderHandle,
                         fragShaderHandle);
}


// 将glsl着色器文本转换成
string GLUtils::readShaderSource(char *filePath) {
    string content;
    ifstream fileStream(filePath, ios::in);
    string line = "";
    while (!fileStream.eof()) {
        getline(fileStream, line);
        content.append(line + "\n");
    }
    fileStream.close();
    return content;
}

