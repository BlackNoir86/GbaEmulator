cmake_minimum_required(VERSION 3.22.1)
project("gbaemulator")

# Disabilita tutte le dipendenze GUI/interfaccia/tool di mGBA
set(BUILD_GL OFF CACHE BOOL "" FORCE)
set(BUILD_GLES2 OFF CACHE BOOL "" FORCE)
set(BUILD_GLES3 OFF CACHE BOOL "" FORCE)
set(BUILD_QT OFF CACHE BOOL "" FORCE)
set(BUILD_SDL OFF CACHE BOOL "" FORCE)
set(BUILD_SUITE OFF CACHE BOOL "" FORCE)
set(USE_FFMPEG OFF CACHE BOOL "" FORCE)
set(USE_ZLIB OFF CACHE BOOL "" FORCE)
set(USE_PNG OFF CACHE BOOL "" FORCE)
set(USE_SQLITE3 OFF CACHE BOOL "" FORCE)
set(USE_ELF OFF CACHE BOOL "" FORCE)

include(FetchContent)
FetchContent_Declare(
    libretro_mgba
    GIT_REPOSITORY https://github.com/libretro/mgba.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(libretro_mgba)

add_library(
    gbaemulator
    SHARED
    gba_core.cpp
)

find_library(log-lib log)
find_library(android-lib android)

target_link_libraries(
    gbaemulator
    ${log-lib}
    ${android-lib}
)
