/*!
 *	\file		io.h
 *	\brief		I/O channel
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

/* I/O channel callback */
typedef int (*io_callback_fn_t)(struct io *io, void *user_data);

/* I/O channel destroy callback */
typedef void (*io_destroy_fn_t)(void *data);

/* Create and initialize new I/O channel using existing socket */
struct io *io_new(
    int                 fd);

/* Extract I/O channel descriptor */
int io_get_descriptor(
    struct io          *io);

/* Set I/O channel close on destroy flag */
int io_set_close_on_destroy(
    struct io          *io,
    int                 do_close);

/* Set read op callbacks in I/O channel */
int io_set_read_handler(
    struct io          *io,
    io_callback_fn_t    callback,
    void               *user_data,
    io_destroy_fn_t     destroy);

/* Set write op callbacks in I/O channel */
int io_set_write_handler(
    struct io          *io,
    io_callback_fn_t    callback,
    void               *user_data,
    io_destroy_fn_t     destroy);

/* Set disonnect callbacks in I/O channel */
int io_set_disconnect_handler(
    struct io          *io,
    io_callback_fn_t    callback,
    void               *user_data,
    io_destroy_fn_t     destroy);

/* Send data through I/O channel */
ssize_t io_send(
    struct io          *io,
    const struct iovec *iov,
    int                 iovcnt);

/* Shutdown I/O channel */
int io_shutdown(
    struct io          *io);

/* Destroy I/O channel */
void io_destroy(
    struct io          *io);

#endif /* __BTEST_IO_H__ */

/* End of file */