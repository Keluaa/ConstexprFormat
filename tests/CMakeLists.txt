
add_executable(FormatTests
        doctest.h
		tests_main.cpp
        digits_tests.cpp
		string_tests.cpp
		floats_tests.cpp)
target_link_libraries(FormatTests ConstexprFormat)

add_executable(FloatBruteforceTests
		float_bruteforce.cpp)
target_link_libraries(FloatBruteforceTests ConstexprFormat)
