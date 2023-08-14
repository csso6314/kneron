cmake_minimum_required(VERSION 3.10.1)

project(720_ncpu_develop)

add_subdirectory(ncpu_main)

if( EXISTS ${PROJECT_SOURCE_DIR}/libs/libDriver/main/src)
    add_subdirectory(libs)
    add_dependencies(720_ncpu_main Xtensa Sys jpeg FreeRTOS Driver ToF)
endif()

