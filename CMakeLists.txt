cmake_minimum_required(VERSION 3.5)
project(serial)

# Default to C99
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

# Add support for C++11
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)

if(APPLE)
  find_library(IOKIT_LIBRARY IOKit)
  find_library(FOUNDATION_LIBRARY Foundation)
endif()

if(UNIX AND NOT APPLE)
  set(RT_LIBRARIES rt)
  set(PTHREAD_LIBRARIES pthread)
endif()

set(SERIAL_SRCS
  src/serial.cc)

if(APPLE)
  list(APPEND SERIAL_SRCS src/impl/list_ports/list_ports_osx.cc)
  list(APPEND SERIAL_SRCS src/impl/unix.cc)
elseif(UNIX)
  list(APPEND SERIAL_SRCS src/impl/list_ports/list_ports_linux.cc)
  list(APPEND SERIAL_SRCS src/impl/unix.cc)
else()
  list(APPEND SERIAL_SRCS src/impl/list_ports/list_ports_win.cc)
  list(APPEND SERIAL_SRCS src/impl/win.cc)
endif()

add_library(${PROJECT_NAME} SHARED
  ${SERIAL_SRCS})

if(APPLE)
  target_link_libraries(${PROJECT_NAME} ${FOUNDATION_LIBRARY} ${IOKIT_LIBRARY})
elseif(UNIX)
  target_link_libraries(${PROJECT_NAME} rt pthread)
else()
  target_link_libraries(${PROJECT_NAME} setupapi)
endif()

target_include_directories(${PROJECT_NAME}
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

install(
  DIRECTORY include/
  DESTINATION include)

install(
  TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_cmake_gmock REQUIRED)

  if(UNIX)
    ament_add_gmock(${PROJECT_NAME}-test
      tests/unix_serial_tests.cc)

    if(TARGET ${PROJECT_NAME}-test)
      target_link_libraries(${PROJECT_NAME}-test ${PROJECT_NAME} ${Boost_LIBRARIES})

      if(NOT APPLE)
        target_link_libraries(${PROJECT_NAME}-test util)
      endif()
    endif()
  endif()

  if(NOT APPLE)  # these tests are unreliable on macOS
    ament_add_gmock(${PROJECT_NAME}-test-timer
      tests/unit/unix_timer_tests.cc)
    target_link_libraries(${PROJECT_NAME}-test-timer ${PROJECT_NAME})
  endif()
endif()

ament_package()
