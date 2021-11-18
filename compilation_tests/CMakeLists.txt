

set(COMPILE_FAIL_SOURCE invalid_formats.cpp)
set(COMPILE_OK_SOURCE correct_formats.cpp)

set(COMPILE_FAIL_TESTS_COUNT 3)
set(COMPILE_OK_TESTS_COUNT 5)


function(compilation_fail_test test_number)
    add_executable(COMPILE_FAIL_${test_number} ${COMPILE_FAIL_SOURCE})
    set_target_properties(COMPILE_FAIL_${test_number} PROPERTIES EXCLUDE_FROM_ALL true EXCLUDE_FROM_DEFAULT_BUILD true)
    target_link_libraries(COMPILE_FAIL_${test_number} ConstexprFormat)
    target_compile_definitions(COMPILE_FAIL_${test_number} PRIVATE TEST_NUMBER=${test_number})

    # Add a test which should try to compile the file and fail
    add_test(NAME COMPILE_FAIL_${test_number}
            COMMAND ${CMAKE_COMMAND} --build . --target COMPILE_FAIL_${test_number} --config $<CONFIGURATION>)
    set_tests_properties(COMPILE_FAIL_${test_number} PROPERTIES WILL_FAIL true)
endfunction()


function(compilation_test test_number)
    add_executable(COMPILE_OK_${test_number} ${COMPILE_OK_SOURCE})
    set_target_properties(COMPILE_OK_${test_number} PROPERTIES EXCLUDE_FROM_ALL true EXCLUDE_FROM_DEFAULT_BUILD true)
    target_link_libraries(COMPILE_OK_${test_number} ConstexprFormat)
    target_compile_definitions(COMPILE_OK_${test_number} PRIVATE TEST_NUMBER=${test_number})

    add_test(NAME COMPILE_OK_${test_number}
            COMMAND ${CMAKE_COMMAND} --build . --target COMPILE_OK_${test_number} --config $<CONFIGURATION>)
endfunction()


foreach(test_number RANGE ${COMPILE_FAIL_TESTS_COUNT})
    compilation_fail_test(${test_number})
endforeach()


foreach(test_number RANGE ${COMPILE_OK_TESTS_COUNT})
    compilation_test(${test_number})
endforeach()
