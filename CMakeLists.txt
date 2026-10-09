cmake_minimum_required(VERSION 3.4.0)
project(linalg_tests LANGUAGES CXX)

add_subdirectory(linalg)

# ¿¿¿
include(FetchContent)
FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
FetchContent_MakeAvailable(googletest)
# ???

add_executable(tests source/test_matrix.cpp)
target_link_libraries(tests PRIVATE linalg GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(tests) # ???