/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xprovision.h
 *
 * This file contains the function prototypes, defines and macros for the ASU Provisioning
 * functionality (USER eFUSE key provisioning).
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
* @addtogroup xprovision_server_apis Provisioning Server APIs
* @{
*/
#ifndef XPROVISION_H_
#define XPROVISION_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xaes.h"
#include "xasufw_dma.h"
#include "xasu_provisioninfo.h"

#ifdef XASU_PROVISION_ENABLE
/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XProvision_UserEfuse(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
			 const XAsu_ProvisionUserEfuseParams *ParamsPtr, u32 SubsystemId);
s32 XProvision_Ude(XAsufw_Dma *DmaPtr, const XAsu_ProvisionUdeParams *ParamsPtr,
		   u32 *CertActualSizePtr);
#endif /* XASU_PROVISION_ENABLE */

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPROVISION_H_ */
/** @} */
