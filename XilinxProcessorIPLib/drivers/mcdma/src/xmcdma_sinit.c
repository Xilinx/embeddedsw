/******************************************************************************
*
* Copyright (C) 2017 - 2019 Xilinx, Inc.  All rights reserved.
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
* @file xmcdma_sinit.c
* @addtogroup mcdma_v1_3
* @{
*
* This file contains static initialization methods for Xilinx MCDMA core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0   adk     18/07/17 Initial version.
* 1.2   mj      05/03/18 Implemented XMcdma_LookupConfigBaseAddr() to lookup
*                        configuration based on base address.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/

#include "xmcdma.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* XMcdma_LookupConfig returns a reference to an XMcdma_Config structure
* based on the unique device id, <i>DeviceId</i>. The return value will refer
* to an entry in the device configuration table defined in the xmcdma_g.c
* file.
*
* @param	DeviceId is the unique device ID of the device for the lookup
*		operation.
*
* @return	CfgPtr is a reference to a config record in the configuration
*		table (in xmcdma_g.c) corresponding to <i>DeviceId</i>, or
*		NULL if no match is found.
*
* @note		None.
******************************************************************************/
XMcdma_Config *XMcdma_LookupConfig(u16 DeviceId)
{
	extern XMcdma_Config XMcdma_ConfigTable[XPAR_XMCDMA_NUM_INSTANCES];
	XMcdma_Config *CfgPtr = NULL;
	u32 Index;

	/* Checks all the instances */
	for (Index = (u32)0x0; Index < (u32)(XPAR_XMCDMA_NUM_INSTANCES);
								Index++) {
		if (XMcdma_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XMcdma_ConfigTable[Index];
			break;
		}
	}

	return (XMcdma_Config *)CfgPtr;
}

/*****************************************************************************/
/**
*
* XMcdma_LookupConfigBaseAddr returns a reference to an XMcdma_Config structure
* based on base address. The return value will refer to an entry in the device
* configuration table defined in the xmcdma_g.c file.
*
* @param	Baseaddr is the base address of the device to lookup for
*
* @return	CfgPtr is a reference to a config record in the configuration
*		table (in xmcdma_g.c) corresponding to Baseaddr, or
*		NULL if no match is found.
*
* @note		None.
******************************************************************************/
XMcdma_Config *XMcdma_LookupConfigBaseAddr(UINTPTR Baseaddr)
{
	extern XMcdma_Config XMcdma_ConfigTable[XPAR_XMCDMA_NUM_INSTANCES];
	XMcdma_Config *CfgPtr = NULL;
	u32 Index;

	/* Checks all the instances */
	for (Index = (u32)0x0; Index < (u32)(XPAR_XMCDMA_NUM_INSTANCES);
								Index++) {
		if (XMcdma_ConfigTable[Index].BaseAddress == Baseaddr) {
			CfgPtr = &XMcdma_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
/** @} */
