# build with current *.c/*.cpp plus common source files in parent folder
# executable name is current folder name.

get_filename_component(app_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" app_name ${app_name})

file(GLOB local_src
    "*.c"
    "*.cpp"
	)

set(common_src
	../common/helper_functions.c
	)

add_executable(${app_name}
	${local_src}
    ${common_src})

target_link_libraries(${app_name} ${KDP2_LIB} ${USB_LIB} ${MATH_LIB} pthread)
