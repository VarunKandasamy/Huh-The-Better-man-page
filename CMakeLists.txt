cmake_minimum_required(VERSION 3.31)
project(HuhTheBetterManPage)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCES "src/*.cpp")

include_directories(include)

add_executable(${PROJECT_NAME} ${SOURCES})
