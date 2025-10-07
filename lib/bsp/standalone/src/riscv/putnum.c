/******************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef UNDEFINE_FILE_OPS
#include "xil_types.h"
#include "xil_printf.h"

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((weak)) void putnum(u32 num);

#ifdef __cplusplus
}
#endif

/*
 * putnum -- print a 32 bit number in hex
 * Declared as __weak attribute
 */

__attribute__((weak)) void putnum(u32 num)
{
	char buf[9];
	int cnt;
	char *ptr;
	int digit;
	ptr = buf;

	for (cnt = 7 ; cnt >= 0 ; cnt--) {
		digit = (num >> (cnt * 4)) & 0xf;
		if (digit <= 9)
			*ptr++ = (char) ('0' + digit);
		else
			*ptr++ = (char) ('a' - 10 + digit);
	}
	*ptr = (char) 0;

	print (buf);
}
#endif
