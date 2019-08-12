/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
*******************************************************************************/
/******************************************************************************/
/**
* @file xdmapcie_sinit.c
*
* This file contains the implementation of XDMA PCIe driver's static
* initialization functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 1.0	tk	01/30/2019	First release
*
* </pre>
*
******************************************************************************/

/****************************** Include Files ********************************/

#include "xparameters.h"
#include "xdmapcie.h"

/*************************** Constant Definitions ****************************/

/***************************** Type Definitions ******************************/

/****************** Macros (Inline Functions) Definitions ********************/

/*************************** Variable Definitions ****************************/

extern XDmaPcie_Config XDmaPcie_ConfigTable[];

/*************************** Function Prototypes *****************************/

/*****************************************************************************/
/**
* Lookup the device configuration based on the unique device ID.  The table
* ConfigTable contains the configuration info for each device in the system.
*
* @param 	DeviceId is the device identifier to lookup.
*
* @return
* 		- XDmaPcie configuration structure pointer if DeviceID is
*		found.
* 		- NULL if DeviceID is not found.
*
* @note		None
*
******************************************************************************/
XDmaPcie_Config *XDmaPcie_LookupConfig(u16 DeviceId)
{
	XDmaPcie_Config *CfgPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XDMAPCIE_NUM_INSTANCES; Index++) {
		if (XDmaPcie_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XDmaPcie_ConfigTable[Index];
			break;
		}
	}

	return (CfgPtr);
}
