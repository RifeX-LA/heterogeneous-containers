cmake_minimum_required(VERSION 3.26)
project(heterogeneous_containers)

set(CMAKE_CXX_STANDARD 23)

set(BOOST_ROOT "C:/")
set (Boost_NO_SYSTEM_PATHS ON)
set (Boost_USE_MULTITHREADED ON)
set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_STATIC_RUNTIME OFF)
set (BOOST_ALL_DYN_LINK OFF)

find_package(Boost 1.81.0 REQUIRED)

include_directories(${Boost_INCLUDE_DIR} .)

add_executable(heterogeneous_containers main.cpp)
