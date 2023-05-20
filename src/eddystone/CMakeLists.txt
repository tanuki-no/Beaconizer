# Build ibeacon proto based beacon

# Define language
ENABLE_LANGUAGE ( C )

# Define build name
SET ( BUILD_NAME	"eddystone_${PROJECT_NAME}" )

# Set includes path
INCLUDE_DIRECTORIES (
    ${CMAKE_BINARY_DIR}/include
    )

# Add sources
SET ( Sources		"eddystone.c" )

# Set up build
ADD_EXECUTABLE ( "${BUILD_NAME}"	${Sources} )
TARGET_LINK_LIBRARIES ( "${BUILD_NAME}" ${BLUETOOTH_LIBRARY} )

# End of file
