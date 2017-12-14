# This is just a list for the CLion IDE
# See README instead

cmake_minimum_required(VERSION 3.8)
project(bakefont3)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
    example.c
    bakefont3.h
    bakefont3.c
        README.md)

add_executable(bakefont3 ${SOURCE_FILES})
