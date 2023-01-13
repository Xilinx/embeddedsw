/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022-2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xis_pm.c
*
* This file contains the wrapper code xilpm
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  skd  01/13/23 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xis_pm.h"
#include "xpm_api.h"
#include "xpm_subsystem.h"
#include "xplmi_util.h"
#include "xplmi.h"
#include "xplmi_hw.h"
#include "xplmi_status.h"
#include "xplmi_cdo.h"
#include "xloader.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
* @brief It calls the XilPm initialization API to initialize its structures.
*
* @return	Status as defined in xplmi_status.h
*
*****************************************************************************/
int XPlm_PmInit(void)
{
	int Status = XST_FAILURE;

	/**
	 * Initialize the XilPm component. It registers the callback handlers,
	 * variables, events
	 */
	Status = XPm_Init(NULL, NULL);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLM_ERR_PM_MOD, Status);
	}

	return Status;
}

int XPlm_ConfigureDefaultNPll(void)
{
	int Status = XST_FAILURE;

	/* Set the PLL helper Data */
	Xil_Out32(CRP_NOCPLL_CFG, XPLM_NOCPLL_CFG_VAL);

	/* Set the PLL Basic Controls */
	Xil_Out32(CRP_NOCPLL_CTRL, XPLM_NOCPLL_CTRL_VAL);

	/* De-assert the PLL Reset; PLL is still in bypass mode only */
	XPlmi_UtilRMW(CRP_NOCPLL_CTRL, CRP_NOCPLL_CTRL_RESET_MASK, 0x0U);

	/* Check for NPLL lock */
	Status = XPlmi_UtilPoll(CRP_PLL_STATUS,
			CRP_PLL_STATUS_NOCPLL_LOCK_MASK,
			CRP_PLL_STATUS_NOCPLL_LOCK_MASK,
			NOCPLL_TIMEOUT, NULL);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLM_ERR_NPLL_LOCK, 0);
		goto END;
	}

	/* Release the bypass mode */
	XPlmi_UtilRMW(CRP_NOCPLL_CTRL, CRP_NOCPLL_CTRL_BYPASS_MASK, 0x0U);

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief This function executes the PMC CDO present in PMC RAM.
*
* @return	Status as defined in xplmi_status.h
*
*****************************************************************************/
int XPlm_ProcessPmcCdo(void)
{
	int Status = XST_FAILURE;
	XPlmiCdo Cdo;
	u32 SlrType = XLOADER_SSIT_INVALID_SLR;

	/**
	 * Configure NoC frequency equivalent to the frequency ROM sets in
	 * Slave devices
	 */
	SlrType = XPlmi_In32(PMC_TAP_SLR_TYPE) &
			PMC_TAP_SLR_TYPE_VAL_MASK;
	if (SlrType == XLOADER_SSIT_MASTER_SLR) {
		Status = XPlm_ConfigureDefaultNPll();
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	/**
	 *  Pass the PLM CDO to CDO parser, PLM CDO contains
	 *  - Device topology
	 *  - PMC block configuration
	 */

	/* Process the PLM CDO */
	Status = XPlmi_InitCdo(&Cdo);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Cdo.BufPtr = (u32 *)XPLMI_PMCRAM_BASEADDR;
	Cdo.BufLen = XPLMI_PMCRAM_LEN;
	Cdo.SubsystemId = PM_SUBSYS_PMC;
	Status = XPlmi_ProcessCdo(&Cdo);

END:
	return Status;
}
