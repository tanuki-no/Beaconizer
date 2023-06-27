/*!
 *	\file		timer.c
 *	\brief		Simple event loop
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		25/06/2022
 *	\version	1.0
 */

#define _GNU_SOURCE

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>

#include "beaconizer/config.h"
#include "beaconizer/loop.h"
#include "beaconizer/timer.h"

#define ENTRY_CHANGE    4

/* Timeout control structure */
typedef struct {
    int                 id;             /* Timer ID     */
    timer_fn_t          callback;       /* Callback     */
    void               *user_data;      /* User data    */
    destructor_t        destructor;     /* Destructor   */
} timer_entry_t;

/* Cascade callback */
static void timer_callback(
    int                 fd,
    uint32_t            events,
    void               *user_data) {

    const uint32_t escape = EPOLLERR | EPOLLHUP;


    if (events & (EPOLLERR | EPOLLHUP)) {
        return;
    }

    if (NULL != user_data) {

        timer_entry_t *p = user_data;
        uint64_t expired = 0;

        if (sizeof(expired) == read(p->id, &expired, sizeof(expired))) {
            if (NULL != p->callback) {
                p->callback(p->id, p->user_data);
            }
        }
    }
}

/* Destructor */
static void timer_destructor(
    void                   *user_data) {

    if (NULL != user_data) {

        timer_entry_t *p = user_data;

        close(p->id);
        p->id = -1;

        if (NULL != p->destructor) {
            p->destructor(p->user_data);
        }

        free(p);
    }
}

/* Set timeout */
static inline int timeout_set(
    const int           fd,
    struct timespec    *timeout) {

    struct itimerspec itimer = {
        .it_interval.tv_sec     = 0,
        .it_interval.tv_nsec    = 0,
        .it_value.tv_sec        = timeout->tv_sec,
        .it_value.tv_nsec       = timeout->tv_nsec
    };

    return timerfd_settime(fd, CLOCK_MONOTONIC, &itimer, NULL);
}

/* Add timeout to event processing */
int create_timer(
    struct timespec    *timeout,
    timer_fn_t          callback,
    void               *user_data,
    destructor_t        destructor) {

    /* No timeout */
    if (NULL == timeout) {
        return -EINVAL;
    }

    /* No callback */
    if ( NULL == callback) {
        return -EINVAL;
    }

    /* Allocate memory */
    timer_entry_t *p_entry = malloc(sizeof(timer_entry_t));
    if (NULL == p_entry) {
        return -ENOMEM;
    }

    /* Initialize */
    p_entry->callback     = callback;
    p_entry->destructor   = destructor;
    p_entry->user_data    = user_data;

    /* Create timer */
    p_entry->id = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (0 > p_entry->id) {
        free(p_entry);
        return errno;
    }

    /* Set timeout */
    if (NULL != timeout) {
        if (0 != timeout_set(p_entry->id, timeout)) {
            close(p_entry->id);
            free(p_entry);
            return -EIO;
        }
    }

    /* Add descriptor to the loop */
    if (0 > loop_add_sd(p_entry->id, EPOLLIN | EPOLLONESHOT, timer_callback, p_entry, timer_destructor)) {
        close(p_entry->id);
        free(p_entry);
        return -EIO;
    }

    return p_entry->id;
}

/* Modify event processing timeout */
int modify_timer(
    const int               id,
    struct timespec        *timeout) {

    if (NULL != timeout) {
        if (0 != timeout_set(id, timeout)) {
            return -EIO;
        }
    }

    if (0 > loop_modify_sd(id, EPOLLIN | EPOLLONESHOT)) {
        return -EIO;
    }

    return EXIT_SUCCESS;
}

/* Remove event processing timeout */
int destroy_timer(
    const int           id) {
    return loop_remove_sd(id);
}

 /* End of file */