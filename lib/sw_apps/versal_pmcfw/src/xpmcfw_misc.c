/******************************************************************************
*
* Copyright (C) 2017-2018 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PRTNICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpmcfw_misc.c
*
* This is the file which contains common utilities code for the PMCFW.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/21/2017 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xpmcfw_misc.h"
#include "xpmcfw_hw.h"
#include "xpmcfw_config.h"
#include "xpmcfw_debug.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************
 * This function runs the scan clear on LPD
 *
 * @param	None
 *
 * @return
 *		- XST_SUCCESS if successful,
 *		- XST_FAILURE if unsuccessful.
 *
 * @note		None.
 *
 *
 *****************************************************************************/
XStatus XPmcFw_RunScanClearLPD(void)
{
	XStatus Status;

	/* Trigger Scan clear on LPD/LPD_IOU */
	XPmcFw_UtilRMW((u32)PMC_ANALOG_SCAN_CLEAR_TRIGGER,
		       (PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_MASK |
			PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_IOU_MASK |
			PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_RPU_MASK),
		       (PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_MASK |
			PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_IOU_MASK |
			PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_RPU_MASK));

	/**
	 * TODO: Update Scan Clear timeout
	 */
	Status = XPmcFw_UtilPollForMask(PMC_ANALOG_SCAN_CLEAR_DONE,
			(PMC_ANALOG_SCAN_CLEAR_DONE_LPD_IOU_MASK |
			 PMC_ANALOG_SCAN_CLEAR_DONE_LPD_MASK |
			 PMC_ANALOG_SCAN_CLEAR_DONE_LPD_RPU_MASK),1000000U);
	if(Status != XST_SUCCESS)
	{
		goto END;
	}

	Status = XPmcFw_UtilPollForMask(PMC_ANALOG_SCAN_CLEAR_PASS,
			(PMC_ANALOG_SCAN_CLEAR_PASS_LPD_IOU_MASK |
			 PMC_ANALOG_SCAN_CLEAR_PASS_LPD_MASK |
			 PMC_ANALOG_SCAN_CLEAR_PASS_LPD_RPU_MASK),1000000U);

	if(Status != XST_SUCCESS)
	{
		goto END;
	}
END:
	return Status;
}
