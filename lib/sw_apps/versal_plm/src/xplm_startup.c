/******************************************************************************
* Copyright (C) 2018-2019 Xilinx, Inc. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
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
* ====  ==== ======== ======================================================-
* 1.00  kc   08/28/2018 Initial release
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
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
#define XPLM_TASK_DEFINE(Func, Arg, Prio) \
	{ \
		.Priority = Prio, \
		.Delay = 0, \
		.TaskNode = { \
			.prev = NULL, \
			.next = NULL, \
		}, \
		.Handler = Func, \
		.PrivData = Arg, \
	}

/**
 * Start up tasks of the PLM.
 * Current they point to the loading of the Boot PDI.
 */
struct XPlmi_TaskNode StartUpTaskList[] =
{
	XPLM_TASK_DEFINE(XPlm_ModuleInit, 0U, XPLM_TASK_PRIORITY_0),
	XPLM_TASK_DEFINE(XPlm_ProcessPlmCdo, 0U, XPLM_TASK_PRIORITY_0),
	XPLM_TASK_DEFINE(XPlm_LpdModuleInit, 0U, XPLM_TASK_PRIORITY_0),
	XPLM_TASK_DEFINE(XPlm_LoadBootPdi, 0U, XPLM_TASK_PRIORITY_0)
};

/*****************************************************************************/

/*****************************************************************************/
/**
 * @brief This function call all the init functions of all the different
 * modules. As a part of init functions, modules can register the
 * command handlers, interrupt handlers with the interface layer.
 *
 * @param	None
 *
 * @return	Status as defined in xplm_status.h
 *
 *****************************************************************************/
int XPlm_AddStartUpTasks(void)
{
	u32 Index;
	XPlmi_TaskNode *Task;
	int Status;

	for (Index = 0;
	     Index < sizeof StartUpTaskList / sizeof *StartUpTaskList;
		Index++)
	{
		Task = XPlmi_TaskCreate(StartUpTaskList[Index].Priority,
					StartUpTaskList[Index].Handler,
					StartUpTaskList[Index].PrivData);
		if (Task == NULL)
		{
			Status = XPLMI_UPDATE_STATUS(XPLM_ERR_TASK_CREATE, 0x0);
			goto END;
		}
		XPlmi_TaskTriggerNow(Task);
	}
	Status = XST_SUCCESS;
END:
	return Status;
}
