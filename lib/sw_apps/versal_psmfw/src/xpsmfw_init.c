/******************************************************************************
* Copyright (c) 2018-2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpsmfw_init.c
*
* This file contains PSM Firmware initialization functions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who		Date		Changes
* ---- ---- -------- ------------------------------
* 1.00  ma   04/09/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#include "xpsmfw_init.h"
#include "xpsmfw_default.h"

u32 Platform;

int XPsmFw_Init(void)
{
	int Status = XST_FAILURE;
	
#ifdef XPAR_PSV_IPI_PSM_DEVICE_ID
	if (XST_SUCCESS != XPsmfw_IpiManagerInit()) {
		XPsmFw_Printf(DEBUG_ERROR, "%s: Error! IPI Manager Initialization failed\r\n", __func__);
	}
	
	/* FIXME: Clear IPI0 status and enable IPI interrupts. Do it else where*/
	XPsmFw_Write32(IPI_PSM_ISR, MASK32_ALL_HIGH);
#endif

	Platform = ((Xil_In32(PMC_TAP_VERSION) & PMC_TAP_VERSION_PLATFORM_MASK)
		    >> PMC_TAP_VERSION_PLATFORM_SHIFT);

	Status = XST_SUCCESS;

	return Status;
}

