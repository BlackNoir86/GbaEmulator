cmake_minimum_required(VERSION 3.22.1)
project("gbaemulator")

# Scarica/Includi dipendenze e core mGBA
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
    mgba_core
    ${log-lib}
    ${android-lib}
)
