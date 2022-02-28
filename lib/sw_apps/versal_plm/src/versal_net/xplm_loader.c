/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplm_loader.c
*
* This file contains the wrapper functions for platform loader
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   08/20/2018 Initial release
* 1.01  ma   08/24/2019 Added code to force bootmode to SBI
*                       for slave SLRs
* 1.02  ana  02/29/2020 Implemented KAT support for crypto engines
*       kc   03/23/2020 Minor code cleanup
* 1.03  kc   06/12/2020 Added IPI mask to PDI CDO commands to get
*                       subsystem information
*       bm   10/14/2020 Code clean up
* 1.04  bm   12/16/2020 Added PLM_SECURE_EXCLUDE macro
*       ma   01/12/2021 Initialize BootMode and PdiInstance with invalid value
*       ma   02/03/2021 Remove redundant call to enable GIC SBI interrupt
* 1.05  ma   03/24/2021 Asterisk alignment
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_loader.h"
#include "xloader_auth_enc.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
* @brief It initializes the loader structures and registers CDO loader
* commands and interrupts
*
* @param	None
*
* @return	None
*
*****************************************************************************/
int XPlm_LoaderInit(void)
{
	int Status =  XST_FAILURE;

	Status = XLoader_Init();

	return Status;
}

/*****************************************************************************/
/**
* @brief It loads the boot PDI
*
* @param	Arg Not used in the function currently.
*
* @return	Status as defined in xplmi_status.h
*
*****************************************************************************/
int XPlm_LoadBootPdi(void *Arg)
{
	int Status = XST_FAILURE;
	PdiSrc_t BootMode = XLOADER_PDI_SRC_INVALID;
	XilPdi *PdiPtr;
	static XilPdi PdiInstance = {0U};

	(void )Arg;
	XPlmi_Printf(DEBUG_PRINT_PERF, "PLM Initialization Time \n\r");

	/**
	 * 1. Read Boot mode register and multiboot offset register
	 * 2. Load subsystem present in PDI
	 */
	PdiPtr = &PdiInstance;
	PdiPtr->SlrType = XPlmi_In32(PMC_TAP_SLR_TYPE) &
					PMC_TAP_SLR_TYPE_VAL_MASK;
	if ((PdiPtr->SlrType == XLOADER_SSIT_MASTER_SLR) ||
		(PdiPtr->SlrType == XLOADER_SSIT_MONOLITIC)) {
		BootMode = XLoader_GetBootMode();
	} else {
		BootMode = XLOADER_PDI_SRC_SBI;
	}

	/**
	 * In case of JTAG boot mode and jtag mode is not SBI,
	 * no PDI gets loaded
	 */
	if ((BootMode == XLOADER_PDI_SRC_JTAG) &&
	    (XLoader_IsJtagSbiMode() == (u8)FALSE)) {
		Status = XST_SUCCESS;
		goto END;
	}

#ifndef PLM_SECURE_EXCLUDE
	Status = XLoader_UpdateKatStatus(PdiPtr);
	if (Status != XST_SUCCESS) {
		goto ERR_END;
	}
#endif

	XPlmi_Printf(DEBUG_GENERAL, "***********Boot PDI Load: Started***********\n\r");

	PdiPtr->PdiType = XLOADER_PDI_TYPE_FULL;
	PdiPtr->IpiMask = 0U;
	Status = XLoader_LoadPdi(PdiPtr, BootMode, 0U);
	if (Status != XST_SUCCESS) {
		goto ERR_END;
	}

	XPlmi_Printf(DEBUG_GENERAL, "***********Boot PDI Load: Done***********\n\r");

	/** Print ROM time and PLM time stamp */
	XPlmi_PrintRomTime();
	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "Total PLM Boot Time \n\r");
END:
	/**
	 * This is used to identify PLM has completed boot PDI
	 */
	XPlmi_SetBootPdiDone();
	Status = XLoader_ClearIntrSbiDataRdy();
	if (Status != XST_SUCCESS) {
		goto ERR_END;
	}

ERR_END:
	return Status;
}
