/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 1.11  ng   04/30/2024 Fixed doxygen grouping
*       ma   09/23/2024 Added support for PSM-PLM IPI events
* 1.12  nb   10/09/2024 Add XilPM hook to XPlm_HookAfterBootPdi
* 1.13  sk   12/13/2024 Added proc buffer init in XPlm_HookBeforePmcCdo
*
* </pre>
*
******************************************************************************/

/**
 * @addtogroup xplm_apis Versal PLM APIs
 * @{
 */

/***************************** Include Files *********************************/
#include "xplm_hooks.h"
#include "xpm_api.h"
#include "xplmi_wdt.h"
#include "xplmi_plat.h"
#include "xplmi_ipi.h"
#include "xplmi.h"
#include "xloader_plat.h"
/** TODO: remove this condition when psm is removed */
#if defined(XILPM_RUNTIME)
#include "xpm_subsystem.h"
#include "xpm_alloc.h"
#else
#include "xpm_alloc.h"
#endif

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
	int Status = XST_FAILURE;
	(void)Arg;

	Status = XPlmi_SetBufferList(PROC_LOCATION_ADDRESS, PROC_LOCATION_LENGTH);

	return Status;
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
	int Status = XST_FAILURE;

	/* Call XilPM hook */
	Status = XPm_HookAfterBootPdi();
	(void)XPm_DumpMemUsage();
	return Status;
}
