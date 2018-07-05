/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
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
* @file xosd_sinit.c
* @addtogroup osd_v4_0
* @{
*
* This file contains the static initialization method for Xilinx Video
* On-Screen-Display (OSD) core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- ------------------------------------------------------
* 1.00a xd     08/18/08 First release
* 2.00a cjm    12/18/12 Converted from xio.h to xil_io.h, translating
*                       basic types, MB cache functions, exceptions and
*                       assertions to xil_io format.
* 4.0   adk    02/18/14 Renamed the following functions:
*                       XOSD_LookupConfig - > XOsd_LookupConfig
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xosd.h"
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
* This function gets a reference to an XOsd_Config structure based on the
* unique device id, <i>DeviceId</i>. The return value will refer to an entry
* in the core configuration table defined in the xosd_g.c file.
*
* @param	DeviceId is the unique core ID of the OSD core for the lookup
*		operation.
*
* @return	XOsd_Config is a reference to a config record in the
*		configuration table (in xosd_g.c) corresponding to
*		<i>DeviceId</i> or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
XOsd_Config *XOsd_LookupConfig(u16 DeviceId)
{
	extern XOsd_Config XOsd_ConfigTable[XPAR_XOSD_NUM_INSTANCES];
	XOsd_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = (u32)0x0; Index < (u32)(XPAR_XOSD_NUM_INSTANCES);
								Index++) {
		if (XOsd_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XOsd_ConfigTable[Index];
			break;
		}
	}

	return (XOsd_Config *)CfgPtr;
}
/** @} */
