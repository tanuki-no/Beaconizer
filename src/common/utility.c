/*!
 *	\file		utility.c
 *	\brief		Utilities implementation
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		28/06/2022
 *	\version	1.0
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <byteswap.h>
#include <ctype.h>
#include <limits.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "beaconizer/utility.h"


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

/* Debug utilities */
void util_debug_va(
    util_debug_fn_t     function,
    void               *user_data,
    const char         *format,
    va_list             va) {

    char str[MAX_INPUT];

    if (!function || !format)
        return;

    vsnprintf(str, sizeof(str), format, va);

    function(str, user_data);
}

void util_debug(
    util_debug_fn_t     function,
    void               *user_data,
    const char         *format, ...) {

    va_list ap;

    if (!function || !format)
        return;

    va_start(ap, format);
    util_debug_va(function, user_data, format, ap);
    va_end(ap);
}

void util_hexdump(
    const char          dir,
    const unsigned char *buf,
    size_t              len,
    util_debug_fn_t     function,
    void               *user_data) {

    static const char hexdigits[] = "0123456789abcdef";
    char str[68];
    size_t i;

    if (!function || !len)
        return;

    str[0] = dir;

    for (i = 0; i < len; i++) {
        str[((i % 16) * 3) + 1] = ' ';
        str[((i % 16) * 3) + 2] = hexdigits[buf[i] >> 4];
        str[((i % 16) * 3) + 3] = hexdigits[buf[i] & 0xf];
        str[(i % 16) + 51] = isprint(buf[i]) ? buf[i] : '.';

        if ((i + 1) % 16 == 0) {
            str[49] = ' ';
            str[50] = ' ';
            str[67] = '\0';
            function(str, user_data);
            str[0] = ' ';
        }
    }

    if (i % 16 > 0) {
        size_t j;
        for (j = (i % 16); j < 16; j++) {
            str[(j * 3) + 1] = ' ';
            str[(j * 3) + 2] = ' ';
            str[(j * 3) + 3] = ' ';
            str[j + 51] = ' ';
        }

        str[49] = ' ';
        str[50] = ' ';
        str[67] = '\0';
        function(str, user_data);
    }
}

/* Helper for getting the dirent type in case readdir returns DT_UNKNOWN */
unsigned char util_get_dt(
    const char         *parent,
    const char         *name) {

    char filename[PATH_MAX];
    struct stat st;

    snprintf(filename, PATH_MAX, "%s/%s", parent, name);
    if (lstat(filename, &st) == 0 && S_ISDIR(st.st_mode))
        return DT_DIR;

    return DT_UNKNOWN;
}

/* Find unique id in range from 1 to max but no bigger than 64. */
uint8_t util_get_uid(
    uint64_t           *bitmap,
    uint8_t             max) {

    uint8_t id;

    id = ffsll(~*bitmap);

    if (!id || id > max)
        return 0;

    *bitmap |= ((uint64_t)1) << (id - 1);

    return id;
}

/* Clear id bit in bitmap */
void util_clear_uid(
    uint64_t           *bitmap,
    uint8_t             id) {

    if (!id || id > 64)
        return;

    *bitmap &= ~(((uint64_t)1) << (id - 1));
}

/* String utilities */
char *strdelimit(char *str, char *del, char c) {

    char *dup;

    if (!str)
        return NULL;

    dup = strdup(str);
    if (dup[0] == '\0')
        return dup;

    while (del[0] != '\0') {
        char *rep = dup;

        while ((rep = strchr(rep, del[0])))
            rep[0] = c;

        del++;
    }

    return dup;
}

int strsuffix(const char *str, const char *suffix) {

    int len;
    int suffix_len;

    if (!str || !suffix)
        return -1;

    if (str[0] == '\0' && suffix[0] != '\0')
        return -1;

    if (suffix[0] == '\0' && str[0] != '\0')
        return -1;

    len = strlen(str);
    suffix_len = strlen(suffix);
    if (len < suffix_len)
        return -1;

    return strncmp(str + len - suffix_len, suffix, suffix_len);
}

 /* End of file */