# Usage:
#
# find_package(serial REQUIRED)
# include_directories(${serial_INCLUDE_DIRS})
# target_link_libraries(<target> serial)

if(serial_CONFIG_INCLUDED)
  return()
endif()
set(serial_CONFIG_INCLUDED TRUE)

get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${SELF_DIR}/serial-targets.cmake)
get_filename_component(serial_INCLUDE_DIRS "${SELF_DIR}/../../../include" ABSOLUTE)
