/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc. All rights reserved.
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
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_loader.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
XilPdi PdiInstance;

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
int XPlm_LoaderInit()
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
	u32 BootMode;
	u32 CryptoKat;
	XilPdi* PdiPtr;

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
	    (!XLoader_IsJtagSbiMode()))
	{
		Status = XST_SUCCESS;
		goto END;
	}

	CryptoKat = XPlmi_In32(EFUSE_CACHE_MISC_CTRL) &
			EFUSE_CACHE_MISC_CTRL_CRYPTO_KAT_EN_MASK;
	if(CryptoKat != 0U) {
		PdiPtr->PlmKatStatus = XPlmi_In32(PMC_GLOBAL_GLOBAL_GEN_STORAGE2);
	} else {
		PdiPtr->PlmKatStatus = XLOADER_KAT_DONE;
	}

	XPlmi_Printf(DEBUG_GENERAL, "***********Boot PDI Load: Started***********\n\r");

	PdiPtr->PdiType = XLOADER_PDI_TYPE_FULL;
	PdiPtr->IpiMask = 0U;
	Status = XLoader_LoadPdi(PdiPtr, BootMode, 0U);
	if (Status != XST_SUCCESS) {
		goto ERR_END;
	}

	XPlmi_Printf(DEBUG_GENERAL, "***********Boot PDI Load: Done*************\n\r");

	/** Print ROM time and PLM time stamp */
	XPlmi_PrintRomTime();
	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "Total PLM Boot Time \n\r");
END:
	/**
	 * This is used to identify PLM has completed boot PDI
	 */
	XPlmi_SetBootPdiDone();
	XLoader_ClearIntrSbiDataRdy();
	/**
	 * Enable the SBI RDY interrupt to get the next PDI
	 */
	XPlmi_PlmIntrEnable(XPLMI_SBI_DATA_RDY);

ERR_END:
	return Status;
}
