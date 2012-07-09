macro(build_serial)

  cmake_minimum_required(VERSION 2.4.6)
  include($ENV{ROS_ROOT}/core/rosbuild/rosbuild.cmake)

  # Set the build type.  Options are:
  #  Coverage       : w/ debug symbols, w/o optimization, w/ code-coverage
  #  Debug          : w/ debug symbols, w/o optimization
  #  Release        : w/o debug symbols, w/ optimization
  #  RelWithDebInfo : w/ debug symbols, w/ optimization
  #  MinSizeRel     : w/o debug symbols, w/ optimization, stripped binaries
  set(ROS_BUILD_TYPE Debug)

  rosbuild_init()

  #set the default path for built executables to the "bin" directory
  set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
  #set the default path for built libraries to the "lib" directory
  set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

  include_directories(include)

  set(SERIAL_SRCS src/serial.cc)
  if(UNIX)
    list(APPEND SERIAL_SRCS src/impl/unix.cc)
  else(UNIX)
    list(APPEND SERIAL_SRCS src/impl/winows.cc)
  endif(UNIX)

  # Build the serial library
  rosbuild_add_library(${PROJECT_NAME} ${SERIAL_SRCS})

  # Collect Link Libraries
  set(SERIAL_LINK_LIBS ${PROJECT_NAME})
  if(UNIX AND NOT APPLE)
    list(APPEND SERIAL_LINK_LIBS rt pthread util)
  endif(UNIX AND NOT APPLE)

  # Build example
  rosbuild_add_executable(serial_example examples/serial_example.cc)
  target_link_libraries(serial_example ${SERIAL_LINK_LIBS})

  # Create unit tests
  rosbuild_add_gtest(serial_tests tests/serial_tests.cc)
  target_link_libraries(serial_tests ${PROJECT_NAME} ${SERIAL_LINK_LIBS})

endmacro(build_serial)
