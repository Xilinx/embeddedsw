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
 * @file align.h
 *
 * @brief
 *
 *****************************************************************************/
#ifndef __ALIGN_H__
#define __ALIGN_H__

#ifndef ALIGN_UP
	#define ALIGN_UP(addr, align)		( ((addr) + ((align)-1)) & ~(uintptr_t)((align)-1) ) //!< Aligns addr to next higher aligned addr; align must be a power of two.
#endif
#define ALIGN_DOWN(addr, align)		( ((addr)              ) & ~(uintptr_t)((align)-1) ) //!< Aligns addr to next lower aligned addr; align must be a power of two.

#define ALIGN_SIZE_16BYTES          ( 0x10 )
#define ALIGN_UP_16BYTES(addr)	    ( ALIGN_UP(addr, ALIGN_SIZE_16BYTES) )

#define ALIGN_SIZE_1K               ( 0x400 )
#define ALIGN_UP_1K(addr)			( ALIGN_UP(addr, ALIGN_SIZE_1K) )

#define ALIGN_SIZE_4K               ( 0x1000 )
#define ALIGN_UP_4K(addr)			( ALIGN_UP(addr, ALIGN_SIZE_4K) )

#define MAX_ALIGNED_SIZE(size, align) ( ALIGN_UP(size, align) + align ) //!< Calcs max size of memory required to be able to hold a block of size bytes with a start address aligned to align.

#endif /* __ALIGN_H__ */
