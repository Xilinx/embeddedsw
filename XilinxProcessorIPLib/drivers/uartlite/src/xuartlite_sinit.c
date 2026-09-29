/******************************************************************************
* Copyright (C) 2005 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xuartlite_sinit.c
* @brief Static initialization for the XUartLite component.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.01a jvb  10/13/05 First release
* 2.00a ktn  10/20/09 Updated to use HAL Processor APIs.
* 3.13  vlt  12/18/25 Update Doxygen comments to include SDT flow details.
* 3.14  vn   09/09/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

/** @addtogroup uartlite Overview
 * @{
 */

/***************************** Include Files ********************************/

#include "xstatus.h"
#include "xparameters.h"
#include "xuartlite_i.h"

/************************** Constant Definitions ****************************/


/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/


/************************** Variable Definitions ****************************/


/************************** Function Prototypes *****************************/

/*****************************************************************************/
/**
* @brief Looks up the device configuration based on the unique device ID or
* base address.
*
* The XUartLite_ConfigTable[] contains the configuration info for each device
* in the system.
*
* @if SDT
* @param[in]   BaseAddress Base address of the device.
* @else
* @param[in]   DeviceId Unique ID of the device.
* @endif
*
* @return      A pointer to the configuration found or NULL if the specified
*              device ID/BaseAddress was not found. See xuartlite.h for the
*              definition of XUartLite_Config.
*
* @note        In XSCT/classic flow, DeviceId is used to look up the device
*              configuration.
*
******************************************************************************/
#ifndef SDT
XUartLite_Config *XUartLite_LookupConfig(u16 DeviceId)
{
	XUartLite_Config *CfgPtr = NULL;
	u32 Index;

	for (Index=0; Index < XPAR_XUARTLITE_NUM_INSTANCES; Index++) {
		if (XUartLite_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XUartLite_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#else
XUartLite_Config *XUartLite_LookupConfig(UINTPTR BaseAddress)
{
	XUartLite_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = (u32)0x0; XUartLite_ConfigTable[Index].Name != NULL; Index++) {
		if ((XUartLite_ConfigTable[Index].RegBaseAddr == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XUartLite_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#endif
/*****************************************************************************/
/**
*
* @brief Initializes a XUartLite instance.
*
* The receive and transmit FIFOs of the UART are not flushed, so they
* may be flushed if desired. The hardware device does not have any way to
* disable the receiver such that any valid data may be present in the
* receive FIFO. This function disables the UART interrupt. The baudrate
* and format of the data are fixed in the hardware at hardware build time.
*
* @param[in]   InstancePtr Pointer to the XUartLite instance.
* @if SDT
* @param[in]   BaseAddress Base address of the device.
* @else
* @param[in]   DeviceId Unique id of the device controlled by this
*              XUartLite instance. Passing in a device id associates the
*              generic XUartLite instance to a specific device, as chosen by
*              the caller or application developer.
* @endif
*
* @return
*              - XST_SUCCESS if everything starts up as expected.
*              - XST_DEVICE_NOT_FOUND if the device is not found in the
*                      configuration table.
*
* @note        In XSCT/classic flow, DeviceId is used to look up the device
*              configuration.
*
******************************************************************************/
#ifndef SDT
int XUartLite_Initialize(XUartLite *InstancePtr, u16 DeviceId)
#else
int XUartLite_Initialize(XUartLite *InstancePtr, UINTPTR BaseAddress)
#endif
{
	XUartLite_Config *ConfigPtr;

	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/*
	 * Lookup the device configuration in the configuration table. Use this
	 * configuration info when initializing this component.
	 */
#ifndef SDT
	ConfigPtr = XUartLite_LookupConfig(DeviceId);
#else
	ConfigPtr = XUartLite_LookupConfig(BaseAddress);
#endif

	if (ConfigPtr == (XUartLite_Config *)NULL) {
		return XST_DEVICE_NOT_FOUND;
	}
	return XUartLite_CfgInitialize(InstancePtr, ConfigPtr,
					ConfigPtr->RegBaseAddr);
}

/** @} */
