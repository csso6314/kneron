cmake_minimum_required(VERSION 2.8.12)
project(hostkdp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CMAKE_CXX_FLAGS "${ASAN_FLAGS} -g -Werror -Wall -static-libstdc++")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_definitions(-fPIC)

#IF USE C++11 or later version,THIS NEED TO BE TRUE,EARLIER NEED BE SET "FALSE"
set(THREAD_USE_CPP11 "FALSE")
if(THREAD_USE_CPP11)
	#supprot for earlier versions of C++11
	add_definitions(-DTHREAD_USE_CPP11)
	set(CMAKE_CXX_STANDARD 11)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++98")
endif()

set(libkdp_SRCS
    BaseDev.cpp
    KdpHostApi.cpp
    KnLog.cpp
    KnMsg.cpp
    KnUtil.cpp
    MsgMgmt.cpp
    MsgReceiver.cpp
    MsgSender.cpp
    MsgXBase.cpp
    SyncMsgHandler.cpp
    UARTPkt.cpp
    USBDev.cpp
    khost_ll.cpp
    khost_api.cpp
    kneron_nef_reader.c
)

if(UNIX)
    message(STATUS "Build environment => UNIX, Skip build UARTDev")
    set(libkdp_SRCS
        ${libkdp_SRCS}
        UARTDev.cpp
    )
endif()

# libhostkdp
set(LIB_NAME ${PROJECT_NAME})
add_library(${LIB_NAME} SHARED ${libkdp_SRCS})
target_include_directories(
    ${LIB_NAME}
    PUBLIC
    "${CMAKE_SOURCE_DIR}/common/include"
    "${CMAKE_SOURCE_DIR}/common/include/khost"
)

# you can use 'make lib -j4' to build only the library
add_custom_target(lib DEPENDS ${LIB_NAME})

if(UNIX)
	message(STATUS "Build for UNIX environment")
	target_link_libraries(${LIB_NAME} ${USB_LIB})
elseif(MSYS OR MINGW)
	message(STATUS "Build for MSYS or MingGW environment")
	target_link_libraries(${LIB_NAME} usb-1.0 ws2_32)
    
    #copy necessary dll files
    add_custom_command(TARGET ${LIB_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DLL_DIR}/* ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
endif()
