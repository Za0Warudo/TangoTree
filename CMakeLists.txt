cmake_minimum_required(VERSION 3.14)
project(RedBlackTree LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch GoogleTest for tests.
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Add the source and executable subdirectories.
add_subdirectory(src)
add_subdirectory(main)

enable_testing()
add_subdirectory(tests)
