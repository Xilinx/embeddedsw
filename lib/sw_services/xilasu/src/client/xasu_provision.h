/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_provision.h
 *
 * This file contains the Provisioning client function prototypes, defines and macros for the
 * ASU Provisioning interface.
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
 * @addtogroup xasu_provision_client_apis Provisioning Client APIs
 * @{
*/
#ifndef XASU_PROVISION_H_
#define XASU_PROVISION_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xasu_client.h"
#include "xasu_provisioninfo.h"
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XAsu_ProvisionUserEfuse(XAsu_ClientParams *ClientParamPtr,
			    XAsu_ProvisionUserEfuseParams *ParamsPtr);
s32 XAsu_ProvisionUde(XAsu_ClientParams *ClientParamPtr, XAsu_ProvisionUdeParams *ParamsPtr);
s32 XAsu_GetDevProvisionCert(XAsu_ClientParams *ClientParamPtr,
			     XAsu_DevProvisionCertParams *ParamsPtr);
s32 XAsu_DeleteDevProvisionCert(XAsu_ClientParams *ClientParamPtr);

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_PROVISION_H_ */
/** @} */
