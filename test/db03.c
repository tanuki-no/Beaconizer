/*!
 *	\file		db03.c
 *	\brief		Check 16-bit device detect string
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		28/06/2022
 *	\version	1.0
 */

#include <stdio.h>
#include <string.h>

#include "beaconizer/db.h"

int
main() {

    int i;

    printf("Checking detect_device() ...\n");
    printf("-------------------------------------\n");

    for (i = 0; UINT16_MAX > i; ++i) {
        const char* s = detect_device(i & 0xFFFF);
        if (NULL != s && 0 != strcmp(s, "Undefined")) {
            printf("16-bit UUID: 0x%0.4X, %s\n", i & 0xFFFF, s);
        }
    }
    printf("-------------------------------------\n");
    printf("Done!\n");

    return 0;
}

 /* End of file */