/*!
 *	\file		timeout.c
 *	\brief		Handling timeouts in a loop
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		28/06/2022
 *	\version	1.0
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>

#include "beaconizer/loop.h"
#include "beaconizer/timeout.h"
#include "beaconizer/utility.h"


typedef struct {
    int id;
    timeout_fn_t            func;
    timeout_destroy_fn_t    destroy;
    unsigned int            timeout;
    void                   *user_data;
} timeout_data_t;

static void timeout_callback(
    int                     id,
    void                   *user_data) {

    timeout_data_t *data = user_data;

    if (data->func(data->user_data) &&
        !loop_modify_timeout(data->id, data->timeout))
        return;

    loop_remove_timeout(data->id);
}

static void timeout_destroy(
    void                   *user_data) {

    timeout_data_t *data = user_data;

    if (data->destroy)
        data->destroy(data->user_data);

    free(data);
}

unsigned int timeout_add(
    unsigned int            timeout,
    timeout_fn_t            func,
    void                   *user_data,
    timeout_destroy_fn_t    destroy) {

    timeout_data_t *data;

    data = malloc(sizeof(timeout_data_t));
    memset(data, 0, sizeof(timeout_data_t));
    data->func = func;
    data->user_data = user_data;
    data->timeout = timeout;
    data->destroy = destroy;

    data->id = loop_add_timeout(
        timeout,
        timeout_callback,
        data,
        timeout_destroy);

    if (data->id < 0) {
        free(data);
        return 0;
    }

    return (unsigned int) data->id;
}

void timeout_remove(
    unsigned int id) {

    if (!id)
        return;

    loop_remove_timeout((int) id);
}

unsigned int timeout_add_seconds(
    unsigned int            timeout,
    timeout_fn_t            func,
    void                   *user_data,
    timeout_destroy_fn_t    destroy)
{
    return timeout_add(timeout * 1000, func, user_data, destroy);
}

/* Timeout entry */
typedef struct {
    int                 sd;             /* Socket descriptor */
    loop_timeout_fn_t   callback;       /* Callback for socket descriptor timeout */
    loop_destroy_fn_t   destroy;        /* Clean up */
    void                *user_data;     /* User data if any */
} loop_timeout_data_t;

/* Timeout clean up */
static void loop_timeout_destroy(
    void                *user_data) {

    loop_timeout_data_t *p_data = user_data;

    close(p_data->sd);
    p_data->sd = -1;

    if (NULL != p_data->destroy)
        p_data->destroy(p_data->user_data);

    free(p_data);
}

/* Timeout callback */
static void loop_timeout_callback(
    const int           sd,
    const uint32_t      event_mask,
    void                *user_data) {

    loop_timeout_data_t *p_data = user_data;
    uint64_t expired;
    ssize_t result;

    if (event_mask & (EPOLLERR | EPOLLHUP))
        return;

    result = read(p_data->sd, &expired, sizeof(expired));
    if (sizeof(expired) != result)
        return;

    if (p_data->callback)
        p_data->callback(p_data->sd, p_data->user_data);
}

/* Set timeout */
static inline int timeout_set(
    const int           fd,
    const unsigned int  msec) {

    struct itimerspec itimer;
    unsigned int sec = msec / 1000;

    memset(&itimer, 0, sizeof(itimer));
    itimer.it_interval.tv_sec = 0;
    itimer.it_interval.tv_nsec = 0;
    itimer.it_value.tv_sec = sec;
    itimer.it_value.tv_nsec = (msec - (sec * 1000)) * 1000 * 1000;

    return timerfd_settime(fd, 0, &itimer, NULL);
}

/* Add timeout to event processing */
int loop_add_timeout(
    const unsigned int  msec,
    loop_timeout_fn_t   callback,
    void               *user_data,
    loop_destroy_fn_t   destroy) {

    loop_timeout_data_t *p_timeout;

    if ( NULL == callback)
        return -EINVAL;

    p_timeout = malloc(sizeof(loop_timeout_data_t));
    if (NULL == p_timeout)
        return -ENOMEM;

    memset(p_timeout, 0, sizeof(*p_timeout));
    p_timeout->callback = callback;
    p_timeout->destroy = destroy;
    p_timeout->user_data = user_data;

    p_timeout->sd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (0 > p_timeout->sd) {
        free(p_timeout);
        return -EIO;
    }

    if (msec > 0) {
        if (0 > timeout_set(p_timeout->sd, msec)) {
            close(p_timeout->sd);
            free(p_timeout);
            return -EIO;
        }
    }

    if (0 > loop_add_descriptor(p_timeout->sd, EPOLLIN | EPOLLONESHOT, loop_timeout_callback, p_timeout, loop_timeout_destroy)) {
        close(p_timeout->sd);
        free(p_timeout);
        return -EIO;
    }

    return p_timeout->sd;
}

/* Modify event processing timeout */
int loop_modify_timeout(
    const int           id,
    unsigned int        msec) {

    if (0 < msec) {
        if (0 > timeout_set(id, msec))
            return -EIO;
    }

    if (0 > loop_modify_descriptor(id, EPOLLIN | EPOLLONESHOT))
        return -EIO;

    return 0;
}

/* Remove event processing timeout */
int loop_remove_timeout(
    const int           id) {
    return loop_remove_descriptor(id);
}

 /* End of file */