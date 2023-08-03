/******************************************************************************
* Copyright (c) 2018 - 2022, Xilinx, Inc. All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplm_module.c
*
* This file contains the module initialization code for PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ====  ==== ======== ======================================================-
* 1.00  kc   08/28/2018 Initial release
* 1.01  sn   07/04/2019 Added code to enable SysMon's over-temperature
*                       interrupt
*       kc   08/01/2019 Added error management module in PLM
*       ma   08/01/2019 Removed LPD module init related code from PLM app
* 1.02  kc   03/23/2020 Minor code cleanup
* 1.03  rama 07/29/2020 Added module support for STL
* 1.04  ma   01/12/2021 Directly call module init function instead of calling
*                       in for-loop
*                       Remove unused Status variable in XPlm_ErrInit
*       rb   01/28/2021 Added Sem PreInit API call
*       bm   02/17/2021 Reinitialize Status in XPlm_ModuleInit to add redundancy
*       rb   03/09/2021 Updated Sem Init API call
*       rama 03/22/2021 Fixed compilation warning on STL inclusion
* 1.05  td   07/08/2021 Fix doxygen warnings
*       kal  07/13/2021 Added module support for XilNvm
*       bm   08/03/2021 Added temporal redundancy check for XPlm_SecureInit
*       bsv  08/13/2021 Code clean up to reduce size
*       bsv  08/13/2021 Removed unwanted goto statements
*       rb   08/17/2021 Fix compilation warning
*       rv   08/19/2021 Updated EmInit to pass subsystem restart handler
*       bsv  09/09/2021 Added PLM_NVM macro
* 1.06  kpt  01/04/2022 Added PLM_PUF macro
*       ma   03/22/2022 Call XPlmi_EmInit after XPlm_PmInit as clearing some of
*                       the EAM errors are dependent on domain isolations which
*                       are handled in XPlm_PmInit
* 1.07  ma   05/10/2022 Call XPlmi_SsitEventsInit during initialization
*       bm   07/06/2022 Refactor versal and versal_net code
*       ma   07/08/2022 Added support for secure lockdown
*       dc   07/13/2022 Added OCP init calls
* 1.08  ng   11/11/2022 Updated doxygen comments
*       am   12/21/2022 Added module support for XilOcp
*		kpt  08/03/2023 Added temporal check for XOcp_KeyInit
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_config.h"
#include "xplm_module.h"
#include "xplmi_sysmon.h"
#include "xpm_api.h"
#include "xsecure_init.h"
#ifdef PLM_NVM
#include "xnvm_init.h"
#endif
#ifdef PLM_PUF
#include "xpuf_init.h"
#endif
#include "xplmi_err_common.h"
#include "xplm_loader.h"
#include "xplm_pm.h"
#include "xplmi.h"
#include "xil_util.h"
#ifndef VERSAL_NET
#include "xplmi_ssit.h"
#endif
#ifdef XPLM_SEM
#include "xilsem.h"
#endif
#ifdef PLM_ENABLE_STL
#include "xplm_stl.h"
#endif
#ifdef PLM_OCP
#include "xocp_keymgmt.h"
#include "xocp_init.h"
#endif

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
int XPlm_ModuleInit(void *Arg)
{
	volatile int Status = XST_FAILURE;

	(void) Arg;

	/**
	 * Initialize the below modules:
	 *  - PLMI
	 *  - PM
	 *  - Error manager
	 *  - Loader
	 *  - Secure
	 *  - NVM
	 *  - Puf
	 *  - STL
	 *  - SEM
	 *  - OCP (only for Versal_net)
	 */

	Status = XPlmi_Init();
	if (Status != XST_SUCCESS) {
		goto END;
	}

#ifdef PLM_ENABLE_PLM_TO_PLM_COMM
	Status = XPlmi_SsitEventsInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}
#endif

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

	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_Init);

#ifdef PLM_NVM
	XNvm_Init();
#endif
#ifdef PLM_PUF
	XPuf_Init();
#endif
#ifdef PLM_ENABLE_STL
	Status = XPlm_StlInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}
#endif
#ifdef XPLM_SEM
	Status = XSem_Init();
	if (Status != XST_SUCCESS) {
		goto END;
	}
#endif
/* OCP module is applicable only for Versalnet */
#ifdef PLM_OCP
	XSECURE_TEMPORAL_CHECK(END, Status, XOcp_KeyInit);
	/**
	 * Init function for OCP module for handler registration to server
	 */
	XOcp_Init();
#endif

END:
	return Status;
}
