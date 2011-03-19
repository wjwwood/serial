cmake_minimum_required(VERSION 2.4.6)

project(Serial)

# set the default path for built executables to the "bin" directory
IF(NOT DEFINED(EXECUTABLE_OUTPUT_PATH))
    set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
ENDIF(NOT DEFINED(EXECUTABLE_OUTPUT_PATH))
# set the default path for built libraries to the "lib" directory
IF(NOT DEFINED(LIBRARY_OUTPUT_PATH))
    set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
ENDIF(NOT DEFINED(LIBRARY_OUTPUT_PATH))

# add the include folder to the include path
include_directories(${PROJECT_SOURCE_DIR}/include)

# Find Boost
find_package(Boost COMPONENTS system filesystem thread REQUIRED)

# Compile the Serial Library
add_library(serial src/serial.cpp include/serial.h)
target_link_libraries(serial ${Boost_SYSTEM_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY})

# Compile the Test program
add_executable(test_serial src/test_serial.cpp)
# Link the Test program to the Serial library
target_link_libraries(test_serial serial)

# Check for OS X and if so disable kqueue support in asio
IF(CMAKE_SYSTEM_NAME MATCHES Darwin)
    add_definitions(-DBOOST_ASIO_DISABLE_KQUEUE)
ENDIF(CMAKE_SYSTEM_NAME MATCHES Darwin)

# Configure make install
IF(NOT CMAKE_INSTALL_PREFIX)
    SET(CMAKE_INSTALL_PREFIX /usr/local)
ENDIF(NOT CMAKE_INSTALL_PREFIX)

INSTALL(TARGETS serial
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

INSTALL(FILES include/serial.h DESTINATION include)

# INSTALL(FILES ../Findserial.cmake DESTINATION ${CMAKE_ROOT}/Modules/)