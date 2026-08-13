cmake_minimum_required(VERSION 3.22.1)
project("gbaemulator")

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
    dl
)
