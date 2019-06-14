/******************************************************************************
*
* Copyright (C) 2017-2018 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PRTNICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
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
* @file xpmcfw_board.c
*
* This file contains board/platform specific code of PMCFW.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc  02/21/2017 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xpmcfw_default.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
 * This function does board, platform specific initialization.
 *
 * @param none
 *
 * @return
 *		- XPMCFW_SUCCESS for successful configuration
 *		- errors as mentioned in xpmcfw_err.h
 *
 *****************************************************************************/
XStatus XPmcFw_BoardInit(void)
{
	XStatus Status;

	/* Add any initialization specific to the platforms */

	Status = XPMCFW_SUCCESS;
	return Status;
}
