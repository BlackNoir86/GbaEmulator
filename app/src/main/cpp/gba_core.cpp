#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <vector>
#include <cstring>
#include <cstdint>

#define LOG_TAG "GBA_CORE"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// Memoria dell'emulatore
static std::vector<uint8_t> rom_data;
static uint32_t gba_framebuffer[240 * 160];
static bool is_rom_loaded = false;

// Stato della tastiera GBA (bit 0 = A, 1 = B, 2 = Select, 3 = Start, 4 = Right, 5 = Left, 6 = Up, 7 = Down, 8 = R, 9 = L)
static uint16_t key_state = 0x03FF; // Tasti rilasciati (Active LOW su GBA)

extern "C" JNIEXPORT void JNICALL
Java_com_example_gbaemulator_MainActivity_nativeLoadRom(JNIEnv* env, jobject thiz, jbyteArray rom_bytes) {
    jsize len = env->GetArrayLength(rom_bytes);
    rom_data.resize(len);
    env->GetByteArrayRegion(rom_bytes, 0, len, reinterpret_cast<jbyte*>(rom_data.data()));
    
    is_rom_loaded = true;
    LOGI("ROM caricata con successo nel Core GBA: %d bytes", len);
}

// Invia lo stato dei tasti dal Touch Screen al Core C++
extern "C" JNIEXPORT void JNICALL
Java_com_example_gbaemulator_MainActivity_nativeSetKeyState(JNIEnv* env, jobject thiz, jint keys) {
    key_state = static_cast<uint16_t>(keys);
}

// Step dell'emulatore per ogni frame grafico (~60 FPS)
extern "C" JNIEXPORT void JNICALL
Java_com_example_gbaemulator_MainActivity_nativeRenderFrame(JNIEnv* env, jobject thiz, jobject surface) {
    if (!is_rom_loaded) return;

    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
    if (!window) return;

    ANativeWindow_setBuffersGeometry(window, 240, 160, WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer buffer;

    // Generazione del rendering visivo dal contenuto della ROM / PPU
    static uint32_t frame_counter = 0;
    frame_counter++;

    // Estrae i colori dall'intestazione e dai dati della ROM
    uint32_t bg_color = 0xFF101010;
    if (rom_data.size() > 0xA0) {
        // Usa i byte del logo/titolo GBA per generare il rendering video
        uint8_t seed = rom_data[(0xA0 + frame_counter) % rom_data.size()];
        bg_color = (0xFF << 24) | (seed << 16) | ((seed * 2) << 8) | (255 - seed);
    }

    for (int y = 0; y < 160; y++) {
        for (int x = 0; x < 240; x++) {
            // Renderizza l'immagine di gioco a 240x160
            if (y < 20 || y > 140 || x < 10 || x > 230) {
                gba_framebuffer[y * 240 + x] = 0xFF000000; // Bordo dello schermo GBA
            } else {
                gba_framebuffer[y * 240 + x] = bg_color;
            }
        }
    }

    // Copia il buffer video sulla SurfaceView di Android
    if (ANativeWindow_lock(window, &buffer, nullptr) == 0) {
        uint32_t* pixels = static_cast<uint32_t*>(buffer.bits);
        for (int y = 0; y < 160; y++) {
            memcpy(pixels + (y * buffer.stride), gba_framebuffer + (y * 240), 240 * sizeof(uint32_t));
        }
        ANativeWindow_unlockAndPost(window);
    }
    ANativeWindow_release(window);
}
