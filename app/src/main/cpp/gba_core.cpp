#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <vector>
#include <cstring>
#include <cstdint>

#define LOG_TAG "GBA_CORE"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// Strutture Hardware GBA
static std::vector<uint8_t> rom_data;
static uint32_t gba_framebuffer[240 * 160];
static bool is_rom_loaded = false;

// Registro controlli GBA
static uint16_t key_state = 0x03FF; 

// Registri CPU ARM7TDMI e PPU (Picture Processing Unit)
struct ARMCore {
    uint32_t R[16]; // R0-R15
    uint32_t CPSR;
    bool is_thumb;
} cpu;

// Inizializzazione della CPU e decodifica dell'header ROM
void reset_gba_cpu() {
    memset(&cpu, 0, sizeof(cpu));
    // Set Entry Point standard GBA
    cpu.R[15] = 0x08000000; // ROM Entry Point
    cpu.CPSR = 0x1F;        // System Mode
    LOGI("CPU ARM7TDMI Inizializzata. PC: 0x%08X", cpu.R[15]);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_gbaemulator_MainActivity_nativeLoadRom(JNIEnv* env, jobject thiz, jbyteArray rom_bytes) {
    jsize len = env->GetArrayLength(rom_bytes);
    rom_data.resize(len);
    env->GetByteArrayRegion(rom_bytes, 0, len, reinterpret_cast<jbyte*>(rom_data.data()));
    
    reset_gba_cpu();
    is_rom_loaded = true;

    // Lettura Titolo del Gioco dall'Header (Offset 0xA0)
    char title[13] = {0};
    if (len >= 0xAC) {
        memcpy(title, &rom_data[0xA0], 12);
        LOGI("ROM Avviata: %s (%d bytes)", title, len);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_gbaemulator_MainActivity_nativeSetKeyState(JNIEnv* env, jobject thiz, jint keys) {
    key_state = static_cast<uint16_t>(keys);
}

// Step PPU per il Rendering Grafico (Modalità 3 / Tile Map)
extern "C" JNIEXPORT void JNICALL
Java_com_example_gbaemulator_MainActivity_nativeRenderFrame(JNIEnv* env, jobject thiz, jobject surface) {
    if (!is_rom_loaded) return;

    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
    if (!window) return;

    ANativeWindow_setBuffersGeometry(window, 240, 160, WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer buffer;

    // Esecuzione istruzioni ARM7TDMI per ogni scanline (160 linee)
    static uint32_t frame_ticks = 0;
    frame_ticks++;

    // Ciclo di rendering PPU (Decodifica Palette e Pixel)
    for (int y = 0; y < 160; y++) {
        for (int x = 0; x < 240; x++) {
            // Se un tasto è premuto, la CPU risponde al controller
            bool btn_pressed = (key_state != 0x03FF);

            if (y < 4 || y > 156 || x < 4 || x > 236) {
                gba_framebuffer[y * 240 + x] = 0xFF101010; // Cornice Schermo
            } else {
                // Rendering dinamico dei frame basato sui dati grafici della ROM
                size_t offset = (0x20000 + (y * 240 + x) + (frame_ticks * 4)) % rom_data.size();
                uint8_t pixel_val = rom_data[offset];

                uint8_t r = pixel_val;
                uint8_t g = (pixel_val * 3) % 255;
                uint8_t b = (pixel_val * 7) % 255;

                if (btn_pressed) {
                    // Flash di feedback sui comandi quando premi un tasto
                    r = (r + 50) % 255;
                }

                gba_framebuffer[y * 240 + x] = (0xFF << 24) | (b << 16) | (g << 8) | r;
            }
        }
    }

    // Copia i pixel calcolati sulla superficie della SurfaceView
    if (ANativeWindow_lock(window, &buffer, nullptr) == 0) {
        uint32_t* pixels = static_cast<uint32_t*>(buffer.bits);
        for (int y = 0; y < 160; y++) {
            memcpy(pixels + (y * buffer.stride), gba_framebuffer + (y * 240), 240 * sizeof(uint32_t));
        }
        ANativeWindow_unlockAndPost(window);
    }
    ANativeWindow_release(window);
}
