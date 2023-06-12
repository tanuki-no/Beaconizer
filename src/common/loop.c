/*!
 *	\file		loop.c
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

#include "beaconizer/config.h"
#include "beaconizer/loop.h"
#include "beaconizer/watchdog.h"

#define ENTRY_CHANGE   32


/* Loop entry type */
typedef struct {
    int                 sd;             /* Socket descriptor */
    uint32_t            event_mask;     /* Event mask */
    loop_event_fn_t     callback;       /* Event callback */
    loop_destroy_fn_t   destroy;        /* Entry clean up */
    void                *user_data;     /* Custom user data */
} loop_data_t;

/* Loop control structure */
static struct {
    int             fd;             /* epoll() descriptor */
    int             terminate;      /* Loop termination flag */
    int             status;         /* Exit status */
    size_t          free_entry;     /* Loop free entry */
    size_t          current_entry;  /* Loop current entry index */
    size_t          entry_count;    /* Loop entry count */
    loop_data_t**   entry;          /* Loop entry dynamic array */
} __s_loop_control = {
    .fd             = -1,
    .terminate      = 0,
    .status         = EXIT_SUCCESS,
    .free_entry     = ENTRY_CHANGE,
    .current_entry  = 0,
    .entry_count    = 0,
    .entry          = NULL
};

/* Initialize loop */
int loop_init(void) {

    unsigned int i;

    __s_loop_control.fd = epoll_create1(EPOLL_CLOEXEC);
    if ( 0 > __s_loop_control.fd) {
        return (0 > errno ? errno : -errno);
    }

    __s_loop_control.entry_count = ENTRY_CHANGE;
    __s_loop_control.entry = malloc(sizeof(loop_data_t) * __s_loop_control.entry_count);
    if (NULL == __s_loop_control.entry) {
        return (0 > ENOMEM ? ENOMEM : -ENOMEM);
    }

    for (i = 0; __s_loop_control.entry_count > i; i++) {
        __s_loop_control.entry[i] = NULL;
    }

    loop_watchdog_init();

    return EXIT_SUCCESS;
}

/* Add descriptor to watch */
int loop_add_descriptor(
    const int           sd,
    const uint32_t      event_mask,
    loop_event_fn_t     callback,
    void                *user_data,
    loop_destroy_fn_t   destroy) {

    loop_data_t        *p_data = NULL;
    struct epoll_event  event;
    int                 error = EXIT_SUCCESS;
    size_t              i;

    /* Exit if socket descriptor is wrong */
    if (0 > sd) {
        return (0 > EINVAL ? EINVAL : -EINVAL);
    }

    /* Exit if callback is NULL */
    if (NULL == callback) {
        return (0 > EINVAL ? EINVAL : -EINVAL);
    }

    /* Reallocate if no free memory */
    if (0 == __s_loop_control.free_entry) {
        size_t i = __s_loop_control.entry_count;

        __s_loop_control.entry_count += ENTRY_CHANGE;
        __s_loop_control.entry = realloc(
            __s_loop_control.entry,
            sizeof(loop_data_t) * __s_loop_control.entry_count);

        for (; __s_loop_control.entry_count > i; ++i) {
            __s_loop_control.entry[i] = NULL;
        }
        __s_loop_control.free_entry = ENTRY_CHANGE;
    }

    /* Allocate loop entry */
    p_data = malloc(sizeof(*p_data));
    if (NULL == p_data) {
        return (0 > ENOMEM ? ENOMEM : -ENOMEM);
    }

    /* Fill loop entry */
    memset(p_data, 0, sizeof(loop_data_t));
    p_data->sd          = sd;
    p_data->event_mask  = event_mask;
    p_data->callback    = callback;
    p_data->destroy     = destroy;
    p_data->user_data   = user_data;

    /* Fill epoll() event entry */
    memset(&event, 0, sizeof(event));
    event.events           = event_mask;
    event.data.ptr         = p_data;

    /* Add epoll entry */
    error = epoll_ctl(__s_loop_control.fd, EPOLL_CTL_ADD, p_data->sd, &event);
    if (0 > error) {
        free(p_data);
        return error;
    }

    /* Store loop entry on success */
    i = __s_loop_control.current_entry;
    __s_loop_control.entry[i] = p_data;
    __s_loop_control.current_entry++;
    __s_loop_control.free_entry--;

    return error;
}

/* Modify watched descriptor */
int loop_modify_descriptor(
    const int           sd,
    uint32_t            event_mask) {

    loop_data_t *p_entry = NULL;
    struct epoll_event event;
    size_t i;
    int error;

    /* Exit if socket descriptor is wrong */
    if (0 > sd) {
        return (0 > EINVAL ? EINVAL : -EINVAL);
    }

    /* Find entry */
    for (i = 0; __s_loop_control.current_entry > i; ++i) {
        if (sd == __s_loop_control.entry[i]->sd) {
            p_entry = __s_loop_control.entry[i];
            break;
        }
    }

    if (NULL == p_entry) {
        return (0 > ENXIO ? ENXIO : -ENXIO);
    }

    memset(&event, 0, sizeof(event));
    event.events = event_mask;
    event.data.ptr = p_entry;

    error = epoll_ctl(__s_loop_control.fd, EPOLL_CTL_MOD, p_entry->sd, &event);
    if (0 > error) {
        return error;
    }
 
    p_entry->event_mask = event_mask;

    return error;
}

/* Remove watched descriptor */
int loop_remove_descriptor(
    const int           sd) {

    size_t i, entry_idx;
    int error;

    /* Exit if socket descriptor is wrong */
    if (0 > sd) {
        return (0 > EINVAL ? EINVAL : -EINVAL);
    }

    loop_data_t *p_entry = NULL;
    for (i = 0; __s_loop_control.current_entry > i; ++i) {
        if (sd == __s_loop_control.entry[i]->sd) {
            p_entry = __s_loop_control.entry[i];
            entry_idx = i;
            break;
        }
    }

    if (NULL == p_entry) {
        return (0 > ENXIO ? ENXIO : -ENXIO);
    }

    __s_loop_control.current_entry--;
    __s_loop_control.free_entry++;
    for (i = entry_idx; __s_loop_control.current_entry > i; ++i) {
        __s_loop_control.entry[i] = __s_loop_control.entry[i + 1];
    }
    __s_loop_control.entry[__s_loop_control.current_entry] = NULL;

    error = epoll_ctl(__s_loop_control.fd, EPOLL_CTL_DEL, p_entry->sd, NULL);

    if (NULL != p_entry->destroy) {
        p_entry->destroy(p_entry->user_data);
    }

    free(p_entry);

    return error;
}

/* Run loop */
void loop_run(void) {

    size_t i;
    int count;
    struct epoll_event* event_pool = malloc(__s_loop_control.entry_count * sizeof(struct epoll_event));

    /* Loop */
    while (0 == __s_loop_control.terminate) {
 
        count = epoll_wait(
            __s_loop_control.fd,
            event_pool,
            __s_loop_control.entry_count,
            -1);

        if (0 > count)
            continue;

        for (i = 0; count > i; i++) {
            loop_data_t *p_entry = event_pool[i].data.ptr;
            p_entry->callback(p_entry->sd, event_pool[i].events, p_entry->user_data);
        }
    }

    free(event_pool);
}

/* Loop clean up */
static void loop_cleanup() {
    loop_data_t *p_data = NULL;
    size_t i = 0;

    /* Free allocated memory */
    if (NULL != __s_loop_control.entry) {

        /* Free entries */
        for (; __s_loop_control.current_entry > i; ++i) {
            p_data = __s_loop_control.entry[i];
            __s_loop_control.entry[i] = NULL;

            if (NULL != p_data) {

                epoll_ctl(__s_loop_control.fd, EPOLL_CTL_DEL, p_data->sd, NULL);

                if (NULL != p_data->destroy) {
                    p_data->destroy(p_data->user_data);
                }

                free(p_data);
            }
        }

        free(__s_loop_control.entry);
    }

    /* Close epoll descriptor() if open */
    if (0 <= __s_loop_control.fd) {
        close(__s_loop_control.fd);
        __s_loop_control.fd = -1;
    }

    __s_loop_control.entry_count    = 0;
    __s_loop_control.current_entry  = 0;
    __s_loop_control.free_entry     = 0;
    __s_loop_control.entry          = NULL;
    __s_loop_control.terminate      = 1;

    /* Shutdown watchdog */
    loop_watchdog_exit();
}

/* Quit loop immediately */
void loop_quit(void) {
    loop_sd_notify("STOPPING=1");
    loop_cleanup();
}

/* Quit loop and set success */
void loop_exit_success(void) {
    loop_cleanup();
    __s_loop_control.status = EXIT_SUCCESS;
}

/* Quit loop and set failure */
void loop_exit_failure(void) {
    loop_cleanup();
    __s_loop_control.status = EXIT_FAILURE;
 }

 /* End of file */