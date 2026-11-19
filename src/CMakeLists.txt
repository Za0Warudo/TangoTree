cmake_minimum_required(VERSION 3.14)

add_library(RedBlackTree STATIC RedBlackTree.cpp)
add_library(TangoTree STATIC TangoTree.cpp)
add_library(TangoVisualization STATIC TangoVisualization.cpp)

target_link_libraries(TangoTree PUBLIC RedBlackTree)
target_link_libraries(TangoVisualization PUBLIC RedBlackTree)

target_include_directories(RedBlackTree PUBLIC ${CMAKE_SOURCE_DIR}/includes)
target_include_directories(TangoTree PUBLIC ${CMAKE_SOURCE_DIR}/includes)
target_include_directories(TangoVisualization PUBLIC ${CMAKE_SOURCE_DIR}/includes)
