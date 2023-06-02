# Build ibeacon proto based beacon

# Set language
enable_language ( C )

# Set up defaults
set ( IBEACON_NAME	                  "ibeacon" )
set ( IBEACON_DEFAULT_ADVERTISE       1000 )
set ( IBEACON_DEFAULT_CONN_MODE       0 )
set ( IBEACON_DEFAULT_HCI_CTRL        0 )
set ( IBEACON_DEFAULT_MAJOR           0 )
set ( IBEACON_DEFAULT_MINOR           0 )
set ( IBEACON_DEFAULT_MEASURED_POWER  -80.0 )
set ( IBEACON_DEFAULT_NAME            "iBeacon" )
set ( IBEACON_DEFAULT_NAME_LENGTH     32 )
set ( IBEACON_DEFAULT_PASSWORD        "4321" )
set ( IBEACON_PASSWORD_LENGTH         4 )
set ( IBEACON_DEFAULT_SERIAL          34251 )
set ( IBEACON_SERIAL_LENGTH           5 )
set ( IBEACON_DEFAULT_TX_POWER        0.0 )

configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/ibeacon.h.in"
  "${CMAKE_BINARY_DIR}/include/beaconizer/ibeacon.h"
  NEWLINE_STYLE UNIX )

# Set includes path
include_directories (
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_SOURCE_DIR}/include
    )

# Add sources
list (APPEND SOURCES "ibeacon.c")

# Set up build
add_executable ( "${IBEACON_NAME}"	${SOURCES} )

# Add libraries to link with
target_link_libraries( ${IBEACON_NAME} PRIVATE ${BLUETOOTH_LIBRARY} )
# TARGET_LINK_LIBRARIES("${IBEACON_NAME}" PRIVATE ${LOOP_LIBRARY_NAME}::${LOOP_LIBRARY_NAME})


# End of file
