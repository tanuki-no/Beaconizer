/*!
 *	\file		timeout.c
 *	\brief		Handling timeouts in a loop
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		28/06/2022
 *	\version	1.0
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>

#include "beaconizer/loop.h"
#include "beaconizer/timeout.h"
#include "beaconizer/utility.h"

// /* Timeout callback */
// static void timeout_callback(
//     int                     id,
//     void                   *user_data) {

//     if (NULL != user_data) {

//         timeout_data_t *p = user_data;

//         if (p->callback(p->user_data) &&
//             !loop_modify_timeout(p->id, p->timeout))
//             return;

//         loop_remove_timeout(p->id);
//     }
// }




 /* End of file */