package com.example.ffmpegstudy;

import android.content.Context;
import android.os.FileUtils;
import android.text.TextUtils;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/**
 * @Author yocn
 * @Date 2019-11-07 17:23
 * @ClassName Constant
 */
public class Constant {

    private static String PATH_YUV = "/yuv";
    private static String PATH_MEDIA_CODEC = "/media_study";

    public static String getRootPath() {
        return BaseApplication.getAppContext().getExternalFilesDir(null).getAbsolutePath();
    }

    public static String getMediaCodecDir() {
        String path = getRootPath() + PATH_MEDIA_CODEC;
        checkDir(path);
        return path;
    }

    public static String getCacheYuvDir() {
        String path = getRootPath() + PATH_YUV;
        checkDir(path);
        return path;
    }

    public static String getTestYuvFilePath() {
        String path = getRootPath() + PATH_YUV;
        checkDir(path);
        String yuvFile = path + "/test.yuv";
        return yuvFile;
    }

    public static String getTestMp4FilePath() {
        return getMediaCodecDir() + "/test.mp4";
    }

    public static String getTestMp4FilePath2() {
        return getMediaCodecDir() + "/test2.mp4";
    }

    public static String getTestMp3FilePath() {
        return getMediaCodecDir() + "/ring.mp3";
    }

    public static String getTestMp3FilePath2() {
        return getMediaCodecDir() + "/test2.mp3";
    }

    public static String getOutTestYuvFilePath() {
        return getMediaCodecDir() + "/test.yuv";
    }

    public static String getTestFilePath(String fileName) {
        return getMediaCodecDir() + "/" + fileName;
    }

    /**
     * 检查目录是否存在，如果不存在创建之
     *
     * @return 目录是否存在
     */
    public static boolean checkDir(String dirPath) {
        if (TextUtils.isEmpty(dirPath))
            return false;
        File dir = new File(dirPath);
        if (dir.exists() && dir.isDirectory())
            return true;
        if (dir.exists())
            dir.delete();
        return dir.mkdirs();
    }


    public static void copyAssets() {
        Context context = BaseApplication.getAppContext();
        Set<String> ignoreSet = new HashSet<>();
        ignoreSet.add("images");
        ignoreSet.add("sound");
        ignoreSet.add("webkit");
        List<String> needCopy = new ArrayList<>();
        try {
            String[] list = context.getAssets().list("");
            for (String ss : list) {
                if (!ignoreSet.contains(ss) && !isFilePathExist(Constant.getTestFilePath(ss))) {
                    needCopy.add(ss);
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        new Thread(() -> {
            for (String name : needCopy) {
                copyAssetsFile2Phone(context, name, Constant.getTestFilePath(name));
            }
        }).start();
    }

    public static boolean isFilePathExist(String pathString) { // path is exists or not
        if (isEmpty(pathString)) {
            return false;
        }
        return new File(pathString).exists();
    }

    public static boolean isEmpty(String str) {
        String string = str;
        if (string == null) {
            return true;
        }
        string = string.trim();
        return TextUtils.isEmpty(string) || str.equalsIgnoreCase("null");
    }


    public static boolean copyAssetsFile2Phone(Context context, String fileName, String targetPath) {
        try {
            InputStream inputStream = context.getAssets().open(fileName);
            //getFilesDir() 获得当前APP的安装路径 /data/data/包名/files 目录
            File file = new File(targetPath);
            if (!file.exists() || file.length() == 0) {
                FileOutputStream fos = new FileOutputStream(file);
                int len = -1;
                byte[] buffer = new byte[1024];
                while ((len = inputStream.read(buffer)) != -1) {
                    fos.write(buffer, 0, len);
                }
                fos.flush();//刷新缓存区
                inputStream.close();
                fos.close();
            } else {
                return true;
            }
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }
        return true;
    }
}
