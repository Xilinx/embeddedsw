/***************************************************************************************************
* Copyright (c) 2025, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xocp_dice_dme.h
* @addtogroup xil_ocpapis APIs
* @{
*
* This file contains the xilocp DME challenge signature declarations for versal_net.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.5   tvp  06/05/25 Initial release
*
* </pre>
*
* @note
*
* @endcond
***************************************************************************************************/

#ifndef XOCP_DICE_DME_H
#define XOCP_DICE_DME_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files ********************************************/

#include "xplmi_config.h"
#ifdef PLM_OCP
#include "xocp.h"

/************************************ Constant Definitions ****************************************/

/************************************** Type Definitions ******************************************/

/************************************ Function Prototypes *****************************************/
int XOcp_GenerateDmeResponseImpl(u64 NonceAddr, u64 DmeStructResAddr);

#endif /* PLM_OCP */

#ifdef __cplusplus
}
#endif
#endif /* XOCP_DICE_DME_H_ */
