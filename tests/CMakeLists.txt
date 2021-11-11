
add_executable(FormatTests
        doctest.h
        digits_tests.cpp)
target_link_libraries(FormatTests ConstexprFormat)
