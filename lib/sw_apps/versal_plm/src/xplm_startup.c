/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc. All rights reserved.
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
#include "xplm_sem_init.h"

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
	/**
	 * Start up tasks of the PLM.
	 * Current they point to the loading of the Boot PDI.
	 */
	const struct XPlmi_TaskNode StartUpTaskList[] =
	{
		{XPLM_TASK_PRIORITY_0, 0U, {NULL, NULL}, XPlm_ModuleInit, 0U},
		{XPLM_TASK_PRIORITY_0, 0U, {NULL, NULL}, XPlm_HookBeforePlmCdo, 0U},
		{XPLM_TASK_PRIORITY_0, 0U, {NULL, NULL}, XPlm_ProcessPlmCdo, 0U},
		{XPLM_TASK_PRIORITY_0, 0U, {NULL, NULL}, XPlm_HookAfterPlmCdo, 0U},
		{XPLM_TASK_PRIORITY_0, 0U, {NULL, NULL}, XPlm_LoadBootPdi, 0U},
		{XPLM_TASK_PRIORITY_0, 0U, {NULL, NULL}, XPlm_HookAfterBootPdi, 0U},
#ifdef XPLM_SEM
		{XPLM_TASK_PRIORITY_0, 0U, {NULL, NULL}, XPlm_SemInit, 0U}
#endif
	};


	for (Index = 0U; Index < ARRAY_SIZE(StartUpTaskList); Index++) {
		Task = XPlmi_TaskCreate(StartUpTaskList[Index].Priority,
					StartUpTaskList[Index].Handler,
					StartUpTaskList[Index].PrivData);
		if (Task == NULL) {
			Status = XPlmi_UpdateStatus(XPLM_ERR_TASK_CREATE, 0);
			goto END;
		}
		XPlmi_TaskTriggerNow(Task);
	}
	Status = XST_SUCCESS;

END:
	return Status;
}
