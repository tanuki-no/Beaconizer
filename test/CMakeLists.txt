# Beaconizer tests

# Tests
# -----------------------------------------------------------------
list ( APPEND TEST   "db00" )
list ( APPEND TEST   "db01" )
list ( APPEND TEST   "db02" )

# Library test
# -----------------------------------------------------------------
foreach ( T ${TEST} )

  add_executable                ( "${T}" "${T}.c" )

  set_target_properties (
    ${T}                        PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/test"
    LIBRARY_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/test"
    RUNTIME_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/test"
    )

  add_dependencies      ( "${T}" ${CFG_HCI_LIBRARY_NAME}		)
  target_link_libraries ( "${T}" ${CFG_HCI_LIBRARY_NAME}		)

  add_test(
    name              "${T}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/test"
    command           "${T}"
  )

endforeach ()

# End of file