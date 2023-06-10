# Beaconizer tests

# Tests
# -----------------------------------------------------------------
list ( APPEND TEST   "db00" )
list ( APPEND TEST   "db01" )
list ( APPEND TEST   "db02" )
list ( APPEND TEST   "db03" )
list ( APPEND TEST   "io00" )
list ( APPEND TEST   "io01" )
list ( APPEND TEST   "loop00" )
list ( APPEND TEST   "loop01" )

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

  add_dependencies      ( "${T}" ${CFG_LOOP_LIBRARY_NAME}		)
  target_link_libraries ( "${T}" ${CFG_LOOP_LIBRARY_NAME}		)

  target_link_libraries ( "${T}" ${BLUETOOTH_LIBRARY}		)

  add_test (
    name              "${T}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/test"
    command           "${T}"
  )

endforeach ()

# End of file