/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* @file xhdcp22_rng_sinit.c
* @addtogroup hdcp22_rng_v1_2
* @{
* @details
*
* This file contains the static initialization methods for the Xilinx HDCP
* 2.2 RNG core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  JO     10/01/15 Initial release.
* </pre>
*
******************************************************************************/


/***************************** Include Files *********************************/
#include "xhdcp22_rng.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/
#ifndef XPAR_XHDCP22_RNG_NUM_INSTANCES
#define XPAR_XHDCP22_RNG_NUM_INSTANCES 0
#endif
/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
extern XHdcp22_Rng_Config XHdcp22_Rng_ConfigTable[];

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function returns a reference to an XHdcp22_Rng_Config structure based
* on the core id, <i>DeviceId</i>. The return value will refer to an entry in
* the device configuration table defined in the xhdcp22_rng_g.c file.
*
* @param  DeviceId is the unique core ID of the XHDCP22 Rng core for the
*         lookup operation.
*
* @return XHdcp22Rng_LookupConfig returns a reference to a config record
*         in the configuration table (in xhdcp22_rng_g.c) corresponding
*         to <i>DeviceId</i>, or NULL if no match is found.
*
* @note   None.
*
******************************************************************************/

/* Definitions for driver XHDCP22 RNG */
XHdcp22_Rng_Config *XHdcp22Rng_LookupConfig(u16 DeviceId)
{
	XHdcp22_Rng_Config *CfgPtr = NULL;
	u32 Index;

	/* Checking for device id for which instance it is matching */
	for (Index = (u32)0x0; Index < (u32)(XPAR_XHDCP22_RNG_NUM_INSTANCES);
		Index++) {

		/* Assigning address of config table if both device ids
		 * are matched
		 */
		if (XHdcp22_Rng_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XHdcp22_Rng_ConfigTable[Index];
			break;
		}
	}

	return (XHdcp22_Rng_Config *)CfgPtr;
}

/** @} */
