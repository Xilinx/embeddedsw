/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
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
*       kal   08/12/2020 Added PlHouseCleaning in case of any PL error.
*       td    08/19/2020 Fixed MISRA C violations Rule 10.3
*       bsv   10/13/2020 Code clean up
* 1.03  kal   12/22/2020 Perform Cfu/Cfi recovery before re-triggering
*                        PL House cleaning in case of error in PL loading.
* 1.04  skd   03/25/2021 Compilation warning fix
* 1.05  td    07/08/2021 Fix doxygen warnings
*       td    07/15/2021 Fix doxygen warnings
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xpm_pldomain.h"
#include "xplmi_hw.h"
#include "xloader.h"
#include "xcframe.h"
#include "xplmi_util.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void XLoader_CfiErrHandler(const XCfupmc *InstancePtr);
static void XLoader_CfuErrHandler(const XCfupmc *InstancePtr);

/************************** Variable Definitions *****************************/
static XCframe XLoader_CframeIns = {0U}; /**< CFRAME Driver Instance */

/*****************************************************************************/

/*****************************************************************************/
/**
 * @brief	This function initializes the Cframe driver.
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
int XLoader_CframeInit(void)
{
	int Status = XST_FAILURE;
	XCframe_Config *Config;

	if (XLoader_CframeIns.IsReady != (u8)FALSE) {
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
 * @brief	This function is used for Cframe error handling
 *
 * @param       InstancePtr is a pointer to the XCfupmc instance.
 *
 * @return      None
 *
 *****************************************************************************/
static void XLoader_CfiErrHandler(const XCfupmc *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XCfupmc_CfiErrHandler(InstancePtr);
	XCframe_ClearCframeErr(&XLoader_CframeIns);
	XCfupmc_ClearIgnoreCfiErr(InstancePtr);

	/** Clear ISRs */
	XPlmi_UtilRMW(PMC_GLOBAL_PMC_ERR1_STATUS,
			PMC_GLOBAL_PMC_ERR1_STATUS_CFRAME_MASK,
			PMC_GLOBAL_PMC_ERR1_STATUS_CFRAME_MASK);
	XPlmi_UtilRMW(PMC_GLOBAL_PMC_ERR2_STATUS,
			PMC_GLOBAL_PMC_ERR2_STATUS_CFI_MASK,
			PMC_GLOBAL_PMC_ERR2_STATUS_CFI_MASK);
	XCfupmc_ClearCfuIsr(InstancePtr);

	return;
}

/*****************************************************************************/
/**
 * @brief	This function is used for CFU error handling
 *
 * @param     	InstancePtr is a pointer to the XCfupmc instance.
 *
 * @return	None
 *
 *****************************************************************************/
static void XLoader_CfuErrHandler(const XCfupmc *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	/** CFU error checking and handling */
	XCfupmc_CfuErrHandler(InstancePtr);
	XPlmi_UtilRMW(PMC_GLOBAL_PMC_ERR1_STATUS,
			PMC_GLOBAL_PMC_ERR1_STATUS_CFU_MASK,
			PMC_GLOBAL_PMC_ERR1_STATUS_CFU_MASK);
	return;
}

/*****************************************************************************/
/**
 * @brief This function is used to check the CFU ISR and PMC_ERR1 and PMC_ERR2
 * status registers to check for any errors in PL and call corresponding error
 * recovery functions if needed.
 *
 * @param       ImageId is Id of the image present in PDI
 *
 * @return      XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_CframeErrorHandler(u32 ImageId)
{
	int Status = XST_FAILURE;
	u32 Err1Status = XPlmi_In32(PMC_GLOBAL_PMC_ERR1_STATUS);
	u32 Err2Status = XPlmi_In32(PMC_GLOBAL_PMC_ERR2_STATUS);
	u32 CfiErrStatus;
	u32 CountVal = 0U;
	u32 CfuIsrStatus = XPlmi_In32(CFU_APB_CFU_ISR);
	u32 CfuStatus = XPlmi_In32(CFU_APB_CFU_STATUS);
	XCfupmc XLoader_CfuIns = {0U}; /** CFU Driver Instance */

	CountVal = XPlmi_In32(CFU_APB_CFU_QWORD_CNT);

	XPlmi_Printf(DEBUG_GENERAL, "Error loading PL data: \n\r"
		"CFU_ISR: 0x%08x, CFU_STATUS: 0x%08x \n\r"
		"PMC ERR1: 0x%08x, PMC ERR2: 0x%08x\n\r",
		CfuIsrStatus, CfuStatus, Err1Status, Err2Status);

	CfiErrStatus = Err1Status & PMC_GLOBAL_PMC_ERR1_STATUS_CFRAME_MASK;
	if (CfiErrStatus == 0U) {
		CfiErrStatus = Err2Status & PMC_GLOBAL_PMC_ERR2_STATUS_CFI_MASK;
	}

	if (CfiErrStatus != 0U) {
		XLoader_CfiErrHandler(&XLoader_CfuIns);
	}

	XLoader_CfuErrHandler(&XLoader_CfuIns);

	if ((CountVal != 0U) && (ImageId == PM_DEV_PLD_0)) {
		XLoader_CfiErrHandler(&XLoader_CfuIns);
		XLoader_CfuErrHandler(&XLoader_CfuIns);

		Status = XPmPlDomain_RetriggerPlHouseClean();
		if (XST_SUCCESS != Status) {
			goto END;
		}
	}
	Status = XST_SUCCESS;

END:
	return Status;
}
