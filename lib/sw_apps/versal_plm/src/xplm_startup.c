/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc. All rights reserved.
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
* 1.04  td   07/08/2021 Fix doxygen warnings
*       ma   07/12/2021 Minor updates to StartupTaskList as per the new
*                       XPlmi_TaskNode structure
*       bsv  08/09/2021 Code clean up to reduce elf size
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
typedef int (*TaskHandler)(void * PrivData);
typedef struct {
	TaskHandler Handler;
	void * PrivData;
} StartupTaskHandler;

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
	const StartupTaskHandler StartUpTaskList[] = {
		{XPlm_ModuleInit, NULL},
		{XPlm_HookBeforePmcCdo, NULL},
		{XPlm_ProcessPmcCdo, NULL},
		{XPlm_HookAfterPmcCdo, NULL},
		{XPlm_HookAfterPmcCdo, NULL},
		{XPlm_LoadBootPdi, NULL},
		{XPlm_HookAfterBootPdi, NULL},
#ifdef XPAR_XIPIPSU_0_DEVICE_ID
		{XPlm_CreateKeepAliveTask, PtrMilliSeconds},
#endif /* XPAR_XIPIPSU_0_DEVICE_ID */
#ifdef XPLM_SEM
		{XPlm_SemScanInit, NULL},
#endif
	};

	for (Index = 0U; Index < XPLMI_ARRAY_SIZE(StartUpTaskList); Index++) {
		Task = XPlmi_TaskCreate(XPLM_TASK_PRIORITY_0,
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
