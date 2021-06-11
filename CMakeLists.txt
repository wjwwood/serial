cmake_minimum_required(VERSION 2.8.3)

# Either this or bump cmake_minimum_required to 3.0+
# Allow us to use Version in project() call : https://cmake.org/cmake/help/v3.0/policy/CMP0048.html#policy:CMP0048
cmake_policy(SET CMP0048 NEW)

project(serial VERSION 1.2.1)


## Options 

# Use Catkin
option(USE_CATKIN on)
# Build serial sample 
option(BUILD_SAMPLE on)


# Find catkin
if(USE_CATKIN)
    find_package(catkin REQUIRED)

    if(UNIX AND NOT APPLE)
    # If Linux, add rt and pthread
    set(rt_LIBRARIES rt)
    set(pthread_LIBRARIES pthread)
    catkin_package(
        LIBRARIES ${PROJECT_NAME}
        INCLUDE_DIRS include
        DEPENDS rt pthread
    )
    else()
        # Otherwise normal call
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
    find_library(IOKIT_LIBRARY IOKit)
    find_library(FOUNDATION_LIBRARY Foundation)

	target_link_libraries(${PROJECT_NAME} ${FOUNDATION_LIBRARY} ${IOKIT_LIBRARY})
elseif(UNIX)
	target_link_libraries(${PROJECT_NAME} rt pthread)
else()
	target_link_libraries(${PROJECT_NAME} setupapi)
endif()

## Include headers
target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)


## Example
if(BUILD_SAMPLE)
    add_executable(serial_example examples/serial_example.cc)
    add_dependencies(serial_example ${PROJECT_NAME})
    target_link_libraries(serial_example ${PROJECT_NAME})
endif()


## Export 

# Set postfix names for exporting
set_target_properties(${PROJECT_NAME} PROPERTIES
    RELEASE_POSTFIX ""
    RELWITHDEBINFO_POSTFIX "-rd"
    MINSIZEREL_POSTFIX "-mr"
    DEBUG_POSTFIX "-d")  


if(USE_CATKIN)
    ## Install headers
    install(FILES include/serial/serial.h include/serial/v8stdint.h
        DESTINATION ${CATKIN_GLOBAL_INCLUDE_DESTINATION}/serial)
    ## Install executable
    install(TARGETS ${PROJECT_NAME}
        ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        RUNTIME DESTINATION ${CATKIN_GLOBAL_BIN_DESTINATION}
    )
else()
    set(EXPORT_TARGET_NAME serialTargets)

    install(EXPORT ${EXPORT_TARGET_NAME} DESTINATION cmake/)
    # Include module with fuction 'write_basic_package_version_file' and 'configure_package_config_file'
    include(CMakePackageConfigHelpers)

    set(OUTPUT_CMAKE_EXPORT_GENERATED_FILES ${CMAKE_BINARY_DIR}/CMakeExportConfig)
    # Configure '<PROJECT-NAME>ConfigVersion.cmake'
    write_basic_package_version_file(${OUTPUT_CMAKE_EXPORT_GENERATED_FILES}/${PROJECT_NAME}Config-version.cmake
        # <AnyNewerVersion|SameMajorVersion|SameMinorVersion|ExactVersion> 
        # Careful : SameMinorVersion and ExactVersion were introduced in CMake 3.11
        COMPATIBILITY SameMajorVersion
    )

    # Configure '<PROJECT-NAME>Config.cmake'
    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
        "${OUTPUT_CMAKE_EXPORT_GENERATED_FILES}/${PROJECT_NAME}Config.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake"
    )

    # Install config file generated
    install(DIRECTORY ${OUTPUT_CMAKE_EXPORT_GENERATED_FILES}/ 
        DESTINATION cmake/)


    ## Install headers
    install(FILES include/serial/serial.h include/serial/v8stdint.h
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/serial)

   ## Install executable
    install(TARGETS ${PROJECT_NAME} EXPORT ${EXPORT_TARGET_NAME}
        INCLUDES DESTINATION $<INSTALL_INTERFACE:include/>
        ARCHIVE DESTINATION "lib" COMPONENT ${PROJECT_NAME}
    )
endif()


## Tests
if(CATKIN_ENABLE_TESTING)
    add_subdirectory(tests)
endif()