/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_sha2.h
 * @addtogroup
 * @{
 * This file Contains the SHA2 client function prototypes, defines and macros for
 * the SHA2 hardware module.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   vns  08/22/24 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/

#ifndef XASU_SHA2_H
#define XASU_SHA2_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasu_shainfo.h"
#include "xasu_client.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XAsu_Sha2Operation(XAsu_ClientParams *ClientParamPtr, XAsu_ShaOperationCmd *ShaClientParamPtr);
s32 XAsu_Sha2Kat(void);

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_SHA2_H */