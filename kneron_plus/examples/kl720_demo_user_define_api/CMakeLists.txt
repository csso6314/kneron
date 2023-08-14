# build with current *.c/*.cpp plus common source files in parent folder
# executable name is current folder name.

include_directories(${PROJECT_SOURCE_DIR}/app_lib/yolo/include)

get_filename_component(app_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" app_name ${app_name})

file(GLOB local_src
    "*.c"
    "*.cpp"
	)

set(common_src
	../../ex_common/helper_functions.c
	)

add_executable(${app_name}
	${local_src}
    ${common_src})

set(KAPP_LIB_NAME "kapp_yolo")

target_link_libraries(${app_name} ${KAPP_LIB_NAME} ${KPLUS_LIB_NAME} ${USB_LIB} ${MATH_LIB} pthread)
