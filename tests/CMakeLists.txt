cmake_minimum_required(VERSION 3.14)

add_executable(RedBlackTreeTest ./unit/RedBlackTreeTest.cpp)
add_executable(TangoTreeTest ./unit/TangoTreeTest.cpp)
add_executable(TangoVisualizationTest ./unit/TangoVisualizationTest.cpp)

target_include_directories(RedBlackTreeTest PRIVATE ${CMAKE_SOURCE_DIR}/includes)

target_link_libraries(RedBlackTreeTest PRIVATE gtest_main RedBlackTree)
target_link_libraries(TangoTreeTest PRIVATE gtest_main TangoTree)
target_link_libraries(TangoVisualizationTest PRIVATE gtest_main TangoTree TangoVisualization)

add_test(NAME RedBlackTreeTest COMMAND RedBlackTreeTest)
add_test(NAME TangoTreeTest COMMAND TangoTreeTest)
add_test(NAME TangoVisualizationTest COMMAND TangoVisualizationTest)

# Speed comparison between the trees. See tests/analysis/compare_trees.py.
add_subdirectory(analysis)
