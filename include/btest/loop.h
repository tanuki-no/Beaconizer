/*!
 *	\file		loop.h
 *	\brief		Common loop using socket notifications
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		25/06/2022
 *	\version	1.0
 */

#pragma once

#ifndef __BTEST_LOOP_H__
#define __BTEST_LOOP_H__

/* User function used to destroy loop and cleanup */
typedef void (*loop_destroy_fn_t) (
    void        *user_data);

/* User function used to process events in a loop */
typedef void (*loop_event_fn_t) (
    int         fd,
    uint32_t    events,
    void        *user_data);

/* User function used to process timeout in a loop */
typedef void (*loop_timeout_fn_t) (
    int         id,
    void        *user_data);

/* User function used to process signal in a loop */
typedef void (*loop_signal_fn_t) (
    int         signum,
    void        *user_data);

/* Initialize loop */
void loop_init(void);

/* Quit loop immediately */
void loop_quit(void);
/* Quit loop and set success */
void loop_exit_success(void);
/* Quit loop and set failure */
void loop_exit_failure(void);

/* Run loop */
int loop_run(void);
/* Run loop with signal processing */
int loop_run_with_signal(
    loop_signal_fn_t    func,
    void                *user_data);

/* Add descriptor to watch */
int loop_add_fd(
    const int           fd,
    const uint32_t      events,
    loop_event_fn_t     callback,
    void                *user_data,
    loop_destroy_fn_t   destroy);

/* Modify watched descriptor */
int loop_modify_fd(
    const int           fd,
    uint32_t            events);

/* Remove watched descriptor */
int loop_remove_fd(
    const int           fd);

/* Add timeout to event processing */
int loop_add_timeout(
    const unsigned int  msec,
    loop_timeout_fn_t   callback,
    void                *user_data,
    loop_destroy_fn_t   destroy);

/* Modify event processing timeout */
int loop_modify_timeout(
    const int           id,
    unsigned int        msec);

/* Remove event processing timeout */
int loop_remove_timeout(
    const int           id);

#endif /* __BTEST_LOOP_H__ */

/* End of file */