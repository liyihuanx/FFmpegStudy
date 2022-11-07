package com.example.ffmpegstudy.camera;

import android.graphics.ImageFormat;
import android.graphics.Point;
import android.media.Image;
import android.util.Size;

import java.nio.ByteBuffer;

public class CameraUtil {
    public static Size getFitInScreenSize(int previewWidth, int previewHeight, int screenWidth,
                                          int screenHeight) {
        Point res = new Point(0, 0);
        float ratioPreview = previewWidth *1f / previewHeight;
        float ratioScreen = 0.0f;

        //landscape
        if (screenWidth > screenHeight) {
            ratioScreen = screenWidth * 1f / screenHeight;
            if (ratioPreview >= ratioScreen) {
                res.x = screenWidth;
                res.y = (int)(res.x * previewHeight * 1f / previewWidth);
            }else {
                res.y = screenHeight;
                res.x = (int)(res.y * previewWidth * 1f / previewHeight);
            }
        //portrait
        }else {
            ratioScreen = screenHeight * 1f / screenWidth;
            if (ratioPreview >= ratioScreen) {
                res.y = screenHeight;
                res.x = (int)(res.y * previewHeight * 1f / previewWidth);
            }else {
                res.x = screenWidth;
                res.y = (int)(res.x * previewWidth * 1f / previewHeight);
            }
        }
        return new Size(res.x, res.y);
    }

    /**
     * 将NV21转换为YUV420P
     * @param image
     * @return
     */
    public static byte[] YUV_420_888_data(Image image) {
        final int imageWidth = image.getWidth();
        final int imageHeight = image.getHeight();
        // 原始数据，planes.length = 3
        final Image.Plane[] planes = image.getPlanes();
        // 此处用来装填最终的YUV数据，需要1.5倍的图片大小，因为Y U V 比例为 4:1:1
        byte[] data = new byte[imageWidth * imageHeight *
                ImageFormat.getBitsPerPixel(ImageFormat.YUV_420_888) / 8];
        int offset = 0;

        // plane[0] = Y
        for (int plane = 0; plane < planes.length; ++plane) {
            final ByteBuffer buffer = planes[plane].getBuffer();
            final int rowStride = planes[plane].getRowStride();
            // U and V planes have pixelStride(像素步长) = 2,则说明是packed方式，需要一个个拷贝
            // Y 的pixelStride(像素步长) = 1，可以从头到尾一下拷贝
            final int pixelStride = planes[plane].getPixelStride();
            // Y-Width = imageWidth, U,V-Width = imageWidth / 2
            final int planeWidth = (plane == 0) ? imageWidth : imageWidth / 2;
            final int planeHeight = (plane == 0) ? imageHeight : imageHeight / 2;
            if (pixelStride == 1 && rowStride == planeWidth) {
                // Copy whole plane from buffer into |data| at once.
                buffer.get(data, offset, planeWidth * planeHeight);
                offset += planeWidth * planeHeight;
            } else {
                // Copy pixels one by one respecting pixelStride and rowStride.
                byte[] rowData = new byte[rowStride];
                for (int row = 0; row < planeHeight - 1; ++row) {
                    buffer.get(rowData, 0, rowStride);
                    for (int col = 0; col < planeWidth; ++col) {
                        data[offset++] = rowData[col * pixelStride];
                    }
                }
                // Last row is special in some devices and may not contain the full
                // |rowStride| bytes of data.
                // See http://developer.android.com/reference/android/media/Image.Plane.html#getBuffer()
                buffer.get(rowData, 0, Math.min(rowStride, buffer.remaining()));
                for (int col = 0; col < planeWidth; ++col) {
                    data[offset++] = rowData[col * pixelStride];
                }
            }
        }

        return data;
    }
}
