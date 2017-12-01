cmake_minimum_required(VERSION 3.9)
project(ArmorDetection2_0)

set(CMAKE_CXX_STANDARD 11)

add_executable(ArmorDetection2_0 main.cpp ArmorDetection.cpp ArmorDetection.hpp)

FIND_PACKAGE(OpenCV REQUIRED)
target_link_libraries(ArmorDetection2_0 ${OpenCV_LIBS})