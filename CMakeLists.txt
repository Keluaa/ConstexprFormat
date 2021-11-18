cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED PROJECT_NAME)
    set(IS_TOP_PROJECT TRUE)
endif()

project(ConstexprFormat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_library(ConstexprFormat INTERFACE const_format.h)

if (IS_TOP_PROJECT)
    include(CTest)
    add_subdirectory(compilation_tests)
    add_subdirectory(tests)
endif()
