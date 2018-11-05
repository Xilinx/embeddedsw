/******************************************************************************
*
* Copyright (C) 2007 - 2017 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
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
* @file xmutex_sinit.c
* @addtogroup mutex_v4_3
* @{
*
* Implements static initialization
* See xmutex.h for more information about the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 1.00a ecm  06/01/07 Cleanup, new coding standard, check into XCS
* 4.3   ms   08/07/17 Fixed compilation warnings.
*
* </pre>
*
******************************************************************************/

/****************************** Include Files ********************************/
#include "xmutex.h"
#include "xparameters.h"

/*************************** Constant Definitions ****************************/

/***************************** Type Definitions ******************************/

/****************** Macros (Inline Functions) Definitions ********************/

/*************************** Variable Definitions ****************************/

extern XMutex_Config XMutex_ConfigTable[];

/*************************** Function Prototypes *****************************/

/*****************************************************************************
*
* Looks up the device configuration based on the unique device ID. The config
* table contains the configuration info for each device in the system.
*
* @param	DeviceId is the unique device ID to search for in the config
*		table.
*
* @return	A pointer to the configuration that matches the given device ID,
*		or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
XMutex_Config *XMutex_LookupConfig(u16 DeviceId)
{
	XMutex_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; Index < XPAR_XMUTEX_NUM_INSTANCES; Index++) {
		if (XMutex_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XMutex_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
/** @} */
