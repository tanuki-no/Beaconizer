/*!
 *	\file		loop.h
 *	\brief		Common loop using socket notifications
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		25/06/2022
 *	\version	1.0
 */

#include <beaconizer/config.h>

#pragma once

#ifndef __BEACONIZER_LOOP_H__
#define __BEACONIZER_LOOP_H__

/* Initialize loop */
int loop_init(void);

/* Add descriptor to watch */
int loop_add_sd(
    const int           sd,                 /* Socket descriptor */
    const uint32_t      event_mask,         /* EPoll mask */
    event_fn_t          callback,           /* Callback on epoll action */
    void               *user_data,          /* User data */
    destructor_t        destructor);        /* Destructor */

/* Modify watched descriptor */
int loop_modify_sd(
    const int           sd,                 /* Socket descriptor */
    uint32_t            event_mask);        /* EPoll mask */

/* Remove watched descriptor */
int loop_remove_sd(
    const int           sd);                /* Socket descriptor */

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