cmake_minimum_required(VERSION 3.5.0)


# Find catkin
find_package(catkin REQUIRED)

set(PROJ_SOVERSION 1)
project(serial
    VERSION 1.2.1
    DESCRIPTION "Cross-platform, Serial Port library written in C++"
    HOMEPAGE_URL "http://wjwwood.io/serial/"
)

if(APPLE)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(FOUNDATION_LIBRARY Foundation)
endif()

# Public options and command line configuration
option(ENABLE_TEST_PROGRAM "Build test program" OFF)

if(UNIX AND NOT APPLE)
    # If Linux, add rt and pthread
    set(rt_LIBRARIES rt)
    set(pthread_LIBRARIES pthread)
    catkin_package(
        LIBRARIES ${PROJECT_NAME}
        INCLUDE_DIRS include
        DEPENDS rt pthread
    )
else()
    # Otherwise normal call
    catkin_package(
        LIBRARIES ${PROJECT_NAME}
        INCLUDE_DIRS include
    )
endif()

## Sources
set(serial_SRCS
    src/serial.cc
    include/serial/serial.h
    include/serial/v8stdint.h
)
if(APPLE)
    # If OSX
    list(APPEND serial_SRCS src/impl/unix.cc)
    list(APPEND serial_SRCS src/impl/list_ports/list_ports_osx.cc)
elseif(UNIX)
    # If unix
    list(APPEND serial_SRCS src/impl/unix.cc)
    list(APPEND serial_SRCS src/impl/list_ports/list_ports_linux.cc)
else()
    # If windows
    list(APPEND serial_SRCS src/impl/win.cc)
    list(APPEND serial_SRCS src/impl/list_ports/list_ports_win.cc)
endif()

## Add serial library
set(serial_HEADERS
    include/serial/serial.h
    include/serial/v8stdint.h
)
# Build, link and install main library
add_library(${PROJECT_NAME} ${serial_SRCS})
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJ_SOVERSION}
    PUBLIC_HEADER "${serial_HEADERS}"
)
target_include_directories(${PROJECT_NAME} PUBLIC include)

if(APPLE)
    target_link_libraries(${PROJECT_NAME} ${FOUNDATION_LIBRARY} ${IOKIT_LIBRARY})
elseif(UNIX)
    target_link_libraries(${PROJECT_NAME} rt pthread)
else()
    target_link_libraries(${PROJECT_NAME} setupapi)
endif()

## Include headers
include_directories(include)

## Install executable
install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
    LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
    RUNTIME DESTINATION ${CATKIN_GLOBAL_BIN_DESTINATION}
    PUBLIC_HEADER DESTINATION
        ${CATKIN_GLOBAL_INCLUDE_DESTINATION}/${PROJECT_NAME}
)


# Other targets: test program, pkg-config and tests.
if(CATKIN_ENABLE_TESTING)
    add_subdirectory(tests)
endif()

if (ENABLE_TEST_PROGRAM)
    add_executable(serial_example examples/serial_example.cc)
    add_dependencies(serial_example ${PROJECT_NAME})
    target_link_libraries(serial_example ${PROJECT_NAME})
endif()
