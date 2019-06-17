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
* @file xloader_cfi.c
*
* This file contains the code related to PDI image loading.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  bsv   06/17/2019 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xloader.h"
#include "xplmi_util.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
/***************** Macros (Inline Functions) Definitions *********************/
/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
XCframe XLoader_CframeIns={0}; /* CFRAME Driver Instance */
XCfupmc XLoader_CfuIns={0}; /* CFU Driver Instance */


/*****************************************************************************/

/*****************************************************************************/
/**
 * This function initializes the Cframe driver
 *
 * @param None
 *
 * @return	Codes as mentioned in xplmi_status.h
 ******************************************************************************/
int XLoader_CframeInit()
{
	int Status;
	XCframe_Config *Config;

	if(XLoader_CframeIns.IsReady)
	{
		Status = XST_SUCCESS;
		goto END;
	}
	/*
	 * Initialize the Cframe driver so that it's ready to use
	 * look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XCframe_LookupConfig((u16)XPAR_XCFRAME_0_DEVICE_ID);
	if (NULL == Config) {
		Status = XLOADER_ERR_CFRAME_LOOKUP;
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_CFRAME_LOOKUP, Status);
		goto END;
	}

	Status = XCframe_CfgInitialize(&XLoader_CframeIns, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_CFRAME_CFG, Status);
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to check the CFU ISR and PMC_ERR1 and PMC_ERR2 status
 * registers to check for any errors in PL and call corresponding error
 * recovery functions if needed.
 *
 * @param 		none
 *
 * @return      none
 *****************************************************************************/
void XLoader_CfiErrorHandler(void)
{
	XPlmi_Printf(DEBUG_INFO, "Error handler for cfi bitstream\n\r");

	u32 RegVal = XPlmi_In32(PMC_GLOBAL_PMC_ERR1_STATUS) &
					PMC_GLOBAL_PMC_ERR1_STATUS_CFRAME_MASK;

	if(RegVal == 0U)
	{
		RegVal = XPlmi_In32(PMC_GLOBAL_PMC_ERR2_STATUS) &
					PMC_GLOBAL_PMC_ERR2_STATUS_CFI_MASK;
	}

	if(RegVal)
	{
		XCfupmc_CfiErrHandler(&XLoader_CfuIns);
		XCframe_ClearCframeErr(&XLoader_CframeIns);
		XCfupmc_ClearIgnoreCfiErr(&XLoader_CfuIns);

		/** Clear ISRs */
		XPlmi_UtilRMW(PMC_GLOBAL_PMC_ERR1_STATUS,
				PMC_GLOBAL_PMC_ERR1_STATUS_CFRAME_MASK,
				PMC_GLOBAL_PMC_ERR1_STATUS_CFRAME_MASK);
		XPlmi_UtilRMW(PMC_GLOBAL_PMC_ERR2_STATUS,
				PMC_GLOBAL_PMC_ERR2_STATUS_CFI_MASK,
				PMC_GLOBAL_PMC_ERR2_STATUS_CFI_MASK);
		XCfupmc_ClearCfuIsr(&XLoader_CfuIns);
	}

	/** CFU error checking and handling */
	XCfupmc_CfuErrHandler(&XLoader_CfuIns);
	XPlmi_UtilRMW(PMC_GLOBAL_PMC_ERR1_STATUS,
				PMC_GLOBAL_PMC_ERR1_STATUS_CFU_MASK,
				PMC_GLOBAL_PMC_ERR1_STATUS_CFU_MASK);
}
