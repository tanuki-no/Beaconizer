/*!
 *	\file		watchdog.h
 *	\brief		Common loop watchdog stuff
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		25/06/2022
 *	\version	1.0
 */

#include <beaconizer/config.h>

#pragma once

#ifndef __BEACONIZER_WATCHDOG_H__
#define __BEACONIZER_WATCHDOG_H__


/* Initialize watchdog in a loop */
void watchdog_init(void);

/* ?? */
unsigned int watchdog_add(
    struct timespec    *timeout,
    watchdog_fn_t       func,
    void               *user_data,
    destructor_t        destructor);

/* ?? */
void watchdog_remove(
    const unsigned int      id);

/* ?? */
unsigned int watchdog_update(
    struct timespec    *timeout,
    watchdog_fn_t       func,
    void               *user_data,
    destructor_t        destructor);

/* Trigger watchdog */
int watchdog_notify(
    const char *state);

/* Destroy watchdog in a loop */
void watchdog_exit(void);

#endif /* __BEACONIZER_WATCHDOG_H__ */

/* End of file */