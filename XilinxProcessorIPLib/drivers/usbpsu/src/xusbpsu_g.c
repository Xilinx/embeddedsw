/*******************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/


#include "xparameters.h"
#include "xusbpsu.h"

/*
* The configuration table for devices
*/

XUsbPsu_Config XUsbPsu_ConfigTable[] = {
	{
		XPAR_PSU_USB_0_DEVICE_ID,
		XPAR_PSU_USB_0_BASEADDR
	}
};
