/*!
 *	\file		watchdog.h
 *	\brief		Common loop watchdog stuff
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		25/06/2022
 *	\version	1.0
 */

#pragma once

#ifndef __BTEST_WATCHDOG_H__
#define __BTEST_WATCHDOG_H__

/* User function used to process signal in a loop */
typedef void (*loop_signal_fn_t) (
    int         signum,
    void        *user_data);

/* Initialize watchdog in a loop */
void loop_watchdog_init(void);

/* Destroy watchdog in a loop */
void loop_watchdog_exit(void);

/* Add signal handler */
int loop_set_signal(
    sigset_t            *mask,
    loop_signal_fn_t    callback,
    void                *user_data,
    loop_destroy_fn_t   destroy);

/* Trigger watchdog */
int loop_sd_notify(
    const char *state);

/* Run loop with signal processing */
int loop_run_with_signal(
    loop_signal_fn_t    func,
    void                *user_data);

#endif /* __BTEST_WATCHDOG_H__ */

/* End of file */