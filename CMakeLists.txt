cmake_minimum_required(VERSION 4.0)

project(WalletSimple)


# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(WalletSimple main.cpp)


target_include_directories(WalletSimple PUBLIC "${PROJECT_BINARY_DIR}")