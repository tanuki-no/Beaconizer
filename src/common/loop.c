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

/* Loop stuff */
typedef struct {
    int                 fd;
    uint32_t            events;
    loop_event_fn_t     callback;
    loop_destroy_fn_t   destroy;
    void                *user_data;
} loop_data_t;

static loop_data_t*  _s_loop_list[__MAX_LOOP_ENTRIES];

/* Timeout stuff */
typedef struct {
    int                 fd;
    loop_timeout_fn_t   callback;
    loop_destroy_fn_t   destroy;
    void                *user_data;
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

    /* Loop */
    while (0 == __s_epoll_terminate) {

        struct epoll_event events[__MAX_EPOLL_EVENTS];
        int n, nfds;

        nfds = epoll_wait(__s_epoll_fd, events, __MAX_EPOLL_EVENTS, -1);
        if (nfds < 0)
            continue;

        for (n = 0; n < nfds; n++) {
            loop_data_t *data = events[n].data.ptr;
            data->callback(data->fd, events[n].events, data->user_data);
        }
    }

    /* Clean up */
    for (i = 0; __MAX_LOOP_ENTRIES > i; i++) {
        loop_data_t *data = _s_loop_list[i];
        _s_loop_list[i] = NULL;

        if (data) {
            epoll_ctl(__s_epoll_fd, EPOLL_CTL_DEL, data->fd, NULL);

            if (data->destroy)
                data->destroy(data->user_data);

            free(data);
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
    const int           fd,
    const uint32_t      events,
    loop_event_fn_t     callback,
    void                *user_data,
    loop_destroy_fn_t   destroy) {

    loop_data_t *data;
    struct epoll_event ev;
    int err;

    if (0 > fd || (__MAX_LOOP_ENTRIES - 1) < fd || !callback)
        return -EINVAL;

    data = malloc(sizeof(*data));
    if (NULL == data)
        return -ENOMEM;

    memset(data, 0, sizeof(loop_data_t));
    data->fd = fd;
    data->events = events;
    data->callback = callback;
    data->destroy = destroy;
    data->user_data = user_data;

    memset(&ev, 0, sizeof(ev));
    ev.events = events;
    ev.data.ptr = data;

    err = epoll_ctl(__s_epoll_fd, EPOLL_CTL_ADD, data->fd, &ev);
    if (0 > err) {
        free(data);
        return err;
    }

    _s_loop_list[fd] = data;

    return 0;
}

/* Modify watched descriptor */
int loop_modify_descriptor(
    const int           fd,
    uint32_t            events) {

    loop_data_t *data;
    struct epoll_event ev;
    int err;

    if (0 > fd || (__MAX_LOOP_ENTRIES - 1) < fd)
        return -EINVAL;

    data = _s_loop_list[fd];
    if (NULL == data)
        return -ENXIO;

    memset(&ev, 0, sizeof(ev));
    ev.events = events;
    ev.data.ptr = data;

    err = epoll_ctl(__s_epoll_fd, EPOLL_CTL_MOD, data->fd, &ev);
    if (err < 0)
        return err;

    data->events = events;

    return 0;
}

/* Remove watched descriptor */
int loop_remove_descriptor(
    const int           fd) {

    loop_data_t *data;
    int err;

    if (0 > fd || (__MAX_LOOP_ENTRIES - 1) < fd)
        return -EINVAL;

    data = _s_loop_list[fd];
    if (NULL == data)
        return -ENXIO;

    _s_loop_list[fd] = NULL;

    err = epoll_ctl(__s_epoll_fd, EPOLL_CTL_DEL, data->fd, NULL);

    if (data->destroy)
        data->destroy(data->user_data);

    free(data);

    return err;
}

/* Timeout clean up */
static void timeout_destroy(
    void                *user_data) {

    timeout_data_t *data = user_data;

    close(data->fd);
    data->fd = -1;

    if (data->destroy)
        data->destroy(data->user_data);

    free(data);
}

/* Timeout callback */
static void timeout_callback(
    const int           fd,
    const uint32_t      events,
    void                *user_data) {

    timeout_data_t *data = user_data;
    uint64_t expired;
    ssize_t result;

    if (events & (EPOLLERR | EPOLLHUP))
        return;

    result = read(data->fd, &expired, sizeof(expired));
    if (result != sizeof(expired))
        return;

    if (data->callback)
        data->callback(data->fd, data->user_data);
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

    timeout_data_t *data;

    if ( NULL == callback)
        return -EINVAL;

    data = malloc(sizeof(timeout_data_t));
    if (NULL == data)
        return -ENOMEM;

    memset(data, 0, sizeof(*data));
    data->callback = callback;
    data->destroy = destroy;
    data->user_data = user_data;

    data->fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (0 > data->fd) {
        free(data);
        return -EIO;
    }

    if (msec > 0) {
        if (0 > timeout_set(data->fd, msec)) {
            close(data->fd);
            free(data);
            return -EIO;
        }
    }

    if (0 > loop_add_descriptor(data->fd, EPOLLIN | EPOLLONESHOT, timeout_callback, data, timeout_destroy)) {
        close(data->fd);
        free(data);
        return -EIO;
    }

    return data->fd;
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