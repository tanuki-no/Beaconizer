/*!
 *	\file		db02.c
 *	\brief		Check UUID to string translation
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		28/06/2022
 *	\version	1.0
 */

#include <stdint.h>
#include <stdio.h>
#include <sys/random.h>

#include "beaconizer/db.h"
#include "beaconizer/utility.h"


int
main() {

    int i, error = 0;
    ssize_t l;
    uint8_t uuid[16];

    printf("Checking bt_uuid128_to_str() ...\n");
    printf("-------------------------------------\n");

    for (i = 0; 10 > i; ++i) {
        l = getrandom(uuid, 16, GRND_NONBLOCK);
        const char* s = uuid2str128(uuid);
        if (NULL != s && 16 == l) {
            printf("UUID: %8.8x-%4.4x-%4.4x-%4.4x-%8.8x%4.4x => %s\n",
                get_le32(&uuid[12]),get_le16(&uuid[10]),
                get_le16(&uuid[8]), get_le16(&uuid[6]),
                get_le32(&uuid[2]), get_le16(&uuid[0]),
                s);
        } else {
            error++;
        }
    }
    printf("-------------------------------------\n");
    printf("Done! Errors found: %d\n", error);

    return 0;
}

 /* End of file */