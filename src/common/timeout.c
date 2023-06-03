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

 /* End of file */