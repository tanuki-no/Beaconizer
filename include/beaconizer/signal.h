/*!
 *	\file		signal.h
 *	\brief		Common loop signall stuff
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		25/06/2022
 *	\version	1.0
 */

#include <beaconizer/config.h>

#pragma once

#ifndef __BEACONIZER_SIGNAL_H__
#define __BEACONIZER_SIGNAL_H__


/* Add signal handler */
int loop_set_signal(
    sigset_t      *mask,
    signal_fn_t    callback,
    void          *user_data,
    destructor_t   destroy);

/* Run loop with signal processing */
int loop_run_with_signal(
    signal_fn_t    func,
    void          *user_data);

#endif /* __BEACONIZER_SIGNAL_H__ */

/* End of file */