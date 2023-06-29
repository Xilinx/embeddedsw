/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
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
#ifdef PSM_ENABLE_STL
#include "xpsmfw_stl.h"
#endif

#define NOT_INITIALIZED			0xFFFFFFFFU

u32 XPsmFw_GetPlatform(void)
{
	static u32 Platform = NOT_INITIALIZED;

	if(Platform != NOT_INITIALIZED) {
		goto done;
	}

	Platform = ((Xil_In32(PMC_TAP_VERSION) & PMC_TAP_VERSION_PLATFORM_MASK)
		    >> PMC_TAP_VERSION_PLATFORM_SHIFT);

done:
	XPsmFw_Printf(DEBUG_DETAILED, "Platform: 0x%x\n\r", Platform);
	return Platform;
}

u32 XPsmFw_GetIdCode(void)
{
	static u32 IdCode = NOT_INITIALIZED;

	if (IdCode != NOT_INITIALIZED) {
		goto done;
	}

	IdCode = Xil_In32(PMC_TAP_IDCODE);

done:
	XPsmFw_Printf(DEBUG_DETAILED, "IdCode: 0x%x\n\r", IdCode);
	return IdCode;
}

XStatus XPsmFw_Init(void)
{
	XStatus Status = XST_FAILURE;

#if defined(XPAR_XIPIPSU_0_DEVICE_ID) || defined(XPAR_XIPIPSU_0_BASEADDR)
	if (XST_SUCCESS != XPsmfw_IpiManagerInit()) {
		XPsmFw_Printf(DEBUG_ERROR, "%s: Error! IPI Manager Initialization failed\r\n", __func__);
	}

	/* Clear all IPI interrupts */
	XPsmFw_Write32(IPI_PSM_ISR, MASK32_ALL_HIGH);
#endif

	Status = XST_SUCCESS;

#ifdef PSM_ENABLE_STL
	Status = XPsmFw_StartUpStlHook();
	if (XST_SUCCESS != Status) {
		XPsmFw_Printf(DEBUG_ERROR, "%s: Error! STL initialization failed\r\n", __func__);
	}
#endif

	return Status;
}
