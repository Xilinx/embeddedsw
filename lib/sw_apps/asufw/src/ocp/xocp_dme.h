/**************************************************************************************************
* Copyright (c) 2025, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xocp.h
 *
 * This file contains declarations for xocp.c file.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date       Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   yog  08/21/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xocp_dme_server_apis OCP DME server APIs
* @{
*/
#ifndef XOCP_DME_H_
#define XOCP_DME_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xasu_ocpinfo.h"
#include "xasufw_dma.h"
#include "xil_types.h"

#ifdef XASU_OCP_ENABLE
/************************************ Constant Definitions ***************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************** Type Definitions *****************************************/

/************************************ Function Prototypes ****************************************/
s32 XOcp_GenerateDmeKek(void);
s32 XOcp_GenerateDmeResponse(XAsufw_Dma *DmaPtr, const XAsu_OcpDmeParams *OcpDmeParamsPtr);
s32 XOcp_EncryptDmeKeys(XAsufw_Dma *DmaPtr, const XAsu_OcpDmeKeyEncrypt *OcpDmeKeyEnc);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif /* XASU_OCP_ENABLE */
#endif /* XOCP_DME_H_ */
/** @} */
