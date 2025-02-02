cmake_minimum_required(VERSION 2.8.3)
project(serial)

if(WIN32)
    # Windows specific settings
    set(serial_SRCS
        src/serial.cc
        include/serial/serial.h
        include/serial/v8stdint.h
        src/impl/win.cc
        src/impl/list_ports/list_ports_win.cc
    )

    add_library(${PROJECT_NAME} ${serial_SRCS})
    target_link_libraries(${PROJECT_NAME} setupapi)

    # Uncomment for example
    add_executable(serial_example examples/serial_example.cc)
    add_dependencies(serial_example ${PROJECT_NAME})
    target_link_libraries(serial_example ${PROJECT_NAME})

    # Include headers
    include_directories(include)

    # Install executable
    install(TARGETS ${PROJECT_NAME}
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
    )

    # Install headers
    install(FILES include/serial/serial.h include/serial/v8stdint.h
       DESTINATION include/serial)

else()
    # Non-Windows specific settings (e.g., Linux, macOS)
    find_package(catkin REQUIRED)

    if(APPLE)
        find_library(IOKIT_LIBRARY IOKit)
        find_library(FOUNDATION_LIBRARY Foundation)
    endif()

    if(UNIX AND NOT APPLE)
        set(rt_LIBRARIES rt)
        set(pthread_LIBRARIES pthread)
        catkin_package(
            LIBRARIES ${PROJECT_NAME}
            INCLUDE_DIRS include
            DEPENDS rt pthread
        )
    else()
        catkin_package(
            LIBRARIES ${PROJECT_NAME}
            INCLUDE_DIRS include
        )
    endif()

    set(serial_SRCS
        src/serial.cc
        include/serial/serial.h
        include/serial/v8stdint.h
    )

    if(APPLE)
        list(APPEND serial_SRCS src/impl/unix.cc)
        list(APPEND serial_SRCS src/impl/list_ports/list_ports_osx.cc)
    elseif(UNIX)
        list(APPEND serial_SRCS src/impl/unix.cc)
        list(APPEND serial_SRCS src/impl/list_ports/list_ports_linux.cc)
    endif()

    add_library(${PROJECT_NAME} ${serial_SRCS})

    if(APPLE)
        target_link_libraries(${PROJECT_NAME} ${FOUNDATION_LIBRARY} ${IOKIT_LIBRARY})
    elseif(UNIX)
        target_link_libraries(${PROJECT_NAME} rt pthread)
    endif()

    add_executable(serial_example examples/serial_example.cc)
    add_dependencies(serial_example ${PROJECT_NAME})
    target_link_libraries(serial_example ${PROJECT_NAME})

    include_directories(include)

    install(TARGETS ${PROJECT_NAME}
        ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        RUNTIME DESTINATION ${CATKIN_GLOBAL_BIN_DESTINATION}
    )

    install(FILES include/serial/serial.h include/serial/v8stdint.h
       DESTINATION ${CATKIN_GLOBAL_INCLUDE_DESTINATION}/serial)

    if(CATKIN_ENABLE_TESTING)
        add_subdirectory(tests)
    endif()
endif()