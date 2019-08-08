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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplm_pm.c
*
* This file contains the wrapper code xilpm
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   07/20/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_pm.h"
#include "xplm_main.h"
#include "xillibpm_api.h"
#include "xpm_subsystem.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

void XPlm_PmRequestCb(const u32 IpiMask, const u32 EventId, u32 *Payload)
{
#ifdef XPAR_XIPIPSU_0_DEVICE_ID
	XStatus Status;

	if ((XPM_INIT_SUSPEND_CB == EventId) || (XPM_NOTIFY_CB == EventId)) {
		Status = XPlmi_IpiWrite(IpiMask, Payload, XPLMI_CMD_RESP_SIZE, XIPIPSU_BUF_TYPE_MSG);
		if (XST_SUCCESS != Status) {
			XPlmi_Printf(DEBUG_PRINT_ALWAYS, "%s Error in IPI write: %d\r\n", __func__, Status);
		}

		Status = XPlmi_IpiTrigger(IpiMask);
		if (XST_SUCCESS != Status) {
			XPlmi_Printf(DEBUG_PRINT_ALWAYS, "%s Error in IPI trigger: %d\r\n", __func__, Status);
		}
	} else {
		XPlmi_Printf(DEBUG_PRINT_ALWAYS, "%s Error: Unsupported EventId: %d\r\n", __func__, EventId);
	}
#else
	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "%s Error: IPI is not defined\r\n", __func__);
#endif /* XPAR_XIPIPSU_0_DEVICE_ID */
}

/*****************************************************************************/
/**
* It initializes libPM
*
* @param	None
* @return	None
*
*****************************************************************************/
int XPlm_PmInit()
{
	int Status;

	/**
	 * Initialize the libPM component. It registers the callback handlers,
	 * variables, events
	 */
	Status = XPm_Init(XPlm_PmRequestCb);
	if (Status != XST_SUCCESS)
	{
		Status = XPLMI_UPDATE_STATUS(XPLM_ERR_PM_MOD, Status);
		goto END;
	}

END:
	return Status;
}


/*****************************************************************************/
/**
* It initializes PM with
*
* @param	None
* @return	None
*
*****************************************************************************/
int XPlm_ProcessPlmCdo(void *arg)
{
	int Status;
	XPlmiCdo Cdo;
	XPlmi_Printf(DEBUG_DETAILED, "%s\n\r", __func__);

	/**
	 * Pass the device topology and policies sections to PM
	 *  - Pass the PLM CDO to CDO parser
	 *  - PM sets device topology
	 *  - PM sets the polices
	 *
	 * Use XPm_CreateSubsystem() to create the PLM subsystem.
	 *  - PM sets the PLM requirements
	 *  - Subsystem handle is updated by PM
	 *
	 * Register the PM callback for PLM subsystem
	 *	- TODO what can we have in this?
	 */

	/** Process the PLM CDO */
	XPlmi_InitCdo(&Cdo);
	Cdo.ImgId = XPM_SUBSYSID_PMC;
	Cdo.PrtnId = 0U;
	Cdo.BufPtr = (u32 *)XPLMI_PMCRAM_BASEADDR;
	Cdo.BufLen = XPLMI_PMCRAM_LEN;
	Status = XPlmi_ProcessCdo(&Cdo);

	return Status;
}
