package com.superlearn.streampush;

import android.app.Activity;
import android.view.SurfaceHolder;

import com.superlearn.streampush.meida.AudioChannel;
import com.superlearn.streampush.meida.VideoChannel;


public class LivePusher {
    private AudioChannel audioChannel;
    private VideoChannel videoChannel;
    static {
        System.loadLibrary("native-lib");
    }

    public LivePusher(Activity activity, int width, int height, int bitrate,
                      int fps, int cameraId) {
        native_init();
        videoChannel = new VideoChannel(this, activity, width, height, bitrate, fps, cameraId);
        audioChannel = new AudioChannel(this);


    }
    public void setPreviewDisplay(SurfaceHolder surfaceHolder) {
        videoChannel.setPreviewDisplay(surfaceHolder);
    }
    public void switchCamera() {
        videoChannel.switchCamera();
    }

    public void startLive(String path) {
        native_start(path);
        videoChannel.startLive();
        audioChannel.startLive();
    }
    // 初始化x264
    public native void native_init();
    public native void native_setVideoEncInfo(int width, int height, int fps, int bitrate);
    // 开始推流，传url，子线程运行到阻塞队列，等待传入推流数据
    public native void native_start(String path);
    // 传入推流数据
    public native void native_pushVideo(byte[] data);

    // 推入音频流
    public native void native_pushAudio(byte[] bytes);

    public native int getInputSamples();

    public native void native_setAudioEncInfo(int i, int channels);

    public native void native_release();
}
