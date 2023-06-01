/*!
 *	\file		watchdog.c
 *	\brief		Watchdog stuff
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

#include "beaconizer/hci_config.h"
#include "beaconizer/io.h"
#include "beaconizer/loop.h"
#include "beaconizer/timeout.h"
#include "beaconizer/watchdog.h"
#include "beaconizer/utility.h"


/* Watchdog stuff */
static int __s_notify_fd = -1;
static unsigned int __s_watchdog = -1;

/* Callback */
typedef struct {
    struct io           *io;
    loop_signal_fn_t    func;
    void                *user_data;
} signal_data_t;

static signal_data_t *signal_data;

static int watchdog_callback(
    void            *user_data) {
    loop_sd_notify("WATCHDOG=1");
    return 1;
}

/* Initialize watchdog */
void loop_watchdog_init(void) {

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

    __s_notify_fd = socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    if (0 > __s_notify_fd)
        return;

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, sock, sizeof(addr.sun_path) - 1);

    if ('@' == addr.sun_path[0])
        addr.sun_path[0] = '\0';

    if (connect(__s_notify_fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        close(__s_notify_fd);
        __s_notify_fd = -1;
        return;
    }

    watchdog_usec = getenv("WATCHDOG_USEC");
    if (!watchdog_usec)
        return;

    msec = atoi(watchdog_usec) / 1000;
    if (msec < 0)
        return;

    __s_watchdog = timeout_add(
        msec / __WATCHDOG_TRIGGER_FREQ,
        watchdog_callback,
        NULL,
        NULL);
}

/* Trigger watchdog */
int loop_sd_notify(
    const char      *state) {

    int err;

    if (1 > __s_notify_fd)
        return -ENOTCONN;

    err = send(__s_notify_fd, state, strlen(state), MSG_NOSIGNAL);
    if (err < 0)
        return -errno;

    return err;
}

static int signal_read(
    struct io       *io,
    void            *user_data) {

    signal_data_t *data = user_data;
    struct signalfd_siginfo si;
    ssize_t result;
    int descriptor;
    
    descriptor = io_get_descriptor(io);

    result = read(descriptor, &si, sizeof(si));
    if (sizeof(si) != result)
        return 0;

    if (data && data->func)
        data->func(si.ssi_signo, data->user_data);

    return 1;
}

static struct io *setup_signalfd(
    void            *user_data) {

    struct io *io;
    sigset_t mask;
    int fd;

    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGUSR2);
    sigaddset(&mask, SIGCHLD);

    if (0 > sigprocmask(SIG_BLOCK, &mask, NULL))
        return NULL;

    fd = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
    if (0 > fd )
        return NULL;

    io = io_new(fd);

    io_set_close_on_destroy(io, 1);
    io_set_read_handler(io, signal_read, user_data, free);

    return io;
}

int loop_run_with_signal(
    loop_signal_fn_t    func,
    void               *user_data) {

    signal_data_t *data = NULL;
    struct io *io = NULL;
    int ret = -1;

    if (NULL == func)
        return -EINVAL;

    data = new0(signal_data_t, 1);
    data->func = func;
    data->user_data = user_data;

    io = setup_signalfd(data);
    if (!io) {
        free(data);
        return -errno;
    }

    ret = loop_run();

    io_destroy(io);
    free(signal_data);

    return ret;
}

/* Destroy watchdog in a loop */
void loop_watchdog_exit(void) {

    if (0 < __s_notify_fd) {
        close(__s_notify_fd);
        __s_notify_fd = -1;
    }

    timeout_remove(__s_watchdog);
}

 /* End of file */