/******************************************************************************
*
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
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
* @file xddrcpsu.c
* @addtogroup ddrcpsu Overview
* @{
*
* This file contains the implementation of the interface functions for DDRCPSU
* driver. Refer to the header file xddrcpsu.h for more detailed information.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ---------------------------------------------------
* 1.5	ht	08/03/23 Added support for system device-tree flow.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xddrcpsu.h"

/************************** Function Prototypes ******************************/


/************************** Function Definitions *****************************/

#ifdef SDT
/*****************************************************************************/
/**
*
* @param	InstancePtr is a pointer to the XCsuDma instance.
* @param	CfgPtr is a reference to a structure containing information
*		about a specific XCsuDma instance.
*
* @return
*		- XST_SUCCESS if initialization was successful.
*
* @note		None.
*
******************************************************************************/
s32 XDdrcPsu_CfgInitialize(XDdrcPsu *InstancePtr, XDdrcpsu_Config *CfgPtr)
{

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);

	/* Setup the instance */
	(void)memcpy((void *) & (InstancePtr->Config), (const void *)CfgPtr,
		     sizeof(XDdrcpsu_Config));


	InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

	return (XST_SUCCESS);

}
#endif
/** @} */
