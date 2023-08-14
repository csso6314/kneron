
project(KDP2)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CMAKE_C_FLAGS "-std=gnu99 -Werror -Wall")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin_kdp2)

add_subdirectory(src)
add_subdirectory(examples)