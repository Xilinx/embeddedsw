/*******************************************************************
* Copyright (C) 2022 Xilinx, Inc. All Rights Reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*
* Description: Driver configuration
*
*******************************************************************/

#include "xparameters.h"
#include "xusbpsu.h"

/*
* The configuration table for devices
*/

XUsbPsu_Config XUsbPsu_ConfigTable[XPAR_XUSBPSU_NUM_INSTANCES] =
{
	{
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_USB_XHCI_0_DEVICE_ID,
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_USB_XHCI_0_BASEADDR,
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_USB_XHCI_0_IS_CACHE_COHERENT,
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_USB_XHCI_0_SUPER_SPEED
	}
};
