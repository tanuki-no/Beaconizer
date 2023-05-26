/*!
 *	\file		loop.h
 *	\brief		Common loop using socket notifications
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		25/06/2022
 *	\version	1.10
 */

#pragma once

#ifndef __BTEST_LOOP_H__
#define __BTEST_LOOP_H__

/* User function used to destroy loop and cleanup */
typedef void (*btest_loop_destroy_fn) (
    void *user_data);

typedef void (*btest_loop_event_fn) (int fd, uint32_t events, void *user_data);
typedef void (*btest_loop_timeout_fn) (int id, void *user_data);
typedef void (*btest_loop_signal_fn) (int signum, void *user_data);

void mainloop_init(void);
void mainloop_quit(void);
void mainloop_exit_success(void);
void mainloop_exit_failure(void);
int mainloop_run(void);
int mainloop_run_with_signal(mainloop_signal_func func, void *user_data);

int mainloop_add_fd(int fd, uint32_t events, mainloop_event_func callback,
                                void *user_data, mainloop_destroy_func destroy);
int mainloop_modify_fd(int fd, uint32_t events);
int mainloop_remove_fd(int fd);

int mainloop_add_timeout(unsigned int msec, mainloop_timeout_func callback,
                                void *user_data, mainloop_destroy_func destroy);
int mainloop_modify_timeout(int fd, unsigned int msec);
int mainloop_remove_timeout(int id);

int mainloop_set_signal(sigset_t *mask, mainloop_signal_func callback,
                                void *user_data, mainloop_destroy_func destroy);
int mainloop_sd_notify(const char *state);

#endif /* __BTEST_LOOP_H__ */

/* End of file */