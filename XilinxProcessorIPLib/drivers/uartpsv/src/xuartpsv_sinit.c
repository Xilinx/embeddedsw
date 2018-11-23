/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
* @file xuartpsv_sinit.c
* @addtogroup uartpsv_v1_0
* @{
*
* The implementation of the XUartPsv driver's static initialzation
* functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who  Date      Changes
* ---  ---  --------- -----------------------------------------------
* 1.0  sg   09/18/17  First Release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xparameters.h"
#include "xuartpsv.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/
extern XUartPsv_Config XUartPsv_ConfigTable[XPAR_XUARTPSV_NUM_INSTANCES];

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
*
* Looks up the device configuration based on the unique device ID. The table
* contains the configuration info for each device in the system.
*
* @param	DeviceId contains the ID of the device
*
* @return	A pointer to the configuration structure or NULL if the
*		specified device is not in the system.
*
* @note 	None.
*
******************************************************************************/
XUartPsv_Config *XUartPsv_LookupConfig(u16 DeviceId)
{
	XUartPsv_Config *CfgPtr = NULL;

	u32 Index;

	for (Index = 0U; Index < (u32)XPAR_XUARTPSV_NUM_INSTANCES; Index++) {
		if (XUartPsv_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XUartPsv_ConfigTable[Index];
			break;
		}
	}

	return (XUartPsv_Config *)CfgPtr;
}
/** @} */
