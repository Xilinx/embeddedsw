/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file versal_net/xplm_hooks.c
*
* This file contains the hook functions for the user in versal_net platform
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  bm   07/06/2022 Initial release
*       ma   07/29/2022 Replaced XPAR_XIPIPSU_0_DEVICE_ID macro with
*                       XPLMI_IPI_DEVICE_ID
* 1.01  ng   11/11/2022 Fixed doxygen file name error
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_hooks.h"
#include "xpm_api.h"
#include "xpm_subsystem.h"
#include "xplmi_update.h"
#include "xplmi_wdt.h"
#include "xplmi_plat.h"
#include "xplmi_ipi.h"
#include "xplmi.h"
#include "xpm_psm_api.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
* @brief This function will be called before processing the PMC CDO. Before
* this only PMC module initialization is done. Most of the HW state will be
* as POR.
*
* @param	Arg is not used
* @return	XST_SUCCESS always
*
*****************************************************************************/
int XPlm_HookBeforePmcCdo(void *Arg)
{
	(void)Arg;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief This function will be called after processing the PMC CDO. All the
* PMC configuration will be completed by this time.
*
* @param	Arg is not used
* @return	XST_SUCCESS on success, any other value for error
*
*****************************************************************************/
int XPlm_HookAfterPmcCdo(void *Arg)
{
	int Status = XST_FAILURE;

	(void)Arg;

	/* In-Place PLM Update is applicable only for versalnet */
	if (XPlmi_IsPlmUpdateDone() == (u8)TRUE) {
#ifdef XPLMI_IPI_DEVICE_ID
		XPlmi_IpiInit(XPmSubsystem_GetSubSysIdByIpiMask);
#endif /* XPLMI_IPI_DEVICE_ID */
		XPlmi_LpdInit();
		/* Call LibPM hook */
		Status = XPm_HookAfterPlmCdo();
		if (XST_SUCCESS != Status) {
			goto END;
		}
		Status = XPmUpdate_RestoreAllNodes();
		if (XST_SUCCESS != Status) {
			goto END;
		}
		Status = XPm_GetPsmToPlmEventAddr();
		if (XST_SUCCESS != Status) {
			goto END;
		}
	}
	else {
		/*
		 * if ROM SWDT usage EFUSE is enabled and no WDT is configured,
		 * enable WDT with default timeout
		 */
		Status = XPlmi_DefaultSWdtConfig();
		if (Status != XST_SUCCESS) {
			goto END;
		}
		/* Call LibPM hook */
		Status = XPm_HookAfterPlmCdo();
		if (XST_SUCCESS != Status) {
			goto END;
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief This function will be called after loading the boot PDI.
*
* @param	Arg is not used
* @return	XST_SUCCESS always
*
*****************************************************************************/
int XPlm_HookAfterBootPdi(void *Arg)
{
	(void)Arg;

	return XST_SUCCESS;
}
