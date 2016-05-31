cmake_minimum_required(VERSION 2.8.3)
project(serial)


## Sources
set(serial_SRCS
    src/serial.cc
    include/serial/serial.h
    include/serial/v8stdint.h
)

# If unix
list(APPEND serial_SRCS src/impl/unix.cc)
list(APPEND serial_SRCS src/impl/list_ports/list_ports_linux.cc)

## Add serial library
add_library(${PROJECT_NAME} SHARED ${serial_SRCS})
	target_link_libraries(${PROJECT_NAME} rt pthread)

add_executable(serial_example examples/serial_example.cc)
add_dependencies(serial_example ${PROJECT_NAME})
target_link_libraries(serial_example ${PROJECT_NAME})

## Include headers
include_directories(include)

## Install executable
install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION /usr/lib
    LIBRARY DESTINATION /usr/lib
)

## Install headers
install(FILES include/serial/serial.h include/serial/v8stdint.h
  DESTINATION /usr/include/serial)

