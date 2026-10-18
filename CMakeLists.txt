cmake_minimum_required(VERSION 3.4.0)
project(linalg_tests LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

# Автоматически скачиваем и подключаем GTest
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

add_subdirectory(linalg)

add_executable(tests source/test_matrix.cpp)
target_link_libraries(tests PRIVATE linalg) # подключаем либу из папки (PRIVATE=видна только tests)
# линкуем чтобы либа подключалась как любой другой include без указания относительного пути
target_include_directories(linalg PUBLIC include)

#find_package(GTest REQUIRED)
target_link_libraries(tests PRIVATE GTest::gtest GTest::gtest_main)
