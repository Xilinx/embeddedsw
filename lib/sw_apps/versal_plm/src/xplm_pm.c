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
#include "xplm_default.h"
#include "xpm_api.h"
#include "xpm_subsystem.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
* @brief This function is registered with XilPm to send any data to IPI master
* when a event comes.
*
* @param	IpiMask IPI master ID
* @param	EventId Id of the event as defined in XilPm
* @param	Payload Data that needs to be sent to the IPI master
*
* @return	None
*
*****************************************************************************/
void XPlm_PmRequestCb(const u32 IpiMask, const u32 EventId, u32 *Payload)
{
#ifdef XPAR_XIPIPSU_0_DEVICE_ID
	XStatus Status = XST_FAILURE;

	if ((PM_INIT_SUSPEND_CB == EventId) || (PM_NOTIFY_CB == EventId)) {
		Status = XPlmi_IpiWrite(IpiMask, Payload, XPLMI_CMD_RESP_SIZE,
					XIPIPSU_BUF_TYPE_MSG);
		if (XST_SUCCESS != Status) {
			XPlmi_Printf(DEBUG_GENERAL,
			 "%s Error in IPI write: %d\r\n", __func__, Status);
		} else {
			Status = XPlmi_IpiTrigger(IpiMask);
			if (XST_SUCCESS != Status) {
				XPlmi_Printf(DEBUG_GENERAL,
				"%s Error in IPI trigger: %d\r\n", __func__, Status);
			}
		}
	} else {
		XPlmi_Printf(DEBUG_GENERAL,
		 "%s Error: Unsupported EventId: %d\r\n", __func__, EventId);
	}
#else
	XPlmi_Printf(DEBUG_GENERAL, "%s Error: IPI is not defined\r\n", __func__);
#endif /* XPAR_XIPIPSU_0_DEVICE_ID */
}

/*****************************************************************************/
/**
* @brief It calls the XilPm initialization API to initialize its structures.
*
* @param	None
*
* @return	Status as defined in xplmi_status.h
*
*****************************************************************************/
int XPlm_PmInit()
{
	int Status;

	/**
	 * Initialize the XilPm component. It registers the callback handlers,
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
* @brief This function executes the PLM CDO present in PMC RAM.
*
* @param	Arg Not used in the function
*
* @return	Status as defined in xplmi_status.h
*
*****************************************************************************/
int XPlm_ProcessPlmCdo(void *Arg)
{
	int Status = XST_FAILURE;
	XPlmiCdo Cdo;
	XPlmi_Printf(DEBUG_DETAILED, "%s\n\r", __func__);

	(void )Arg;
	/**
	 *  Pass the PLM CDO to CDO parser, PLM CDO contains
	 *  - Device topology
	 *  - PMC block configuration
	 */

	/** Process the PLM CDO */
	XPlmi_InitCdo(&Cdo);
	Cdo.ImgId = PM_SUBSYS_PMC;
	Cdo.PrtnId = 0U;
	Cdo.BufPtr = (u32 *)XPLMI_PMCRAM_BASEADDR;
	Cdo.BufLen = XPLMI_PMCRAM_LEN;
	Status = XPlmi_ProcessCdo(&Cdo);

	return Status;
}
