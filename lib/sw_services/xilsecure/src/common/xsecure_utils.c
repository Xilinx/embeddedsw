/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_utils.c
* This file contains common functionalities required for xilsecure library
* like functions to read/write hardware registers, SSS switch configurations.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 4.0   vns     03/09/19 Initial release
*       psl     03/26/19 Fixed MISRA-C violation
*       psl     04/05/19 Fixed IAR warnings.
* 4.1   psl     07/31/19 Fixed MISRA-C violation
* 4.2   har     01/03/20 Added blind write check for SssCfg
*       vns     01/24/20 Added assert statements to input arguments
*       har     03/26/20 Removed code for SSS configuration
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_utils.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes ******************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief
 * This function takes the hardware core out of reset.
 *
 * @param	BaseAddress	Base address of the core.
 * @param	BaseAddress	Offset of the reset register.
 *
 * @return	None
 *
 *****************************************************************************/
void XSecure_ReleaseReset(u32 BaseAddress, u32 Offset)
{
	/* Assert validates the input arguments */
	Xil_AssertVoid(BaseAddress != 0x00);

	XSecure_WriteReg(BaseAddress, Offset, XSECURE_RESET_SET);
	XSecure_WriteReg(BaseAddress, Offset, XSECURE_RESET_UNSET);
}

/*****************************************************************************/
/**
 * @brief
 * This function places the hardware core into the reset.
 *
 * @param	BaseAddress	Base address of the core.
 * @param	BaseAddress	Offset of the reset register.
 *
 * @return	None
 *
 *****************************************************************************/
void XSecure_SetReset(u32 BaseAddress, u32 Offset)
{
	/* Assert validates the input arguments */
	Xil_AssertVoid(BaseAddress != 0x00);

	XSecure_WriteReg(BaseAddress, Offset, XSECURE_RESET_SET);
}

