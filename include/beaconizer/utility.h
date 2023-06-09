/*!
 *	\file		utility.h
 *	\brief		Various utilities used here and there
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		30/06/2022
 *	\version	1.0
 */

#include <stdint.h>

#pragma once

#ifndef __BEACONIZER_UTILITY_H__
#define __BEACONIZER_UTILITY_H__

/* Byte converter utilities */
int8_t      get_s8(const void *ptr);
uint8_t     get_u8(const void *ptr);

uint16_t    get_le16(const void *ptr);
uint16_t    get_be16(const void *ptr);

uint32_t    get_le24(const void *ptr);
uint32_t    get_be24(const void *ptr);

uint32_t    get_le32(const void *ptr);
uint32_t    get_be32(const void *ptr);

uint64_t    get_le64(const void *ptr);
uint64_t    get_be64(const void *ptr);

void put_le16(uint16_t val, void *dst);
void put_be16(uint16_t val, const void *ptr);

void put_le24(uint32_t val, void *ptr);
void put_be24(uint32_t val, void *ptr);

void put_le32(uint32_t val, void *dst);
void put_be32(uint32_t val, void *dst);

void put_le64(uint64_t val, void *dst);
void put_be64(uint64_t val, void *dst);

/* Memory utilities */
void *util_malloc(size_t size);
void *util_memdup(const void *src, size_t size);

/* Debug utilities */
typedef void (*util_debug_fn_t)(const char *str, void *user_data);

void util_debug_va(
    util_debug_fn_t     function,
    void               *user_data,
    const char         *format,
    va_list             va);

void util_debug(
    util_debug_fn_t     function,
    void               *user_data,
    const char         *format, ...)
    __attribute__((format(printf, 3, 4)));

void util_hexdump(
    const char          dir,
    const unsigned char *buf,
    size_t              len,
    util_debug_fn_t     function,
    void               *user_data);

/* Helper for getting the dirent type in case readdir returns DT_UNKNOWN */
unsigned char util_get_dt(const char *parent, const char *name);

/* Helpers for bitfield operations */
uint8_t util_get_uid(uint64_t *bitmap, uint8_t max);
void util_clear_uid(uint64_t *bitmap, uint8_t id);

/* String utilities */
char *strdelimit(char *str, char *del, char c);
int strsuffix(const char *str, const char *suffix);

#endif /* __BEACONIZER_UTILITY_H__ */

/* End of file */