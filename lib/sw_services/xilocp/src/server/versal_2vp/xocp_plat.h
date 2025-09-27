/***************************************************************************************************
* Copyright (C) 2025, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xocp_plat.h
*
* This file contains the platform specific declarations for Versal_2vp.
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
#ifndef XOCP_PLAT_H
#define XOCP_PLAT_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files ********************************************/
#include "xplmi_config.h"

#ifdef PLM_OCP
#include "xocp_hw.h"

/************************************ Constant Definitions ****************************************/

/************************************** Type Definitions ******************************************/
/**
 * Data structure to hold HW register addresses required for OCP key management
 */
typedef struct {
	u32 DmeSignRAddr;		/**< Address of DME Challenge R */
	u32 DmeSignSAddr;		/**< Address of DME Challenge S */
	u32 DiceCdiSeedAddr;		/**< Address of DICE CDI SEED_0 */
	u32 DiceCdiSeedValidAddr;	/**< Address of DICE CDI SEED VALIDITY */
	u32 DiceCdiSeedParityAddr;	/**< Address of DICE CDI SEED PARITY */
	u32 DevIkPvtAddr;		/**< Address of DEV_IK_PVT_KEY_0 */
	u32 DevIkPubXAddr;		/**< Address of DEV_IK_PUBLIC_KEY_X_0 */
	u32 DevIkPubYAddr;		/**< Address of DEV_IK_PUBLIC_KEY_Y_0 */
} XOcp_RegSpace;

/*************************** Macros (Inline Functions) Definitions ********************************/

/************************************ Function Prototypes *****************************************/
XOcp_RegSpace* XOcp_GetRegSpace(void);

/************************************ Variable Definitions ****************************************/

#endif /* PLM_OCP */

#ifdef __cplusplus
}
#endif
#endif  /* XOCP_PLAT_H */
