/******************************************************************************
* Copyright (C) 2018 Xilinx, Inc. All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
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
#include "xplm_pli.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XPlm_StaticNpiInit();
int XPlm_NpiInit(struct metal_event *event, void *arg);

/************************** Variable Definitions *****************************/
#define XPLM_METAL_EVENT_DEFINE(Func, Arg, Prio) \
	{ \
		.state = METAL_EVENT_IDLE, \
		.hd = { \
			.func = Func, \
			.arg = Arg, \
		}, \
		.priority = Prio, \
		.node = { \
			.prev = NULL, \
			.next = NULL, \
		}, \
	}

/**
 * Start up tasks of the PLM.
 * Current they point to the loading of the Boot PDI.
 * TODO need to static NPI initialization for SPP
 */
struct metal_event StartUpTaskList[] =
{
	XPLM_METAL_EVENT_DEFINE(XPlm_ProcessPlmCdo, 0U, XPLM_TASK_PRIORITY_0),
	XPLM_METAL_EVENT_DEFINE(XPlm_NpiInit, 0U, XPLM_TASK_PRIORITY_0),
	XPLM_METAL_EVENT_DEFINE(XPlm_LoadBootPdi, 0U, XPLM_TASK_PRIORITY_0)
};

/*****************************************************************************/
/*****************************************************************************/
/**
 * @brief Static NPI configuration is added for SPP to initialize NoC and
 * DDR
 *
 * @param	event pointer corresponding to Npi Init
 *
 * @return	Status as defined in event.h
 *
 *****************************************************************************/
int XPlm_NpiInit(struct metal_event *event, void *arg)
{
	int Status;

	XPlmi_Printf(DEBUG_INFO, "Static NPI Initialiation\n\r");
	XPlm_StaticNpiInit();

	Status = METAL_EVENT_HANDLED;
	return Status;
}


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

	for (Index = 0; Index < sizeof StartUpTaskList / sizeof *StartUpTaskList;
			Index++)
	{
		metal_event_enable(&StartUpTaskList[Index]);
	}

	return XST_SUCCESS;
}
