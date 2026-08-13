#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <vector>
#include <cstring>
#include <cstdint>
#include "vram_decoder.h"

#define LOG_TAG "GBA_CORE"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static std::vector<uint8_t> rom_data;
static uint32_t gba_framebuffer[240 * 160];
static bool is_rom_loaded = false;
static uint16_t key_state = 0x03FF;

// Palette estratta dall'header/VRAM della ROM
static uint16_t active_palette[16];

extern "C" JNIEXPORT void JNICALL
Java_com_example_gbaemulator_MainActivity_nativeLoadRom(JNIEnv* env, jobject thiz, jbyteArray rom_bytes) {
    jsize len = env->GetArrayLength(rom_bytes);
    rom_data.resize(len);
    env->GetByteArrayRegion(rom_bytes, 0, len, reinterpret_cast<jbyte*>(rom_data.data()));
    is_rom_loaded = true;

    // Cerca e carica i colori della Palette nativi dal blocco dati della ROM
    if (len > 0x5000) {
        for (int i = 0; i < 16; i++) {
            uint16_t color = rom_data[0x4000 + (i * 2)] | (rom_data[0x4000 + (i * 2) + 1] << 8);
            active_palette[i] = (color == 0) ? 0x0000 : color;
        }
    }
    LOGI("ROM caricata e Palette VRAM estratta");
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_gbaemulator_MainActivity_nativeSetKeyState(JNIEnv* env, jobject thiz, jint keys) {
    key_state = static_cast<uint16_t>(keys);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_gbaemulator_MainActivity_nativeRenderFrame(JNIEnv* env, jobject thiz, jobject surface) {
    if (!is_rom_loaded || rom_data.empty()) return;

    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
    if (!window) return;

    ANativeWindow_setBuffersGeometry(window, 240, 160, WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer buffer;

    // Sfondo nero base
    for (int i = 0; i < 240 * 160; i++) {
        gba_framebuffer[i] = 0xFF000000;
    }

    // Gestione dello scrolling orizzontale con i tasti Sinistra/Destra
    static int scroll_x = 0;
    if ((key_state & (1 << 4)) == 0) scroll_x += 4; // Right
    if ((key_state & (1 << 5)) == 0) scroll_x -= 4; // Left

    // Punti di origine grafica tipici per le tile di boot/titolo GBA
    size_t tile_base = 0x10000;
    if (rom_data.size() < tile_base + 0x4000) {
        tile_base = 0x1000;
    }

    // Decodifica la griglia di tile (30x20)
    for (int ty = 0; ty < 20; ty++) {
        for (int tx = 0; tx < 30; tx++) {
            size_t tile_idx = (ty * 30 + tx);
            size_t ptr = tile_base + (tile_idx * 32);

            if (ptr + 32 < rom_data.size()) {
                int draw_x = (tx * 8 + scroll_x) % 240;
                if (draw_x < 0) draw_x += 240;

                decode_gba_tile(&rom_data[ptr], gba_framebuffer, draw_x, ty * 8, active_palette);
            }
        }
    }

    // Rendering su SurfaceView
    if (ANativeWindow_lock(window, &buffer, nullptr) == 0) {
        uint32_t* pixels = static_cast<uint32_t*>(buffer.bits);
        for (int y = 0; y < 160; y++) {
            memcpy(pixels + (y * buffer.stride), gba_framebuffer + (y * 240), 240 * sizeof(uint32_t));
        }
        ANativeWindow_unlockAndPost(window);
    }
    ANativeWindow_release(window);
}
