/******************************************************************************
 * Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xplm_main.c
 *
 * This is the main file which contains code for the PLM.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  ng   05/31/24 Initial release
 * 1.01  ng   11/05/24 Add boot time measurements
 *       ng   02/11/25 Fix PLM boot time print
 * </pre>
 *
 ******************************************************************************/

/**
 * @defgroup spartanup_plm_apis SpartanUP PLM APIs
 * @{
 */

/***************************** Include Files *********************************/
#include "xplm_debug.h"
#include "xplm_load.h"
#include "xplm_init.h"
#include "xplm_status.h"
#include "xplm_post_boot.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	Main function for Platform Loader and Manager.
 *
 *****************************************************************************/
u32 main(void)
{
	volatile u32 Status = (u32)XST_FAILURE;
	u64 TStart, TCur;
	XPlm_PerfTime PerfTime;

	/** - Perform Pre-Boot initialization. */
	Status = XPlm_Init();
	if (Status != (u32)XST_SUCCESS) {
		XPlm_ErrMgr(Status);
	}

	/* Capture the CDO load start time. */
	TStart = XPlm_GetTimerValue();

	/** - Load full PDI. */
	Status = (u32)XPLM_ERR_GLITCH_DETECTED;
	Status = XPlm_LoadFullPdi();
	if (Status != (u32)XST_SUCCESS) {
		XPlm_ErrMgr(Status);
	}
	TCur = XPlm_GetTimerValue();

	XPlm_Printf(DEBUG_PRINT_ALWAYS, "PLM Boot Done\n\r");
	XPlm_GetPerfTime(TCur, TStart, XPlm_PmcIroFreq(), &PerfTime);
	XPlm_Printf(DEBUG_PRINT_ALWAYS, "%u.%03u ms: PLM Time\r\n",
		(u32)PerfTime.TPerfMs, (u32)PerfTime.TPerfMsFrac);
	XPlm_PrintRomTime();

	/** - Perform Post-Boot configuration. */
	Status = (u32)XPLM_ERR_GLITCH_DETECTED;
	Status = XPlm_PostBoot();
	if (Status != (u32)XST_SUCCESS) {
		XPlm_ErrMgr(Status);
	}

	/** - Process Run-time Events. */
	XSECURE_REDUNDANT_IMPL(XPlm_EventLoop);

	/* Should never reach here */
}

/** @} end of spartanup_plm_apis group*/
