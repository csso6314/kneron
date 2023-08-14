# build with current *.c/*.cpp plus common source files in parent folder
# executable name is current folder name.
if (WITH_DFUT)
    set(CMAKE_CXX_STANDARD 11)

    get_filename_component(app_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    string(REPLACE " " "_" app_name ${app_name})

    if (WIN32)
        include_directories("${PROJECT_SOURCE_DIR}/thirdparty/windows/include")
    endif()

    include_directories("${PROJECT_SOURCE_DIR}/src/include/local")
    include_directories("${PROJECT_SOURCE_DIR}/src/include/soc_common")
    include_directories("${PROJECT_SOURCE_DIR}/tools/DFUT/DFUT_core")

    file(GLOB local_src
        "*.c"
        "*.cpp"
        )

    set(dfut_src
        ../DFUT_core/KneronDFUT.cpp
        )
    
    add_executable(${app_name}
        ${local_src}
        ${dfut_src})

    target_link_libraries(${app_name} ${KPLUS_LIB_NAME} ${USB_LIB} pthread)
endif ()
