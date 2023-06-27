/*!
 *	\file		timeout.h
 *	\brief		Timeout used in a loop
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		25/06/2022
 *	\version	1.0
 */

#include <time.h>
#include <beaconizer/config.h>

#pragma once

#ifndef __BEACONIZER_TIMEOUT_H__
#define __BEACONIZER_TIMEOUT_H__

/* ?? */
unsigned int timeout_add(
    struct timespec        *timeout,
    watchdog_fn_t     func,
    void                   *user_data,
    destructor_t            destructor);

/* ?? */
void timeout_remove(
    const unsigned int      id);

/* ?? */
unsigned int watchdog_update(
    struct timespec        *timeout,
    watchdog_fn_t     func,
    void                   *user_data,
    destructor_t            destructor);


#endif /* __BEACONIZER_TIMEOUT_H__ */

/* End of file */