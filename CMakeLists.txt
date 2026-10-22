cmake_minimum_required(VERSION 3.20.0)
project(linalg_tests LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(linalg)

file(GLOB tests_sources CONFIGURE_DEPENDS "source/test_*.cpp")
add_executable(${PROJECT_NAME} ${tests_sources})
target_link_libraries(${PROJECT_NAME} PRIVATE linalg) # подключаем либу из папки (PRIVATE=видна только tests)

# Автоматически скачиваем и подключаем GTest
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)
#find_package(GTest REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE GTest::gtest GTest::gtest_main)


# для себя
add_executable(temp main.cpp)
target_link_libraries(temp PRIVATE linalg)