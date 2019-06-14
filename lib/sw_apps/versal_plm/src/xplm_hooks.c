/******************************************************************************
* Copyright (C) 2019 Xilinx, Inc. All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplm_hooks.c
*
* This file contains the hook functions for the user
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   03/08/2019 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_hooks.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
* @brief This function will be called before processing the PLM CDO. Before
* this only PMC module initialization is done. Most of the HW state will be
* as POR.Please note that PS LPD UART is not initialized by this time
*
* @param	None
* @return	XST_SUCCESS on success, any other value for error
*
*****************************************************************************/
int XPlm_HookBeforePlmCdo()
{

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief This function will be called after processing the PLM CDO. All the
* PMC and LPD configuration(optional) will be completed by this time.
*
* @param	None
* @return	XST_SUCCESS on success, any other value for error
*
*****************************************************************************/
int XPlm_HookAfterPlmCdo()
{

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief This function will be called after loading the boot PDI.
*
* @param	None
* @return	XST_SUCCESS on success, any other value for error
*
*****************************************************************************/
int XPlm_HookAfterBootPdi()
{

	return XST_SUCCESS;
}
