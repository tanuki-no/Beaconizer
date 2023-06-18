/*!
 *	\file		loop01.c
 *	\brief		Create, run, break and destroy the loop
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		02/06/2022
 *	\version	1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include "beaconizer/io.h"
#include "beaconizer/loop.h"
#include "beaconizer/watchdog.h"


typedef void (*sighandler_t)(int);

int hci_device_id = -1;
int descriptor = -1;
const char *hci_dev_name = "hci0";

/* Signal callback */
static void stop(int signum)
{
    static int __s_terminated = 0;

    switch (signum) {
    case SIGINT: 
    case SIGTERM:
        if (!__s_terminated) {
            __s_terminated = 1;
            loop_quit();
        }
        break;
    }
}

int
main() {

    struct io *data = NULL;
    sighandler_t sp = NULL;

    printf("Checking I/O channel ...\n");
    printf("-------------------------------------\n");

    printf("Detecting %s ... ", hci_dev_name),
    hci_device_id = hci_devid(hci_dev_name);
    if (0 > hci_device_id) {
        printf("No HCI device found. Exiting ...\n");
        return EXIT_FAILURE;
    }
    printf("OK!\n");

    printf("Opening HCI %d ... ", hci_device_id),
    descriptor = hci_open_dev(hci_device_id);
    if (0 > descriptor) {
        printf("HCI %d open failed: %s, %d\n", hci_device_id, strerror(errno), errno);
        return EXIT_FAILURE;
    }
    printf("OK!\n");

    printf("Creating loop ... ");
    loop_init();
    printf("OK!\n");

    printf("Add SIGINT signal handler ...");
    errno = EXIT_SUCCESS;
    sp = signal(SIGINT, &stop);
    if (SIG_ERR == sp) {
        printf("%s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    printf("OK!\n");

    printf("Add SIGTERM signal handler ...");
    errno = EXIT_SUCCESS;
    sp = signal(SIGTERM, &stop);
    if (SIG_ERR == sp) {
        printf("%s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    printf("OK!\n");

    printf("Start loop!\n");
    loop_run();
    printf("Stop loop!\n");

    printf("Loop quit!\n");

    printf("Closing HCI %d ... ", hci_device_id),
    hci_close_dev(descriptor);
    printf("OK!\n");

    printf("-------------------------------------\n");
    printf("Done!\n");

    return EXIT_SUCCESS;
}

 /* End of file */