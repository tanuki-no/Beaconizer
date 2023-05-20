# Build ibeacon proto based beacon

# Define language
ENABLE_LANGUAGE ( C )

# Define build name
STRING ( BUILD_NAME	"ibeacon_${PROJECT_NAME}" )

# Set includes path
INCLUDE_DIRECTORIES (
    ${CMAKE_BINARY_DIR}/include
    )

# Add sources
SET ( Sources
    "ibeacon.c"
    )

# Set up build
ADD_EXECUTABLE ( "${BUILD_NAME}"	${Sources} )
TARGET_LINK_LIBRARIES ( "${BUILD_NAME}" ${BLUEZ_LIBRARY} )

# End of file
