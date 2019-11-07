cmake_minimum_required(VERSION 3.5)
project(serial)

# Add support for C++11
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 11)
endif()

find_package(ament_cmake REQUIRED)

set(INCLUDE_DIRS include ${ament_cmake_INCLUDE_DIRS})
include_directories(${INCLUDE_DIRS})

set(LIBRARY_DIRS ${ament_cmake_LIBRARY_DIRS})

link_directories(${LIBRARY_DIRS})

set(LIBS ${ament_cmake_LIBRARIES})

# Add support for C++11
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 11)
endif()

find_package(ament_cmake REQUIRED)

set(INCLUDE_DIRS include ${ament_cmake_INCLUDE_DIRS})

set(LIBRARY_DIRS ${ament_cmake_LIBRARY_DIRS})

set(LIBS ${ament_cmake_LIBRARIES})

# Find catkin

if(APPLE)
  find_library(IOKIT_LIBRARY IOKit)
  find_library(FOUNDATION_LIBRARY Foundation)
endif()

if(UNIX AND NOT APPLE)
  # If Linux, add rt and pthread
  set(rt_LIBRARIES rt)
  set(pthread_LIBRARIES pthread)
else()
  # Otherwise normal call
endif()

## Sources
set(serial_SRCS src/serial.cc include/serial/serial.h include/serial/v8stdint.h)
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
add_library(${PROJECT_NAME} ${serial_SRCS})
if(APPLE)
  target_link_libraries(${PROJECT_NAME} ${FOUNDATION_LIBRARY} ${IOKIT_LIBRARY})
elseif(UNIX)
  target_link_libraries(${PROJECT_NAME} rt pthread)
else()
  target_link_libraries(${PROJECT_NAME} setupapi)
endif()

## Uncomment for example
add_executable(serial_example examples/serial_example.cc)
add_dependencies(serial_example ${PROJECT_NAME})
target_link_libraries(serial_example ${PROJECT_NAME})

## Include headers

## Install executable
install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib LIBRARY DESTINATION lib)

## Install headers
install(FILES include/serial/serial.h include/serial/v8stdint.h
  DESTINATION include/serial)

## Tests
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

ament_export_dependencies(ament_cmake)
ament_export_include_directories(${INCLUDE_DIRS})
ament_export_libraries(${PROJECT_NAME} ${LIBS})

ament_package()

ament_export_dependencies(ament_cmake)
ament_export_include_directories(${INCLUDE_DIRS})
ament_export_libraries(${PROJECT_NAME} ${LIBS})

ament_package()
