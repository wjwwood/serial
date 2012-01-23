macro(build_serial)
cmake_minimum_required(VERSION 2.4.6)
include($ENV{ROS_ROOT}/core/rosbuild/rosbuild.cmake)

# Set the build type.  Options are:
#  Coverage       : w/ debug symbols, w/o optimization, w/ code-coverage
#  Debug          : w/ debug symbols, w/o optimization
#  Release        : w/o debug symbols, w/ optimization
#  RelWithDebInfo : w/ debug symbols, w/ optimization
#  MinSizeRel     : w/o debug symbols, w/ optimization, stripped binaries
set(ROS_BUILD_TYPE RelWithDebInfo)

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
  list(APPEND SERIAL_SRCS src/impl/windows.cc)
endif(UNIX)
list(APPEND SERIAL_SRCS src/serial_listener.cc)

# Build the serial library
rosbuild_add_library(${PROJECT_NAME} ${SERIAL_SRCS})

# Add boost dependencies
rosbuild_add_boost_directories()
rosbuild_link_boost(${PROJECT_NAME} system filesystem thread)

# Build example
rosbuild_add_executable(serial_example examples/serial_example.cc)
target_link_libraries(serial_example ${PROJECT_NAME})

rosbuild_add_executable(serial_listener_example
  examples/serial_listener_example.cc)
target_link_libraries(serial_listener_example ${PROJECT_NAME})

# Create unit tests
rosbuild_add_gtest(serial_tests tests/serial_tests.cc)
target_link_libraries(serial_tests ${PROJECT_NAME})
rosbuild_add_gtest(serial_listener_tests tests/serial_listener_tests.cc)
target_link_libraries(serial_listener_tests ${PROJECT_NAME})

endmacro(build_serial)
