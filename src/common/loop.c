/*!
 *	\file		loop.c
 *	\brief		Simple event loop
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
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "beaconizer/config.h"
#include "beaconizer/loop.h"
#include "beaconizer/watchdog.h"


/* Epoll stuff */
static int __s_epoll_fd = -1;
static int __s_epoll_terminate = 0;
static int __s_exit_status = EXIT_SUCCESS;

/* Loop entry */
typedef struct {
    int                 sd;             /* Socket descriptor */
    uint32_t            event_mask;     /* Event mask */
    loop_event_fn_t     callback;       /* Event callback */
    loop_destroy_fn_t   destroy;        /* Entry clean up */
    void                *user_data;     /* Custom user data */
} loop_data_t;

static loop_data_t*  _s_loop_list[__MAX_LOOP_ENTRIES];

/* Timeout entry */
typedef struct {
    int                 sd;             /* Socket descriptor */
    loop_timeout_fn_t   callback;       /* Callback for socket descriptor timeout */
    loop_destroy_fn_t   destroy;        /* Clean up */
    void                *user_data;     /* User data if any */
} timeout_data_t;

/* Initialize loop */
void loop_init(void) {

    unsigned int i;

    __s_epoll_fd = epoll_create1(EPOLL_CLOEXEC);

    for (i = 0; __MAX_LOOP_ENTRIES > i; i++)
        _s_loop_list[i] = NULL;

    __s_epoll_terminate = 0;

    loop_watchdog_init();
}

/* Run loop */
int loop_run(void) {

    unsigned int i;
    struct epoll_event event[__MAX_EPOLL_EVENTS];

    /* Loop */
    while (0 == __s_epoll_terminate) {

        int count = epoll_wait(__s_epoll_fd, event, __MAX_EPOLL_EVENTS, -1);

        if (0 > count)
            continue;

        unsigned int j;
        for (j = 0; count > j; j++) {
            loop_data_t *p_data = event[j].data.ptr;
            p_data->callback(p_data->sd, event[j].events, p_data->user_data);
        }
    }

    /* Clean up */
    for (i = 0; __MAX_LOOP_ENTRIES > i; i++) {
        loop_data_t *p_data = _s_loop_list[i];
        _s_loop_list[i] = NULL;

        if (NULL != p_data) {

            epoll_ctl(__s_epoll_fd, EPOLL_CTL_DEL, p_data->sd, NULL);

            if (NULL != p_data->destroy)
                p_data->destroy(p_data->user_data);

            free(p_data);
        }
    }

    /* Close socket */
    close(__s_epoll_fd);
    __s_epoll_fd = 0;

    /* Shutdown watchdog */
    loop_watchdog_exit();

    return __s_exit_status;
}

/* Add descriptor to watch */
int loop_add_descriptor(
    const int           sd,
    const uint32_t      event_mask,
    loop_event_fn_t     callback,
    void                *user_data,
    loop_destroy_fn_t   destroy) {

    loop_data_t *p_data = NULL;
    struct epoll_event ev;

    if (0 > sd || (__MAX_LOOP_ENTRIES - 1) < sd || NULL != callback)
        return -EINVAL;

    /* Allocate loop entry */
    p_data = malloc(sizeof(*p_data));
    if (NULL == p_data)
        return -ENOMEM;

    /* Fill loop entry */
    memset(p_data, 0, sizeof(loop_data_t));
    p_data->sd = sd;
    p_data->event_mask = event_mask;
    p_data->callback = callback;
    p_data->destroy = destroy;
    p_data->user_data = user_data;

    /* Fill epoll() event entry */
    memset(&ev, 0, sizeof(ev));
    ev.events = event_mask;
    ev.data.ptr = p_data;

    /* Add epoll entry */
    int e = epoll_ctl(__s_epoll_fd, EPOLL_CTL_ADD, p_data->sd, &ev);
    if (0 > e) {
        free(p_data);
        return e;
    }

    /* Store loop entry on success */
    _s_loop_list[sd] = p_data;

    return 0;
}

/* Modify watched descriptor */
int loop_modify_descriptor(
    const int           sd,
    uint32_t            event_mask) {

    loop_data_t *p_data = NULL;
    struct epoll_event ev;

    if (0 > sd || (__MAX_LOOP_ENTRIES - 1) < sd)
        return -EINVAL;

    p_data = _s_loop_list[sd];
    if (NULL == p_data)
        return -ENXIO;

    memset(&ev, 0, sizeof(ev));
    ev.events = event_mask;
    ev.data.ptr = p_data;

    int e = epoll_ctl(__s_epoll_fd, EPOLL_CTL_MOD, p_data->sd, &ev);
    if (0 > e)
        return e;

    p_data->event_mask = event_mask;

    return e;
}

/* Remove watched descriptor */
int loop_remove_descriptor(
    const int           sd) {

    if (0 > sd || (__MAX_LOOP_ENTRIES - 1) < sd)
        return -EINVAL;

    loop_data_t *p_data = _s_loop_list[sd];

    if (NULL == p_data)
        return -ENXIO;

    _s_loop_list[sd] = NULL;

    int e = epoll_ctl(__s_epoll_fd, EPOLL_CTL_DEL, p_data->sd, NULL);

    if (NULL != p_data->destroy)
        p_data->destroy(p_data->user_data);

    free(p_data);

    return e;
}

/* Timeout clean up */
static void timeout_destroy(
    void                *user_data) {

    timeout_data_t *p_data = user_data;

    close(p_data->sd);
    p_data->sd = -1;

    if (NULL != p_data->destroy)
        p_data->destroy(p_data->user_data);

    free(p_data);
}

/* Timeout callback */
static void timeout_callback(
    const int           sd,
    const uint32_t      event_mask,
    void                *user_data) {

    timeout_data_t *p_data = user_data;
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

    timeout_data_t *p_timeout;

    if ( NULL == callback)
        return -EINVAL;

    p_timeout = malloc(sizeof(timeout_data_t));
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

    if (0 > loop_add_descriptor(p_timeout->sd, EPOLLIN | EPOLLONESHOT, timeout_callback, p_timeout, timeout_destroy)) {
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

/* Quit loop immediately */
void loop_quit(void) {
    __s_epoll_terminate = 1;
    loop_sd_notify("STOPPING=1");
}

/* Quit loop and set success */
void loop_exit_success(void) {
    __s_exit_status = EXIT_SUCCESS;
    __s_epoll_terminate = 1;
}

/* Quit loop and set failure */
void loop_exit_failure(void) {
    __s_exit_status = EXIT_FAILURE;
    __s_epoll_terminate = 1;
}

 /* End of file */