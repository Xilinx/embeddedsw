/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpsmfw_main.c
*
* This is the main file for PSM Firmware
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

#include "xil_printf.h"
#include "xil_io.h"
#include "mb_interface.h"
#include "xpsmfw_main.h"
#include "xpsmfw_debug.h"
#include "xpsmfw_init.h"
#include "xpsmfw_iomodule.h"
#include "xpsmfw_plat.h"

#define XPAR_IOMODULE_0_DEVICE_ID 0U

int main(void)
{
	XStatus Status = XST_FAILURE;

	XPsmFw_Printf(DEBUG_PRINT_ALWAYS, "PSM Firmware version: %s "
			"[Build: %s %s ] \r\n",	versal_PSMFW_VERSION, __DATE__, __TIME__);

	Status = XPsmFw_Init();
	/* Init IOModule and connect interrupts */
	if (XST_SUCCESS != XPsmFw_IoModuleInit((u16)XPAR_IOMODULE_0_DEVICE_ID)) {
		XPsmFw_Printf(DEBUG_ERROR, "%s: Error! IO Module Initialization failed\r\n", __func__);
	}

	if (Status != XST_SUCCESS) {
		XPsmFw_Printf(DEBUG_ERROR, "%s: Error! PSM Initialization failed\r\n", __func__);
	}

	XPsmFw_UtilRMW(PSM_GLOBAL_REG_GLOBAL_CNTRL,
		       PSM_GLOBAL_REG_GLOBAL_CNTRL_FW_IS_PRESENT_MASK,
		       PSM_GLOBAL_REG_GLOBAL_CNTRL_FW_IS_PRESENT_MASK);

	/* Put Microblaze to Sleep in an infinite loop */
	do{
		mb_sleep();
	} while(TRUE);
}
