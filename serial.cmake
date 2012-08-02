macro(build_serial)

  ## Project Setup
  cmake_minimum_required(VERSION 2.4.6)

  if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 NEW)
  endif(COMMAND cmake_policy)

  project(Serial)

  ## Configurations
  # Enable warnings
  # Assuming unix means a gcc style compiler, eg. g++ or clang++.
  IF(UNIX)
      #set(CMAKE_CXX_FLAGS "-Wall -Weffc++ -pedantic -pedantic-errors -Wextra  -Wall -Waggregate-return -Wcast-align -Wcast-qual  -Wchar-subscripts  -Wcomment -Wconversion -Wdisabled-optimization -Wfloat-equal  -Wformat  -Wformat=2 -Wformat-nonliteral -Wformat-security  -Wformat-y2k -Wimplicit  -Wimport  -Winit-self  -Winline -Winvalid-pch   -Wlong-long -Wmissing-braces -Wmissing-field-initializers -Wmissing-format-attribute   -Wmissing-include-dirs -Wmissing-noreturn -Wpacked -Wparentheses  -Wpointer-arith -Wredundant-decls -Wreturn-type -Wsequence-point  -Wshadow -Wsign-compare  -Wstack-protector -Wstrict-aliasing -Wstrict-aliasing=2 -Wswitch  -Wswitch-default -Wswitch-enum -Wtrigraphs  -Wuninitialized -Wunknown-pragmas  -Wunreachable-code -Wunused -Wunused-function  -Wunused-label  -Wunused-parameter -Wunused-value  -Wunused-variable  -Wvariadic-macros -Wvolatile-register-var  -Wwrite-strings")
  ELSEIF(WIN32)
    # Force to always compile with W4
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
      string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()
  endif(UNIX)

  IF(NOT DEFINED BUILD_NO_CLANG)
    option(BUILD_NO_CLANG "Do not use the Clang compiler." OFF)
  ENDIF(NOT DEFINED BUILD_NO_CLANG)

  # Use clang if available
  IF(EXISTS /usr/bin/clang AND NOT BUILD_NO_CLANG)
    set(CMAKE_CXX_COMPILER /usr/bin/clang++)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "")
    set(SERIAL_BUILD_WARNINGS TRUE)
    set(CMAKE_BUILD_TYPE Debug)
  ENDIF(EXISTS /usr/bin/clang AND NOT BUILD_NO_CLANG)

  option(SERIAL_BUILD_TESTS "Build all of the Serial tests." OFF)
  option(SERIAL_BUILD_EXAMPLES "Build all of the Serial examples." OFF)

  # Allow for building shared libs override
  IF(NOT BUILD_SHARED_LIBS)
      set(BUILD_SHARED_LIBS OFF)
  ENDIF(NOT BUILD_SHARED_LIBS)

  # Threading libraries added for mutexs
  FIND_PACKAGE (Threads)

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
  set(SERIAL_SRCS src/serial.cc)
  IF(WIN32)
    list(APPEND SERIAL_SRCS src/impl/win.cc)
  ELSE(WIN32)
    list(APPEND SERIAL_SRCS src/impl/unix.cc)
  ENDIF(WIN32)
  # Add default header files
  set(SERIAL_HEADERS include/serial/serial.h)


  set(OTHER_LIBS "")
  if(UNIX)
    set(OTHER_LIBS util)
  endif(UNIX)

  if(UNIX AND NOT APPLE)
    list(APPEND OTHER_LIBS rt pthread)
  endif(UNIX AND NOT APPLE)
  
  ## Build the Serial Library

  # Compile the Library
  add_library(serial ${SERIAL_SRCS})
  target_link_libraries(serial ${CMAKE_THREAD_LIBS_INIT} ${OTHER_LIBS})

  ## Build Examples

  # If asked to
  IF(SERIAL_BUILD_EXAMPLES)
      # Compile the Serial Test program
      add_executable(serial_example examples/serial_example.cc)
      # Link the Test program to the Serial library
      target_link_libraries(serial_example serial)
  ENDIF(SERIAL_BUILD_EXAMPLES)

  ## Build tests

  # If asked to
  IF(SERIAL_BUILD_TESTS)
      # Find Google Test
      enable_testing()
      find_package(GTest REQUIRED)
      include_directories(${GTEST_INCLUDE_DIRS})

      # Compile the Serial Test program
      add_executable(serial_tests tests/serial_tests.cc)
      # Link the Test program to the serial library
      target_link_libraries(serial_tests ${GTEST_BOTH_LIBRARIES}
                            serial)

      add_test(AllTestsIntest_serial serial_tests)
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
    
      INSTALL(FILES include/serial/serial.h
                    include/serial/v8stdint.h
              DESTINATION include/serial)
    
      IF(NOT CMAKE_FIND_INSTALL_PATH)
          set(CMAKE_FIND_INSTALL_PATH ${CMAKE_ROOT})
      ENDIF(NOT CMAKE_FIND_INSTALL_PATH)
    
      INSTALL(FILES Findserial.cmake
              DESTINATION ${CMAKE_FIND_INSTALL_PATH}/Modules/)
    
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
