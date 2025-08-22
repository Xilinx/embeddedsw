/***************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file versal_2vp/xplm_hooks.c
*
* This file contains the hook functions for the user in versal_2vp platform
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 1.0   tvp  07/07/25 Initial release
*
* </pre>
*
***************************************************************************************************/

/**
 * @addtogroup xplm_apis Versal_2VP PLM APIs
 * @{
 */

/*************************************** Include Files ********************************************/
#include "xplm_hooks.h"
#include "xpm_api.h"
#ifdef PLM_ENABLE_STL
#include "xstl_plminterface.h"
#endif
#include "xpm_device.h"
#include "xpm_nodeid.h"
#include "xplmi_plat.h"

/************************************ Constant Definitions ****************************************/

/************************************** Type Definitions ******************************************/

/*************************** Macros (Inline Functions) Definitions ********************************/

/************************************ Function Prototypes *****************************************/

/************************************ Variable Definitions ****************************************/

/**************************************************************************************************/
/**
 * @brief	This function will be called before processing the PMC CDO. Before this only PMC
 * 		module initialization is done. Most of the HW state will be as POR.
 *
 * @param	Arg is not used.
 *
 * @return
 * 		- XST_SUCCESS always.
 *
 **************************************************************************************************/
int XPlm_HookBeforePmcCdo(void *Arg)
{
	(void)Arg;

	return XST_SUCCESS;
}

/**************************************************************************************************/
/**
 * @brief	This function will be called after processing the PMC CDO. All the PMC configuration
 * 		will be completed by this time. If safety is enabled then start-up STLs which are
 * 		dependent on PMC CDO will be triggered.
 *
 * @param	Arg is not used.
 *
 * @return
 * 		- XST_SUCCESS on success, any other value for error.
 *
 **************************************************************************************************/
int XPlm_HookAfterPmcCdo(void *Arg)
{
	int Status = XST_FAILURE;

	(void)Arg;

	/* Call XilPM hook */
	Status = XPm_HookAfterPlmCdo();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Check if XRAM is available in the device */
	if (XPmDevice_GetById(PM_DEV_XRAM_0) != NULL) {
		XPlmi_SetXRamAvailable();
	}

#ifdef PLM_ENABLE_STL
	/* Execute start-up STLs after PMC CDO */
	Status = XStl_PlmStartupPreCdoTask(XSTL_PMC_CDO_DONE);
#endif

END:
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function will be called after loading the boot PDI.
 *
 * @param	Arg is not used.
 *
 * @return
 * 		- XST_SUCCESS always.
 *
 **************************************************************************************************/
int XPlm_HookAfterBootPdi(void *Arg)
{
	int Status = XST_FAILURE;

	(void)Arg;

	/* Call XilPM hook */
	Status = XPm_HookAfterBootPdi();

	return Status;
}
