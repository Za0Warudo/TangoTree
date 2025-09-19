cmake_minimum_required(VERSION 3.31)
project(TangoTree)

set(CMAKE_CXX_STANDARD 20)

add_executable(TangoTree main.cpp
        RedBlackTree.cpp
        RedBlackTree.h
        TangoTree.cpp
        TangoTree.h)
