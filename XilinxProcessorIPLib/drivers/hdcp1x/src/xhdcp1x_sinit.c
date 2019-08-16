/******************************************************************************
*
* Copyright (C) 2015 - 2016 Xilinx, Inc. All rights reserved.
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
* @file xhdcp1x_sinit.c
* @addtogroup hdcp1x_v4_3
* @{
*
* This file contains static initialization method for Xilinx HDCP driver
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  fidus  07/16/15 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xhdcp1x.h"
#include "xhdcp1x_cipher.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

#ifndef XPAR_XHDCP_NUM_INSTANCES
#define XPAR_XHDCP_NUM_INSTANCES 0	/**< Number of HDCP Instances */
#endif

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

extern XHdcp1x_Config XHdcp1x_ConfigTable[];	/**< Instance of Lookup table
						  *  of HDCP instance(s) in
						  *  the design */

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* This function returns a reference to an XHdcp1x_Config structure based on
* specified device ID.
*
* @param	DeviceId is the unique core ID of the HDCP interface.
*
* @return	A reference to the config record in the configuration table (in
*		xhdcp_g.c) corresponding the specified DeviceId. NULL if no
*		match is found.
*
* @note		None.
*
******************************************************************************/
XHdcp1x_Config *XHdcp1x_LookupConfig(u16 DeviceId)
{
	XHdcp1x_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; Index < XPAR_XHDCP_NUM_INSTANCES; Index++) {
		if (XHdcp1x_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XHdcp1x_ConfigTable[Index];
			break;
		}
	}

	return (CfgPtr);
}
/** @} */
