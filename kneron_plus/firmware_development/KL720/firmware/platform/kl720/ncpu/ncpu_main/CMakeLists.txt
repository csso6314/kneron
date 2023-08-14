cmake_minimum_required(VERSION 3.10.2)

project(720_ncpu_main)

# specify Debug/Release configuration
if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_C_FLAGS_DEBUG "-g3 -O3")
set(CMAKE_C_FLAGS_RELEASE "-O3")

# specifiy target compile options
#add_compile_options(-g -O3)
add_compile_options(-std=c11)
add_compile_options(-Wall -Werror)
add_compile_options(-mlongcalls -ffunction-sections)

# specify defind symbols
add_definitions(-DTARGET_NCPU=1)
add_definitions(-DLOG_ENABLE=1)
add_definitions(-DLOG_USE_PRINTF=0)
add_definitions(-DSIM_INTERRUPT_GEN=0)
add_definitions(-DBOOT_FROM_IRAM=1)
add_definitions(-DDSP_STANDALONE_BRINGUP=0)
add_definitions(-DGEN_INTERRUPT_ON_ISS=0)
add_definitions(-DGEN_INTERRUPT_ON_HW=1)
add_definitions(-DDSP_NPU_PERF_PROFILE=1)
add_definitions(-DYOLO_FIX_POINT_DUMP=0)
add_definitions(-DTEST_PARALLEL=1)
add_definitions(-DKL720=1)
add_definitions(-DFIXED=1)
add_definitions(-DFIXED_DSP=1)
add_definitions(-DPRINT_INPROC_REG=0)
add_definitions(-DINPROC_POLL_ENABLE=1)
add_definitions(-DPRINT_NPU_REG=0)
add_definitions(-DPRINT_PARSER_NODE_VALUE=0)
add_definitions(-DSTACK_HEAP_MEASURE=0)
add_definitions(-DKL720_IPC=1)
add_definitions(-D_BOARD_SN720HAPS_H_=0)
add_definitions(-D_VECC_=1)
add_definitions(-DENABLE_JPEGLIB=0)

# specify include paths
include_directories(${PROJECT_SOURCE_DIR}/../../common)
include_directories(${PROJECT_SOURCE_DIR}/../libs/libXtensa/inc)
include_directories(${PROJECT_SOURCE_DIR}/../libs/libxi/inc)
include_directories(${PROJECT_SOURCE_DIR}/../libs/libFreeRTOS/inc)
include_directories(${PROJECT_SOURCE_DIR}/../libs/libDriver/inc)

# specify file list to compile
file(GLOB SRCS ./src/*.c)

add_executable(${PROJECT_NAME} ${SRCS})

# use prebuild libraries
#link_directories(${PROJECT_SOURCE_DIR}/../libs/)

add_library( Xtensa_imported STATIC IMPORTED )
set_target_properties( Xtensa_imported PROPERTIES
    IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/../libs/libXtensa.a)

add_library( FreeRTOS_imported STATIC IMPORTED )
set_target_properties( FreeRTOS_imported PROPERTIES
    IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/../libs/libFreeRTOS.a)

add_library( xi_imported STATIC IMPORTED )
set_target_properties( xi_imported PROPERTIES
    IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/../libs/libxi.a)

add_library( Sys_imported STATIC IMPORTED )
set_target_properties( Sys_imported PROPERTIES
    IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/../libs/libSys.a)

add_library( jpeg_imported STATIC IMPORTED )
set_target_properties( jpeg_imported PROPERTIES
    IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/../libs/libjpeg.a)

add_library( ToF_imported STATIC IMPORTED )
set_target_properties( ToF_imported PROPERTIES
    IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/../libs/libToF.a)

add_library( Driver_imported STATIC IMPORTED )
set_target_properties( Driver_imported PROPERTIES
    IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/../libs/libDriver.a)

# specify libraries
set(ncpu_lib_list
    ToF_imported
    Driver_imported
    jpeg_imported
    Sys_imported
    xi_imported
    FreeRTOS_imported
    Xtensa_imported)

target_link_libraries(${PROJECT_NAME} ${ncpu_lib_list} m)


# trying
#add_dependencies(
#    Driver_imported
#    ToF_imported
#    jpeg_imported
#    Sys_imported
#    xi_imported
#    FreeRTOS_imported
#    Xtensa_imported)

# specify link options
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mlsp=${PROJECT_SOURCE_DIR}/link_map")


# specifiy post build operations
add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD

    COMMAND
        ${CMAKE_OBJCOPY}
        -j .ResetVector.text
        -j .WindowVectors.text
        -j .Level2InterruptVector.text
        -j .Level3InterruptVector.text
        -j .DebugExceptionVector.text
        -j .NMIExceptionVector.text
        -j .KernelExceptionVector.text
        -j .UserExceptionVector.text
        -j .DoubleExceptionVector.text
        -j .libDriver.text
        -j .clib.rodata
        -j .libDriver.rodata
        -j .libDriver.data
        -j .libDriver.bss
        -j .rtos.rodata
        -j .rodata
        -j .text
        -j .clib.data
        -j .rtos.percpu.data
        -j .data
        -j .bss
        -j .debug_pubnames
        -j .debug_info
        -j .debug_abbrev
        -j .debug_line
        -j .xt.prop
        -j .xtensa.info
        -j .comment
        -j .debug_ranges
        -O binary ${PROJECT_BINARY_DIR}/${PROJECT_NAME} fw_ncpu_tmp.bin

    COMMAND
        ${KNERON_GEN_DFU_TOOL}
        -ncpu fw_ncpu_tmp.bin fw_ncpu.bin
    COMMAND
        ${CMAKE_COMMAND} -E
        echo fw_ncpu.bin is built

    VERBATIM
)

