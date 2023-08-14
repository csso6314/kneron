message("-- PACK_PYTHON " ${PYTHON_PACK})
if (PYTHON_PACK)
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/library/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/library/lib)

set(MATH_LIB "m")

include_directories(${PROJECT_SOURCE_DIR}/src/include/soc_common)
include_directories(${PROJECT_SOURCE_DIR}/src/include/local)
include_directories(${PROJECT_SOURCE_DIR}/src/python_wrapper/include)

if(MSYS OR MINGW)
    set(LIB_WDI_INCLUDE ${PROJECT_SOURCE_DIR}/thirdparty/windows/include)
    set(LIB_WDI_LIB     ${PROJECT_SOURCE_DIR}/thirdparty/windows/dll/libwdi.dll)
    include_directories(${LIB_WDI_INCLUDE})
endif()

add_definitions(-fPIC)

set(code_src
    kp_usb.c
    kp_core.c
    kp_errstring.c
    kp_inference.c
    kp_set_key.c
    kp_update_flash.c
    kneron_nef_reader.c
    setup_reader.c
    model_descriptor_builder.c
    utils.c

    python_wrapper/src/kp_python_wrap.c

    legacy/kp_inference_v1.c
)

add_library(${KPLUS_LIB_NAME} SHARED ${code_src})

if(UNIX)
	message(STATUS "Build for UNIX environment")
	target_link_libraries(${KPLUS_LIB_NAME} ${USB_LIB} ${MATH_LIB})
elseif(MSYS OR MINGW)
	message(STATUS "Build for MSYS or MingGW environment")
	target_link_libraries(${KPLUS_LIB_NAME} usb-1.0 ws2_32 ${LIB_WDI_LIB} ${MATH_LIB})
endif()

# copy headers and so/dll
add_custom_command(
    TARGET ${KPLUS_LIB_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/*${KPLUS_LIB_NAME}* ${CMAKE_BINARY_DIR}/bin
    COMMAND ${CMAKE_COMMAND} -E copy_directory  ${PROJECT_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/bin/library/include
)
