cmake_minimum_required(VERSION 2.8.3)
project(serial)

#Catkin is not required at all in modern cmake, dependencies are handled fine through target_link_libraries.
#We set the default to be on to not break anyone's builds.
option(SERIAL_ENABLE_CATKIN "Enable using the Catkin make extension to cmake (ie for ROS)" ON)
if(${SERIAL_ENABLE_CATKIN})
    # Find catkin
    find_package(catkin REQUIRED)
endif()

if(APPLE)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(FOUNDATION_LIBRARY Foundation)
endif()

if(UNIX AND NOT APPLE)
    # If Linux, add rt and pthread
    set(rt_LIBRARIES rt)
    set(pthread_LIBRARIES pthread)
    if(${SERIAL_ENABLE_CATKIN})
        catkin_package(
            LIBRARIES ${PROJECT_NAME}
            INCLUDE_DIRS include
            DEPENDS rt pthread
        )
    endif()
else()
    # Otherwise normal call
    if(${SERIAL_ENABLE_CATKIN})
        catkin_package(
            LIBRARIES ${PROJECT_NAME}
            INCLUDE_DIRS include
        )
    endif()
endif()

## Sources
set(serial_SRCS
    src/serial.cc
    include/serial/serial.h
    include/serial/v8stdint.h
)
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

## Add serial library
add_library(${PROJECT_NAME} ${serial_SRCS})
if(APPLE)
    target_link_libraries(${PROJECT_NAME} ${FOUNDATION_LIBRARY} ${IOKIT_LIBRARY})
elseif(UNIX)
    target_link_libraries(${PROJECT_NAME} rt pthread)
else()
    target_link_libraries(${PROJECT_NAME} setupapi)
endif()

## Uncomment for example
add_executable(serial_example examples/serial_example.cc)
add_dependencies(serial_example ${PROJECT_NAME})
target_link_libraries(serial_example ${PROJECT_NAME})



if(${SERIAL_ENABLE_CATKIN})
    ## Include headers
    include_directories(include)
    ## Install executable
    install(TARGETS ${PROJECT_NAME}
        ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        RUNTIME DESTINATION ${CATKIN_GLOBAL_BIN_DESTINATION}
    )

    ## Install headers
    install(FILES include/serial/serial.h include/serial/v8stdint.h
      DESTINATION ${CATKIN_GLOBAL_INCLUDE_DESTINATION}/serial)
    ## Tests
    if(CATKIN_ENABLE_TESTING)
        add_subdirectory(tests)
    endif()
else()
    # while some of this uses stuff from newer CMake versions, the code before already ignored 2.8 compat,
    # so we ignore it here as well.
    # see this: https://www.youtube.com/watch?v=m0DwB4OvDXk and associated slides hosted on github to learn more. 
    include(GNUInstallDirs)
    target_include_directories(${PROJECT_NAME} PUBLIC
            $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
    # Install targets
    install(TARGETS ${PROJECT_NAME}
            EXPORT ${PROJECT_NAME}Targets
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                    COMPONENT ${PROJECT_NAME}_RunTime
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
                    COMPONENT ${PROJECT_NAME}_RunTime
                    NAMELINK_COMPONENT ${PROJECT_NAME}_Development
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
                    COMPONENT ${PROJECT_NAME}_Development

            )
    # Install header files
    install(FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/include/serial/serial.h
            ${CMAKE_CURRENT_SOURCE_DIR}/include/serial/v8stdint.h
            DESTINATION
            ${CMAKE_INSTALL_INCLUDEDIR})

    # Install cmake stuff so we can use our target
    install(EXPORT ${PROJECT_NAME}Targets
            FILE ${PROJECT_NAME}Targets.cmake
            NAMESPACE ${PROJECT_NAME}::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
            )
    option(SERIAL_ENABLE_TESTING "Enable testing" OFF)
    if(${SERIAL_ENABLE_TESTING})
        add_subdirectory(tests)
    endif()
endif()

