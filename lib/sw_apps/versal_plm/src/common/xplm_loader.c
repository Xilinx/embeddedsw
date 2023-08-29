/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 1.06  td   07/08/2021 Fix doxygen warnings
*       bsv  07/19/2021 Disable UART prints when invalid header is encountered
*                       in slave boot modes
*       bsv  08/02/2021 Updated function return type as part of code clean up
*       bsv  08/13/2021 Code clean up to reduce size
* 1.07  bsv  11/08/2021 Move XLoader_IsJtagSbiMode to Xilloader
* 1.08  skd  04/20/2022 Misra-C violation Rule 10.3 fixed
*       ma   05/10/2022 Enable SSIT interrupts for Master SLR
*       ma   06/03/2022 Removed extra braces for SlrType if condition
*       bm   07/06/2022 Refactor versal and versal_net code
*       kpt  07/21/2022 Added XPlmi_GetBootKatStatus
*       bm   07/22/2022 Shutdown modules gracefully during update
* 1.09  sk   11/22/2022 Removed Subsystems ValidHeader member variable init
*                       from XPlm_LoadBootPdi to unify for In-Place Update
*       ng   11/23/2022 Updated doxygen comments
*       bm   01/03/2023 Switch to SSIT Events as soon as basic Noc path is
*                       configured
*       sk   19/05/2023 Added call to save Boot PDI info
*       sk   06/12/2023 Read Kek src & Plmkat status after In-Place PLM update
*       vns  07/06/2023 Added regeneration of DEVAK post in place PLM update
*       sk   08/18/2023 Fixed security review comments
*       sk   08/28/2023 Added redundant call for XLoader_GetKekSrc
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_loader.h"
#include "xplmi.h"
#include "xloader_sbi.h"
#include "xplmi_err_common.h"
#include "xloader_plat.h"
#include "xplmi_plat.h"
#ifdef PLM_OCP
#include "xocp_keymgmt.h"
#endif

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
int XPlm_LoadBootPdi(void *Arg)
{
	int Status = XST_FAILURE;
	PdiSrc_t BootMode = XLOADER_PDI_SRC_INVALID;
	XilPdi *PdiInstPtr = XLoader_GetPdiInstance();
	XilBootPdiInfo *BootPdiInfo = XLoader_GetBootPdiInfo();
	#ifndef PLM_SECURE_EXCLUDE
	volatile u32 DecKeySrcTmp;
	#endif

	(void )Arg;

	if (PdiInstPtr == NULL) {
		goto ERR_END;
	}

	/* In-Place PLM Update is applicable only for versalnet */
	if (XPlmi_IsPlmUpdateDone() == (u8)TRUE) {
		XPlmi_Printf(DEBUG_GENERAL, "In-Place PLM Update Done\n\r");
		/**
		 * Update KEK red key availability status if PLM is encrypted with
		 * Black key
		 */
	#ifndef PLM_SECURE_EXCLUDE
		XSECURE_REDUNDANT_CALL(BootPdiInfo->DecKeySrc, DecKeySrcTmp, XLoader_GetKekSrc);
		if (BootPdiInfo->DecKeySrc != DecKeySrcTmp) {
			Status = XST_GLITCH_ERROR;
			goto ERR_END;
		}
		XPlmi_GetBootKatStatus((volatile u32*)&BootPdiInfo->PlmKatStatus);
	#endif
		/* Regenerate DEVAK keys of the sub-systems */
#ifdef PLM_OCP
		Status = XOcp_RegenSubSysDevAk();
#else
		Status = XST_SUCCESS;
#endif
		goto ERR_END;
	}
	XPlmi_Printf(DEBUG_PRINT_PERF, "PLM Initialization Time \n\r");

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

#ifndef PLM_SECURE_EXCLUDE
	XPlmi_GetBootKatStatus((volatile u32*)&PdiInstPtr->PlmKatStatus);
#endif

	XPlmi_Printf(DEBUG_GENERAL, "***********Boot PDI Load: Started***********\n\r");

	PdiInstPtr->PdiType = XLOADER_PDI_TYPE_FULL;
	PdiInstPtr->IpiMask = 0U;
	PdiInstPtr->DiscardUartLogs = (u8)FALSE;
	Status = XLoader_LoadPdi(PdiInstPtr, BootMode, 0U);
	if (Status != XST_SUCCESS) {
		goto ERR_END;
	}
	/* Save Boot PDI info */
	Xloader_SaveBootPdiInfo(PdiInstPtr);
	XPlmi_Printf(DEBUG_GENERAL, "***********Boot PDI Load: Done***********\n\r");

	/* Print ROM time and PLM time stamp */
	XPlmi_PrintRomTime();
	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "Total PLM Boot Time \n\r");

END:
	/* This is used to identify PLM has completed boot PDI */
	XPlmi_SetBootPdiDone();
	XLoader_ClearIntrSbiDataRdy();

ERR_END:
	return Status;
}
