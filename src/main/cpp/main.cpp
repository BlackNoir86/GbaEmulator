#include <android/native_activity.h>
#include <android/log.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "GbaEmulator", __VA_ARGS__)

void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize) {
    LOGI("GBA Emulator Native Activity Started!");
}
