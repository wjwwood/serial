get_filename_component(SERIAL_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
set(SERIAL_INCLUDE_DIRS "${SERIAL_CMAKE_DIR}/../../../include")
find_library(SERIAL_LIBRARIES serial PATHS ${SERIAL_CMAKE_DIR}/../../../lib/serial)
