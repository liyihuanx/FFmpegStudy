cmake_minimum_required(VERSION 3.10.2)

project("ffmpegstudy")

# 批量导入
file(GLOB AllCPP
        ${CMAKE_SOURCE_DIR}/cpp/*.c
        ${CMAKE_SOURCE_DIR}/cpp/*.h
        ${CMAKE_SOURCE_DIR}/cpp/*.cpp
        ${CMAKE_SOURCE_DIR}/cpp/*/*/*.h
        ${CMAKE_SOURCE_DIR}/cpp/*/*/*.cpp
        ${CMAKE_SOURCE_DIR}/cpp/*/*.h
        ${CMAKE_SOURCE_DIR}/cpp/*/*.cpp
        )

add_library(
        ffmpegstudy
        SHARED
        ${AllCPP})


find_library(
        log-lib
        log)


# 第一步：导入头文件
include_directories("${CMAKE_SOURCE_DIR}/../../libs/${CMAKE_ANDROID_ARCH_ABI}/include")
# 第二步：导入库文件 （方式一）
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/../../libs/${CMAKE_ANDROID_ARCH_ABI}")


# 静态库导入
#add_library(getndk STATIC IMPORTED)
#set_target_properties(getndk PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/libgetndk.a)


message(WARNING "CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")
message(WARNING "PROJECT_SOURCE_DIR = ${PROJECT_SOURCE_DIR}")
message(WARNING "CMAKE_BINARY_DIR = ${CMAKE_BINARY_DIR}")
message(WARNING "path = ${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/../../libs/${CMAKE_ANDROID_ARCH_ABI}/lib")


target_link_libraries(
        ffmpegstudy
        ${log-lib}

        x264
        fdk-aac

        # 忽略顺序的方式，导入
        -Wl,--start-group
        avformat avcodec avfilter avutil swresample swscale
        -Wl,--end-group

        z # libz.so库，是FFmpeg需要用ndk的z库，FFMpeg需要额外支持  libz.so
        android # ANativeWindow 用来渲染画面的
        OpenSLES # OpenSLES 用来播放声音的

        EGL
        GLESv3
)