# BTest CMake code generation script
#
# ----------------------------------
cmake_minimum_required(VERSION 3.20)

project(
    beaconizer
    LANGUAGES		C)

# Project version
set ( PROJECT_VERSION_MAJOR	0 )
set ( PROJECT_VERSION_MINOR	0 )
set ( PROJECT_VERSION_PATCH	3 )
set ( PROJECT_VERSION		    ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH} )

set ( CFG_HCI_LIBRARY_NAME      "hci" )
set ( CFG_HCI_LIBRARY_MAJOR     0 )
set ( CFG_HCI_LIBRARY_MINOR     0 )
set ( CFG_HCI_LIBRARY_PATCH     3 )
set ( CFG_HCI_LIBRARY_VERSION   ${CFG_HCI_LIBRARY_MAJOR}.${CFG_HCI_LIBRARY_MINOR}.${CFG_HCI_LIBRARY_PATCH} )
set ( CFG_MAX_EPOLL_EVENTS      16 )
set ( CFG_MAX_LOOP_ENTRIES      128 )
set ( CFG_WATCHDOG_TRG_FREQ     2 )

# Project details
set ( ALIAS             "Beaconizer" )
set ( AUTHOR            "Vladislav \\\"Tanuki\\\" Mikhailikov" )
set ( COPYRIGHT         "GNU GPLv3" )
set ( DESCRIPTION       "BLE beacon sample code for Linux and C" )

# Service setup
string ( TIMESTAMP TODAY "%d/%m/%Y" )
string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" is_top_level)

# Check compiler and features
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  message("Clang")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  message("GNU")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  message("Intel")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  message("MSVC")
endif()

# Detect required functions and libraries
list (APPEND HEADER_FILES_TO_CHECK_LIST "errno.h")
list (APPEND HEADER_FILES_TO_CHECK_LIST "limits.h")
list (APPEND HEADER_FILES_TO_CHECK_LIST "signal.h")
list (APPEND HEADER_FILES_TO_CHECK_LIST "stdio.h")
list (APPEND HEADER_FILES_TO_CHECK_LIST "stdlib.h")
list (APPEND HEADER_FILES_TO_CHECK_LIST "stddef.h")
list (APPEND HEADER_FILES_TO_CHECK_LIST "string.h")
list (APPEND HEADER_FILES_TO_CHECK_LIST "unistd.h")
list (APPEND HEADER_FILES_TO_CHECK_LIST "getopt.h")

list (APPEND HEADER_FILES_TO_CHECK_LIST "sys/epoll.h")
list (APPEND HEADER_FILES_TO_CHECK_LIST "sys/ioctl.h")
list (APPEND HEADER_FILES_TO_CHECK_LIST "sys/signalfd.h")
list (APPEND HEADER_FILES_TO_CHECK_LIST "sys/socket.h")
list (APPEND HEADER_FILES_TO_CHECK_LIST "sys/timerfd.h")
list (APPEND HEADER_FILES_TO_CHECK_LIST "sys/random.h")
list (APPEND HEADER_FILES_TO_CHECK_LIST "sys/un.h")

include (CheckIncludeFiles)
CHECK_INCLUDE_FILES (
    "${HEADER_FILES_TO_CHECK_LIST}" REQUIRED_HEADERS_FOUND LANGUAGE C
)

if (NOT REQUIRED_HEADERS_FOUND)
  message ( FATAL_ERROR "Required headers absent!" )
endif ()

# Add GNU install dirs
include ( GNUInstallDirs )

# HCI_SHARED_LIBS determines static/shared build when defined
option ( HCI_BUILD_TESTS    "Build HCI library tests" off )
option ( HCI_BUILD_EXAMPLES "Build HCI examples"      off )

if ( DEFINED HCI_SHARED_LIBS )
  set ( BUILD_SHARED_LIBS ${HCI_SHARED_LIBS} )
endif ()

# Build HCI library

# Set includes path
include_directories (
  ${CMAKE_BINARY_DIR}/include
  ${CMAKE_SOURCE_DIR}/include
  )

# Add sources
list ( APPEND SOURCES   "${CMAKE_SOURCE_DIR}/src/common/db.c" )
list ( APPEND SOURCES   "${CMAKE_SOURCE_DIR}/src/common/io.c" )
list ( APPEND SOURCES   "${CMAKE_SOURCE_DIR}/src/common/loop.c" )
list ( APPEND SOURCES   "${CMAKE_SOURCE_DIR}/src/common/timeout.c" )
list ( APPEND SOURCES   "${CMAKE_SOURCE_DIR}/src/common/utility.c" )
list ( APPEND SOURCES   "${CMAKE_SOURCE_DIR}/src/common/watchdog.c" )

# Set up build
if ( BUILD_SHARED_LIBS )

  add_library ( ${CFG_HCI_LIBRARY_NAME} STATIC ${SOURCES} )

else ()

  add_library ( ${CFG_HCI_LIBRARY_NAME} SHARED ${SOURCES} )

  set_target_properties (
    ${CFG_HCI_LIBRARY_NAME}     PROPERTIES
    SOVERSION               ${CFG_HCI_LIBRARY_MAJOR}
    VERSION                 ${CFG_HCI_LIBRARY_VERSION}
  )

endif ()
add_library ( ${CFG_HCI_LIBRARY_NAME}::${CFG_HCI_LIBRARY_NAME} ALIAS ${CFG_HCI_LIBRARY_NAME} )

# Add includes
add_subdirectory ( include )

# Add sources
add_subdirectory ( src )

# Tests
if (BUILD_TESTS)
  enable_testing()
  include (CTest)
  add_subdirectory ( test )
endif ()

# End of file
