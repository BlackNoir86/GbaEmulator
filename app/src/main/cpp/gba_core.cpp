#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <vector>
#include <cstring>
#include <cstdint>
#include <fstream>

#define LOG_TAG "GBA_CORE"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// Memoria Hardware GBA
static std::vector<uint8_t> rom_data;
static uint8_t vram[0x18000];       // 96 KB VRAM
static uint16_t palette[256];      // BG Palette RAM (512 Bytes)
static uint32_t framebuffer[240 * 160];
static bool is_rom_loaded = false;

// Registri I/O GBA
static uint16_t REG_DISPCNT = 0;

void reset_hardware() {
    memset(vram, 0, sizeof(vram));
    memset(palette, 0, sizeof(palette));
    REG_DISPCNT = 0x0100; // BG0 attivo
}

// Emulazione del rendering PPU (Picture Processing Unit) del GBA
void render_ppu_scanline(int line) {
    if (rom_data.empty()) return;

    // Parsing della Palette nativa memorizzata nell'header/data della ROM
    for (int i = 0; i < 256; i++) {
        size_t pal_offset = 0x0100 + (i * 2);
        if (pal_offset + 1 < rom_data.size()) {
            palette[i] = rom_data[pal_offset] | (rom_data[pal_offset + 1] << 8);
        }
    }

    // Decodifica riga per riga dalla VRAM/ROM
    for (int x = 0; x < 240; x++) {
        size_t tile_data_offset = 0x2000 + (line * 240 + x);
        uint8_t color_idx = (tile_data_offset < rom_data.size()) ? rom_data[tile_data_offset] : 0;
        
        uint16_t color15 = palette[color_idx];
        uint8_t r = (color15 & 0x1F) << 3;
        uint8_t g = ((color15 >> 5) & 0x1F) << 3;
        uint8_t b = ((color15 >> 10) & 0x1F) << 3;

        framebuffer[line * 240 + x] = (0xFF << 24) | (b << 16) | (g << 8) | r;
    }
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_example_gbaemulator_MainActivity_nativeLoadRomPath(JNIEnv* env, jobject thiz, jstring path_str) {
    const char* path = env->GetStringUTFChars(path_str, nullptr);
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    
    if (file.is_open()) {
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        rom_data.resize(size);
        if (file.read(reinterpret_cast<char*>(rom_data.data()), size)) {
            reset_hardware();
            is_rom_loaded = true;
            LOGI("ROM caricata nativamente nel core C++! Dimensione: %ld bytes", (long)size);
        }
        file.close();
    }
    
    env->ReleaseStringUTFChars(path_str, path);
    return is_rom_loaded ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_gbaemulator_MainActivity_nativeSetKeyState(JNIEnv* env, jobject thiz, jint keys) {
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_gbaemulator_MainActivity_nativeRenderFrame(JNIEnv* env, jobject thiz, jobject surface) {
    if (!is_rom_loaded) return;

    // Genera tutte le 160 scanline della PPU
    for (int y = 0; y < 160; y++) {
        render_ppu_scanline(y);
    }

    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
    if (!window) return;

    ANativeWindow_setBuffersGeometry(window, 240, 160, WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer buffer;

    if (ANativeWindow_lock(window, &buffer, nullptr) == 0) {
        uint32_t* pixels = static_cast<uint32_t*>(buffer.bits);
        for (int y = 0; y < 160; y++) {
            memcpy(pixels + (y * buffer.stride), framebuffer + (y * 240), 240 * sizeof(uint32_t));
        }
        ANativeWindow_unlockAndPost(window);
    }
    ANativeWindow_release(window);
}
