# build with current *.c/*.cpp plus common source files in parent folder
# executable name is current folder name.

get_filename_component(app_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" app_name ${app_name})

file(GLOB cur_folder_src
    "*.c"
    "*.cpp"
	)

add_executable(${app_name}
	${cur_folder_src}
	${extra_src})

target_link_libraries(${app_name} ${HOST_LIB} ${USB_LIB} pthread)
