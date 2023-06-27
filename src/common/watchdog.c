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
#include <stdarg.h>
#include <string.h>
#include <signal.h>

#include <sys/signalfd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "beaconizer/config.h"
#include "beaconizer/io.h"
#include "beaconizer/loop.h"
#include "beaconizer/timer.h"
#include "beaconizer/watchdog.h"
#include "beaconizer/utility.h"


/* Watchdog stuff */
static int __s_notify_fd = -1;
static unsigned int __s_watchdog = -1;

/* Timeout entry */
typedef struct {
    int                 id;             /* Socket descriptor */
    watchdog_fn_t       callback;       /* Callback for socket descriptor timeout */
    destructor_t        destructor;     /* Clean up */
    void               *user_data;      /* User data if any */
} watchdog_data_t;

/* Callback */
static int watchdog_stop(
    void            *user_data) {
    watchdog_notify("WATCHDOG=1");
    return 1;
}

static void watchdog_callback(int id, void *user_data)
{
    watchdog_data_t *data = user_data;

    // if (data->func(data->user_data) &&
    //     !modify_timer(data->id, data->timeout))
    //     return;

    destroy_timer(data->id);
}

static void watchdog_destroy(void *user_data)
{
    if (NULL != user_data) {

        watchdog_data_t *p_entry = user_data;

        if (NULL != p_entry->destructor) {
            p_entry->destructor(p_entry->user_data);
        }

        free(p_entry);
    }
}

/* Initialize watchdog */
void watchdog_init(void) {

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

    struct timespec ts;
    ts.tv_nsec = atoi(watchdog_usec);
    ts.tv_sec = ts.tv_nsec / ( 1000000 * __WATCHDOG_TRIGGER_FREQ );
    ts.tv_sec = ts.tv_nsec % ( 1000000 * __WATCHDOG_TRIGGER_FREQ );
    
    __s_watchdog = watchdog_add(&ts, watchdog_stop, NULL, NULL);
}

/* Trigger watchdog */
int watchdog_notify(
    const char      *state) {

    int err;

    if (1 > __s_notify_fd)
        return -ENOTCONN;

    err = send(__s_notify_fd, state, strlen(state), MSG_NOSIGNAL);
    if (err < 0)
        return -errno;

    return err;
}

/* Timeout  */
unsigned int watchdog_add(
    struct timespec    *timeout,
    watchdog_fn_t       func,
    void               *user_data,
    destructor_t        destructor) {

    watchdog_data_t *p = malloc(sizeof(watchdog_data_t));
    p->callback     = func;
    p->user_data    = user_data;
    p->destructor   = destructor;

    p->id = create_timer(timeout, watchdog_callback, p, watchdog_destroy);
    if (p->id < 0) {
        free(p);
        return 0;
    }

    return (unsigned int) p->id;
}

void watchdog_remove(
    unsigned int id) {

    if (!id) {
        return;
    }

    destroy_timer((int) id);
}

unsigned int watchdog_update(
    struct timespec    *timeout,
    watchdog_fn_t       func,
    void               *user_data,
    destructor_t        destructor)
{
    return watchdog_add(timeout, func, user_data, destructor);
}

/* Destroy watchdog in a loop */
void watchdog_exit(void) {

    if (0 < __s_notify_fd) {
        close(__s_notify_fd);
        __s_notify_fd = -1;
    }

    watchdog_remove(__s_watchdog);
}

 /* End of file */