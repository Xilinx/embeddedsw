/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnvm_bbramclient.h
*
* This file Contains the client function prototypes, defines and macros for
* the BBRAM programming.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kal  07/05/21 Initial release
*       kpt  03/16/22 Removed IPI related code and added mailbox support
* 3.1   skg  10/04/22 Added macro for SlrIndex shifting
*       skg  10/23/22 Added In body comments for APIs
* 3.4   har  08/22/24 Added support for provisioning configuration limiter
* 3.4   ng   09/05/24 Fixed doxygen grouping
*
* </pre>
*
******************************************************************************/

/**
 * @addtogroup xnvm_bbram_client_api XilNvm BBRAM Client APIs
 * @{
 */

#ifndef XNVM_BBRAMCLIENT_H
#define XNVM_BBRAMCLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xnvm_mailbox.h"
#include "xnvm_defs.h"

/************************** Constant Definitions *****************************/

/**< shift constant to place slr id*/
#define XNVM_SLR_INDEX_SHIFT (6U)

/**< SlrIndexs constants*/
#define XNVM_SLR_INDEX_0 (0U) /**< SLR Index 0*/
#define XNVM_SLR_INDEX_1 (1U) /**< SLR Index 1*/
#define XNVM_SLR_INDEX_2 (2U) /**< SLR Index 2*/
#define XNVM_SLR_INDEX_3 (3U) /**< SLR Index 3*/
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XNvm_BbramWriteAesKey(const XNvm_ClientInstance *InstancePtr, const u64 KeyAddr, const u32 KeyLen);
int XNvm_BbramZeroize(const XNvm_ClientInstance *InstancePtr);
int XNvm_BbramWriteUsrData(const XNvm_ClientInstance *InstancePtr, const u32 UsrData);
int XNvm_BbramReadUsrData(const XNvm_ClientInstance *InstancePtr, const u64 OutDataAddr);
int XNvm_BbramLockUsrDataWrite(const XNvm_ClientInstance *InstancePtr);
#ifdef VERSAL_2VE_2VM
int XNvm_BbramWriteConfigLimiterParams(const XNvm_ClientInstance *InstancePtr, const u32 ClEnFlag,
	const u32 ClMode, const u32 MaxNumOfConfigs);
#endif
/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XNVM_BBRAMCLIENT_H */
