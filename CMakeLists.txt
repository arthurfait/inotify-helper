cmake_minimum_required(VERSION 2.8)
project(raw_test)

set(RAW_SOURCES
    raw_test.cc
)

add_executable(raw_test ${RAW_SOURCES})

project(raw_test)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(HELPER_SOURCES
    helper_test.cc
)


add_executable(helper_test ${HELPER_SOURCES})
target_link_libraries(helper_test pthread)
