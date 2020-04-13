/******************************************************************************
*
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
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

