/***************************************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xocp_hw.h
*
* This file contains versal_2vp related hardware macros and declarations.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- --------   -------------------------------------------------------
* 1.6   tvp  05/16/25 Initial release
*
* </pre>
*
******************************************************************************/
#ifndef XOCP_HW_H
#define XOCP_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files ********************************************/
#include "xplmi_hw.h"

/************************************ Constant Definitions ****************************************/
/**
 * Register: XOCP_PMC_GLOBAL_PMC_FW_AUTH_HASH_0
 */
#define XOCP_PMC_GLOBAL_PMC_FW_AUTH_HASH_0	(PMC_GLOBAL_BASEADDR + 0x00000750U)

/************************************** Type Definitions ******************************************/

/*************************** Macros (Inline Functions) Definitions ********************************/

#define XOCP_PMC_GLOBAL_DICE_CDI_SEED_PARITY_ERROR_MASK		(0x00001000U)
								/**< Error mask for CDI SEED parity*/

/************************************ Function Prototypes *****************************************/

/************************************ Variable Definitions ****************************************/

#ifdef __cplusplus
}
#endif
#endif  /* XOCP_HW_H */
