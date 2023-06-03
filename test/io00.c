/*!
 *	\file		io00.c
 *	\brief		Check I/O channel
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		28/05/2022
 *	\version	1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include "beaconizer/io.h"
#include "beaconizer/loop.h"


int hci_device_id = -1;
int descriptor = -1;

int
main() {

    struct io *data = NULL;

    printf("Checking I/O channel ...\n");
    printf("-------------------------------------\n");

    hci_device_id = hci_devid("hci0");
    if (0 > hci_device_id) {
        printf("No HCI device found. Exiting ...\n");
        return EXIT_FAILURE;
    }

    printf("HCI %d detected!\n", hci_device_id),

    descriptor = hci_open_dev(hci_device_id);
    if (0 > descriptor) {
        printf("HCI %d open failed: %s, %d\n", hci_device_id, strerror(errno), errno);
        return EXIT_FAILURE;
    }

    printf("HCI %d opened!\n", hci_device_id),

    loop_init();
    data = io_new(descriptor);

    if (NULL != data) {
        printf("I/O allocated. Data: %p\n", data);

        io_destroy(data);
        printf("I/O destroyed. Data: %p\n", data);
    } else {
        printf("io_new(%d) failed: %s, %d\n", descriptor, strerror(errno), errno);
        return EXIT_FAILURE;
    }
    loop_quit();
    hci_close_dev(descriptor);

    printf("HCI %d closed!\n", hci_device_id),

    printf("-------------------------------------\n");
    printf("Done!\n");

    return EXIT_SUCCESS;
}

 /* End of file */