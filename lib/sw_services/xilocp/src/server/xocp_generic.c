/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xocp_generic.c
* @addtogroup xilocp_server_api XilOcp common APIs
* @{
*
* This file contains the implementation of the common APIs required for
* OCP functionalities.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
*  1.7  rmv  01/30/26 Refactor OCP library
*       rpu  03/11/26 Validate input parameters
*
* </pre>
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_OCP
#include "xil_types.h"
#include "xocp_generic.h"
#include "xocp_hw.h"
#include "xplmi_hw.h"

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function reads tap configuration
 *
 * @param TapConfig Pointer to XOcp_SecureTapConfig
 *
*****************************************************************************/
void XOcp_ReadTapConfig(XOcp_SecureTapConfig* TapConfig)
{
	/** Validate input parameters */
	if (TapConfig == NULL) {
		goto END;
	}

	TapConfig->DapCfg = Xil_In32(XOCP_PMC_TAP_DAP_CFG_OFFSET);
	TapConfig->InstMask0 = Xil_In32(XOCP_PMC_TAP_INST_MASK_0_OFFSET);
	TapConfig->InstMask1 = Xil_In32(XOCP_PMC_TAP_INST_MASK_1_OFFSET);
	TapConfig->DapSecurity = Xil_In32(XOCP_PMC_TAP_DAP_SECURITY_OFFSET);
	TapConfig->BootDevice = Xil_In32(CRP_BOOT_MODE_USER) &
			CRP_BOOT_MODE_USER_BOOT_MODE_MASK;
END:
	return;
}

#endif /* PLM OCP */
/** @} */
