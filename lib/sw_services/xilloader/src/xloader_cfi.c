/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
* 1.01  bsv   04/09/2020 Code clean up of xilloader
* 1.02  kc    08/03/2020 Added status prints for CFU/CFI errors
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_hw.h"
#include "xloader.h"
#include "xplmi_util.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
XCframe XLoader_CframeIns = {0U}; /** CFRAME Driver Instance */
XCfupmc XLoader_CfuIns = {0U}; /** CFU Driver Instance */

/*****************************************************************************/

/*****************************************************************************/
/**
 * @brief	This function initializes the Cframe driver.
 *
 * @param 	None
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
int XLoader_CframeInit(void)
{
	int Status = XST_FAILURE;
	XCframe_Config *Config;

	if (XLoader_CframeIns.IsReady) {
		Status = XST_SUCCESS;
		goto END;
	}

	/**
	 * Initialize the Cframe driver so that it's ready to use
	 * look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XCframe_LookupConfig((u16)XPAR_XCFRAME_0_DEVICE_ID);
	if (NULL == Config) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_CFRAME_LOOKUP, 0);
		goto END;
	}

	Status = XCframe_CfgInitialize(&XLoader_CframeIns, Config,
				Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_CFRAME_CFG, Status);
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief This function is used to check the CFU ISR and PMC_ERR1 and PMC_ERR2
 * status registers to check for any errors in PL and call corresponding error
 * recovery functions if needed.
 *
 * @param 	None
 *
 * @return      None
 *
 *****************************************************************************/
void XLoader_CframeErrorHandler(void)
{
	u32 Err1Status = XPlmi_In32(PMC_GLOBAL_PMC_ERR1_STATUS);
	u32 Err2Status = XPlmi_In32(PMC_GLOBAL_PMC_ERR2_STATUS);
	u32 CfiErrStatus = Err1Status & PMC_GLOBAL_PMC_ERR1_STATUS_CFRAME_MASK;
	u32 CfuErrStatus = Err1Status & PMC_GLOBAL_PMC_ERR1_STATUS_CFU_MASK;

	if (CfiErrStatus == 0U) {
		CfiErrStatus = Err2Status & PMC_GLOBAL_PMC_ERR2_STATUS_CFI_MASK;
	}

	if ((CfiErrStatus != 0U) || (CfuErrStatus != 0U)) {
		XPlmi_Printf(DEBUG_GENERAL, "Error loading CFI data: \n\r"
		     "CFU_ISR: 0x%08x, CFU_STATUS: 0x%08x \n\r"
		     "PMC ERR1: 0x%08x, PMC ERR2: 0x%08x\n\r",
		     XPlmi_In32(CFU_APB_CFU_ISR),
		     XPlmi_In32(CFU_APB_CFU_STATUS),
		     Err1Status, Err2Status);
	}

	if (CfiErrStatus != 0U) {
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
