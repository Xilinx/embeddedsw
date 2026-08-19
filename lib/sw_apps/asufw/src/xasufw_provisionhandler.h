/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_provisionhandler.h
 *
 * This file contains declarations for xasufw_provisionhandler.c file in ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   kp   06/27/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
#ifndef XASUFW_PROVISIONHANDLER_H_
#define XASUFW_PROVISIONHANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasu_provisioninfo.h"

#ifdef XASU_PROVISION_ENABLE
/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XAsufw_ProvisionInit(void);

/************************************ Variable Definitions ***************************************/
#endif /* XASU_PROVISION_ENABLE */

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_PROVISIONHANDLER_H_ */
/** @} */
