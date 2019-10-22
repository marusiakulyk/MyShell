cmake_minimum_required(VERSION 3.14)
project(lab3)

set(CMAKE_CXX_STANDARD 14)
find_package(Boost COMPONENTS filesystem system REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
add_executable(myshell main.cpp built_ins.cpp)
target_link_libraries(myshell ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})