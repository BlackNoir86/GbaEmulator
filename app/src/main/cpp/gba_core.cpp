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

// Palette di default per l'interfaccia GBA (16 Colori BGR555)
static const uint16_t default_palette[16] = {
    0x0000, 0x7FFF, 0x001F, 0x03E0, 0x7C00, 0x03FF, 0x7C1F, 0x7FE0,
    0x3DEF, 0x18C6, 0x2104, 0x0010, 0x0200, 0x4000, 0x1C0E, 0x630C
};

extern "C" JNIEXPORT void JNICALL
Java_com_example_gbaemulator_MainActivity_nativeLoadRom(JNIEnv* env, jobject thiz, jbyteArray rom_bytes) {
    jsize len = env->GetArrayLength(rom_bytes);
    rom_data.resize(len);
    env->GetByteArrayRegion(rom_bytes, 0, len, reinterpret_cast<jbyte*>(rom_data.data()));
    is_rom_loaded = true;
    LOGI("ROM caricata con successo: %d bytes", len);
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

    // Sfondo predefinito scuro
    for (int i = 0; i < 240 * 160; i++) {
        gba_framebuffer[i] = 0xFF121212;
    }

    // Offset dove risiedono solitamente le sprite/tile del gioco nella ROM
    size_t tile_offset = 0x2000;
    static int scroll_x = 0;
    if ((key_state & (1 << 4)) == 0) scroll_x += 2; // Right pressed
    if ((key_state & (1 << 5)) == 0) scroll_x -= 2; // Left pressed

    // Renderizza una griglia di Tile 8x8 (30x20 tile = 240x160 pixel)
    for (int ty = 0; ty < 20; ty++) {
        for (int tx = 0; tx < 30; tx++) {
            size_t current_tile_ptr = tile_offset + ((ty * 30 + tx) * 32);
            if (current_tile_ptr + 32 < rom_data.size()) {
                int render_x = (tx * 8 + scroll_x) % 240;
                if (render_x < 0) render_x += 240;
                decode_gba_tile(&rom_data[current_tile_ptr], gba_framebuffer, render_x, ty * 8, default_palette);
            }
        }
    }

    // Copia sullo schermo Android
    if (ANativeWindow_lock(window, &buffer, nullptr) == 0) {
        uint32_t* pixels = static_cast<uint32_t*>(buffer.bits);
        for (int y = 0; y < 160; y++) {
            memcpy(pixels + (y * buffer.stride), gba_framebuffer + (y * 240), 240 * sizeof(uint32_t));
        }
        ANativeWindow_unlockAndPost(window);
    }
    ANativeWindow_release(window);
}
