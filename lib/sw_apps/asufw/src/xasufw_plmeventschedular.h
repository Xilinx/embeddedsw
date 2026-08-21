/**************************************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_plmeventschedular.h
 *
 * This file contains function declarations, macro and structure defines related to PLM event handle
 * code in ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   rmv  08/07/25 Initial release
 *       vm   07/30/26 Added post-update PLM event trigger API, PLM event command IDs and
 *                     command count
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
#ifndef XASUFW_PLMEVENTSCHEDULAR_H_
#define XASUFW_PLMEVENTSCHEDULAR_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XASUFW_OCP_GEN_DEV_KEYS_CMD_ID		(0U)	/**< Command ID for device key generation */
#define XASUFW_ASU_UPDATE_CMD_ID		(1U)	/**< Command ID for ASUFW update */
#define XASUFW_PLM_EVT_MAX_CMDS			(2U)	/**< Number of commands supported by the
							     PLM event module. It also sizes the
							     pending event flag array. */

/************************************ Function Prototypes ****************************************/
s32 XAsufw_PlmEventInit(void);
void XAsufw_HandlePlmEvent(void);
void XAsufw_PostUpdatePlmEvents(void);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_PLMEVENTSCHEDULAR_H_ */
/** @} */
