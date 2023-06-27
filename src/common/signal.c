/*!
 *	\file		signal.c
 *	\brief		Handling signals in a loop
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		28/06/2022
 *	\version	1.0
 */

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/signalfd.h>

#include "beaconizer/io.h"
#include "beaconizer/loop.h"
#include "beaconizer/signal.h"


/* Callback */
typedef struct {
    struct io           *io;
    signal_fn_t          func;
    void                *user_data;
} signal_data_t;

static signal_data_t *__s_p_signal_data = NULL;

static int signal_read(
    struct io       *io,
    void            *user_data) {

    signal_data_t *data = user_data;
    struct signalfd_siginfo si;
    ssize_t result;
    int descriptor;
    
    descriptor = io_get_descriptor(io);

    result = read(descriptor, &si, sizeof(si));
    if (sizeof(si) != result)
        return 0;

    if (data && data->func)
        data->func(si.ssi_signo, data->user_data);

    return 1;
}

static struct io *setup_signalfd(
    void            *user_data) {

    struct io *io;
    sigset_t mask;
    int fd;

    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGUSR2);
    sigaddset(&mask, SIGCHLD);

    if (0 > sigprocmask(SIG_BLOCK, &mask, NULL))
        return NULL;

    fd = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
    if (0 > fd )
        return NULL;

    io = io_new(fd);

    io_set_close_on_destroy(io, 1);
    io_set_read_handler(io, signal_read, user_data, free);

    return io;
}

int loop_set_signal(
    sigset_t      *mask,
    signal_fn_t    callback,
    void          *user_data,
    destructor_t   destroy) {

    return EXIT_SUCCESS;
}

int loop_run_with_signal(
    signal_fn_t    func,
    void          *user_data) {

    signal_data_t *data = NULL;
    struct io *io = NULL;
    int ret = -1;

    if (NULL == func)
        return -EINVAL;

    data = malloc(sizeof(signal_data_t));
    memset(data, 0, sizeof(signal_data_t));
    data->func = func;
    data->user_data = user_data;

    io = setup_signalfd(data);
    if (!io) {
        free(data);
        return -errno;
    }

    loop_run();

    io_destroy(io);
    free(__s_p_signal_data);

    return ret;
}

 /* End of file */