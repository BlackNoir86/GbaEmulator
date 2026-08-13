#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <vector>
#include <cstring>
#include <cstdint>

#define LOG_TAG "GBA_CORE"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static std::vector<uint8_t> rom_data;
static bool is_rom_loaded = false;
static uint16_t current_keys = 0x03FF;

extern "C" JNIEXPORT void JNICALL
Java_com_example_gbaemulator_MainActivity_nativeLoadRom(JNIEnv* env, jobject thiz, jbyteArray rom_bytes) {
    jsize len = env->GetArrayLength(rom_bytes);
    rom_data.resize(len);
    jbyte* buffer = env->GetByteArrayElements(rom_bytes, NULL);
    memcpy(rom_data.data(), buffer, len);
    env->ReleaseByteArrayElements(rom_bytes, buffer, 0);

    is_rom_loaded = true;
    LOGI("ROM inviata al core nativo. Dimensione: %d bytes", len);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_gbaemulator_MainActivity_nativeSetKeyState(JNIEnv* env, jobject thiz, jint keys) {
    current_keys = static_cast<uint16_t>(keys);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_gbaemulator_MainActivity_nativeRenderFrame(JNIEnv* env, jobject thiz, jobject surface) {
    if (!is_rom_loaded || rom_data.empty()) return;

    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
    if (!window) return;

    ANativeWindow_setBuffersGeometry(window, 240, 160, WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer buffer;

    if (ANativeWindow_lock(window, &buffer, nullptr) == 0) {
        uint32_t* pixels = static_cast<uint32_t*>(buffer.bits);
        
        // Renderizza il frame sincronizzato a 240x160 a tutto schermo
        for (int y = 0; y < 160; y++) {
            for (int x = 0; x < 240; x++) {
                size_t idx = (y * 240 + x) % rom_data.size();
                uint8_t val = rom_data[idx];
                pixels[y * buffer.stride + x] = (0xFF << 24) | (val << 16) | (val << 8) | val;
            }
        }
        ANativeWindow_unlockAndPost(window);
    }
    ANativeWindow_release(window);
}
