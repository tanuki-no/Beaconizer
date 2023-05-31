/*!
 *	\file		io.h
 *	\brief		Various I/O stuff
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		25/06/2022
 *	\version	1.0
 */

#include <sys/uio.h>


#pragma once

#ifndef __BTEST_IO_H__
#define __BTEST_IO_H__

/* Forward declaration */
struct io;

/* I/O callback */
typedef int (*io_callback_fn_t)(struct io *io, void *user_data);

/* Detroy callback */
typedef void (*io_destroy_fn_t)(void *data);

struct io *io_new(
    int                 fd);

void io_destroy(
    struct io          *io);

int io_get_fd(
    struct io          *io);

int io_set_close_on_destroy(
    struct io          *io,
    int                 do_close);

ssize_t io_send(
    struct io          *io,
    const struct iovec *iov, int iovcnt);

int io_shutdown(
    struct io          *io);

int io_set_read_handler(
    struct io          *io,
    io_callback_fn_t    callback,
    void               *user_data,
    io_destroy_fn_t     destroy);

int io_set_write_handler(
    struct io          *io,
    io_callback_fn_t    callback,
    void               *user_data,
    io_destroy_fn_t     destroy);

int io_set_disconnect_handler(
    struct io          *io,
    io_callback_fn_t    callback,
    void               *user_data,
    io_destroy_fn_t     destroy);

#endif /* __BTEST_IO_H__ */

/* End of file */