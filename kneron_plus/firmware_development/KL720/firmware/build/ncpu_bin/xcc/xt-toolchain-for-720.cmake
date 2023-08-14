# Generic because we build for embedded system
set(CMAKE_SYSTEM_NAME Generic)
# It should be always set when CMAKE_SYSTEM_NAME is changed
set(CMAKE_SYSTEM_VERSION 1)

# EXECUTABLE EXTENSION
#SET (CMAKE_EXECUTABLE_SUFFIX ".elf")

# Check if toolchain envrionment is set
set(xtensa_inst $ENV{XTENSA_INST})
if(NOT xtensa_inst)
    message(FATAL_ERROR "Xtensa Toolchain envrionment is not set")
else()
    message(STATUS "XTENSA toolchain is found")
endif()

# Make CMake use "xt-xcc" for compiling C files
set(CMAKE_C_COMPILER    xt-xcc)
set(CMAKE_ASM_COMPILER 	xt-as)
# Override ar and ranlib tools that CMake should use for linking lib
set(CMAKE_AR            xt-ar      CACHE STRING "")
set(CMAKE_RANLIB        xt-ranlib  CACHE STRING "")
set(CMAKE_OBJCOPY       xt-objcopy CACHE INTERNAL "objcopy tool")
set(CMAKE_OBJDUMP       xt-objdump CACHE INTERNAL "objdump tool")

# Check Kneron gen_dfu_binary tools used in post build
if(WIN32)
    set(KNERON_GEN_DFU_TOOL "${CMAKE_CURRENT_LIST_DIR}/../../../utils/dfu/gen_dfu_binary_for_win.exe")
    set(KNERON_SBTENC_TOOL "${CMAKE_CURRENT_LIST_DIR}/../../../utils/sbtenc/sbtenc.exe")
else()
    set(KNERON_GEN_DFU_TOOL "${CMAKE_CURRENT_LIST_DIR}/../../../utils/dfu/gen_dfu_binary_for_linux")
endif()

if(EXISTS ${KNERON_GEN_DFU_TOOL})
    message(STATUS "[For post build] Kneron gen_dfu_binary tool is found")
else()
    message(FATAL_ERROR "Could not find Kneron gen_tfu tool")
endif()


message(STATUS  "BUILD TYPE: " ${CMAKE_BUILD_TYPE})
