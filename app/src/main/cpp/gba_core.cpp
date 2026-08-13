#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <vector>

#define LOG_TAG "GBA_CORE"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static std::vector<uint8_t> rom_data;

extern "C" JNIEXPORT void JNICALL
Java_com_example_gbaemulator_MainActivity_nativeLoadRom(JNIEnv* env, jobject thiz, jbyteArray rom_bytes) {
    jsize len = env->GetArrayLength(rom_bytes);
    rom_data.resize(len);
    env->GetByteArrayRegion(rom_bytes, 0, len, reinterpret_cast<jbyte*>(rom_data.data()));
    LOGI("ROM caricata con successo nel core C++: %d bytes", len);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_gbaemulator_MainActivity_nativeRenderFrame(JNIEnv* env, jobject thiz, jobject surface) {
    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
    if (!window) return;

    ANativeWindow_setBuffersGeometry(window, 240, 160, WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer buffer;

    if (ANativeWindow_lock(window, &buffer, nullptr) == 0) {
        uint32_t* pixels = static_cast<uint32_t*>(buffer.bits);
        
        // Renderizza un pattern di test (GBA Display test)
        static int frameCount = 0;
        frameCount++;

        for (int y = 0; y < 160; y++) {
            for (int x = 0; x < 240; x++) {
                uint8_t r = (x + frameCount) % 255;
                uint8_t g = (y + frameCount) % 255;
                uint8_t b = 128;
                pixels[y * buffer.stride + x] = (0xFF << 24) | (b << 16) | (g << 8) | r;
            }
        }
        ANativeWindow_unlockAndPost(window);
    }
    ANativeWindow_release(window);
}
