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

#define ENTRY_CHANGE   4


/* Loop entry type */
typedef struct {
    int                 sd;             /* Socket descriptor */
    uint32_t            event_mask;     /* Event mask */
    loop_event_fn_t     callback;       /* Event callback */
    loop_destroy_fn_t   destroy;        /* Entry clean up */
    void                *user_data;     /* Custom user data */
} loop_data_t;

/* Entry storage */
typedef struct {
    size_t              count;          /* Allocated entry count */
    size_t              top;            /* Current entry. Always point to the top */  
    loop_data_t       **entry;          /* Pointer to entry */
} storage_t;

typedef struct epoll_event epoll_event_t;

/* Loop control structure */
static struct {
    int             fd;             /* epoll() descriptor */
    
    int             terminate;      /* Loop termination flag */
    int             status;         /* Exit status */

    storage_t       storage;        /* Entry storage */
} __s_data = {
    .fd             = -1,

    .terminate      = 0,
    .status         = EXIT_SUCCESS,

    .storage        = {
        .count      = 0,
        .top        = 0,
        .entry      = NULL
    }
};

/* Initialize loop */
int loop_init(void) {

    size_t i;

    /* Create epoll() descriptor */
    __s_data.fd = epoll_create1(EPOLL_CLOEXEC);
    if ( 0 > __s_data.fd) {
        return (0 > errno ? errno : -errno);
    }

    /* Allocate storage */
    __s_data.storage.entry = malloc(sizeof(loop_data_t *) * ENTRY_CHANGE);
    if (NULL == __s_data.storage.entry) {
        return (0 > ENOMEM ? ENOMEM : -ENOMEM);
    }

    /* Initialize storage */
    __s_data.storage.count = ENTRY_CHANGE;
    for (i = 0; __s_data.storage.count > i; i++) {
        __s_data.storage.entry[i] = NULL;
    }

    /* Initialize watchdog */
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
    if (__s_data.storage.count <= (__s_data.storage.top + 1)) {

        loop_data_t** ep = malloc(sizeof(loop_data_t *) * (__s_data.storage.count + ENTRY_CHANGE));
        if (NULL == ep) {
            return (0 > ENOMEM ? ENOMEM : -ENOMEM);
        }

        for (i = 0; __s_data.storage.count > i; ++i) {
            ep[i] = __s_data.storage.entry[i];
            __s_data.storage.entry[i] = NULL;
        }

        __s_data.storage.count += ENTRY_CHANGE;

        do {
            ep[i++] = NULL;
        } while (__s_data.storage.count > i);

        free(__s_data.storage.entry);
        __s_data.storage.entry = ep;
    }

    /* Allocate loop entry */
    p_data = malloc(sizeof(*p_data));
    if (NULL == p_data) {
        return (0 > ENOMEM ? ENOMEM : -ENOMEM);
    }

    /* Fill loop entry */
    p_data->sd          = sd;
    p_data->event_mask  = event_mask;
    p_data->callback    = callback;
    p_data->destroy     = destroy;
    p_data->user_data   = user_data;

    /* Fill epoll() event entry */
    event.events        = event_mask;
    event.data.ptr      = p_data;

    /* Add epoll entry */
    if (0 != epoll_ctl(__s_data.fd, EPOLL_CTL_ADD, p_data->sd, &event)) {
        free(p_data);
        return errno;
    }

    /* Store loop entry on success */
    __s_data.storage.entry[__s_data.storage.top++] = p_data;

    return EXIT_SUCCESS;
}

/* Modify watched descriptor */
int loop_modify_descriptor(
    const int           sd,
    uint32_t            event_mask) {

    loop_data_t *p_entry = NULL;
    struct epoll_event event;
    size_t i;
    int error = EXIT_SUCCESS;

    /* Exit if socket descriptor is wrong */
    if (0 > sd) {
        return (0 > EINVAL ? EINVAL : -EINVAL);
    }

    /* Find entry */
    for (i = 0; __s_data.storage.top > i; ++i) {
        if (sd == __s_data.storage.entry[i]->sd) {
            p_entry = __s_data.storage.entry[i];
            break;
        }
    }

    /* Entry not found. Exit! */
    if (NULL == p_entry) {
        return (0 > ENXIO ? ENXIO : -ENXIO);
    }

    memset(&event, 0, sizeof(event));
    event.events = event_mask;
    event.data.ptr = p_entry;

    error = epoll_ctl(__s_data.fd, EPOLL_CTL_MOD, p_entry->sd, &event);
    if (0 > error) {
        return error;
    }
 
    p_entry->event_mask = event_mask;

    return error;
}

/* Remove watched descriptor */
int loop_remove_descriptor(
    const int           sd) {

    size_t i, entry_to_delete = 0;
    loop_data_t *p_entry = NULL;
    int error = EXIT_SUCCESS;

    /* Exit if socket descriptor is wrong */
    if (0 > sd) {
        return (0 > EINVAL ? EINVAL : -EINVAL);
    }

    /* Find entry */
    for (i = 0; __s_data.storage.top > i; ++i) {
        if (sd == __s_data.storage.entry[i]->sd) {
            p_entry = __s_data.storage.entry[i];
            entry_to_delete = i;
            break;
        }
    }

    /* Entry not found */
    if (NULL == p_entry) {
        return (0 > ENXIO ? ENXIO : -ENXIO);
    }

    /* Shift entries */
    for (i = entry_to_delete; __s_data.storage.top > (i + 2); ++i) {
        __s_data.storage.entry[i] = __s_data.storage.entry[i + 1];
    }
    __s_data.storage.top--;
    __s_data.storage.entry[__s_data.storage.top] = NULL;

    /* Remove entry from epoll() queue */
    error = epoll_ctl(__s_data.fd, EPOLL_CTL_DEL, p_entry->sd, NULL);

    /* Call destructor */
    if (NULL != p_entry->destroy) {
        p_entry->destroy(p_entry->user_data);
    }

    /* Clean up */
    free(p_entry);

    return error;
}

/* Run loop */
void loop_run(void) {

    size_t i;
    int count;
    struct epoll_event* event_pool = malloc(sizeof(struct epoll_event) * __s_data.storage.count);

    /* Loop */
    while (0 == __s_data.terminate) {
 
        /* Wait for events */
        count = epoll_wait(__s_data.fd, event_pool, __s_data.storage.top, -1);

        /* Nothing to process */
        if (0 > count)
            continue;

        /* Process events */
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

    /* Clean up loop entries */
    if (NULL != __s_data.storage.entry) {

        /* Free entries */
        for (; __s_data.storage.top > i; ++i) {
            p_data = __s_data.storage.entry[i];
            __s_data.storage.entry[i] = NULL;

            if (NULL != p_data) {

                epoll_ctl(__s_data.fd, EPOLL_CTL_DEL, p_data->sd, NULL);

                if (NULL != p_data->destroy) {
                    p_data->destroy(p_data->user_data);
                }

                free(p_data);
            }
        }

        free(__s_data.storage.entry);

        __s_data.storage.top = 0;
        __s_data.storage.count = 0;
        __s_data.storage.entry = NULL;
    }

    /* Close epoll descriptor() if open */
    if (0 <= __s_data.fd) {
        close(__s_data.fd);
        __s_data.fd = -1;
    }

    /* Done! */
    __s_data.terminate = 1;
    
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
    __s_data.status = EXIT_SUCCESS;
}

/* Quit loop and set failure */
void loop_exit_failure(void) {
    loop_cleanup();
    __s_data.status = EXIT_FAILURE;
 }

 /* End of file */