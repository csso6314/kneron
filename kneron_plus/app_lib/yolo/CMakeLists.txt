
include_directories(${PROJECT_SOURCE_DIR}/include
                    ${CMAKE_CURRENT_SOURCE_DIR}/include
                    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/include)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/library/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/library/lib)

set(LIB_NAME "kapp_yolo")
add_definitions(-fPIC)
add_library(${LIB_NAME} SHARED
    src/kp_app_yolo.c
    ../utils/src/app_helper.c
)
target_link_libraries(${LIB_NAME} ${KPLUS_LIB_NAME})

# copy headers and so/dll
add_custom_command(
    TARGET ${LIB_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/*${LIB_NAME}* ${CMAKE_BINARY_DIR}/bin
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h ${CMAKE_BINARY_DIR}/bin/library/include
)
