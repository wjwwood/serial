cmake_minimum_required(VERSION 2.4.6)

if(DEFINED $ENV{ROS_ROOT})
  # Build with ROS
  include(serial_ros.cmake)
  build_serial()
else(DEFINED $ENV{ROS_ROOT})
  # Build normally
  include(serial.cmake)
  build_serial()
endif(DEFINED $ENV{ROS_ROOT})