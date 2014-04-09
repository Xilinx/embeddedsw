/******************************************************************************
*
* (c) Copyright 2007-2013 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xmutex_hw.h
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
