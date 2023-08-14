cmake_minimum_required(VERSION 2.8.12)

if (${APPLE})
        set(CMAKE_C_COMPILER "gcc-10")
        set(CMAKE_CXX_COMPILER "g++-10")
endif()

project(kdphostliball)

option(BUILD_OPENCV_EX      "Build opencv examples" OFF)
option(KNERON_BUILD_TEST    "Build testing program" OFF)
option(TEST_720             "Build 720 testing program" OFF)
option(TEST_520             "Build 520 testing program" OFF)
option(BUILD_FDR_APP        "Build 520/720 APP example" OFF)
option(KNERON_MEM_CHECK     "Enable run-time memory check" OFF)

message(STATUS "Build testing program => [${KNERON_BUILD_TEST}]")
if(KNERON_BUILD_TEST)
    message(STATUS "\tBuild 520 testing program => [${TEST_520}]")
    message(STATUS "\tBuild 720 testing program => [${TEST_720}]")
endif()
message(STATUS "Build 520/720 APP example => [${BUILD_FDR_APP}]")
message(STATUS "Build opencv examples => [${BUILD_OPENCV_EX}]")
message(STATUS "Build with ASAN => [${KNERON_MEM_CHECK}]")

# ASAN
if (KNERON_MEM_CHECK)
    set(ASAN_FLAGS "-fsanitize=address -static-libasan")
endif ()
 
# Mac configurations
if (${APPLE})
        message(STATUS "Build environment => APPLE Mac OS")
        set(CMAKE_MACOSX_RPATH 1)
        include_directories("/usr/local/include")
        find_library(USB_LIB
                NAMES usb-1.0 libusb-1.0
                HINTS /usr/local/lib /usr/bin)
        add_definitions(-D__linux__ -DOS_TYPE_MACOS)
else()
        set(USB_LIB usb-1.0)
endif()


message(STATUS "USB_LIB: [${USB_LIB}]")

add_definitions(-DHOST_LIB_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
set(DLL_DIR ${PROJECT_SOURCE_DIR}/dll)

add_subdirectory(src)
add_subdirectory(example)
add_subdirectory(kdp2)

if(KNERON_BUILD_TEST)
  add_definitions(-DKNERON_BUILD_TEST)
  
  add_subdirectory(test)
endif()
