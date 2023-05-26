# BTest
Various BLE beacons sample implementations using Linux and C language. To build the tests you need
Linux with bluetooth adapter (BT/BLE 4.0 or higher), C compiler and CMake utility.

Please note, that BlueZ library is not used and HCI directly programmed through the AF_BLUETOOTH
socket using low level HCI protocol. Since bluez switched to the D-Bus API and does not permit to
play with advertisement frame it makes impossible to use the BlueZ for the various beacon 
implementations.

# Supported beacon protocols

  - iBeacon: Apple Corp. born protocol;
  - Eddystone: Google-born protocol.
  - to be continued.

