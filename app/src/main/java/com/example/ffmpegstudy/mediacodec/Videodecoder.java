package com.example.ffmpegstudy.mediacodec;

import android.graphics.Bitmap;
import android.os.Environment;

/**
 * @ClassName: VideoEncode
 * @Description: java类作用描述
 * @Author: liyihuan
 * @Date: 2022/11/23 21:54
 */
class Videodecoder {

    public interface PreviewCallback {
        void info(int width, int height, int fps);

        void getBitmap(Bitmap bitmap);

        void progress(int progress);
    }

    private final String inputVideo = Environment.getExternalStorageDirectory() +"/Atestvideo/test1.mp4";
    private final String outputFile = Environment.getExternalStorageDirectory() +"/Atestvideo/mediacodecout.mp4";

    public void init(){

    }

}
