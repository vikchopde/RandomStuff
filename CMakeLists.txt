cmake_minimum_required(VERSION 3.10)
project(Timer)

set(CMAKE_CXX_STANDARD 14)

find_package(Boost)
include_directories(${Boost_INCLUDE_DIR})

add_executable(Timer main.cpp)

target_link_libraries(Timer libboost_system.dylib libboost_thread-mt.dylib)