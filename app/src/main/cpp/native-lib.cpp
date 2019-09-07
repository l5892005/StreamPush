#include <jni.h>
#include <string>
#include <faac.h>
#include "x264.h"
#include "librtmp/rtmp.h"
#include "VideoChannel.h"
#include "macro.h"
#include "safe_queue.h"
#include "AudioChannel.h"

VideoChannel *videoChannel;
AudioChannel *audioChannel;
bool isStart;
pthread_t pid;
uint32_t start_time;
int readyPushing = 0;
//队列
SafeQueue<RTMPPacket *> packets;
void callback(RTMPPacket *packet) {
    if (packet) {
        //设置时间戳，需要根据时间戳，来
        packet->m_nTimeStamp = RTMP_GetTime() - start_time;
//        加入队列
        packets.put(packet);
    }
}

void releasePackets(RTMPPacket *&packet) {
    if (packet) {
        RTMPPacket_Free(packet);
        delete packet;
        packet = 0;
    }
}

void *start(void *args) {
    char *url = static_cast<char *>(args);
    RTMP *rtmp = 0;
    // 申请内存
    rtmp = RTMP_Alloc();
    if (!rtmp) {
        LOGE("alloc rtmp失败");
        return NULL;
    }
    // 初始化
    RTMP_Init(rtmp);
    // 设置地址
    int ret = RTMP_SetupURL(rtmp, url);
    if (!ret) {
        LOGE("设置地址失败:%s", url);
        return NULL;
    }
    // 设置超时时间
    rtmp->Link.timeout = 5;
    // 开启输出模式
    RTMP_EnableWrite(rtmp);
    // 链接服务器，内部使用的socket链接
    ret = RTMP_Connect(rtmp, 0);
    if (!ret) {
        LOGE("连接服务器:%s", url);
        return NULL;
    }
    // 链接流
    ret = RTMP_ConnectStream(rtmp, 0);
    if (!ret) {
        LOGE("连接流:%s", url);
        return NULL;
    }
    // 获取推流时间
    start_time = RTMP_GetTime();
    //表示可以开始推流了
    readyPushing = 1;
    // 开始工作
    packets.setWork(1);
    RTMPPacket *packet = 0;
    // 把第一帧的音频编解码信息传给服务器
    callback(audioChannel->getAudioTag());
    while (readyPushing) {
//        队列取数据  pakets
        packets.get(packet);
        LOGE("取出一帧数据");
        if (!readyPushing) {
            break;
        }
        if (!packet) {
            continue;
        }
        packet->m_nInfoField2 = rtmp->m_stream_id;
        // 发送数据包
        ret = RTMP_SendPacket(rtmp, packet, 1);
        //        packet 释放
        releasePackets(packet);
    }

    // 结束直播的时候
    isStart = 0;
    readyPushing = 0;
    packets.setWork(0);
    packets.clear();
    if (rtmp) {
        // 关闭链接
        RTMP_Close(rtmp);
        // 释放链接
        RTMP_Free(rtmp);
    }
    delete (url);
    return 0;
}


extern "C" JNIEXPORT jstring JNICALL
Java_com_superlearn_streampush_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {

    x264_picture_t *x264_picture = new x264_picture_t;
    RTMP_Alloc();

    std::string hello = "Hello from C++";

    return env->NewStringUTF(hello.c_str());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_superlearn_streampush_LivePusher_native_1init(JNIEnv *env, jobject instance) {

    // TODO
    videoChannel = new VideoChannel;
    videoChannel->setVideoCallback(callback);
    audioChannel = new AudioChannel;
    audioChannel->setAudioCallback(callback);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_superlearn_streampush_LivePusher_native_1setVideoEncInfo(JNIEnv *env, jobject instance,
                                                                  jint width, jint height, jint fps,
                                                                  jint bitrate) {
    // TODO
    if (!videoChannel) {
        return;
    }
    videoChannel->setVideoEncInfo(width, height, fps, bitrate);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_superlearn_streampush_LivePusher_native_1start(JNIEnv *env, jobject instance,
                                                        jstring path_) {
    const char *path = env->GetStringUTFChars(path_, 0);

    // TODO
    if (isStart) {
        return;
    }
    isStart = 1;
    char *url = new char[strlen(path) + 1];
    strcpy(url, path);
    // 创建线程，new Thread的start方法，就是调用这个函数
    pthread_create(&pid, 0, start, url);
    env->ReleaseStringUTFChars(path_, path);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_superlearn_streampush_LivePusher_native_1pushVideo(JNIEnv *env, jobject instance,
                                                            jbyteArray data_) {
    // TODO
    if (!videoChannel || !readyPushing) {
        return;
    }
    jbyte *data = env->GetByteArrayElements(data_, NULL);
    // nv21 -- > I420编码
    videoChannel->encodeData(data);
    env->ReleaseByteArrayElements(data_, data, 0);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_superlearn_streampush_LivePusher_native_1pushAudio(JNIEnv *env, jobject instance,
                                                            jbyteArray bytes_) {
    // TODO
    if (!audioChannel || !readyPushing) {
        return;
    }
    jbyte *data = env->GetByteArrayElements(bytes_, NULL);
    // pcm -- > aac编码
    audioChannel->encodeData(data);
    env->ReleaseByteArrayElements(bytes_, data, 0);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_superlearn_streampush_LivePusher_native_1setAudioEncInfo(JNIEnv *env, jobject instance,
                                                                  jint sampleRateInHz,
                                                                  jint channels) {
    // TODO
    if (audioChannel) {
        audioChannel->setAudioEncInfo(sampleRateInHz, channels);
    }
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_superlearn_streampush_LivePusher_getInputSamples(JNIEnv *env, jobject instance) {
    // TODO
    if (audioChannel) {
        return audioChannel->getInputSamples();
    }
    return -1;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_superlearn_streampush_LivePusher_native_1release(JNIEnv *env, jobject instance) {

    DELETE(videoChannel);
    DELETE(audioChannel);

}