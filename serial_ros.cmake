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

# Check for OS X and if so disable kqueue support in asio
IF(CMAKE_SYSTEM_NAME MATCHES Darwin)
    add_definitions(-DBOOST_ASIO_DISABLE_KQUEUE)
ENDIF(CMAKE_SYSTEM_NAME MATCHES Darwin)

# Build the serial library
rosbuild_add_library(serial src/serial.cpp include/serial/serial.h)

# Add boost dependencies
rosbuild_add_boost_directories()
rosbuild_link_boost(serial system filesystem thread)

# Build example
rosbuild_add_executable(serial_example examples/serial_example.cpp)
target_link_libraries(serial_example serial)

endmacro(build_serial)
