#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <dlfcn.h>
#include <vector>
#include <cstring>
#include <cstdint>
#include <string>

#define LOG_TAG "GBA_CORE"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

typedef void (*retro_init_t)(void);
typedef bool (*retro_load_game_t)(const void* game);
typedef void (*retro_run_t)(void);
typedef void (*retro_set_video_refresh_t)(void (*)(const void*, unsigned, unsigned, size_t));
typedef void (*retro_set_environment_t)(bool (*)(unsigned, void*));

struct retro_game_info {
    const char* path;
    const void* data;
    size_t size;
    const char* meta;
};

static void* libretro_handle = nullptr;
static retro_run_t retro_run_fn = nullptr;
static uint32_t gba_framebuffer[240 * 160];
static bool is_rom_loaded = false;
static int pixel_format = 0;

bool environment_callback(unsigned cmd, void* data) {
    if (cmd == 10 && data) { // RETRO_ENVIRONMENT_SET_PIXEL_FORMAT
        pixel_format = *static_cast<int*>(data);
        LOGI("Formato pixel mGBA: %d", pixel_format);
        return true;
    }
    return false;
}

void video_refresh_callback(const void* data, unsigned width, unsigned height, size_t pitch) {
    if (!data) return;

    const uint16_t* src16 = static_cast<const uint16_t*>(data);
    for (unsigned y = 0; y < height && y < 160; y++) {
        for (unsigned x = 0; x < width && x < 240; x++) {
            uint16_t pixel = src16[y * (pitch / 2) + x];
            uint8_t r = (pixel & 0x1F) << 3;
            uint8_t g = ((pixel >> 5) & 0x1F) << 3;
            uint8_t b = ((pixel >> 10) & 0x1F) << 3;
            gba_framebuffer[y * 240 + x] = (0xFF << 24) | (b << 16) | (g << 8) | r;
        }
    }
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_example_gbaemulator_MainActivity_nativeLoadRomPath(JNIEnv* env, jobject thiz, jstring path_str) {
    const char* path = env->GetStringUTFChars(path_str, nullptr);
    std::string rom_path(path);
    env->ReleaseStringUTFChars(path_str, path);

    if (!libretro_handle) {
        libretro_handle = dlopen("libmgba_core.so", RTLD_NOW);
        if (!libretro_handle) {
            LOGE("Impossibile caricare libmgba_core.so: %s", dlerror());
            return JNI_FALSE;
        }

        auto retro_init = (retro_init_t)dlsym(libretro_handle, "retro_init");
        auto retro_load_game = (retro_load_game_t)dlsym(libretro_handle, "retro_load_game");
        auto retro_set_env = (retro_set_environment_t)dlsym(libretro_handle, "retro_set_environment");
        auto retro_set_video = (retro_set_video_refresh_t)dlsym(libretro_handle, "retro_set_video_refresh");
        retro_run_fn = (retro_run_t)dlsym(libretro_handle, "retro_run");

        if (retro_set_env) retro_set_env(environment_callback);
        if (retro_set_video) retro_set_video(video_refresh_callback);
        if (retro_init) retro_init();

        retro_game_info game_info = { rom_path.c_str(), nullptr, 0, nullptr };
        if (retro_load_game && retro_load_game(&game_info)) {
            is_rom_loaded = true;
            LOGI("ROM caricata con successo da file system!");
            return JNI_TRUE;
        }
    }
    return JNI_FALSE;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_gbaemulator_MainActivity_nativeSetKeyState(JNIEnv* env, jobject thiz, jint keys) {
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_gbaemulator_MainActivity_nativeRenderFrame(JNIEnv* env, jobject thiz, jobject surface) {
    if (!is_rom_loaded) return;

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
