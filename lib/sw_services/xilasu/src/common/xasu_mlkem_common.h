/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_mlkem_common.h
 *
 * This file contains the ML-KEM function prototypes which are common across the client and server.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   08/21/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_mlkem_common_apis ML-KEM Common APIs
 * @{
*/

#ifndef XASU_MLKEM_COMMON_H_
#define XASU_MLKEM_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasu_mlkeminfo.h"

/*********************************** Constant Definitions ****************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Variable Definitions ***************************************/

/************************************ Function Prototypes ****************************************/
s32 XAsu_MlkemValidateParams(const XAsu_MlkemEncapsDecapsParams *MlkemParamsPtr);
s32 XAsu_MlkemValidateEncapsDerandParams(const XAsu_MlkemEncapsDerandParams *MlkemParamsPtr);

#ifdef __cplusplus
}
#endif

#endif  /* XASU_MLKEM_COMMON_H_ */
/** @} */
