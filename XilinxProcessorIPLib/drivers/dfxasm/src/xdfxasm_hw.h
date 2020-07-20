/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfxasm_hw.h
* @addtogroup dfxasm_v1_0
* @{
*
* This header file contains the identifiers and basic driver functions (or
* macros) that can be used to access the Xilinx Axi Shutdown manager.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who      Date         Changes
* ----- -----  -----------  ----------------------------------------------
* 1.0   dp     07/14/2020    First release
*
* </pre>
*
******************************************************************************/

#ifndef XDFXASM_HW_H_ /* prevent circular inclusions */
#define XDFXASM_HW_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

/** @name Register Offset
 * @{
 */
#define XDFX_ASM_CTRL_OFFSET		(0x000)	/**< Control register Offset
						  *  and status register is
						  *  also mappped to same
						  *  address as Control
						  *  register*/
/*@}*/

/** @name Register Mask
 * @{
 */
#define XDFX_ASM_CTRL_SHUTDOWN_MASK	(0x0000000F) /**< Shutdown manager
						       *  state mask */
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
#define XDfxasm_ReadReg(Address)	Xil_In32(Address)

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
#define XDfxasm_WriteReg(Address, Data)	Xil_Out32(Address, (u32)(Data))

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
/** @} */
