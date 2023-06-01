# Beaconizer tests

# Tests
# -----------------------------------------------------------------
LIST ( APPEND TEST   "db00" )

# Library test
# -----------------------------------------------------------------
FOREACH ( T ${TEST} )

  ADD_EXECUTABLE                ( "${T}" "${T}.c" )

  SET_TARGET_PROPERTIES (
    ${T}                        PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/test"
    LIBRARY_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/test"
    RUNTIME_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/test"
    )

  ADD_DEPENDENCIES      ( "${T}" ${CFG_HCI_LIBRARY_NAME}		)
  TARGET_LINK_LIBRARIES ( "${T}" ${CFG_HCI_LIBRARY_NAME}		)

  ADD_TEST(
    NAME              "${T}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/test"
    COMMAND           "${T}"
  )

ENDFOREACH ()

# End of file