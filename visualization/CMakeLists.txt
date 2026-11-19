cmake_minimum_required(VERSION 3.14)
project(TangoVisualizationPage LANGUAGES CXX)

# Compiles the Tango Tree visualization to WebAssembly, as the file tango.js in this directory. The page index.html loads it.
#
# This is a separate project from the one at the repository root, because it needs the Emscripten compiler. From the repository root, run:
#
#   emcmake cmake -S visualization -B build/wasm
#   cmake --build build/wasm

if(NOT EMSCRIPTEN)
  message(FATAL_ERROR "This project must be configured with Emscripten: emcmake cmake -S visualization -B build/wasm")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(tango
  ${ROOT}/src/RedBlackTree.cpp
  ${ROOT}/src/TangoVisualization.cpp
  ${ROOT}/main/MainTV.cpp
)
target_include_directories(tango PRIVATE ${ROOT}/includes)
target_compile_definitions(tango PRIVATE NDEBUG)
target_compile_options(tango PRIVATE -O2)

# - SINGLE_FILE puts the WebAssembly code inside tango.js, so the page also works when index.html is opened directly from the disk.
# - MODULARIZE and EXPORT_NAME make tango.js define one function, createTangoModule, instead of many global names.
# - There is no main function: the page calls the three exported functions through cwrap.
target_link_options(tango PRIVATE
  -O2
  --no-entry
  -sSINGLE_FILE=1
  -sMODULARIZE=1
  -sEXPORT_NAME=createTangoModule
  -sENVIRONMENT=web,node
  -sALLOW_MEMORY_GROWTH=1
  -sEXPORTED_FUNCTIONS=_tvInit,_tvSearch,_tvState
  -sEXPORTED_RUNTIME_METHODS=cwrap
)

set_target_properties(tango PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  OUTPUT_NAME tango
  SUFFIX .js
)
