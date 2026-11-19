cmake_minimum_required(VERSION 3.14)

add_executable(MainRBT MainRBT.cpp)
add_executable(MainTT MainTT.cpp)
add_executable(MainTV MainTV.cpp)

target_link_libraries(MainRBT PRIVATE RedBlackTree)
target_link_libraries(MainTT PRIVATE TangoTree)
target_link_libraries(MainTV PRIVATE TangoVisualization)
