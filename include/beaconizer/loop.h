/*!
 *	\file		loop.h
 *	\brief		Common loop using socket notifications
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		25/06/2022
 *	\version	1.0
 */

#pragma once

#ifndef __BEACONIZER_LOOP_H__
#define __BEACONIZER_LOOP_H__

/* User function used to process events in a loop */
typedef void (*loop_event_fn_t) (
    int         sd,
    uint32_t    event_mask,
    void        *user_data);

/* User function used to destroy loop and cleanup */
typedef void (*loop_destroy_fn_t) (
    void        *user_data);

/* Initialize loop */
int loop_init(void);

/* Add descriptor to watch */
int loop_add_descriptor(
    const int           fd,
    const uint32_t      event_mask,
    loop_event_fn_t     event_callback,
    void                *user_data,
    loop_destroy_fn_t   destroy_callback);

/* Modify watched descriptor */
int loop_modify_descriptor(
    const int           fd,
    uint32_t            events);

/* Remove watched descriptor */
int loop_remove_descriptor(
    const int           fd);

/* Run loop */
void loop_run(void);

/* Quit loop immediately */
void loop_quit(void);

/* Quit loop and set success */
void loop_exit_success(void);

/* Quit loop and set failure */
void loop_exit_failure(void);

#endif /* __BEACONIZER_LOOP_H__ */

/* End of file */