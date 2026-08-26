/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_mlkem.h
 *
 * This file contains the ML-KEM (FIPS 203) client function prototypes for the ASU ML-KEM module.
 * The client APIs marshal the request parameters into an IPI command and forward it to ASUFW;
 * the actual encapsulation and decapsulation are performed by the ASUFW ML-KEM module. Key pair
 * generation is not an ML-KEM module command: it is requested through the key manager, using
 * XAsu_KmGenerateMlkemKeyPair.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   08/20/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_mlkem_client_apis ML-KEM Client APIs
 * @{
*/
#ifndef XASU_MLKEM_H_
#define XASU_MLKEM_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasu_mlkeminfo.h"
#include "xasu_client.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
/* Key pair generation lives in the key manager: see XAsu_KmGenerateMlkemKeyPair. */
s32 XAsu_MlkemEncapsulate(XAsu_ClientParams *ClientParamPtr,
			  XAsu_MlkemEncapsDecapsParams *MlkemParamPtr);
/*
 * Decapsulation never reports a bad ciphertext: per FIPS 203 a ciphertext that does not match
 * the key yields a pseudorandom shared secret. Detect tampering through key confirmation.
 */
s32 XAsu_MlkemDecapsulate(XAsu_ClientParams *ClientParamPtr,
			  XAsu_MlkemEncapsDecapsParams *MlkemParamPtr);
s32 XAsu_MlkemKat(XAsu_ClientParams *ClientParamPtr);
/*
 * Derandomized encapsulation, served only by an ASUFW built with CAVP support; see
 * XASU_MLKEM_ENCAPS_DERAND_CMD_ID. A production ASUFW rejects the command.
 */
s32 XAsu_MlkemEncapsulateDerand(XAsu_ClientParams *ClientParamPtr,
				XAsu_MlkemEncapsDerandParams *MlkemParamPtr);

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_MLKEM_H_ */
/** @} */
