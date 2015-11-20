cmake_minimum_required(VERSION 3.3)
project(completed)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    cs2123p4.c
    cs2123p4.h
    cs2123p4_DS.c
    cs2123p4_helper.c)

add_executable(completed ${SOURCE_FILES})