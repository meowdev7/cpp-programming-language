cmake_minimum_required(VERSION 3.10)
project(zor VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB_RECURSE SOURCES
    ${CMAKE_SOURCE_DIR}/frontend/*.cpp
    ${CMAKE_SOURCE_DIR}/runtime/*.cpp
    ${CMAKE_SOURCE_DIR}/main.cpp
)

add_executable(zor ${SOURCES})

target_compile_options(zor PRIVATE -Wall -Wextra -pedantic)

target_include_directories(zor PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)
