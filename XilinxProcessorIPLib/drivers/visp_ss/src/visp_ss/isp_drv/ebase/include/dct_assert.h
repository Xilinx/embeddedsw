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
 *   @file dct_assert.h
 *
 *  This file defines the API for the assertion facility of the embedded lib.
 *
 *****************************************************************************/
/*****************************************************************************/
/**
 * @defgroup module_assert Assert macros
 *
 * @brief The assertion system used by Dream Chip.
 *
 * Example use of the assert system:
 *
 *
 * - In your source file just use the macro
 *
 * @code
 * void foo( uint8_t* pData, size_t size)
 * {
 *     DCT_ASSERT(pData != NULL);
 *     DCT_ASSERT(size > 0);
 * }
 * @endcode
 *
 * @{
 *
 *****************************************************************************/
#ifndef ASSERT_H_
#define ASSERT_H_

#include "types.h"

#define RET_FAILURE             1   //!< general failure

/**
 * @brief   The type of the assert handler. @see assert_handler
 *
 *****************************************************************************/
typedef void (*ASSERT_HANDLER)(void) __attribute__((noreturn));


/**
 *          The assert handler is a function that is called in case an
 *          assertion failed. If no handler is registered, which is the
 *          default, exit() is called.
 *
 *****************************************************************************/
extern ASSERT_HANDLER assert_handler;

#if defined(ENABLE_ASSERT) || !defined(NDEBUG)
/**
 *              Dump information on stderr and exit.
 *
 *  @param      file  Filename where assertion occured.
 *  @param      line  Linenumber where assertion occured.
 *
 *****************************************************************************/
#ifdef __cplusplus
	extern "C"
#endif
void exit_(const char *file, int line) __attribute__((noreturn));

/**
 *              The assert macro.
 *
 *  @param      exp Expression which assumed to be true.
 *
 *****************************************************************************/
#define DCT_ASSERT(exp) do { if (!(exp)){ static CHAR filename[] = __FILE__; exit_(&filename[0], __LINE__);}}while(0)
#else
#define DCT_ASSERT(exp)\
	do {\
		if ((exp)) {\
		} else { \
		}\
	} while(0)
#endif

/* @} module_tracer*/

#endif /*ASSERT_H_*/
