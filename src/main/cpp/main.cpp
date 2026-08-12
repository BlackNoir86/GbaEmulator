#include <jni.h>

extern "C" JNIEXPORT jboolean JNICALL
Java_com_example_gbaemulator_MainActivity_loadRomNative(JNIEnv *env, jobject instance, jbyteArray romData) {
    return JNI_TRUE;
}
