/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
 * Copyright (c) 2014-2022 Vivante Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 ****************************************************************************/

/* VeriSilicon 2020 */

/**
 *   @file builtins.h
 *
 *	This file defines some macros for standard library functions. Usually we
 *	dont link against glibc, so we use the builtins. Standard library function
 *	calls are only permitted in debug mode.
 *
 *****************************************************************************/
#ifndef BUILTINS_H_
#define BUILTINS_H_

#include "types.h"

#if defined(__GNUC__)
	#include <stddef.h>

	void *__builtin_memset(void* s, int32_t c, size_t n);
	#define MEMSET(	TARGET, C, LEN)	__builtin_memset(TARGET, C, LEN)

	void *__builtin_memcpy(void* s1, const void* s2, size_t n);
	#define MEMCPY( DST, SRC, LEN)	__builtin_memcpy(DST,SRC,LEN)
#else
	#include <string.h>
	#define MEMSET(	TARGET, C, LEN)	memset(TARGET,C,LEN)
	#define MEMCPY( DST, SRC, LEN)	memcpy(DST,SRC,LEN)
#endif

#define WIPEOBJ( TARGET ) MEMSET( &TARGET, 0, sizeof( TARGET ) )

#endif /*BUILTINS_H_*/
