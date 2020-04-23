/******************************************************************************
* Copyright (c) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/


#include "xparameters.h"
#include "xusbpsu.h"

/*
* The configuration table for devices
*/

XUsbPsu_Config XUsbPsu_ConfigTable[XPAR_XUSBPSU_NUM_INSTANCES] =
{
	{
		XPAR_PSU_USB_XHCI_0_DEVICE_ID,
		XPAR_PSU_USB_XHCI_0_BASEADDR,
		XPAR_PSU_USB_XHCI_0_IS_CACHE_COHERENT,
		XPAR_PSU_USB_XHCI_0_SUPER_SPEED
	}
};
