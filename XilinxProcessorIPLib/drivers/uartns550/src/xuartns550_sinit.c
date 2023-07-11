/******************************************************************************
* Copyright (C) 2002 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xuartns550_sinit.c
* @addtogroup uartns550 Overview
* @{
*
* The implementation of the XUartNs550 component's static initialization
* functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 1.01a jvb  10/13/05 First release
* 1.11a sv   03/20/07 Updated to use the new coding guidelines.
* 2.00a ktn  10/20/09 Updated to use HAL Processor APIs.
* 3.9   gm   07/09/23 Added SDT support
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xstatus.h"
#include "xparameters.h"
#include "xuartns550_i.h"

/************************** Constant Definitions ****************************/

#ifndef XPAR_DEFAULT_BAUD_RATE
#define XPAR_DEFAULT_BAUD_RATE 19200
#endif

/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/


/************************** Variable Definitions ****************************/


/************************** Function Prototypes *****************************/

/****************************************************************************/
/**
*
* Looks up the device configuration based on the unique device ID. A table
* contains the configuration info for each device in the system.
*
* @param	DeviceId contains the ID of the device to look up the
*		configuration for.
*
* @return	A pointer to the configuration found or NULL if the specified
*		device ID was not found.
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
XUartNs550_Config *XUartNs550_LookupConfig(u16 DeviceId)
{
	XUartNs550_Config *CfgPtr = NULL;
	u32 Index;

	for (Index=0; Index < XPAR_XUARTNS550_NUM_INSTANCES; Index++) {
		if (XUartNs550_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XUartNs550_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#else
XUartNs550_Config *XUartNs550_LookupConfig(UINTPTR BaseAddress)
{
	XUartNs550_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; XUartNs550_ConfigTable[Index].Name != NULL; Index++) {
		if ((XUartNs550_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XUartNs550_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#endif
/****************************************************************************/
/**
*
* Initializes a specific XUartNs550 instance such that it is ready to be used.
* The data format of the device is setup for 8 data bits, 1 stop bit, and no
* parity by default. The baud rate is set to a default value specified by
* XPAR_DEFAULT_BAUD_RATE if the symbol is defined, otherwise it is set to
* 19.2K baud. If the device has FIFOs (16550), they are enabled and the a
* receive FIFO threshold is set for 8 bytes. The default operating mode of the
* driver is polled mode.
*
* @param	InstancePtr is a pointer to the XUartNs550 instance .
* @param	DeviceId is the unique id of the device controlled by this
*		XUartNs550 instance. Passing in a device id associates the
*		generic XUartNs550 instance to a specific device, as chosen
*		by the caller or application developer.
*
* @return
*
* 		- XST_SUCCESS if initialization was successful
* 		- XST_DEVICE_NOT_FOUND if the device ID could not be found in
*		the configuration table
* 		- XST_UART_BAUD_ERROR if the baud rate is not possible because
*		the input clock frequency is not divisible with an acceptable
*		amount of error
*
* @note		None.
*
*****************************************************************************/
#ifndef SDT
int XUartNs550_Initialize(XUartNs550 *InstancePtr, u16 DeviceId)
#else
int XUartNs550_Initialize(XUartNs550 *InstancePtr, UINTPTR BaseAddress)
#endif
{
	XUartNs550_Config *ConfigPtr;

	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/*
	 * Lookup the device configuration in the temporary CROM table. Use this
	 * configuration info down below when initializing this component
	 */
#ifndef SDT
	ConfigPtr = XUartNs550_LookupConfig(DeviceId);
#else
	ConfigPtr = XUartNs550_LookupConfig(BaseAddress);
#endif
	if (ConfigPtr == (XUartNs550_Config *)NULL) {
		return XST_DEVICE_NOT_FOUND;
	}

	ConfigPtr->DefaultBaudRate = XPAR_DEFAULT_BAUD_RATE;
	return XUartNs550_CfgInitialize(InstancePtr, ConfigPtr,
					ConfigPtr->BaseAddress);
}
/** @} */
