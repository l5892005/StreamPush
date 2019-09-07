package com.superlearn.streampush.meida;


import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;

import com.superlearn.streampush.LivePusher;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledThreadPoolExecutor;


public class AudioChannel {
    private LivePusher mLivePusher;
    private AudioRecord audioRecord;
    private int channels = 2;
    private int channelConfig;

    private int inputSamples;
    int minBufferSize;
    private ExecutorService executor;
    private boolean isLiving;

    public AudioChannel(LivePusher livePusher) {
        mLivePusher = livePusher;
        executor = Executors.newSingleThreadExecutor();
        if (channels == 2) {
            // 双通道
            channelConfig = AudioFormat.CHANNEL_IN_STEREO;
        } else {
            // 单通道
            channelConfig = AudioFormat.CHANNEL_IN_MONO;
        }

        mLivePusher.native_setAudioEncInfo(44100, channels);
        //16 位 2个字节
        inputSamples = mLivePusher.getInputSamples() * 2;
//        minBufferSize
        minBufferSize = AudioRecord.getMinBufferSize(44100,
                channelConfig, AudioFormat.ENCODING_PCM_16BIT) * 2;
        audioRecord = new AudioRecord(MediaRecorder.AudioSource.MIC, 44100, channelConfig,
                AudioFormat.ENCODING_PCM_16BIT, minBufferSize < inputSamples ? inputSamples : minBufferSize
        );

    }

    public void startLive() {
        isLiving = true;
        executor.submit(new AudioTask());
    }

    public void setChannels(int channels) {
        this.channels = channels;
    }

    public void release() {
        audioRecord.release();
    }

    class AudioTask implements Runnable {

        @Override
        public void run() {
            audioRecord.startRecording();
//    pcm  音频原始数据
            byte[] bytes = new byte[inputSamples];
            while (isLiving) {
                // 麦克风读pcm数据，需要单独开线程
                int len = audioRecord.read(bytes, 0, bytes.length);
                mLivePusher.native_pushAudio(bytes);
            }
        }
    }

}
