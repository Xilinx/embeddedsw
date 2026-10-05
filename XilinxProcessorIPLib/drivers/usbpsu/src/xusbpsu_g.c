/*******************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/
/**
 * @file xusbpsu_g.c
 * @brief USBPSU device configuration table.
 */


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
