/******************************************************************************
*
* Copyright (C) 2007 - 2014 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xmutex_hw.h
* @addtogroup mutex_v4_3
* @{
*
* This header file contains identifiers/definitions and macros that can be used
* to access the device.  The user should refer to the hardware device
* specification for more details of the device operation. The driver functions
* are defined in xmutex.h.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a va            First release
* 1.00a ecm  06/01/07 Cleanup, new coding standard, check into XCS
* 3.00a hbm  10/15/09 Migrated to HAL phase 1 to use xil_io, xil_types,
*			and xil_assert.
* </pre>
*
******************************************************************************/

#ifndef XMUTEX_HW_H		/* prevent circular inclusions */
#define XMUTEX_HW_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/


/** @name Register Offset Definitions
 * Register offsets within a Mutex, there are multiple
 * Mutexes within a single device
 * @{
 */

#define XMU_MUTEX_REG_OFFSET	0 /**< Mutex register */
#define XMU_USER_REG_OFFSET	4 /**< User register */

/* @} */



/** @name Mutex Register Bit Definitions
 * @{
 */
#define LOCKED_BIT	0x00000001 /**< This is the Lock bit. Set to 1 indicates
				     *  that the lock is  currently owned by the
				     *  processor ID specified by the CPU_ID
				     *  (OWNER_MASK) in this register. Set to 0
				     *  indicates that the lock is free.
				     */
#define OWNER_MASK	0x000001FE /**< This is CPU_ID Mask. CPU_ID indicates
				     *  the ID the processor holding the lock.
				     */
#define OWNER_SHIFT	0x00000001 /**< This is CPU_ID Shift */
/* @} */


/*
 * Each Mutex consumes 256 bytes of address space
 */
#define XMU_MUTEX_OFFSET 256


/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define XMutex_Offset(i)   (XMU_MUTEX_OFFSET * i)

/*****************************************************************************/
/**
* Read one of the Mutex registers.
*
* @param	BaseAddress contains the base address of the Mutex device.
* @param	MutexNumber contains the specific Mutex within the device,
*		a zero based number, 0 - (NumMutexConfigured in HW - 1).
* @param	RegOffset contains the offset from the 1st register of the Mutex
*		to select the specific register of the Mutex.
*
* @return	The 32 bit value read from the register.
*
* @note		C-style signature:
*		u32 XMutex_ReadReg(u32 BaseAddress, u8 MutexNumber,
*				unsigned RegOffset)
*
******************************************************************************/
#define XMutex_ReadReg(BaseAddress, MutexNumber, RegOffset)		\
	Xil_In32((BaseAddress) + XMutex_Offset(MutexNumber) + (RegOffset))

/*****************************************************************************/
/**
* Write a specified value to a register of a Mutex.
*
* @param	BaseAddress is the base address of the Mutex device.
* @param	MutexNumber is the specific Mutex within the device, a
*		zero based number, 0 - (NumMutexConfigured in HW - 1).
* @param	RegOffset contain the offset from the 1st register of the
*		Mutex to select the specific register of the Mutex.
* @param	ValueToWrite is the 32 bit value to be written to the register.
*
* @return	C-style signature:
*		void XMutex_WriteReg(u32 BaseAddress, u8 MutexNumber,
*					unsigned RegOffset, u32 ValueToWrite)
*
******************************************************************************/
#define XMutex_WriteReg(BaseAddress, MutexNumber, RegOffset, ValueToWrite) \
	Xil_Out32(((BaseAddress) + XMutex_Offset(MutexNumber) +		   \
			(RegOffset)), (ValueToWrite))

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
