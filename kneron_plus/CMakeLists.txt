
cmake_minimum_required(VERSION 2.8.12)

project(kneron_plus_sdk)

#cross compile for raspbian .so file needed by wheel file generation
set(CMAKE_CROSSCOMPILE OFF CACHE BOOL "is crosscompiled")
message(STATUS "CMAKE_CROSSCOMPILE ${CMAKE_CROSSCOMPILE}")

message(STATUS "USB_LIB: [${USB_LIB}]")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Werror -Wall")

set(KPLUS_LIB_NAME "kplus")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Mac configurations
if (${APPLE})
        message(STATUS "Build environment => APPLE Mac OS")
        set(CMAKE_MACOSX_RPATH 1)
        include_directories("/usr/local/include")

        find_library(USB_LIB
                NAMES usb-1.0 libusb-1.0
                HINTS /usr/local/lib /usr/bin)
        add_definitions(-D__linux__ -DOS_TYPE_MACOS)

        if (APPLE_BUILD_RELEASE)
                message("-- APPLE macOS Release build.")
                set(CMAKE_INSTALL_RPATH "$ORIGIN")
                set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
        else()
                message("-- APPLE macOS Debug build.")
                set(CMAKE_BUILD_TYPE "Debug")
        endif()

        # Use with kneronhunter/macos_cross-compiler docker image.
        include_directories(/opt/osxcross/macports/pkgs/opt/local/include)
        include_directories(/opt/osxcross/SDK/MacOSX10.14.sdk/usr/include/machine)
elseif (CMAKE_CROSSCOMPILE_PI4_ARMV7)
        message(STATUS "cmake_find_root_path: ${CMAKE_FIND_ROOT_PATH}")
        message(STATUS "Build environment => cross compile for Raspbian")

        include_directories("${CMAKE_FIND_ROOT_PATH}/usr/local/include")
        find_library(USB_LIB
                NAMES usb-1.0 libusb-1.0
                HINTS ${CMAKE_FIND_ROOT_PATH}/usr/local/lib)
else()
        set(USB_LIB usb-1.0)
endif()

add_definitions(-DHOST_LIB_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

set(DLL_DIR ${PROJECT_SOURCE_DIR}/thirdparty/windows/dll)

#define macros
MACRO(SUBDIRLIST result curdir wildcard)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/${wildcard})
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
      LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()


add_subdirectory(src)
add_subdirectory(app_lib)

#add examples*
SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR} "examples*")
FOREACH(subdir ${SUBDIRS})
ADD_SUBDIRECTORY(${subdir})
ENDFOREACH()

#add tools
SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR} "tools")
FOREACH(subdir ${SUBDIRS})
ADD_SUBDIRECTORY(${subdir})
ENDFOREACH()

# for windows system DLLs
if(MSYS OR MINGW)
    get_filename_component(COMPILER_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)

    add_custom_target(
        copy_dll_files
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DLL_DIR}/* ${CMAKE_BINARY_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${COMPILER_DIR}/libstdc++-6.dll ${CMAKE_BINARY_DIR}/bin
    )
    add_dependencies(${KPLUS_LIB_NAME} copy_dll_files)
endif()
