/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc. All rights reserved.
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
* 1.00  bsv  10/03/2022 Initial release
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