/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022-2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xis_loader.c
*
* This file contains the wrapper code xilloader
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
#include "xis_loader.h"
#include "xplmi.h"
#include "xloader_sbi.h"
#include "xloader_plat.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
* @brief It loads the boot PDI
*
* @param	Arg Not used in the function currently.
*
* @return	Status as defined in xplmi_status.h
*
*****************************************************************************/
int XPlm_LoadBootPdi(void)
{
	int Status = XST_FAILURE;
	PdiSrc_t BootMode = XLOADER_PDI_SRC_INVALID;
	XilPdi *PdiInstPtr = XLoader_GetPdiInstance();

	if (PdiInstPtr == NULL) {
		goto ERR_END;
	}

	/**
	 * Read Boot mode register and multiboot offset register.
	 * Load subsystem present in PDI
	 */
	PdiInstPtr->SlrType = (u8)(XPlmi_In32(PMC_TAP_SLR_TYPE) &
					PMC_TAP_SLR_TYPE_VAL_MASK);
	if ((PdiInstPtr->SlrType == XLOADER_SSIT_MASTER_SLR) ||
		(PdiInstPtr->SlrType == XLOADER_SSIT_MONOLITIC)) {
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

	PdiInstPtr->PdiType = XLOADER_PDI_TYPE_FULL;
	PdiInstPtr->IpiMask = 0U;
	PdiInstPtr->DiscardUartLogs = (u8)FALSE;
	Status = XLoader_LoadPdi(PdiInstPtr, BootMode, 0U);
	if (Status != XST_SUCCESS) {
		goto ERR_END;
	}

	XPlmi_Printf(DEBUG_PRINT_ALWAYS,"***********Versal Image Selector***********\n\r");

END:
	/* This is used to identify PLM has completed boot PDI */
	XPlmi_SetBootPdiDone();
	XLoader_ClearIntrSbiDataRdy();

ERR_END:
	return Status;
}
