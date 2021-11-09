cmake_minimum_required(VERSION 3.20)

# Are we imported from another project?
if(NOT DEFINED PROJECT_NAME)
    set(IS_TOP_PROJECT TRUE)
endif()

project(ConstexprFormat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_library(ConstexprFormat INTERFACE const_format.h)

if (IS_TOP_PROJECT)
    add_executable(ConstexprFormat_tests main.cpp)
    target_link_libraries(ConstexprFormat_tests ConstexprFormat)
endif()
