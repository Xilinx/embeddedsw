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
* @file xddrcpsu_sinit.c
* @addtogroup ddrcpsu Overview
* @{
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ---------------------------------------------------
* 1.5	ht	08/03/2023 Added support for system device-tree flow.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xddrcpsu.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

#ifdef SDT
/*****************************************************************************/
/**
*
* XDdrcPsu_LookupConfig returns a reference to an XDdrcpsu_Config structure
* based on the unique device id, <i>DeviceId</i>. The return value will refer
* to an entry in the device configuration table defined in the xddrcpsu_g.c
* file.
*
* @param	BaseAddress is the unique base address of the device for the
* 		lookup operation.
*
* @return	CfgPtr is a reference to a config record in the configuration
*		table (in xddrcpsu_g.c) corresponding to <i>BaseAddress</i>, or
*		NULL if no match is found.
*
* @note		None.
******************************************************************************/
XDdrcpsu_Config *XDdrcPsu_LookupConfig(UINTPTR BaseAddress)
{
	extern XDdrcpsu_Config XDdrcpsu_ConfigTable[];
	XDdrcpsu_Config *CfgPtr = NULL;
	u32 Index;

	/* Checks all the instances */
	for (Index = (u32)0x0; XDdrcpsu_ConfigTable[Index].Name != NULL; Index++) {
		if ((XDdrcpsu_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XDdrcpsu_ConfigTable[Index];
			break;
		}
	}

	return (XDdrcpsu_Config *)CfgPtr;
}
#endif
/** @} */
