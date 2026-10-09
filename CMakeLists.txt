cmake_minimum_required(VERSION 3.4.0)
project(linalg_tests LANGUAGES CXX)

add_subdirectory(linalg)

add_executable(tests source/test_matrix.cpp)
target_link_libraries(tests PRIVATE linalg)

include(GoogleTest)