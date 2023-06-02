/*!
 *	\file		io00.c
 *	\brief		Check I/O channel
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		28/06/2022
 *	\version	1.0
 */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/uio.h>

#include "beaconizer/io.h"

#define BTPROTO_HCI 1


int
main() {

    int i;

    printf("Checking I/O channel ...\n");
    printf("-------------------------------------\n");

    int descriptor = -1;


    printf("-------------------------------------\n");
    printf("Done!\n");

    return 0;
}

 /* End of file */