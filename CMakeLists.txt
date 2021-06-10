cmake_minimum_required(VERSION 3.5.0)

# Public options and command line configuration
option(ENABLE_TEST_PROGRAM "Build test program" OFF)
option(CATKIN_ENABLE_TESTING "Enable catkin unit tests" ON)


option(USE_CXX_SERIAL "build package name cxx-serial" OFF)
if (USE_CXX_SERIAL)
    set(PKG_NAME cxx-serial)
else ()
    set(PKG_NAME serial)
endif ()
message(STATUS "Building package ${PKG_NAME}")

set(SERIAL_DOCDIR ${CMAKE_INSTALL_PREFIX}/share/doc/${PKG_NAME}
    CACHE STRING "Installation root for doxygen docs."
)
option(DISABLE_CATKIN "Disable build of catkin package and tests" OFF)
if (DISABLE_CATKIN AND "${CATKIN_ENABLE_TESTING}" STREQUAL "" )
    set(CATKIN_ENABLE_TESTING OFF)
endif ()

set(PROJ_SOVERSION 1)
project(${PKG_NAME}
    VERSION 1.2.1
    DESCRIPTION "Cross-platform, Serial Port library written in C++"
    HOMEPAGE_URL "http://wjwwood.io/serial/"
)
include(GNUInstallDirs)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# Locate packages, headers and libraries
if (NOT DISABLE_CATKIN)
    find_package(catkin REQUIRED)
endif ()

find_path(HAVE_STDINT_H NAMES stdint.h)

if (NOT DISABLE_CATKIN)
    # Build the catkin library
    find_library(PTHREAD_LIB NAMES pthread REQUIRED)
    if (PTHREAD_LIB)
        set(PTHREAD_LIBRARIES ${PTHREAD_LIB})
    endif ()
    find_package(Rt)
    if (RT_FOUND)
        set(_RT RT)
    endif ()
    configure_file(package.xml.in ${PROJECT_SOURCE_DIR}/package.xml @ONLY)
    catkin_package(
        LIBRARIES ${PROJECT_NAME}
        INCLUDE_DIRS include
        DEPENDS ${_RT} PTHREAD
    )
    set(CMAKE_INSTALL_LIBDIR ${CATKIN_PACKAGE_LIB_DESTINATION})
    set(CMAKE_INSTALL_BINDIR ${CATKIN_GLOBAL_BIN_DESTINATION})
    set(CMAKE_INSTALL_INCLUDEDIR ${CATKIN_GLOBAL_INCLUDE_DESTINATION})
endif ()

## Sources
set(serial_SRCS src/serial.cc include/serial/serial.h)
if (NOT HAVE_STDINT_H)
    list(APPEND serial_SRCS include/serial/v8stdint.h)
endif ()

if(APPLE)
    list(APPEND serial_SRCS src/impl/unix.cc)
    list(APPEND serial_SRCS src/impl/list_ports/list_ports_osx.cc)
elseif(UNIX)
    # linux
    list(APPEND serial_SRCS src/impl/unix.cc)
    list(APPEND serial_SRCS src/impl/list_ports/list_ports_linux.cc)
else()
    # win32
    list(APPEND serial_SRCS src/impl/win.cc)
    list(APPEND serial_SRCS src/impl/list_ports/list_ports_win.cc)
endif()

set(serial_HEADERS include/serial/serial.h)
if (NOT HAVE_STDINT_H)
    list(APPEND serial_HEADERS include/serial/v8stdint.h)
endif ()

# Build  and link main library
add_library(${PROJECT_NAME} ${serial_SRCS})
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJ_SOVERSION}
    PUBLIC_HEADER "${serial_HEADERS}"
)
target_include_directories(${PROJECT_NAME} PUBLIC include)
if (HAVE_STDINT_H)
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DHAVE_STDINT_H)
endif ()

if (APPLE)
    find_library(IOKIT_LIB IOKit)
    find_library(FOUNDATION_LIB Foundation)
    target_link_libraries(${PROJECT_NAME} ${FOUNDATION_LIB} ${IOKIT_LIB})
elseif (UNIX)
    target_link_libraries(${PROJECT_NAME} ${RT_LIBRARIES} ${PTHREAD_LIBRARIES})
else ()
    target_link_libraries(${PROJECT_NAME} setupapi)
endif ()


## Install main library, possibly the catkin one.
install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)

# Other targets: test program, pkg-config and tests.
if (CATKIN_ENABLE_TESTING)
    include(CTest)
    find_package(GTest REQUIRED)
    enable_testing()
    add_subdirectory(tests)
endif()

if (DISABLE_CATKIN)
    configure_file(serial.pc.in ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.pc @ONLY)
    install(
        FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
    )
endif()

if (ENABLE_TEST_PROGRAM)
    add_executable(serial_example examples/serial_example.cc)
    add_dependencies(serial_example ${PROJECT_NAME})
    target_link_libraries(serial_example ${PROJECT_NAME})
endif()

find_package(Doxygen)
if (DOXYGEN_FOUND AND DOXYGEN_DOT_FOUND)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile)
    add_custom_target(doc ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
    install(DIRECTORY  ${CMAKE_BINARY_DIR}/doc/html
        DESTINATION ${SERIAL_DOCDIR}
    )
endif ()
