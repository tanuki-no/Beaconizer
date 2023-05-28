# Build ibeacon proto based beacon

# Define language
ENABLE_LANGUAGE ( C )

# Set up defaults
SET ( IBEACON_NAME	                  "ibeacon" )
SET ( IBEACON_DEFAULT_ADVERTISE       1000 )
SET ( IBEACON_DEFAULT_CONN_MODE       0 )
SET ( IBEACON_DEFAULT_HCI_CTRL        0 )
SET ( IBEACON_DEFAULT_MAJOR           0 )
SET ( IBEACON_DEFAULT_MINOR           0 )
SET ( IBEACON_DEFAULT_MEASURED_POWER  -80.0 )
SET ( IBEACON_DEFAULT_NAME            "iBeacon" )
SET ( IBEACON_DEFAULT_NAME_LENGTH     32 )
SET ( IBEACON_DEFAULT_PASSWORD        "4321" )
SET ( IBEACON_PASSWORD_LENGTH         4 )
SET ( IBEACON_DEFAULT_SERIAL          34251 )
SET ( IBEACON_SERIAL_LENGTH           5 )
SET ( IBEACON_DEFAULT_TX_POWER        0.0 )

CONFIGURE_FILE (
  "${CMAKE_CURRENT_SOURCE_DIR}/ibeacon.h.in"
  "${CMAKE_BINARY_DIR}/include/btest/ibeacon.h"
  NEWLINE_STYLE UNIX )

# Set includes path
INCLUDE_DIRECTORIES (
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_SOURCE_DIR}/include
    )

# Add sources
LIST (APPEND SOURCES "ibeacon.c")

# Set up build
ADD_EXECUTABLE ( "${IBEACON_NAME}"	${SOURCES} )

# Add libraries to link with
# TARGET_LINK_LIBRARIES("${IBEACON_NAME}" PRIVATE ${HCI_LIBRARY_NAME}::${HCI_LIBRARY_NAME})


# End of file
