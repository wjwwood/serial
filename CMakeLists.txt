cmake_minimum_required(VERSION 2.8.3)
project(serial)

if(APPLE)
	find_library(IOKIT_LIBRARY IOKit)
	find_library(FOUNDATION_LIBRARY Foundation)
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
add_library(${PROJECT_NAME} STATIC ${serial_SRCS})
if(APPLE)
	target_link_libraries(${PROJECT_NAME} ${FOUNDATION_LIBRARY} ${IOKIT_LIBRARY})
elseif(UNIX)
	target_link_libraries(${PROJECT_NAME} rt pthread)
else()
	target_link_libraries(${PROJECT_NAME} setupapi)
endif()

## Add example project
add_executable(serial_example examples/serial_example.cc)
add_dependencies(serial_example ${PROJECT_NAME})
target_link_libraries(serial_example ${PROJECT_NAME})

## Include headers
include_directories(include)

## Install
set(INSTALL_LIB_DIR lib)
set(INSTALL_INCLUDE_DIR include)
set(INSTALL_CMAKE_DIR share/serial/cmake)

## Install executable
install(TARGETS ${PROJECT_NAME}
    DESTINATION ${INSTALL_LIB_DIR}
    EXPORT ${PROJECT_NAME}-targets
)

## Install headers
install(FILES include/serial/serial.h include/serial/v8stdint.h
    DESTINATION ${INSTALL_INCLUDE_DIR}/serial)

## Install CMake files
install(EXPORT ${PROJECT_NAME}-targets DESTINATION ${INSTALL_CMAKE_DIR})

install(FILES ${CMAKE_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake ${CMAKE_SOURCE_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake
        DESTINATION ${INSTALL_CMAKE_DIR})

## Tests
# FIXME
#if(CATKIN_ENABLE_TESTING)
#    add_subdirectory(tests)
#endif()
