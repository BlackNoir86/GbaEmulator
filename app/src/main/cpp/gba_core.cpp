#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <dlfcn.h>
#include <vector>
#include <cstring>
#include <cstdint>

#define LOG_TAG "GBA_CORE"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Tipi per l'API Libretro
typedef void (*retro_init_t)(void);
typedef bool (*retro_load_game_t)(const void* game);
typedef void (*retro_run_t)(void);
typedef void (*retro_set_video_refresh_t)(void (*)(const void*, unsigned, unsigned, size_t));

struct retro_game_info {
    const char* path;
    const void* data;
    size_t size;
    const char* meta;
};

static void* libretro_handle = nullptr;
static retro_init_t retro_init_fn = nullptr;
static retro_load_game_t retro_load_game_fn = nullptr;
static retro_run_t retro_run_fn = nullptr;

static std::vector<uint8_t> rom_data;
static uint32_t gba_framebuffer[240 * 160];
static bool is_rom_loaded = false;

// Callback per ricevere i frame video generati da mGBA (in formato RGB565 o XRGB8888)
void video_refresh_callback(const void* data, unsigned width, unsigned height, size_t pitch) {
    if (!data) return;

    const uint16_t* src16 = static_cast<const uint16_t*>(data);
    for (unsigned y = 0; y < height && y < 160; y++) {
        for (unsigned x = 0; x < width && x < 240; x++) {
            uint16_t pixel = src16[y * (pitch / 2) + x];
            // Conversione RGB565 -> RGBA8888
            uint8_t r = ((pixel >> 11) & 0x1F) << 3;
            uint8_t g = ((pixel >> 5) & 0x3F) << 2;
            uint8_t b = (pixel & 0x1F) << 3;
            gba_framebuffer[y * 240 + x] = (0xFF << 24) | (b << 16) | (g << 8) | r;
        }
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_gbaemulator_MainActivity_nativeLoadRom(JNIEnv* env, jobject thiz, jbyteArray rom_bytes) {
    jsize len = env->GetArrayLength(rom_bytes);
    rom_data.resize(len);
    jbyte* buffer = env->GetByteArrayElements(rom_bytes, NULL);
    memcpy(rom_data.data(), buffer, len);
    env->ReleaseByteArrayElements(rom_bytes, buffer, 0);

    // Carica dinamicamente il core libmgba_core.so dalla cartella jniLibs
    if (!libretro_handle) {
        libretro_handle = dlopen("libmgba_core.so", RTLD_NOW);
        if (!libretro_handle) {
            LOGE("Errore caricamento libmgba_core.so: %s", dlerror());
            return;
        }

        retro_init_fn = (retro_init_t)dlsym(libretro_handle, "retro_init");
        retro_load_game_fn = (retro_load_game_t)dlsym(libretro_handle, "retro_load_game");
        retro_run_fn = (retro_run_t)dlsym(libretro_handle, "retro_run");
        auto retro_set_video_refresh = (retro_set_video_refresh_t)dlsym(libretro_handle, "retro_set_video_refresh");

        if (retro_set_video_refresh) {
            retro_set_video_refresh(video_refresh_callback);
        }

        if (retro_init_fn) retro_init_fn();
    }

    retro_game_info game_info = { nullptr, rom_data.data(), rom_data.size(), nullptr };
    if (retro_load_game_fn && retro_load_game_fn(&game_info)) {
        is_rom_loaded = true;
        LOGI("ROM agganciata con successo al core mGBA!");
    } else {
        LOGE("Fallito il caricamento della ROM nel core mGBA");
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_gbaemulator_MainActivity_nativeSetKeyState(JNIEnv* env, jobject thiz, jint keys) {
    // Riservato per l'input polling
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_gbaemulator_MainActivity_nativeRenderFrame(JNIEnv* env, jobject thiz, jobject surface) {
    if (!is_rom_loaded) return;

    // Esegue 1 frame CPU del gioco vero tramite mGBA
    if (retro_run_fn) retro_run_fn();

    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
    if (!window) return;

    ANativeWindow_setBuffersGeometry(window, 240, 160, WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer buffer;

    if (ANativeWindow_lock(window, &buffer, nullptr) == 0) {
        uint32_t* pixels = static_cast<uint32_t*>(buffer.bits);
        for (int y = 0; y < 160; y++) {
            memcpy(pixels + (y * buffer.stride), gba_framebuffer + (y * 240), 240 * sizeof(uint32_t));
        }
        ANativeWindow_unlockAndPost(window);
    }
    ANativeWindow_release(window);
}
