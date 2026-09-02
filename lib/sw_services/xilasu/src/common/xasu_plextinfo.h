/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_plextinfo.h
 *
 * This file contains the PL extension definitions which are common across the client and server.
 * The module uses XAsu_AesParams and XAsu_AesKeyObject from xasu_aesinfo.h, hence no new types are
 * defined here.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   am   08/21/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_common_defs Common Defs
 * @{
*/
#ifndef XASU_PLEXTINFO_H_
#define XASU_PLEXTINFO_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/
/* PL extension module command IDs */
#define XASU_PLEXT_OPERATION_CMD_ID	(0x0U) /**< Command ID for PL extension operation */
#define XASU_PLEXT_MAX_CMDS		(0x1U) /**< Commands supported by PL extension module */

/************************************ Type Definitions *******************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASU_PLEXTINFO_H_ */
/** @} */
