/*!
 *	\file		utility.c
 *	\brief		Utilities implementation
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		28/06/2022
 *	\version	1.0
 */

#include <byteswap.h>
#include <stdio.h>
#include <string.h>

#include "btest/utility.h"


#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define BIT(n)  (1 << (n))

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define le16_to_cpu(val) (val)
#define le32_to_cpu(val) (val)
#define le64_to_cpu(val) (val)
#define cpu_to_le16(val) (val)
#define cpu_to_le32(val) (val)
#define cpu_to_le64(val) (val)
#define be16_to_cpu(val) bswap_16(val)
#define be32_to_cpu(val) bswap_32(val)
#define be64_to_cpu(val) bswap_64(val)
#define cpu_to_be16(val) bswap_16(val)
#define cpu_to_be32(val) bswap_32(val) 
#define cpu_to_be64(val) bswap_64(val)
#elif __BYTE_ORDER == __BIG_ENDIAN
#define le16_to_cpu(val) bswap_16(val)
#define le32_to_cpu(val) bswap_32(val)
#define le64_to_cpu(val) bswap_64(val)
#define cpu_to_le16(val) bswap_16(val)
#define cpu_to_le32(val) bswap_32(val)
#define cpu_to_le64(val) bswap_64(val)
#define be16_to_cpu(val) (val)
#define be32_to_cpu(val) (val)
#define be64_to_cpu(val) (val)
#define cpu_to_be16(val) (val)
#define cpu_to_be32(val) (val)
#define cpu_to_be64(val) (val)
#else
#error "Unknown byte order"
#endif


/* LE/BE byte conversion */
int8_t get_s8(
    const void *ptr) {
    return *((int8_t *) ptr);
}

uint8_t get_u8(
    const void *ptr) {
    return *((uint8_t *) ptr);
}

uint16_t get_le16(
    const void *ptr) {
    return le16_to_cpu(get_unaligned((const uint16_t *) ptr));
}

uint16_t get_be16(
    const void *ptr) {
    return be16_to_cpu(get_unaligned((const uint16_t *) ptr));
}

uint32_t get_le24(
    const void *ptr) {
    const uint8_t *src = ptr;
    return ((uint32_t)src[2] << 16) | get_le16(ptr);
}

uint32_t get_be24(
    const void *ptr) {
    const uint8_t *src = ptr;
    return ((uint32_t)src[0] << 16) | get_be16(&src[1]);
}

uint32_t get_le32(
    const void *ptr) {
    return le32_to_cpu(get_unaligned((const uint32_t *) ptr));
}

uint32_t get_be32(
    const void *ptr) {
    return be32_to_cpu(get_unaligned((const uint32_t *) ptr));
}

uint64_t get_le64(
    const void *ptr) {
    return le64_to_cpu(get_unaligned((const uint64_t *) ptr));
}

uint64_t get_be64(
    const void *ptr) {
    return be64_to_cpu(get_unaligned((const uint64_t *) ptr));
}

void put_le16(
    uint16_t    val,
    void       *dst) {
    put_unaligned(cpu_to_le16(val), (uint16_t *) dst);
}

void put_be16(
    uint16_t    val,
    const void *ptr) {
    put_unaligned(cpu_to_be16(val), (uint16_t *) ptr);
}

void put_le24(
    uint32_t    val,
    void       *ptr) {
    put_le16(val, ptr);
    put_unaligned(val >> 16, (uint8_t *) ptr + 2);
}

void put_be24(
    uint32_t    val,
    void       *ptr) {
    put_unaligned(val >> 16, (uint8_t *) ptr + 2);
    put_be16(val, ptr + 1);
}

void put_le32(
    uint32_t    val,
    void       *dst) {
    put_unaligned(cpu_to_le32(val), (uint32_t *) dst);
}

void put_be32(
    uint32_t    val,
    void       *dst) {
    put_unaligned(cpu_to_be32(val), (uint32_t *) dst);
}

void put_le64(
    uint64_t    val,
    void       *dst) {
    put_unaligned(cpu_to_le64(val), (uint64_t *) dst);
}

void put_be64(
    uint64_t    val,
    void       *dst) {
    put_unaligned(cpu_to_be64(val), (uint64_t *) dst);
}

/* Memory utilities */
void *util_malloc(
    size_t      size) {                       

    if (__builtin_expect(!!size, 1)) {
        void *ptr;
                        
        ptr = malloc(size);
        if (ptr)
            return ptr;

        fprintf(stderr, "failed to allocate %zu bytes\n", size);
        abort();
    }       
                        
    return NULL;    
}                       
                        
void *util_memdup(
    const void *src,
    size_t      size) {               

    void *cpy;
                
    if (!src || !size)
        return NULL;

    cpy = util_malloc(size);
    if (!cpy)
        return NULL;

    memcpy(cpy, src, size);

    return cpy;
}

/* String utilities */


 /* End of file */