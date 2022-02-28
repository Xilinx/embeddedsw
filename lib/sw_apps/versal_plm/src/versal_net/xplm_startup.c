/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplm_startup.c
*
* This file contains the startup tasks related code for PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   08/28/2018 Initial release
* 1.01  ma   08/01/2019 Removed LPD module init related code from PLM app
*       rm   09/08/2019 Adding xilsem library in place of source code
* 1.02  kc   02/26/2020 Added XPLM_SEM macro to include/disable SEM
*                       functionality
*       kc   03/23/2020 Minor code cleanup
* 1.03  bm   01/08/2021 Updated PmcCdo hook function name
*       rb   02/02/2021 Added XPLM_SEM macro to SEM header file
*       bm   02/08/2021 Renamed PlmCdo to PmcCdo
*       rb   03/09/2021 Updated Sem Scan Init API call
*       skd  03/16/2021 Added XPlm_CreateKeepAliveTask to task list
*                       for psm is alive feature
*       bm   04/03/2021 Updated StartupTaskList to be in line with the new
*                       TaskNode structure
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_startup.h"
#include "xplm_pm.h"
#include "xplm_module.h"
#include "xplm_loader.h"
#ifdef XPLM_SEM
#include "xplm_sem_init.h"
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/

/*****************************************************************************/
/**
 * @brief This function call all the init functions of all the different
 * modules. As a part of init functions, modules can register the
 * command handlers, interrupt handlers with the interface layer.
 *
 * @param	None
 *
 * @return	Status as defined in xplmi_status.h
 *
 *****************************************************************************/
int XPlm_AddStartUpTasks(void)
{
	int Status = XST_FAILURE;
	u32 Index;
	XPlmi_TaskNode *Task;
#ifdef XPAR_XIPIPSU_0_DEVICE_ID
	static u32 MilliSeconds = XPLM_DEFAULT_FTTI_TIME;
	void *PtrMilliSeconds = &MilliSeconds;
#endif /* XPAR_XIPIPSU_0_DEVICE_ID */

	/**
	 * Start up tasks of the PLM.
	 * Current they point to the loading of the Boot PDI.
	 */
	const struct XPlmi_TaskNode StartUpTaskList[] =
	{
		{XPLM_TASK_PRIORITY_0, XPLMI_INVALID_INTR_ID, 0U, {NULL, NULL},
			XPlm_ModuleInit, 0U, (u8)FALSE, (u8)FALSE},
		{XPLM_TASK_PRIORITY_0, XPLMI_INVALID_INTR_ID, 0U, {NULL, NULL},
			XPlm_HookBeforePmcCdo, 0U, (u8)FALSE, (u8)FALSE},
		{XPLM_TASK_PRIORITY_0, XPLMI_INVALID_INTR_ID, 0U, {NULL, NULL},
			XPlm_ProcessPmcCdo, 0U, (u8)FALSE, (u8)FALSE},
		{XPLM_TASK_PRIORITY_0, XPLMI_INVALID_INTR_ID, 0U, {NULL, NULL},
			XPlm_HookAfterPmcCdo, 0U, (u8)FALSE, (u8)FALSE},
		{XPLM_TASK_PRIORITY_0, XPLMI_INVALID_INTR_ID, 0U, {NULL, NULL},
			XPlm_LoadBootPdi, 0U, (u8)FALSE, (u8)FALSE},
		{XPLM_TASK_PRIORITY_0, XPLMI_INVALID_INTR_ID, 0U, {NULL, NULL},
			XPlm_HookAfterBootPdi, 0U, (u8)FALSE, (u8)FALSE},
#ifdef XPAR_XIPIPSU_0_DEVICE_ID
		{XPLM_TASK_PRIORITY_0, XPLMI_INVALID_INTR_ID, 0U, {NULL, NULL},
			XPlm_CreateKeepAliveTask, PtrMilliSeconds, (u8)FALSE, (u8)FALSE},
#endif /* XPAR_XIPIPSU_0_DEVICE_ID */
#ifdef XPLM_SEM
		{XPLM_TASK_PRIORITY_0, XPLMI_INVALID_INTR_ID, 0U, {NULL, NULL},
			XPlm_SemScanInit, 0U, (u8)FALSE, (u8)FALSE}
#endif
	};

	for (Index = 0U; Index < XPLMI_ARRAY_SIZE(StartUpTaskList); Index++) {
		Task = XPlmi_TaskCreate(StartUpTaskList[Index].Priority,
					StartUpTaskList[Index].Handler,
					StartUpTaskList[Index].PrivData);
		if (Task == NULL) {
			Status = XPlmi_UpdateStatus(XPLM_ERR_TASK_CREATE, 0);
			goto END;
		}
		microblaze_disable_interrupts();
		XPlmi_TaskTriggerNow(Task);
		microblaze_enable_interrupts();
	}
	Status = XST_SUCCESS;

END:
	return Status;
}
