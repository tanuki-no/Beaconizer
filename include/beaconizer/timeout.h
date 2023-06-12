/*!
 *	\file		timeout.h
 *	\brief		Timeout used in a loop
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		25/06/2022
 *	\version	1.0
 */

#pragma once

#ifndef __BEACONIZER_TIMEOUT_H__
#define __BEACONIZER_TIMEOUT_H__

/* User function used to process timeout in a loop */
typedef void (*loop_timeout_fn_t) (
    int         id,
    void        *user_data);

/* User callback */
typedef int  (*timeout_fn_t)(void *user_data);

/* Destroy timeout */
typedef void (*timeout_destroy_fn_t)(void *user_data);

unsigned int timeout_add(
    const unsigned int      timeout,
    timeout_fn_t            func,
    void                   *user_data,
    timeout_destroy_fn_t    destroy);

void timeout_remove(
    const unsigned int      id);

unsigned int timeout_add_seconds(
    const unsigned int      timeout,
    timeout_fn_t            func,
    void                   *user_data,
    timeout_destroy_fn_t    destroy);

/* Add timeout to event processing */
int loop_add_timeout(
    const unsigned int  msec,
    loop_timeout_fn_t   timeout_callback,
    void                *user_data,
    loop_destroy_fn_t   destroy_callback);

/* Modify event processing timeout */
int loop_modify_timeout(
    const int           id,
    unsigned int        msec);

/* Remove event processing timeout */
int loop_remove_timeout(
    const int           id);

#endif /* __BEACONIZER_TIMEOUT_H__ */

/* End of file */