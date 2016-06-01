cmake_minimum_required(VERSION 2.8.3)

project(serial)

find_package(ament_cmake REQUIRED)

## Add serial library
add_library(${PROJECT_NAME} SHARED
    src/serial.cc
	src/impl/unix.cc
	src/impl/list_ports/list_ports_linux.cc
)

target_link_libraries(${PROJECT_NAME} rt pthread)

ament_export_libraries(${PROJECT_NAME})

ament_package()

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