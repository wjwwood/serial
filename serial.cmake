macro(build_serial)
## Project Setup
cmake_minimum_required(VERSION 2.4.6)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

project(Serial)

## Configurations

option(SERIAL_BUILD_TESTS "Build all of the Serial tests." OFF)
option(SERIAL_BUILD_EXAMPLES "Build all of the Serial examples." OFF)

# Allow for building shared libs override
IF(NOT BUILD_SHARED_LIBS)
    set(BUILD_SHARED_LIBS OFF)
ENDIF(NOT BUILD_SHARED_LIBS)

# Set the default path for built executables to the "bin" directory
IF(NOT DEFINED(EXECUTABLE_OUTPUT_PATH))
    set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
ENDIF(NOT DEFINED(EXECUTABLE_OUTPUT_PATH))
# set the default path for built libraries to the "lib" directory
IF(NOT DEFINED(LIBRARY_OUTPUT_PATH))
    set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
ENDIF(NOT DEFINED(LIBRARY_OUTPUT_PATH))

## Configure the build system

# Add the include folder to the include path
include_directories(${PROJECT_SOURCE_DIR}/include)

# Add default source files
set(SERIAL_SRCS src/serial.cpp)
# Add default header files
set(SERIAL_HEADERS include/serial/serial.h)

# Find Boost, if it hasn't already been found
IF(NOT Boost_FOUND OR NOT Boost_SYSTEM_FOUND OR NOT Boost_FILESYSTEM_FOUND OR NOT Boost_THREAD_FOUND)
    find_package(Boost COMPONENTS system filesystem thread REQUIRED)
ENDIF(NOT Boost_FOUND OR NOT Boost_SYSTEM_FOUND OR NOT Boost_FILESYSTEM_FOUND OR NOT Boost_THREAD_FOUND)

link_directories(${Boost_LIBRARY_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

set(SERIAL_LINK_LIBS ${Boost_SYSTEM_LIBRARY}
                     ${Boost_FILESYSTEM_LIBRARY}
                     ${Boost_THREAD_LIBRARY})

## Build the Serial Library

# Compile the Library
add_library(serial ${SERIAL_SRCS} ${SERIAL_HEADERS})
target_link_libraries(serial ${SERIAL_LINK_LIBS})
IF( WIN32 )
	target_link_libraries(serial wsock32)
ENDIF( )

# Check for OS X and if so disable kqueue support in asio
IF(CMAKE_SYSTEM_NAME MATCHES Darwin)
    add_definitions(-DBOOST_ASIO_DISABLE_KQUEUE)
ENDIF(CMAKE_SYSTEM_NAME MATCHES Darwin)

## Build Examples

# If asked to
IF(SERIAL_BUILD_EXAMPLES)
    # Compile the Test program
    add_executable(serial_example examples/serial_example.cpp)
    # Link the Test program to the Serial library
    target_link_libraries(serial_example serial)
ENDIF(SERIAL_BUILD_EXAMPLES)

## Build tests

# If asked to
IF(SERIAL_BUILD_TESTS)
    # none yet...
ENDIF(SERIAL_BUILD_TESTS)

## Setup install and uninstall

# Unless asked not to...
IF(NOT SERIAL_DONT_CONFIGURE_INSTALL)
    # Configure make install
    IF(NOT CMAKE_INSTALL_PREFIX)
        SET(CMAKE_INSTALL_PREFIX /usr/local)
    ENDIF(NOT CMAKE_INSTALL_PREFIX)
    
    INSTALL(TARGETS serial
      RUNTIME DESTINATION bin
      LIBRARY DESTINATION lib
      ARCHIVE DESTINATION lib
    )
    
    INSTALL(FILES include/serial/serial.h DESTINATION include/serial)
    
    IF(NOT CMAKE_FIND_INSTALL_PATH)
        set(CMAKE_FIND_INSTALL_PATH ${CMAKE_ROOT})
    ENDIF(NOT CMAKE_FIND_INSTALL_PATH)
    
    INSTALL(FILES Findserial.cmake DESTINATION ${CMAKE_FIND_INSTALL_PATH}/Modules/)
    
    ADD_CUSTOM_TARGET(uninstall @echo uninstall package)
    
    IF (UNIX)
      ADD_CUSTOM_COMMAND(
        COMMENT "uninstall package"
        COMMAND xargs ARGS rm < install_manifest.txt
        
        TARGET  uninstall
      )
    ELSE(UNIX)
      ADD_CUSTOM_COMMAND(
        COMMENT "uninstall only implemented in unix"
        TARGET  uninstall
      )
    ENDIF(UNIX)
ENDIF(NOT SERIAL_DONT_CONFIGURE_INSTALL)
endmacro(build_serial)
