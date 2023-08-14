cmake_minimum_required(VERSION 2.8.12)
project(example)

include_directories(${hostkdp_SOURCE_DIR}/include)

add_definitions(-DTEST_IMG_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../input_images/")

if(BUILD_OPENCV_EX)
	find_package(OpenCV 3.4 REQUIRED)
	include_directories(${OpenCV_INCLUDE_DIRS})
endif()

#IF USE C++11 or later version,THIS NEED TO BE TRUE,EARLIER NEED BE SET "FALSE" 
set(THREAD_USE_CPP11 "FALSE")
if(THREAD_USE_CPP11)	
	#supprot for earlier versions of C++11
	add_definitions(-DTHREAD_USE_CPP11)
	set(CMAKE_CXX_STANDARD 11)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
	message(STATUS "Build example just for test")
else()
	set(CMAKE_CXX_FLAGS "-std=c++98")
	message(STATUS "Build example just for test c++98")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_FLAGS "${ASAN_FLAGS} ${CMAKE_CXX_FLAGS} -g -Werror -Wall")
set(HOST_LIB "hostkdp")

# auto-scan sub-folders

MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
      LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})

FOREACH(subdir ${SUBDIRS})
  ADD_SUBDIRECTORY(${subdir})
ENDFOREACH()
