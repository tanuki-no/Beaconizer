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
SET ( PROJECT_VERSION_PATCH	1 )
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
INCLUDE ( ${CMAKE_SOURCE_DIR}/cmake/FindBlueZ.cmake )
IF (NOT BLUETOOTH_FOUND)
    MESSAGE ( ERROR "BlueZ library and headers not found!" )
ENDIF ()

# Add GNU install dirs
INCLUDE ( GNUInstallDirs )

# Add includes
ADD_SUBDIRECTORY (include)

# Add sources
ADD_SUBDIRECTORY (src)

# End of file
