/******************************************************************************
*
* Copyright (C) 2002 - 2014 Xilinx, Inc.  All rights reserved.
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
/*****************************************************************************/
/**
*
* @file xenv_vxworks.h
* @addtogroup common_v1_00_a
* @{
*
* Defines common services specified by xenv.h.
*
* @note
* 	This file is not intended to be included directly by driver code.
* 	Instead, the generic xenv.h file is intended to be included by driver
* 	code.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a wgr  02/28/07 Added cache handling macros.
* 1.00a wgr  02/27/07 Simplified code. Deprecated old-style macro names.
* 1.00a xd   11/03/04 Improved support for doxygen.
*       rmm  09/13/03 CR 177068: Fix compiler warning in XENV_MEM_FILL
*       rmm  10/24/02 Added XENV_USLEEP macro
* 1.00a rmm  07/16/01 First release
* 1.10a wgr  03/22/07 Converted to new coding style.
* </pre>
*
*
******************************************************************************/

#ifndef XENV_VXWORKS_H
#define XENV_VXWORKS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xbasic_types.h"
#include "vxWorks.h"
#include "vxLib.h"
#include "sysLibExtra.h"
#include "cacheLib.h"
#include <string.h>

/*****************************************************************************/
/**
 *
 * Copies a non-overlapping block of memory.
 *
 * @param	DestPtr
 *		Destination address to copy data to.
 *
 * @param	SrcPtr
 * 		Source address to copy data from.
 *
 * @param	Bytes
 * 		Number of bytes to copy.
 *
 * @return	None.
 *
 * @note	XENV_MEM_COPY is deprecated. Use memcpy() instead.
 *
 *****************************************************************************/

#define XENV_MEM_COPY(DestPtr, SrcPtr, Bytes) \
	memcpy((void *) DestPtr, (const void *) SrcPtr, (size_t) Bytes)


/*****************************************************************************/
/**
 *
 * Fills an area of memory with constant data.
 *
 * @param	DestPtr
 *		Destination address to copy data to.
 *
 * @param	Data
 * 		Value to set.
 *
 * @param	Bytes
 * 		Number of bytes to copy.
 *
 * @return	None.
 *
 * @note	XENV_MEM_FILL is deprecated. Use memset() instead.
 *
 *****************************************************************************/

#define XENV_MEM_FILL(DestPtr, Data, Bytes) \
	memset((void *) DestPtr, (int) Data, (size_t) Bytes)


#if (CPU_FAMILY==PPC)
/**
 * A structure that contains a time stamp used by other time stamp macros
 * defined below. This structure is processor dependent.
 */
typedef struct
{
	u32 TimeBaseUpper;
	u32 TimeBaseLower;
} XENV_TIME_STAMP;

/*****************************************************************************/
/**
 *
 * Time is derived from the 64 bit PPC timebase register
 *
 * @param   StampPtr is the storage for the retrieved time stamp.
 *
 * @return  None.
 *
 * @note
 *
 * Signature: void XENV_TIME_STAMP_GET(XTIME_STAMP *StampPtr)
 *
 *****************************************************************************/
#define XENV_TIME_STAMP_GET(StampPtr)                   \
{                                                       \
    vxTimeBaseGet((UINT32*)&(StampPtr)->TimeBaseUpper,  \
                  (UINT32*)&(StampPtr)->TimeBaseLower); \
}

/*****************************************************************************/
/**
 *
 * This macro is not yet implemented and always returns 0.
 *
 * @param   Stamp1Ptr is the first sampled time stamp.
 * @param   Stamp2Ptr is the second sampled time stamp.
 *
 * @return  0
 *
 * @note    None.
 *
 *****************************************************************************/
#define XENV_TIME_STAMP_DELTA_US(Stamp1Ptr, Stamp2Ptr)     (0)

/*****************************************************************************/
/**
 *
 * This macro is not yet implemented and always returns 0.
 *
 * @param   Stamp1Ptr is the first sampled time stamp.
 * @param   Stamp2Ptr is the second sampled time stamp.
 *
 * @return  0
 *
 * @note
 *
 * None.
 *
 *****************************************************************************/
#define XENV_TIME_STAMP_DELTA_MS(Stamp1Ptr, Stamp2Ptr)     (0)


/* For non-PPC systems the above macros are not defined. Generate a error to
 * make the developer aware of the problem.
 */
#else
#error "XENV_TIME_STAMP_GET used in a non-PPC system. Aborting."
#endif


/*****************************************************************************/
/**
 *
 * Delay the specified number of microseconds.
 *
 * @param	delay
 * 		Number of microseconds to delay.
 *
 * @return	None.
 *
 *****************************************************************************/

#define XENV_USLEEP(delay)	sysUsDelay(delay)

#define udelay(delay)	sysUsDelay(delay)


/******************************************************************************
 *
 * CACHE handling macros / mappings
 *
 ******************************************************************************/
/******************************************************************************
 *
 * PowerPC case
 *
 ******************************************************************************/

#if (CPU_FAMILY==PPC)

#define XCACHE_ENABLE_CACHE()	\
		{ XCACHE_ENABLE_DCACHE(); XCACHE_ENABLE_ICACHE(); }

#define XCACHE_DISABLE_CACHE()	\
		{ XCACHE_DISABLE_DCACHE(); XCACHE_DISABLE_ICACHE(); }


#define XCACHE_ENABLE_DCACHE()		cacheEnable(DATA_CACHE)
#define XCACHE_DISABLE_DCACHE()		cacheDisable(DATA_CACHE)
#define XCACHE_ENABLE_ICACHE()		cacheEnable(INSTRUCTION_CACHE)
#define XCACHE_DISABLE_ICACHE()		cacheDisable(INSTRUCTION_CACHE)


#define XCACHE_INVALIDATE_DCACHE_RANGE(Addr, Len) \
		cacheInvalidate(DATA_CACHE, (void *)(Addr), (Len))

#define XCACHE_FLUSH_DCACHE_RANGE(Addr, Len) \
		cacheFlush(DATA_CACHE, (void *)(Addr), (Len))

#define XCACHE_INVALIDATE_ICACHE_RANGE(Addr, Len) \
		cacheInvalidate(INSTRUCTION_CACHE, (void *)(Addr), (Len))

#define XCACHE_FLUSH_ICACHE_RANGE(Addr, Len) \
		cacheFlush(INSTRUCTION_CACHE, (void *)(Addr), (Len))


/******************************************************************************
 *
 * Unknown processor / architecture
 *
 ******************************************************************************/

#else
#error "Unknown processor / architecture. Must be PPC for VxWorks."
#endif


#ifdef __cplusplus
}
#endif

#endif	/* #ifdef XENV_VXWORKS_H */

/** @} */
