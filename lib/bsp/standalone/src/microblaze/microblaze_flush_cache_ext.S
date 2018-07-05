/******************************************************************************
*
* Copyright (C) 2008 - 2018 Xilinx, Inc. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/******************************************************************************
*
* microblaze_flush_cache_ext()
*
* Flush the entire L2 Cache
*
*
*******************************************************************************/

#include "xparameters.h"
#include "microblaze_instructions.h"

#define XPAR_MICROBLAZE_EXT_CACHE_LINE_LEN	16

#define CACHEABLE_REGION_SIZE (XPAR_MICROBLAZE_DCACHE_HIGHADDR - XPAR_MICROBLAZE_DCACHE_BASEADDR)


	.text
	.globl	microblaze_flush_cache_ext
	.ent	microblaze_flush_cache_ext
	.align	2

microblaze_flush_cache_ext:

#if ((XPAR_MICROBLAZE_INTERCONNECT==3) && (XPAR_MICROBLAZE_USE_DCACHE==1))
	ADDIK	r5, r0, XPAR_MICROBLAZE_DCACHE_BASEADDR & (-(4 * XPAR_MICROBLAZE_DCACHE_LINE_LEN))

	ADDIK	r6, r0, CACHEABLE_REGION_SIZE-(4 * XPAR_MICROBLAZE_EXT_CACHE_LINE_LEN)
	ANDI	r6, r6, -(4 * XPAR_MICROBLAZE_EXT_CACHE_LINE_LEN)

Loop_start:
	wdc.ext.flush r5, r6
#if defined (__arch64__)
	addlik	r6, r6,-(4 * XPAR_MICROBLAZE_EXT_CACHE_LINE_LEN)
	beagei r6, Loop_start
#else
	bgtid	r6,Loop_start
	addik	r6, r6,-(4 * XPAR_MICROBLAZE_EXT_CACHE_LINE_LEN)
#endif
#endif
	rtsd	r15, 8
	nop
	.end	microblaze_flush_cache_ext
