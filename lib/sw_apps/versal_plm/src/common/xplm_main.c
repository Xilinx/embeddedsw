/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
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
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   07/12/2018 Initial release
* 1.01  kc   04/08/2019 Added code to request UART if debug prints
*                       are enabled
*       kc   05/09/2019 Addeed code to disable CFRAME isolation
*                       as soon as we boot in PLM
*       ma   08/01/2019 Removed LPD module init related code from PLM app
* 1.02  kc   02/19/2020 Moved PLM banner print to XilPlmi
*       kc   03/23/2020 Minor code cleanup
*       td   10/19/2020 MISRA C Fixes
* 1.03  skd  03/16/2021 Warnings Fixed
*       ma   03/24/2021 Store DebugLog structure to RTCA
* 1.04  td   07/08/2021 Fix doxygen warnings
*       bsv  07/18/2021 Print PLM banner at the beginning of PLM execution
*       kc   07/22/2021 Issue internal POR for VP1802 ES1 devices
*       bsv  07/24/2021 Clear RTC area at the beginning of PLM
*       rb   07/28/2021 Check Efuse DNA_57 bit before issuing internal POR
*       bsv  08/13/2021 Code clean up to reduce size
*       rb   08/11/2021 Fix compilation warning
*       ma   08/23/2021 Move XPlmi_InitDebugLogBuffer and XPlm_InitProc to
*                       happen as soon as PLM starts
* 1.05  am   11/24/2021 Fixed doxygen warning
* 1.06  bm   07/06/2022 Refactor versal and versal_net code
*       bm   07/13/2022 Added compatibility check for In-Place PLM Update
* 1.07  ng   11/11/2022 Updated doxygen comments
* 1.08  bm   07/17/2023 Moved Exception Init early in the code flow
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_proc.h"
#include "xplm_startup.h"
#include "xpm_api.h"
#include "xpm_subsystem.h"
#include "xplm_loader.h"
#include "xplmi_err_common.h"
#include "xplmi.h"
#include "xloader_plat.h"
#include "xplmi_plat.h"
#include "xplm_plat.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int XPlm_Init(void);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief This is PLM main function
 *
 * @return	Ideally should not return, in case if it reaches end,
 *		error is returned
 *
 *****************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;

	/** Initialize the processor, timers and tasks list */
	Status = XPlm_Init();
	if (Status != XST_SUCCESS)
	{
		XPlmi_ErrMgr(Status);
	}

    /** Print PLM banner since timestamps are enabled now */
	XPlmi_PrintPlmBanner();

	/** Initialize the start up events */
	Status = XPlm_AddStartUpTasks();
	if (Status != XST_SUCCESS)
	{
		XPlmi_ErrMgr(Status);
	}

	/** Run the handlers in task loop based on the priority */
	XPlmi_TaskDispatchLoop();

	/* Should never reach here */
	while (TRUE) {
		;
	}
	Status = XST_FAILURE;

	return Status;
}

/*****************************************************************************/
/**
 * @brief This function initializes DMA, Run Time Config area, the processor
 * 		and task list structures.
 *
 * @return	Status as defined in xplmi_status.h
 *
 *****************************************************************************/
static int XPlm_Init(void)
{
	int Status = XST_FAILURE;

	/** Initialize PLM exceptions */
	XPlm_ExceptionInit();

	/**
	 * Disable CFRAME isolation for VCCRAM for Versal ES1 Silicon
	 */
	XPlmi_DisableCFrameIso();

	/**
	 * Reset the wakeup signal set by ROM
	 * Otherwise MB will always wakeup, irrespective of the sleep state
	 */
	XPlmi_PpuWakeUpDis();

	/**
	 * Initialize PLM Update related handlers and perform
	 * DS restoring. This is applicable only for Versal Net
	 */
	Status = XPlmi_UpdateInit(XPlm_CompatibilityCheck);
	if (Status != XST_SUCCESS) {
		XPlmi_ErrMgr(Status);
	}

	/** Initialize debug log buffer structure */
	if (XPlmi_IsPlmUpdateDone() != (u8)TRUE) {
		XPlmi_InitDebugLogBuffer();
	}

	/** Initialize timers */
	Status = XPlmi_StartTimer();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Initializes PMC DMA */
	Status = XPlmi_DmaInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPlmi_RunTimeConfigInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}

#ifdef DEBUG_UART_MDM
	/* If MDM UART, banner can be printed before any initialization */
	XPlmi_InitUart();
#endif

	/** Do Internal POR if any specific case */
	XLoader_PerformInternalPOR();

	/** Initialize the tasks list */
	XPlmi_TaskInit();

END:
	return Status;
}
