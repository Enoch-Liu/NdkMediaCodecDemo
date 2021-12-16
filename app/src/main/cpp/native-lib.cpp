#include <jni.h>
#include <string>
#include "DecodeDemo.h"

extern "C" JNIEXPORT jstring JNICALL
Java_com_enoch_ndkmediacodecdemo_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_enoch_ndkmediacodecdemo_MainActivity_autoplay(JNIEnv* env, jobject) {
    DecodeDemo demo;
    if (demo.AutoPlay()) {
        return JNI_TRUE;
    } else {
        return JNI_FALSE;
    }
}
static DecodeDemo* decoder = nullptr;
extern "C" JNIEXPORT jboolean JNICALL
Java_com_enoch_ndkmediacodecdemo_MainActivity_init(JNIEnv* env, jobject) {
    decoder = new DecodeDemo();
    if (decoder->Init()) {
        return JNI_TRUE;
    } else {
        delete decoder;
        decoder = nullptr;
        return JNI_FALSE;
    }
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_enoch_ndkmediacodecdemo_MainActivity_seek(JNIEnv* env, jobject, jint sec) {
    if (decoder && decoder->Seek(sec)) {
        return JNI_TRUE;
    } else {
        delete decoder;
        decoder = nullptr;
        return JNI_FALSE;
    }
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_enoch_ndkmediacodecdemo_MainActivity_play(JNIEnv* env, jobject, jint sec) {
    if (decoder && decoder->Play(sec)) {
        return JNI_TRUE;
    } else {
        delete decoder;
        decoder = nullptr;
        return JNI_FALSE;
    }
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_enoch_ndkmediacodecdemo_MainActivity_release(JNIEnv* env, jobject) {
    if (decoder && decoder->Release()) {
        delete decoder;
        decoder = nullptr;
        return JNI_TRUE;
    } else {
        delete decoder;
        decoder = nullptr;
        return JNI_FALSE;
    }
}