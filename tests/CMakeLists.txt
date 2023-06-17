if(UNIX)
    find_package(GTest REQUIRED)

    add_executable(${PROJECT_NAME}-test unix_serial_tests.cc)
    target_link_libraries(${PROJECT_NAME}-test ${PROJECT_NAME} GTest::GTest)
    if(NOT APPLE)
        target_link_libraries(${PROJECT_NAME}-test util)
    endif()
    add_test("${PROJECT_NAME}-test-gtest" ${PROJECT_NAME}-test)

    if(NOT APPLE)  # these tests are unreliable on macOS
      add_executable(${PROJECT_NAME}-test-timer unit/unix_timer_tests.cc)
      target_link_libraries(${PROJECT_NAME}-test-timer ${PROJECT_NAME} GTest::GTest)
      add_test("${PROJECT_NAME}-test-timer-gtest" ${PROJECT_NAME}-test-timer)
    endif()
    # Boost is only required for tests/proof_of_concepts which are not enabled by default
    # find_package(Boost REQUIRED)
endif()
