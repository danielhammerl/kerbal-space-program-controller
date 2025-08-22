cmake_minimum_required(VERSION 3.13)

project(ksp-controller VERSION 1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED true)

add_executable(ksp-controller src/main.cpp)

target_include_directories(ksp-controller PUBLIC)
target_compile_features(ksp-controller PRIVATE cxx_std_11)
target_link_libraries(ksp-controller)
