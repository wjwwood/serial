cmake_minimum_required(VERSION 3.0)
project(serial-testsuite VERSION 1.0)

file(GLOB TEST_SRCS unit/unix_timer_tests.cc)

if (UNIX)
    if (NOT APPLE)
        add_executable(service_testsuite ${TEST_SRCS})
        target_link_libraries(service_testsuite PUBLIC serial ${MAIN_LIBS})
    endif ()
endif ()
