/******************************************************************************
* Copyright (C) 2018-2020 Xilinx, Inc. All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
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
* ====  ==== ======== ======================================================-
* 1.00  kc   07/12/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_default.h"
#include "xplm_proc.h"
#include "xplm_startup.h"
#include "xpm_api.h"
#include "xpm_pldomain.h"
#include "xpm_subsystem.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int XPlm_Init();

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief This is PLM main function
 *
 * @param	None
 *
 * @return	Ideally should not return, in case if it reaches end,
 *		error is returned
 *
 *****************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;

#ifdef DEBUG_UART_MDM
	/** If MDM UART, banner can be printed before any initialization */
	XPlmi_InitUart();
	/** Print PLM banner  */
	XPlm_PrintPlmBanner();
#endif

	/** Initialize the processor, tasks lists */
	Status = XPlm_Init();
	if (Status != XST_SUCCESS)
	{
		XPlmi_ErrMgr(Status);
	}

	/** Initialize the start up events */
	Status = XPlm_AddStartUpTasks();
	if (Status != XST_SUCCESS)
	{
		XPlmi_ErrMgr(Status);
	}

	/** Run the handlers in task loop based on the priority */
	XPlmi_TaskDispatchLoop();

	/** should never reach here */
	while(1U);
	return XST_FAILURE;
}

/*****************************************************************************/
/**
 * @brief This function initializes the processor and task list structures
 *
 * @param	None
 *
 * @return	Status as defined in xplmi_status.h
 *
 *****************************************************************************/
static int XPlm_Init()
{
	int Status = XST_FAILURE;

	/**
	 * Disable CFRAME isolation for VCCRAM
	 */
	XPlmi_UtilRMW(PMC_GLOBAL_DOMAIN_ISO_CNTRL,
	 PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_CFRAME_MASK, 0U);

	/**
	 * Reset the wakeup signal set by ROM
	 * Otherwise MB will always wakeup, irrespective of the sleep state
	 */
	XPlmi_PpuWakeUpDis();

	/** Initialize the processor, enable exceptions */
	Status = XPlm_InitProc();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Initialize the tasks lists */
	XPlmi_TaskInit();

END:
	return Status;
}
