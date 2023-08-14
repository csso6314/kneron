
include_directories(${PROJECT_SOURCE_DIR}/src/include/soc_common)
include_directories(${PROJECT_SOURCE_DIR}/src/include/local)

add_definitions(-fPIC)

# you can use 'make kdp2lib -j4' to build only the library
add_custom_target(kdp2lib DEPENDS ${LIB_NAME})

set(code_src
    kdp2_ll.c
    kdp2_core.c
	kdp2_app.c
    kdp2_postprocess.c
    kneron_nef_reader.c
)

set(LIB_NAME "kdp2")
add_library(${LIB_NAME} SHARED ${code_src})

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
