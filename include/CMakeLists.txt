# Headers

CONFIGURE_FILE (
  "${CMAKE_CURRENT_SOURCE_DIR}/btest/btest.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/btest/btest.h"
  NEWLINE_STYLE UNIX )

CONFIGURE_FILE (
  "${CMAKE_SOURCE_DIR}/include/btest/hci_config.h.in"
  "${CMAKE_BINARY_DIR}/include/btest/hci_config.h"
  NEWLINE_STYLE UNIX )

# End of file
