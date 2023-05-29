/*!
 *	\file		utility.h
 *	\brief		Various utilities used here and there
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		30/06/2022
 *	\version	1.0
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>


#pragma once

#ifndef __BTEST_UTILITY_H__
#define __BTEST_UTILITY_H__

#define get_unaligned(ptr)                      \
__extension__ ({                                \
        struct __attribute__((packed)) {        \
                __typeof__(*(ptr)) __v;         \
        } *__p = (__typeof__(__p)) (ptr);       \
        __p->__v;                               \
})

#define put_unaligned(val, ptr)                 \
do {                                            \
        struct __attribute__((packed)) {        \
                __typeof__(*(ptr)) __v;         \
        } *__p = (__typeof__(__p)) (ptr);       \
        __p->__v = (val);                       \
} while (0)

#define PTR_TO_UINT(p) ((unsigned int) ((uintptr_t) (p)))
#define UINT_TO_PTR(u) ((void *) ((uintptr_t) (u)))

#define PTR_TO_INT(p) ((int) ((intptr_t) (p)))
#define INT_TO_PTR(u) ((void *) ((intptr_t) (u)))

#define new0(type, count)                       \
        (type *) (__extension__ ({              \
                size_t __n = (size_t) (count);  \
                size_t __s = sizeof(type);      \
                void *__p;                      \
                __p = util_malloc(__n * __s);   \
                memset(__p, 0, __n * __s);      \
                __p;                            \
        }))

#define newa(t, n) ((t*) alloca(sizeof(t)*(n)))
#define malloc0(n) (calloc((n), 1))

/* Byte converter utilities */
const char *bt_uuid16_to_str(uint16_t uuid);
const char *bt_uuid32_to_str(uint32_t uuid);
const char *bt_uuid128_to_str(const uint8_t uuid[16]);
const char *bt_uuidstr_to_str(const char *uuid);
const char *bt_appear_to_str(uint16_t appearance);

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

/* String utilities */
char *strdelimit(char *str, char *del, char c);
int strsuffix(const char *str, const char *suffix);

typedef void (*util_debug_func_t)(const char *str, void *user_data);

void util_debug_va(util_debug_func_t function, void *user_data,
                                const char *format, va_list va);

void util_debug(util_debug_func_t function, void *user_data,
                                                const char *format, ...)
                                        __attribute__((format(printf, 3, 4)));

void util_hexdump(const char dir, const unsigned char *buf, size_t len,
                                util_debug_func_t function, void *user_data);

unsigned char util_get_dt(const char *parent, const char *name);

ssize_t util_getrandom(void *buf, size_t buflen, unsigned int flags);

uint8_t util_get_uid(uint64_t *bitmap, uint8_t max);
void util_clear_uid(uint64_t *bitmap, uint8_t id);

#endif /* __BTEST_UTILITY_H__ */

/* End of file */