cmake_minimum_required(VERSION 3.14)

add_executable(RedBlackTreeTest RedBlackTreeTest.cpp)

target_include_directories(RedBlackTreeTest PRIVATE ${CMAKE_SOURCE_DIR}/includes)

target_link_libraries(RedBlackTreeTest PRIVATE gtest_main RedBlackTree)

add_test(NAME RedBlackTreeTest COMMAND RedBlackTreeTest)
