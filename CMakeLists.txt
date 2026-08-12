cmake_minimum_required(VERSION 3.22.1)
project("gbaemulator")

add_library(
    gbaemulator
    SHARED
    src/main/cpp/main.cpp
)

find_library(log-lib log)

target_link_libraries(
    gbaemulator
    android
    GLESv2
    EGL
    ${log-lib}
)
 
