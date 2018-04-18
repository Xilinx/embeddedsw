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
* @file xrfdc_sinit.c
* @addtogroup rfdc_v4_0
* @{
*
* The implementation of the XRFdc component's static initialization
* functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   sk     05/16/17 Initial release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xrfdc.h"
#include "xparameters.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
extern XRFdc_Config XRFdc_ConfigTable[];

/*****************************************************************************/
/**
*
* Looks up the device configuration based on the unique device ID. A table
* contains the configuration info for each device in the system.
*
* @param	DeviceId contains the ID of the device to look up the
*		configuration for.
*
* @return
*
* A pointer to the configuration found or NULL if the specified device ID was
* not found. See xrfdc.h for the definition of XRFdc_Config.
*
* @note		None.
*
******************************************************************************/
XRFdc_Config *XRFdc_LookupConfig(u16 DeviceId)
{
	XRFdc_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; Index < (u32)XPAR_XRFDC_NUM_INSTANCES; Index++) {
		if (XRFdc_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XRFdc_ConfigTable[Index];
			break;
		}
	}
	return (XRFdc_Config *)CfgPtr;
}
/** @} */
