#[=======================================================================[.rst:
FindBlueZ
--------

Find Linux Bluetooth protocol stack (BlueZ)

Find the bluez libraries (``bluetooth``)

IMPORTED Targets
^^^^^^^^^^^^^^^^

.. versionadded:: 0.1

This module defines :prop_tgt:`IMPORTED` target ``BLUETOOTH::BLUETOOTH``, if
BlueZ has been found.

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

``BLUETOOTH_FOUND``
  True if BLUETOOTH_INCLUDE_DIR & BLUETOOTH_LIBRARY are found

``BLUETOOTH_LIBRARIES``
  List of libraries when using BlueZ.

``BLUETOOTH_INCLUDE_DIRS``
  Where to find the BlueZ headers.

Cache variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``BLUETOOTH_INCLUDE_DIR``
  the BLUETOOTH include directory

``BLUETOOTH_LIBRARY``
  the absolute path of the BlueZ library
#]=======================================================================]

FIND_PATH ( BLUETOOTH_INCLUDE_DIR NAMES bluetooth/bluetooth.h 
    DOC "The bluetooth (BlueZ) include directory" )

FIND_LIBRARY ( BLUETOOTH_LIBRARY NAMES bluetooth 
    DOC "The bluetooth (BlueZ) library" )

INCLUDE ( FindPackageHandleStandardArgs )
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    BLUETOOTH
    REQUIRED_VARS BLUETOOTH_LIBRARY BLUETOOTH_INCLUDE_DIR )

IF(BLUETOOTH_FOUND)
    SET ( BLUETOOTH_LIBRARIES ${BLUETOOTH_LIBRARY} )
    SET( BLUETOOTH_INCLUDE_DIRS ${BLUETOOTH_INCLUDE_DIR} )
    IF(NOT TARGET BLUETOOTH::BLUETOOTH)
	ADD_LIBRARY(BLUETOOTH::BLUETOOTH UNKNOWN IMPORTED)
	SET_TARGET_PROPERTIES(BLUETOOTH::BLUETOOTH PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${BLUETOOTH_INCLUDE_DIRS}")
	SET_PROPERTY(TARGET BLUETOOTH::BLUETOOTH APPEND PROPERTY IMPORTED_LOCATION "${BLUETOOTH_LIBRARY}")
    ENDIF()
ENDIF()

MARK_AS_ADVANCED(BLUETOOTH_INCLUDE_DIR BLUETOOTH_LIBRARY)

# End of file
