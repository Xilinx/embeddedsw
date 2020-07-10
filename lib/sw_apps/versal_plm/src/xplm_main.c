/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc. All rights reserved.
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
*						are enabled
*       kc   05/09/2019 Addeed code to disable CFRAME isolation
*						as soon as we boot in PLM
*       ma   08/01/2019 Removed LPD module init related code from PLM app
* 1.02  kc   02/19/2020 Moved PLM banner print to XilPlmi
*       kc   03/23/2020 Minor code cleanup
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
