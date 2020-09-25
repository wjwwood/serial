if (UNIX AND NOT DISABLE_CATKIN)
    catkin_add_gtest(${PROJECT_NAME}-test unix_serial_tests.cc)
    target_link_libraries(${PROJECT_NAME}-test ${PROJECT_NAME} ${Boost_LIBRARIES})
    if(NOT APPLE)
        target_link_libraries(${PROJECT_NAME}-test util)
    endif()

    if(NOT APPLE)  # these tests are unreliable on macOS
        catkin_add_gtest(${PROJECT_NAME}-test-timer unit/unix_timer_tests.cc)
        target_link_libraries(${PROJECT_NAME}-test-timer ${PROJECT_NAME})
    endif()
elseif (UNIX)

    add_executable(serial_test
        unix_serial_tests.cc
    )
    target_link_libraries(serial_test
        ${PROJECT_NAME} gtest gtest_main pthread
    )
    set_target_properties(serial_test
        PROPERTIES
            CXX_STANDARD 11
            CXX_STANDARD_REQUIRED YES
    )
    if (NOT APPLE)
        target_link_libraries(serial_test util)
    endif ()

    add_executable(timer_test
        unit/unix_timer_tests.cc
    )
    target_link_libraries(timer_test
        ${PROJECT_NAME} gtest gtest_main pthread
    )
    set_target_properties(timer_test
        PROPERTIES
            CXX_STANDARD 11
            CXX_STANDARD_REQUIRED YES
    )

    add_test(NAME serial_test COMMAND serial_test)
    add_test(NAME timer_test COMMAND timer_test)
endif()
