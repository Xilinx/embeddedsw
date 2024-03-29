/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xprd_hw.h
* @addtogroup prd Overview
* @{
*
* This header file contains the identifiers and basic driver functions (or
* macros) that can be used to access the Xilinx PR Decoupler.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who      Date         Changes
* ----- -----  -----------  ----------------------------------------------
* 1.0   ms     07/14/2016    First release
*
* </pre>
*
******************************************************************************/

#ifndef XPRD_HW_H_ /* prevent circular inclusions */
#define XPRD_HW_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

/** @name Register Offset
 * @{
 */
#define XPRD_CTRL_OFFSET		(0x000)	/**< Control register Offset
						  *  and status register is
						  *  also mappped to same
						  *  address as Control
						  *  register*/
/*@}*/

/** @name Register Mask
 * @{
 */
#define XPRD_CTRL_DECOUPLER_MASK	(0x00000001)	/**< Decoupler Mask */
/*@}*/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* This macro reads a value from the given register.
*
* @param	Address is the address of the register to read from.
*
* @return	The 32-bit value read from register.
*
* @note		None.
*
******************************************************************************/
#define XPrd_ReadReg(Address)	Xil_In32(Address)

/*****************************************************************************/
/**
*
* This macro writes a value to the given register.
*
* @param	Address is the address of the register to write to.
* @param	Data is the 32-bit value to write to the register.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
#define XPrd_WriteReg(Address, Data)	Xil_Out32(Address, (u32)(Data))

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
/** @} */
