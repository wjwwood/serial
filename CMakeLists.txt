cmake_minimum_required(VERSION 3.22)
project(serial LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(${PROJECT_NAME} STATIC)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

set(serial_SRCS "")
set(serial_LIBS "")
if(APPLE)
    # If OSX
    list(APPEND serial_SRCS src/impl/unix.cc)
    list(APPEND serial_SRCS src/impl/list_ports/list_ports_osx.cc)
elseif(UNIX)
    # If unix
    list(APPEND serial_SRCS src/impl/unix.cc)
    list(APPEND serial_SRCS src/impl/list_ports/list_ports_linux.cc)
else()
    # If windows
    list(APPEND serial_SRCS src/impl/win.cc)
    list(APPEND serial_SRCS src/impl/list_ports/list_ports_win.cc)
endif()

if(APPLE)
    list(APPEND serial_LIBS ${FOUNDATION_LIBRARY} ${IOKIT_LIBRARY})
elseif(UNIX)
    list(APPEND serial_LIBS rt pthread)
else()
    list(APPEND serial_LIBS setupapi)
endif()


target_sources(${PROJECT_NAME}
    PRIVATE
        src/serial.cc
        ${serial_SRCS}
    PUBLIC
        include/serial/serial.h
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC

    PRIVATE
        ${serial_LIBS}
)

target_include_directories(
  ${PROJECT_NAME}

  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
set_property(TARGET  ${PROJECT_NAME} PROPERTY PUBLIC_HEADER include/serial/serial.h)

install(TARGETS  ${PROJECT_NAME})
