/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_provision_common.h
 *
 * This file contains the Provisioning function prototypes which are common across the client and
 * server.
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
 * @addtogroup xasu_provision_common_apis Provisioning Common APIs
 * @{
*/
#ifndef XASU_PROVISION_COMMON_H_
#define XASU_PROVISION_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasu_provisioninfo.h"

/*********************************** Constant Definitions ****************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XAsu_ProvisionValidateUserEfuseParams(const XAsu_ProvisionUserEfuseParams *ParamsPtr);
s32 XAsu_ProvisionValidateUdeParams(const XAsu_ProvisionUdeParams *ParamsPtr);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASU_PROVISION_COMMON_H_ */
/** @} */
