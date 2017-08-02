/******************************************************************************
*
* Copyright (C) 2010 - 2018 Xilinx, Inc.  All rights reserved.
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
/**
*
* @file xaxiethernet_sinit.c
* @addtogroup axiethernet_v5_8
* @{
*
* This file contains static initialzation functionality for Axi Ethernet driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a asa  6/30/10  First release
* 5.6   ms   08/07/17 Fixed compilation warning.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xaxiethernet.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
* XAxiEthernet_LookupConfig returns a reference to an XAxiEthernet_Config
* structure based on an unique device id, <i>DeviceId</i>. The return value
* will refer to an entry in the device configuration table defined in the
* xaxiethernet_g.c file.
*
* @param	DeviceId is the unique device ID of the device for the lookup
*		operation.
*
* @return
*		- Returns a reference to a config record in the
*		  configuration table (in xaxiethernet_g.c) corresponding to
*		  <i>DeviceId</i>, or NULL
*		- NULL if no match is found.
*
******************************************************************************/
XAxiEthernet_Config *XAxiEthernet_LookupConfig(u16 DeviceId)
{
	extern XAxiEthernet_Config XAxiEthernet_ConfigTable[];
	XAxiEthernet_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; Index < XPAR_XAXIETHERNET_NUM_INSTANCES; Index++) {
		if (XAxiEthernet_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XAxiEthernet_ConfigTable[Index];
			break;
		}
	}

	return (CfgPtr);
}
/** @} */
