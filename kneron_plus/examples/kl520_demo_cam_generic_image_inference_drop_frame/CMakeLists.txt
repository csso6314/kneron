# build with current *.c/*.cpp plus common source files in parent folder
# executable name is current folder name.
if (WITH_OPENCV)
    find_package(PkgConfig REQUIRED)
    find_package(OpenCV 3.2.0 REQUIRED)

    set(CMAKE_CXX_STANDARD 11)

    get_filename_component(app_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    string(REPLACE " " "_" app_name ${app_name})

    include_directories(
        ${OpenCV_INCLUDE_DIRS}                          #openCV header
    )

    file(GLOB local_src
        "*.c"
        "*.cpp"
        )

    set(common_src
        ../../ex_common/helper_functions.c
        ../../ex_common/postprocess.c
        )

    add_executable(${app_name}
        ${local_src}
        ${common_src})

    target_link_libraries(${app_name} ${KPLUS_LIB_NAME} ${USB_LIB} ${MATH_LIB} ${OpenCV_LIBS} pthread)
endif ()
