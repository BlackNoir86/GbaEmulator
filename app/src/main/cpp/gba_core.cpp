#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <vector>
#include <cstring>
#include <cstdint>

#define LOG_TAG "GBA_CORE"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// Memoria sicura: separata per ROM e VRAM
static std::vector<uint8_t> rom_buffer;
static uint16_t vram[0x18000]; // Dimensione corretta VRAM GBA (96KB)
static bool is_rom_loaded = false;

extern "C" JNIEXPORT void JNICALL
Java_com_example_gbaemulator_MainActivity_nativeLoadRom(JNIEnv* env, jobject thiz, jbyteArray rom_bytes) {
    jsize len = env->GetArrayLength(rom_bytes);
    rom_buffer.resize(len);
    jbyte* buffer = env->GetByteArrayElements(rom_bytes, NULL);
    
    // Copia sicura
    memcpy(rom_buffer.data(), buffer, len);
    
    env->ReleaseByteArrayElements(rom_bytes, buffer, 0);
    is_rom_loaded = true;
    LOGI("ROM caricata in buffer sicuro. Dimensione: %d bytes", len);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_gbaemulator_MainActivity_nativeRenderFrame(JNIEnv* env, jobject thiz, jobject surface) {
    if (!is_rom_loaded) return;

    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
    if (!window) return;

    ANativeWindow_Buffer buffer;
    if (ANativeWindow_lock(window, &buffer, nullptr) == 0) {
        uint32_t* pixels = static_cast<uint32_t*>(buffer.bits);
        
        // Rendering semplificato: leggiamo dalla ROM per simulare il contenuto
        // In un emulatore vero, qui leggeremmo la VRAM processata dalla CPU
        for (int y = 0; y < 160; y++) {
            for (int x = 0; x < 240; x++) {
                // Leggiamo un offset basato su x,y per evitare accessi fuori bounds
                size_t offset = (y * 240 + x) % rom_buffer.size();
                uint8_t data = rom_buffer[offset];
                
                // Colore RGB basilare derivato dal dato
                pixels[y * buffer.stride + x] = (0xFF << 24) | (data << 16) | (data << 8) | data;
            }
        }
        ANativeWindow_unlockAndPost(window);
    }
    ANativeWindow_release(window);
}
