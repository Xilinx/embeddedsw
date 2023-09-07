/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file versal/xplm_hooks.c
*
* This file contains the hook functions for the user in versal platform
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   03/08/2019 Initial release
* 1.01  ma   08/01/2019 Removed LPD module init related code from PLM app
*       kc   08/29/2019 Added xilpm hook to be called after plm cdo
* 1.02  kc   02/19/2020 Moved PLM banner print to XilPlmi
*       kc   03/23/2020 Minor code cleanup
* 1.03  bm   01/08/2021 Updated PmcCdo function names and related comments
*       bsv  08/13/2021 Removed unnecessary header file
* 1.04  skd  04/20/2022 Misra-C violation Rule 2.7 fixed
* 1.05  ng   11/11/2022 Fixed doxygen file name error
* 1.06  rama 04/28/2023 Added start-up STL invocation after PMC CDO is done
*       bm   09/07/2023 Allow loading of ELFs into XRAM
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_hooks.h"
#include "xpm_api.h"
#ifdef PLM_ENABLE_STL
#include "xstl_plminterface.h"
#endif
#include "xpm_device.h"
#include "xpm_nodeid.h"
#include "xplmi_plat.h"

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
* If safety is enabled then start-up STLs which are dependent on PMC CDO will
* be triggered.
*
* @param	Arg is not used
* @return	XST_SUCCESS on success, any other value for error
*
*****************************************************************************/
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
	int Status = XST_FAILURE;

	(void)Arg;

	/* Call XilPM hook */
	Status = XPm_HookAfterBootPdi();

	return Status;
}
