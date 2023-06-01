/*!
 *	\file		db01.c
 *	\brief		Check 32-bit to string translation
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		28/06/2022
 *	\version	1.0
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "beaconizer/db.h"

int
main() {

    uint64_t i;

    printf("Checking bt_uuid32_to_str() ...\n");
    printf("-------------------------------------\n");

    for (i = 0; UINT16_MAX > i; ++i) {
        const char* s = uuid2str32(i & 0xFFFFFFFF);
        if (NULL != s && 0 != strcmp(s, "Unknown")) {
            printf("32-bit UUID: 0x%0.8X, %s\n", (uint32_t) (i & 0xFFFFFFFF), s);
        }
    }
    printf("-------------------------------------\n");
    printf("Done!\n");

    return 0;
}

 /* End of file */