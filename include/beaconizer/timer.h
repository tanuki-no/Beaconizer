/*!
 *	\file		timer.h
 *	\brief		Timer interface to common loop
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		25/06/2022
 *	\version	1.0
 */

#include <beaconizer/config.h>

#pragma once

#ifndef __BEACONIZER_TIMER_H__
#define __BEACONIZER_TIMER_H__

/* Create timer and add it to event processing */
int create_timer(
    struct timespec    *timeout,            /* Time interval */
    timer_fn_t          callback,           /* Callback */
    void               *user_data,          /* User data */
    destructor_t        destructor);        /* Destructor */

/* Modify existing timer */
int modify_timer(
    const int           id,                 /* Timer ID */
    struct timespec    *timeout);           /* Time interval */

/* Remove event timer */
int destroy_timer(
    const int           id);                /* Timer ID */

#endif /* __BEACONIZER_TIMER_H__ */

/* End of file */