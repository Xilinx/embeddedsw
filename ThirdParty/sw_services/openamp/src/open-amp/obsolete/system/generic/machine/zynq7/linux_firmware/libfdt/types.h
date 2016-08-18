#ifndef _LINUX_TYPES_H
#define _LINUX_TYPES_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define cpu_to_be32(x)	\
	((((x) & 0xff000000) >> 24) | \
	 (((x) & 0x00ff0000) >>  8) | \
	 (((x) & 0x0000ff00) <<  8) | \
	 (((x) & 0x000000ff) << 24))

#define be32_to_cpu(x) \
	((((x) & 0xff000000) >> 24) | \
	 (((x) & 0x00ff0000) >>  8) | \
	 (((x) & 0x0000ff00) <<  8) | \
	 (((x) & 0x000000ff) << 24))

#define cpu_to_be64(x)	\
	((((x) & 0xff00000000000000ull) >> 56) | \
	 (((x) & 0x00ff000000000000ull) >> 40) | \
	 (((x) & 0x0000ff0000000000ull) >> 24) | \
	 (((x) & 0x000000ff00000000ull) >>  8) | \
	 (((x) & 0x00000000ff000000ull) <<  8) | \
	 (((x) & 0x0000000000ff0000ull) << 24) | \
	 (((x) & 0x000000000000ff00ull) << 40) | \
	 (((x) & 0x00000000000000ffull) << 56))

#define be64_to_cpu(x)	\
	((((x) & 0xff00000000000000ull) >> 56) | \
	 (((x) & 0x00ff000000000000ull) >> 40) | \
	 (((x) & 0x0000ff0000000000ull) >> 24) | \
	 (((x) & 0x000000ff00000000ull) >>  8) | \
	 (((x) & 0x00000000ff000000ull) <<  8) | \
	 (((x) & 0x0000000000ff0000ull) << 24) | \
	 (((x) & 0x000000000000ff00ull) << 40) | \
	 (((x) & 0x00000000000000ffull) << 56))

#ifndef _STDINT_H

typedef unsigned char u_int8_t;
typedef char int8_t;
typedef unsigned short u_int16_t;
typedef short int16_t;
typedef unsigned int u_int32_t;
typedef int int32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef unsigned long long u_int64_t;
typedef long long int64_t;

typedef long intptr_t;
typedef unsigned long uintptr_t;
typedef unsigned int size_t;

#endif

typedef unsigned short fdt16_t;
typedef unsigned int fdt32_t;
typedef unsigned long long fdt64_t;

#define fdt32_to_cpu(x)		be32_to_cpu(x)
#define cpu_to_fdt32(x)		cpu_to_be32(x)
#define fdt64_to_cpu(x)		be64_to_cpu(x)
#define cpu_to_fdt64(x)		cpu_to_be64(x)

#endif				/* _LINUX_TYPES_H */
