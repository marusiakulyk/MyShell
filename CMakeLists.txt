cmake_minimum_required(VERSION 3.14)
project(lab3)

set(CMAKE_CXX_STANDARD 14)
find_package(Boost COMPONENTS filesystem system REQUIRED)
find_library(Readline_LIBRARY
        NAMES readline
        HINTS ${Readline_ROOT_DIR}/lib
        )
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
include_directories(${Readline_INCLUDE_DIR})
set(EXECUTABLE_OUTPUT_PATH  "${CMAKE_BINARY_DIR}/..")
add_executable(myshell main.cpp builtins.h)
target_link_libraries(myshell ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY}  ${Readline_LIBRARY})