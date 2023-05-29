/*!
 *	\file		timeout.h
 *	\brief		Timeout used in a loop
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		25/06/2022
 *	\version	1.0
 */

#pragma once

#ifndef __BTEST_TIMEOUT_H__
#define __BTEST_TIMEOUT_H__

typedef int  (*timeout_fn_t)(void *user_data);
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

#endif /* __BTEST_TIMEOUT_H__ */

/* End of file */