/******************************************************************************
*
* Copyright (C) 2016-2019 Xilinx, Inc.  All rights reserved.
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
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xprc_sinit.c
* @addtogroup prc_v1_2
* @{
*
* This file contains the implementation of the XPrc driver's static
* initialization functionality.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who     Date        Changes
* ---- ---- ------------ --------------------------------------------------
* 1.0   ms   07/18/2016    First release
* 1.2   Nava 29/03/19      Updated the tcl logic to generated the
*                          XPrc_ConfigTable properly.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xprc.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/
extern XPrc_Config XPrc_ConfigTable[];

/****************************** Functions  ***********************************/

/*****************************************************************************/
/**
*
* This function Looks for the device configuration based on the unique device
* ID. The table XPrc_ConfigTable[] contains the configuration information for
* each device in the system.
*
* @param	DeviceId is the unique device ID of the device being looked up.
*
* @return	A pointer to the configuration table entry corresponding to the
*		given device ID, or NULL if no match was found.
*
* @note		None.
*
******************************************************************************/
XPrc_Config *XPrc_LookupConfig(u16 DeviceId)
{
	XPrc_Config *ConfigPtr = NULL;
	u32 Index;

	for (Index = 0; Index < (u32)XPAR_XPRC_NUM_INSTANCES; Index++) {
		if (XPrc_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XPrc_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}
/** @} */
