/*!
 *	\file		loop02.c
 *	\brief		Loop stress test
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		02/06/2022
 *	\version	1.0
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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
typedef struct {
    size_t  count;
    int*    fd;
    char**  name;
} test_file_t;


int hci_device_id = -1;
int descriptor = -1;
const char *hci_dev_name = "hci0";
const char *test_dir_path = "/tmp";
const size_t test_file_count = 960;
test_file_t test = {
    .count  = 0,
    .fd     = NULL,
    .name   = NULL
};

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

/* Allocate and initialized X files */
int create_and_open_test_file(
    const size_t count) {

    size_t i;
    void* p;
    const size_t buffer_size = 256;
    char buffer[buffer_size];

    /* Allocate memory */
    p = malloc(count * sizeof(int));
    if (NULL == p) {
        printf("Memory allocation error!");
        return EXIT_FAILURE;
    }
    test.fd = p;

    p = malloc(count * sizeof(char *));
    if (NULL == p) {
        printf("Memory allocation error!");
        return EXIT_FAILURE;
    }
    test.name = p;

    test.count = count;

    for (i = 0; test.count > i; ++i) {
        
        test.fd[i] = -1;
        test.name[i] = NULL;

        snprintf(buffer, buffer_size, "%s/beaconize_test%lu.tmp", test_dir_path, i);
        size_t l = strlen(buffer);
        p = calloc(l + 1, sizeof(char));
        if (NULL == p) {
            continue;
        }
        strncpy(p, buffer, l + 1);
        test.name[i] = p;

        test.fd[i] = open(test.name[i], O_CREAT, S_IRUSR | S_IWUSR);
        if (-1 == test.fd[i]) {
            printf("File \"%s\" creation error: %s.\n", test.name[i], strerror(errno));
            continue;
        }
    }

    return EXIT_SUCCESS;
}

/* Clean up X files */
void close_and_remove_test_file() {

    size_t i;

    /* Close files, free memory */
    if (NULL != test.fd) {
        for (i = 0; test.count > i; i++) {
            if (-1 < test.fd[i]) {
                close(test.fd[i]);
            }
        }
        free(test.fd);
        test.fd = NULL;
    }

    /* Remove files */
    if (NULL != test.name) {
        for (i = 0; test.count > i; i++) {
            char* p = test.name[i];
            test.name[i] = NULL;
            if (NULL != p) {
                remove(p);
                free(p);
            }
        }
        free(test.name);
        test.name = NULL;
    }

    test.count = 0;
}

/* Main course */
int
main() {

    int e;
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

    printf("Creating %lu file descriptors ...", test_file_count);
    e = create_and_open_test_file(test_file_count);
    if (EXIT_SUCCESS != e) {
        return EXIT_FAILURE;
    }
    printf("OK!\n");
    
    printf("Adding %lu file descriptors ...", test_file_count);
    printf("OK!\n");

    printf("Start loop!\n");
    loop_run();
    printf("Stop loop!\n");

    printf("Loop quit!\n");

    printf("Closing and removing files ... ");
    close_and_remove_test_file();
    printf("OK!\n");

    printf("Closing HCI %d ... ", hci_device_id);
    hci_close_dev(descriptor);
    printf("OK!\n");

    printf("-------------------------------------\n");
    printf("Done!\n");

    return EXIT_SUCCESS;
}

 /* End of file */