cmake_minimum_required(VERSION 2.8.3)

project(serial)

find_package(ament_cmake REQUIRED)
if(APPLE)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(FOUNDATION_LIBRARY Foundation)
endif()

## Add serial library
if(APPLE)
add_library(${PROJECT_NAME} SHARED
    src/serial.cc
	src/impl/unix.cc
    src/impl/list_ports/list_ports_osx.cc
)
elseif(UNIX)    
add_library(${PROJECT_NAME} SHARED
    src/serial.cc
	src/impl/unix.cc
	src/impl/list_ports/list_ports_linux.cc
)
endif()


if(APPLE)
    target_link_libraries(${PROJECT_NAME} ${FOUNDATION_LIBRARY} ${IOKIT_LIBRARY})
elseif(UNIX)    
    target_link_libraries(${PROJECT_NAME} rt pthread)
endif()

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
