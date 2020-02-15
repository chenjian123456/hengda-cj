cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(Hengda)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

file(COPY   ${CMAKE_SOURCE_DIR}/graph.config
            ${CMAKE_SOURCE_DIR}/models/ssd_512.om
            DESTINATION ${CMAKE_BINARY_DIR})

# DDK
find_package(DDK REQUIRED)
include_directories(${DDK_INCLUDE_DIRS} ${DDK_CEREAL_INCLUDE_DIRS} ${DDK_PROTOBUF_INCLUDE_DIRS})

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/Common)

add_subdirectory(DeviceCMakefile)
add_subdirectory(HostCMakefile)