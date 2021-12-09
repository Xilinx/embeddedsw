/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_lite_regdef_aie.h
* @{
*
* This header file defines register offsets for lightweight version for AIE
* APIs.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Nishad  08/30/2021  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIE_LITE_REGDEF_AIE_H
#define XAIE_LITE_REGDEF_AIE_H

/***************************** Include Files *********************************/
#include "xaiegbl_params.h"

/************************** Constant Definitions *****************************/
#define XAIE_NOC_MOD_INTR_L2_ENABLE		XAIEGBL_NOC_INTCON2NDLEVENA
#define XAIE_NOC_MOD_INTR_L2_DISABLE		XAIEGBL_NOC_INTCON2NDLEVDIS
#define XAIE_NOC_MOD_INTR_L2_STATUS		XAIEGBL_NOC_INTCON2NDLEVSTA
#define XAIE_NOC_MOD_INTR_L2_IRQ		XAIEGBL_NOC_INTCON2NDLEVINT

/************************** Variable Definitions *****************************/
/************************** Function Prototypes  *****************************/

#endif		/* end of protection macro */

/** @} */
