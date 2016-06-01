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

## Include headers
include_directories(include)

install(
	TARGETS ${PROJECT_NAME}
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin
)

install(
  DIRECTORY include/
  DESTINATION include
)
