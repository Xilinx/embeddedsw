/***************************************************************************************************
* Copyright (c) 2025, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xocp_plat.c
*
* This file contains the implementation of the platform specific OCP functions for Versal_net.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.6   tvp  05/16/25 Initial release
*
* </pre>
*
***************************************************************************************************/

/*************************************** Include Files ********************************************/
#include "xplmi_config.h"

#ifdef PLM_OCP
#include "xocp.h"
#include "xocp_plat.h"
#include "xocp_hw.h"

/************************************ Constant Definitions ****************************************/

/************************************** Type Definitions ******************************************/

/*************************** Macros (Inline Functions) Definitions ********************************/

/************************************ Function Prototypes *****************************************/

/************************************ Variable Definitions ****************************************/

/************************************ Function Definitions ****************************************/

/**************************************************************************************************/
/**
 * @brief      This function provides OCP related register space.
 *
 * @return
 * 		- XOcp_RegSpace pointer to register space.
 *
 **************************************************************************************************/
XOcp_RegSpace* XOcp_GetRegSpace(void)
{
	static XOcp_RegSpace RegSpace = {
		.DmeSignRAddr = XOCP_PMC_GLOBAL_DME_CHALLENGE_SIGNATURE_R_0,
		.DmeSignSAddr = XOCP_PMC_GLOBAL_DME_CHALLENGE_SIGNATURE_S_0,
		.DiceCdiSeedAddr = XOCP_PMC_GLOBAL_DICE_CDI_SEED_0,
		.DiceCdiSeedValidAddr = XOCP_PMC_GLOBAL_DICE_CDI_SEED_VALID,
		.DiceCdiSeedParityAddr = XOCP_PMC_GLOBAL_DICE_CDI_SEED_PARITY,
		.DevIkPvtAddr = XOCP_PMC_GLOBAL_DEV_IK_PRIVATE_0,
		.DevIkPubXAddr = XOCP_PMC_GLOBAL_DEV_IK_PUBLIC_X_0,
		.DevIkPubYAddr = XOCP_PMC_GLOBAL_DEV_IK_PUBLIC_Y_0,
	};

	return &RegSpace;
}
#endif /* PLM OCP */
