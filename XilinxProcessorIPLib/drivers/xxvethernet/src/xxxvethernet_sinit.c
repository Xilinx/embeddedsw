/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
/**
*
* @file xxxvethernet_sinit.c
* @addtogroup xxvethernet_v1_0
* @{
*
* This file contains static initialzation functionality for XXV Ethernet driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   hk   6/16/17  First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xxxvethernet.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
* XXxvEthernet_LookupConfig returns a reference to an XXxvEthernet_Config
* structure based on an unique device id, <i>DeviceId</i>. The return value
* will refer to an entry in the device configuration table defined in the
* xxxvethernet_g.c file.
*
* @param	DeviceId is the unique ID for the lookup operation.
*
* @return
*		- Returns a reference to a config record in the
*		  configuration table (in xxxvethernet_g.c) corresponding to
*		  <i>DeviceId</i>, or NULL
*		- NULL if no match is found.
*
******************************************************************************/
XXxvEthernet_Config *XXxvEthernet_LookupConfig(u16 DeviceId)
{
	extern XXxvEthernet_Config XXxvEthernet_ConfigTable[];
	XXxvEthernet_Config *CfgPtr = NULL;
	int Index;

	for (Index = 0; Index < XPAR_XXXVETHERNET_NUM_INSTANCES; Index++) {
		if (XXxvEthernet_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XXxvEthernet_ConfigTable[Index];
			break;
		}
	}

	return (CfgPtr);
}
/** @} */
