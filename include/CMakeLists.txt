# Headers

CONFIGURE_FILE (
  "${CMAKE_CURRENT_SOURCE_DIR}/beaconizer/btest.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/beaconizer/btest.h"
  NEWLINE_STYLE UNIX )

CONFIGURE_FILE (
  "${CMAKE_SOURCE_DIR}/include/beaconizer/hci_config.h.in"
  "${CMAKE_BINARY_DIR}/include/beaconizer/hci_config.h"
  NEWLINE_STYLE UNIX )

# End of file
