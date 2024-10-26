/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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
#ifdef VERSAL_NET
#include "xpsmfw_update.h"
#endif
#define XPAR_IOMODULE_0_DEVICE_ID 0U

/****************************************************************************/
/**
 * @brief	Main function. This is the entry point of the program.
 *
 * @return	Returns an integer value, typically 0 for successful execution
 *
 * @note	None
 ****************************************************************************/
int main(void)
{
	XStatus Status = XST_FAILURE;

#ifdef VERSAL_NET
	if (XPsmFw_GetUpdateState() == PSM_UPDATE_STATE_LOAD_ELF_DONE) {
		Status = XPsmFw_ReStoreData();
		if (Status != XST_SUCCESS) {
			XPsmFw_Printf(DEBUG_ERROR, "%s: Error! PSM Initialization failed. Status= %x\r\n", __func__, Status);
			goto done;
		}
		XPsmFw_SetUpdateState(PSM_UPDATE_STATE_FINISHED);
	}
#endif

#ifdef XPSMFW_DATE_TIME_EXCLUDE
	XPsmFw_Printf(DEBUG_PRINT_ALWAYS, "PSM Firmware version: %s \r\n", versal_PSMFW_VERSION);
#else
	XPsmFw_Printf(DEBUG_PRINT_ALWAYS, "PSM Firmware version: %s "
			"[Build: %s %s ] \r\n",	versal_PSMFW_VERSION, __DATE__, __TIME__);
#endif

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


#ifdef VERSAL_NET
	while (TRUE) {
		if (XPsmFw_GetUpdateState() == PSM_UPDATE_STATE_SHUTDOWN_START) {
			Status = XST_DEVICE_BUSY;
			while (Status == XST_DEVICE_BUSY){
				Status = XPsm_DoShutdown();
				if (Status ==  XST_DEVICE_BUSY) {
					/**  Retry when device busy */
					XPsmFw_Printf(DEBUG_PRINT_ALWAYS, "%s: Pending Interrupts during shutting down. \
						Trying again later. \r\n", __func__);
				} else {
					if (Status != XST_SUCCESS) {
						XPsmFw_Printf(DEBUG_ERROR, "%s: Error! PSM Shutdown failed. \
							Status= %x\r\n", __func__, Status);
						goto done;
					}
				}
			}
		}
		mb_sleep();
	}
done:
	return Status;
#else
	/* Put Microblaze to Sleep in an infinite loop */
	do{
		mb_sleep();
	} while(TRUE);
#endif
}
