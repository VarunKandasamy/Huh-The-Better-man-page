cmake_minimum_required(VERSION 3.31)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# All lib sources except main.cpp
file(GLOB_RECURSE LIB_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")
list(FILTER LIB_SOURCES EXCLUDE REGEX ".*/main\\.cpp$")

file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(huh_tests ${TEST_SOURCES} ${LIB_SOURCES})
target_include_directories(huh_tests PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_compile_features(huh_tests PRIVATE cxx_std_20)
set_target_properties(huh_tests PROPERTIES CXX_EXTENSIONS ON)

target_link_libraries(huh_tests GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(huh_tests)
