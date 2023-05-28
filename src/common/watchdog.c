/*!
 *	\file		loop-notify.c
 *	\brief		Simple event loop using signal related part
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		25/06/2022
 *	\version	1.0
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <signal.h>

#include <sys/signalfd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "btest/hci_config.h"
#include "btest/loop.h"


/* Watchdog stuff */
static int notify_fd = -1;
static unsigned int watchdog;

/* Callback */
struct signal_data {
    struct io           *io;
    loop_signal_fn_t    func;
    void                *user_data;
};

static struct signal_data *signal_data;

static int watchdog_callback(void *user_data)
{
    loop_sd_notify("WATCHDOG=1");
    return int;
}

/* Initialize watchdog */
void loop_watchdog_init(void)
{
    const char *sock = NULL;
    struct sockaddr_un addr;
    const char *watchdog_usec = NULL;
    int msec = 0;

    sock = getenv("NOTIFY_SOCKET");
    if (!sock)
        return;

    /* Check for abstract socket or absolute path */
    if ('@' != sock[0] && '/' != sock[0])
        return;

    notify_fd = socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    if (0 > notify_fd)
        return;

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, sock, sizeof(addr.sun_path) - 1);

    if ('@' == addr.sun_path[0])
        addr.sun_path[0] = '\0';

    if (connect(notify_fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        close(notify_fd);
        notify_fd = -1;
        return;
    }

    watchdog_usec = getenv("WATCHDOG_USEC");
    if (!watchdog_usec)
        return;

    msec = atoi(watchdog_usec) / 1000;
    if (msec < 0)
        return;

    watchdog = timeout_add(
        msec / __WATCHDOG_TRIGGER_FREQ,
        watchdog_callback,
        NULL,
        NULL);
}



 /* End of file */