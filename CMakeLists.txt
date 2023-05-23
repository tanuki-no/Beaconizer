# BTest CMake code generation script
#
# ----------------------------------
CMAKE_MINIMUM_REQUIRED(VERSION 3.20)

PROJECT(
    btest
    LANGUAGES		C)

# Project version
SET ( PROJECT_VERSION_MAJOR	0 )
SET ( PROJECT_VERSION_MINOR	0 )
SET ( PROJECT_VERSION_PATCH	2 )
SET ( PROJECT_VERSION		${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH} )

# Project details
SET ( ALIAS		"BTest" )
SET ( AUTHOR		"Vladislav \\\"Tanuki\\\" Mikhailikov" )
SET ( COPYRIGHT		"GNU GPLv3" )
SET ( DESCRIPTION	"BLE beacon sample code for Linux, BlueZ and C" )

# Service setup
STRING ( TIMESTAMP TODAY "%d/%m/%Y" )
STRING(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" is_top_level)

# Detect required functions and libraries
LIST (APPEND HEADER_FILES_TO_CHECK_LIST "errno.h")
LIST (APPEND HEADER_FILES_TO_CHECK_LIST "limits.h")
LIST (APPEND HEADER_FILES_TO_CHECK_LIST "stdio.h")
LIST (APPEND HEADER_FILES_TO_CHECK_LIST "stdlib.h")
LIST (APPEND HEADER_FILES_TO_CHECK_LIST "string.h")
LIST (APPEND HEADER_FILES_TO_CHECK_LIST "getopt.h")
LIST (APPEND HEADER_FILES_TO_CHECK_LIST "sys/ioctl.h")
LIST (APPEND HEADER_FILES_TO_CHECK_LIST "sys/random.h")
LIST (APPEND HEADER_FILES_TO_CHECK_LIST "sys/socket.h")

INCLUDE (CheckIncludeFiles)
CHECK_INCLUDE_FILES (
    "${HEADER_FILES_TO_CHECK_LIST}" REQUIRED_HEADERS_FOUND LANGUAGE C
)

IF (NOT REQUIRED_HEADERS_FOUND)
    MESSAGE ( FATAL_ERROR "Required headers absent!" )
ENDIF ()

INCLUDE ( ${CMAKE_SOURCE_DIR}/cmake/FindBlueZ.cmake )
IF (NOT BLUETOOTH_FOUND )
    MESSAGE ( FATAL_ERROR "BlueZ library and headers not found!" )
ELSE()
    MESSAGE ( STATUS "BlueZ library found!" )
ENDIF ()

# Preset common libraries for the project
LINK_LIBRARIES( ${BLUETOOTH_LIBRARY} )

# Add GNU install dirs
INCLUDE ( GNUInstallDirs )

# Add includes
ADD_SUBDIRECTORY (include)

# Add sources
ADD_SUBDIRECTORY (src)

# End of file
