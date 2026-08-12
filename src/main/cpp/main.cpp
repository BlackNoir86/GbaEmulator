#include <jni.h>
#include <vector>
#include <android/log.h>

#define LOG_TAG "GBA_CORE"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

std::vector<uint8_t> romBuffer;

extern "C" JNIEXPORT jboolean JNICALL
Java_com_example_gbaemulator_MainActivity_loadRomNative(JNIEnv *env, jobject instance, jbyteArray romData) {
    jsize length = env->GetArrayLength(romData);
    romBuffer.resize(length);
    env->GetByteArrayRegion(romData, 0, length, reinterpret_cast<jbyte*>(romBuffer.data()));
    
    LOGI("ROM caricata con successo! Dimensione: %d byte", length);
    return JNI_TRUE;
}
