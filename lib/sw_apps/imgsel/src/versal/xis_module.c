/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022-2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xis_module.c
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
#include "xis_module.h"
#include "xplmi_config.h"
#include "xpm_api.h"
#include "xplmi_err_common.h"
#include "xplmi.h"
#include "xis_loader.h"
#include "xis_pm.h"


/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/

/*****************************************************************************/
/**
 * @brief This function call all the init functions of all the different
 * modules. As a part of init functions, modules can register the
 * command handlers, interrupt handlers with the interface layer.
 *
 * @param	Arg is not used
 *
 * @return	Status as defined in xplmi_status.h
 *
 *****************************************************************************/
int XPlm_ModuleInit(void)
{
	int Status = XST_FAILURE;

	Status = XPlmi_Init();
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XPlm_PmInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPlmi_EmInit(XPm_SystemShutdown, XPm_IdleRestartHandler);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XLoader_Init();
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	return Status;
}
