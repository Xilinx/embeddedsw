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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/****************************************************************************/
/**
*
* @file xtmr_inject_sinit.c
* @addtogroup tmr_inject_v1_1
* @{
*
* The implementation of the XTMRInject component's static initialzation
* functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.0   sa   04/05/17 First release
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xstatus.h"
#include "xparameters.h"
#include "xtmr_inject_i.h"

/************************** Constant Definitions ****************************/


/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/


/************************** Variable Definitions ****************************/


/************************** Function Prototypes *****************************/

/****************************************************************************
*
* Looks up the device configuration based on the unique device ID.  The table
* TmrInjectConfigTable contains the configuration info for each device in the
* system.
*
* @param	DeviceId is the unique device ID to match on.
*
* @return	A pointer to the configuration data for the device, or
*		NULL if no match was found.
*
* @note		None.
*
******************************************************************************/
XTMR_Inject_Config *XTMR_Inject_LookupConfig(u16 DeviceId)
{
	XTMR_Inject_Config *CfgPtr = NULL;
	u32 Index;

	for (Index=0; Index < XPAR_XTMR_INJECT_NUM_INSTANCES; Index++) {
		if (XTMR_Inject_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XTMR_Inject_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}

/****************************************************************************/
/**
*
* Initialize a XTMR_Inject instance.
*
* @param	InstancePtr is a pointer to the XTMR_Inject instance.
* @param	DeviceId is the unique id of the device controlled by this
*		XTMR_Inject instance.  Passing in a device id associates the
*		generic XTMR_Inject instance to a specific device, as chosen by
*		the caller or application developer.
*
* @return
* 		- XST_SUCCESS if everything starts up as expected.
* 		- XST_DEVICE_NOT_FOUND if the device is not found in the
*			configuration table.
*
* @note		None.
*
*****************************************************************************/
int XTMR_Inject_Initialize(XTMR_Inject *InstancePtr, u16 DeviceId)
{
	XTMR_Inject_Config *ConfigPtr;

	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/*
	 * Lookup the device configuration in the configuration table. Use this
	 * configuration info when initializing this component.
	 */
	ConfigPtr = XTMR_Inject_LookupConfig(DeviceId);

	if (ConfigPtr == (XTMR_Inject_Config *)NULL) {
		return XST_DEVICE_NOT_FOUND;
	}
	return XTMR_Inject_CfgInitialize(InstancePtr, ConfigPtr,
					ConfigPtr->RegBaseAddr);
}

/** @} */
