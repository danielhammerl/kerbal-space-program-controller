cmake_minimum_required(VERSION 3.13)

project(ksp-controller VERSION 1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED true)

add_executable(ksp-controller
        src/main.cpp
)

target_include_directories(ksp-controller PUBLIC)
target_compile_features(ksp-controller PRIVATE cxx_std_11)
target_link_libraries(ksp-controller)

include(FetchContent)

set(BOOST_INCLUDE_LIBRARIES asio)
set(BOOST_ENABLE_CMAKE ON)

FetchContent_Declare(
        Boost
        URL https://github.com/boostorg/boost/releases/download/boost-1.89.0/boost-1.89.0-cmake.tar.xz
        DOWNLOAD_NO_EXTRACT FALSE
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(Boost)

target_include_directories(ksp-controller PUBLIC)

target_link_libraries(ksp-controller PUBLIC Boost::asio)
