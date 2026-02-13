/**************************************************************************************************
* Copyright (c) 2025 - 2026, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_ocp.h
 *
 * This file Contains the OCP client function prototypes, defines and macros for
 * the OCP hardware module.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   rmv  08/04/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_ocp_client_apis OCP Client APIs
 * @{
*/
#ifndef XASU_OCP_H_
#define XASU_OCP_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xasu_client.h"
#include "xasu_ocpinfo.h"
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XAsu_OcpGetDevIkX509Certificate(XAsu_ClientParams *ClientParamPtr,
				    XAsu_OcpCertParams *OcpCertClientParamPtr);
s32 XAsu_OcpGetDevAkX509Certificate(XAsu_ClientParams *ClientParamPtr,
				    XAsu_OcpCertParams *OcpCertClientParamPtr);
s32 XAsu_OcpGetDevIkCsr(XAsu_ClientParams *ClientParamPtr,
			XAsu_OcpCertParams *OcpCertClientParamPtr);
s32 XAsu_OcpDevAkAttestation(XAsu_ClientParams *ClientParamPtr,
			     XAsu_OcpDevAkAttest *OcpDevAkAttestParamPtr);
s32 XAsu_OcpUdeChallengeReq(XAsu_ClientParams *ClientParamPtr, XAsu_OcpUdeParams *OcpUdeParamsPtr);
s32 XAsu_OcpUdeKeysEncrypt(XAsu_ClientParams *ClientParamPtr, XAsu_OcpUdeKeyEncrypt *OcpUdeKeyEnc);

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_OCP_H_ */
/** @} */
