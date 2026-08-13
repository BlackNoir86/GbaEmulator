#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <vector>
#include <cstring>
#include <cstdint>

#define LOG_TAG "GBA_CORE"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// Emulazione memoria GBA
static uint8_t gba_memory[0x02000000]; // 32MB di RAM totale
static uint32_t gba_framebuffer[240 * 160];
static bool is_running = false;

extern "C" JNIEXPORT void JNICALL
Java_com_example_gbaemulator_MainActivity_nativeLoadRom(JNIEnv* env, jobject thiz, jbyteArray rom_bytes) {
    jsize len = env->GetArrayLength(rom_bytes);
    jbyte* buffer = env->GetByteArrayElements(rom_bytes, NULL);
    
    // Carica la ROM nel banco memoria 0x08000000 (Cartridge ROM)
    memcpy(&gba_memory[0x08000000], buffer, len);
    
    env->ReleaseByteArrayElements(rom_bytes, buffer, 0);
    is_running = true;
    LOGI("Core inizializzato. ROM caricata in memoria GBA.");
}

// Simulazione ciclo CPU ARM7TDMI
void emu_step() {
    // Qui in un emulatore completo girerebbe l'interprete ARM
    // Per ora, garantiamo che la VRAM venga letta correttamente
    static int frame = 0;
    frame++;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_gbaemulator_MainActivity_nativeRenderFrame(JNIEnv* env, jobject thiz, jobject surface) {
    if (!is_running) return;

    emu_step();

    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
    if (!window) return;

    ANativeWindow_Buffer buffer;
    if (ANativeWindow_lock(window, &buffer, nullptr) == 0) {
        uint32_t* pixels = static_cast<uint32_t*>(buffer.bits);
        
        // Leggi direttamente dalla VRAM (0x06000000)
        // Se la ROM ha completato il boot, qui troveremo i dati scompattati
        uint16_t* vram = (uint16_t*)&gba_memory[0x06000000];
        
        for (int y = 0; y < 160; y++) {
            for (int x = 0; x < 240; x++) {
                uint16_t color16 = vram[y * 240 + x];
                // Conversione veloce BGR555 -> RGBA8888
                uint32_t r = (color16 & 0x1F) << 3;
                uint32_t g = ((color16 >> 5) & 0x1F) << 3;
                uint32_t b = ((color16 >> 10) & 0x1F) << 3;
                pixels[y * buffer.stride + x] = (0xFF << 24) | (b << 16) | (g << 8) | r;
            }
        }
        ANativeWindow_unlockAndPost(window);
    }
    ANativeWindow_release(window);
}
