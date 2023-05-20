# BTest CMake code generation script
#
# ----------------------------------
CMAKE_MINIMUM_REQUIRED(VERSION 3.20)

PROJECT(
    btest
    VERSION		0.0.1
    DESCRIPTION		"BLE beacon sample code for Linux, BlueZ and C"
    LANGUAGES		C)

STRING(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" is_top_level)

INCLUDE(src)

# End of file
