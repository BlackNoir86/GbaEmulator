#include <jni.h>
#include <vector>
#include <android/log.h>

#define LOG_TAG "GBA_CORE"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

std::vector<uint8_t> romBuffer;

jboolean loadRomNative(JNIEnv *env, jobject instance, jbyteArray romData) {
    if (!romData) return JNI_FALSE;
    jsize length = env->GetArrayLength(romData);
    romBuffer.resize(length);
    env->GetByteArrayRegion(romData, 0, length, reinterpret_cast<jbyte*>(romBuffer.data()));
    LOGI("ROM caricata con successo! Dimensione: %d byte", length);
    return JNI_TRUE;
}

static JNINativeMethod methods[] = {
    {"loadRomNative", "([B)Z", (void*)loadRomNative}
};

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    jclass clazz = env->FindClass("com/example/gbaemulator/MainActivity");
    if (clazz == nullptr) {
        return JNI_ERR;
    }
    if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) < 0) {
        return JNI_ERR;
    }
    return JNI_VERSION_1_6;
}
