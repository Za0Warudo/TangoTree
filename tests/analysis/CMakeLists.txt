cmake_minimum_required(VERSION 3.14)

# The benchmark compiles the trees itself, so its times are always optimized, whatever build type the project is configured with.
function(add_tree_benchmark name)
  add_executable(${name}
    TreeBenchmark.cpp
    ${CMAKE_SOURCE_DIR}/src/RedBlackTree.cpp
    ${CMAKE_SOURCE_DIR}/src/TangoTree.cpp
  )
  target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/includes)
  target_compile_definitions(${name} PRIVATE NDEBUG)
  target_compile_options(${name} PRIVATE -O2)
endfunction()

# Measures the time, without the cost of counting.
add_tree_benchmark(TreeBenchmark)

# Counts the node accesses. Its times are not used.
add_tree_benchmark(TreeBenchmarkCounted)
target_compile_definitions(TreeBenchmarkCounted PRIVATE COUNT_NODE_ACCESSES)
