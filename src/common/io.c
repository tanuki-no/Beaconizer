/*!
 *	\file		io.c
 *	\brief		I/O stuff implementation
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		28/06/2022
 *	\version	1.0
 */

#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include "beaconizer/loop.h"
#include "beaconizer/io.h"
#include "beaconizer/utility.h"

typedef struct io {
    int                 reference_count;

    int                 descriptor;
    uint32_t            events;
    int8_t              close_on_destroy;

    /* Read */   
    io_callback_fn_t    read_callback;
    io_destroy_fn_t     read_destroy;
    void               *read_data;

    /* Write */
    io_callback_fn_t    write_callback;
    io_destroy_fn_t     write_destroy;
    void               *write_data;

    /* Disconnect */
    io_callback_fn_t    disconnect_callback;
    io_destroy_fn_t     disconnect_destroy;
    void               *disconnect_data;
} io_t;

/* Reference count :: refer */
static io_t *io_ref(
    io_t *data) {
    
    if (NULL == data)
        return NULL;

    __sync_fetch_and_add(&data->reference_count, 1);

    return data;
}

/* Reference count :: deref */
static void io_unref(
    io_t *data)
{
    if (NULL == data)
        return;

    if (__sync_sub_and_fetch(&data->reference_count, 1))
        return;

    free(data);
}

/* Clean up */
static void io_destroy_callback(
    void *data) {

    io_t *pdata = data;

    if (NULL != pdata->write_destroy)
        pdata->write_destroy(pdata->write_data);

    if (NULL != pdata->read_destroy)
        pdata->read_destroy(pdata->read_data);

    if (NULL != pdata->disconnect_destroy)
        pdata->disconnect_destroy(pdata->disconnect_data);

    if (0 != pdata->close_on_destroy)
        close(pdata->descriptor);

    pdata->descriptor = -1;
}

/* Handle poll events */
static void io_process_event(
    int         descriptor,
    uint32_t    events,
    void       *user_data) {

    io_t *_io = user_data;

    io_ref(_io);

    /* Handle errors */
    if ((events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))) {
        
        _io->read_callback = NULL;
        _io->write_callback = NULL;

        if (!_io->disconnect_callback) {
            loop_remove_descriptor(_io->descriptor);
            io_unref(_io);
            return;
        }

        if (!_io->disconnect_callback(_io, _io->disconnect_data)) {
            
            if (NULL != _io->disconnect_destroy)
                _io->disconnect_destroy(_io->disconnect_data);

            _io->disconnect_callback = NULL;
            _io->disconnect_destroy = NULL;
            _io->disconnect_data = NULL;

            _io->events &= ~EPOLLRDHUP;

            loop_modify_descriptor(_io->descriptor, _io->events);
        }
    }

    /* Handle reads */
    if ((events & EPOLLIN) && NULL != _io->read_callback) {
        if (!_io->read_callback(_io, _io->read_data)) {
            if (_io->read_destroy)
                _io->read_destroy(_io->read_data);

            _io->read_callback = NULL;
            _io->read_destroy = NULL;
            _io->read_data = NULL;

            _io->events &= ~EPOLLIN;

            loop_modify_descriptor(_io->descriptor, _io->events);
        }
    }

    /* Handle writes */
    if ((events & EPOLLOUT) && _io->write_callback) {
        if (!_io->write_callback(_io, _io->write_data)) {
                
        if (_io->write_destroy)
            _io->write_destroy(_io->write_data);

            _io->write_callback = NULL;
            _io->write_destroy = NULL;
            _io->write_data = NULL;

            _io->events &= ~EPOLLOUT;

            loop_modify_descriptor(_io->descriptor, _io->events);
        }
    }

    io_unref(_io);
}

/* Create and initialize new I/O channel using existing socket */
io_t *io_new(
    int descriptor) {

    io_t *_io = NULL;

    if (0 > descriptor)
        return NULL;

    _io = malloc(sizeof(io_t));
    memset(_io, 0, sizeof(io_t));
    _io->descriptor = descriptor;
    _io->events = 0;
    _io->close_on_destroy = 0;

    if (0 > loop_add_descriptor(_io->descriptor, _io->events, io_process_event, _io, io_destroy_callback)) {
        free(_io);
        return NULL;
    }

    return io_ref(_io);
}

/* Extract descriptor */
int io_get_descriptor(io_t *_io) {

    if (NULL == _io)
        return -ENOTCONN;

    return _io->descriptor;
}

/* Set close on destroy flag */
int io_set_close_on_destroy(
    io_t    *_io,
    int     do_close) {
    
    if (NULL == _io)
        return 0;

    _io->close_on_destroy = do_close;

    return 1;
}

/* Set read callbacks in I/O tracker */
int io_set_read_handler(
    io_t                *_io,
    io_callback_fn_t    callback,
    void                *user_data,
    io_destroy_fn_t     destroy)
{
    uint32_t events;

    if (NULL == _io || 0 > _io->descriptor)
        return 0;

    if (NULL != _io->read_destroy)
            _io->read_destroy(_io->read_data);

    if (callback)
        events = _io->events | EPOLLIN;
    else
        events = _io->events & ~EPOLLIN;

    _io->read_callback = callback;
    _io->read_destroy = destroy;
    _io->read_data = user_data;

    if (events == _io->events)
        return 1;

    if (loop_modify_descriptor(_io->descriptor, events) < 0)
        return 0;

    _io->events = events;

    return 1;
}

/* Set write callbacks in I/O tracker */
int io_set_write_handler(
    io_t            *_io,
    io_callback_fn_t callback,
    void            *user_data,
    io_destroy_fn_t  destroy) {

    uint32_t events;

    if (NULL == _io || 0 > _io->descriptor)
        return 0;

    if (_io->write_destroy)
        _io->write_destroy(_io->write_data);

    if (callback)
        events = _io->events | EPOLLOUT;
    else
        events = _io->events & ~EPOLLOUT;

    _io->write_callback = callback;
    _io->write_destroy = destroy;
    _io->write_data = user_data;

    if (events == _io->events)
        return 1;

    if (loop_modify_descriptor(_io->descriptor, events) < 0)
        return 0;

    _io->events = events;

    return 1;
}

/* Set disonnect callbacks in I/O tracker */
int io_set_disconnect_handler(
    io_t               *_io,
    io_callback_fn_t    callback,
    void               *user_data,
    io_destroy_fn_t     destroy) {

    uint32_t events;

    if (NULL == _io || 0 > _io->descriptor)
        return 0;

    if (NULL != _io->disconnect_destroy)
        _io->disconnect_destroy(_io->disconnect_data);

    if (callback)
        events = _io->events | EPOLLRDHUP;
    else
        events = _io->events & ~EPOLLRDHUP;

    _io->disconnect_callback = callback;
    _io->disconnect_destroy = destroy;
    _io->disconnect_data = user_data;

    if (events == _io->events)
        return 1;

    if (0 > loop_modify_descriptor(_io->descriptor, events))
        return 0;

    _io->events = events;

    return 1;
}

/* Send data through I/O channel */
ssize_t io_send(
    io_t                *_io,
    const struct iovec  *iov,
    int iovcnt)
{
    ssize_t ret;

    if (NULL == _io || 0 > _io->descriptor)
        return -ENOTCONN;

    do {
        ret = writev(_io->descriptor, iov, iovcnt);
    } while (0 > ret && EINTR == errno);

    if (0 > ret)
        return -errno;

    return ret;
}

/* Shutdown I/O channel */
int io_shutdown(
    struct io *_io)
{
    if (NULL == _io || 0 > _io->descriptor)
        return -ENOTCONN;;

    return shutdown(_io->descriptor, SHUT_RDWR);
}


/* Destroy I/O channel */
void io_destroy(
    io_t *_io) {

    if (!_io)
        return;

    _io->read_callback = NULL;
    _io->write_callback = NULL;
    _io->disconnect_callback = NULL;

    loop_remove_descriptor(_io->descriptor);

    io_unref(_io);
}

 /* End of file */