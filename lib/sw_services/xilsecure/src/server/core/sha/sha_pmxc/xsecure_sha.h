/******************************************************************************
* Copyright (c) 2024-2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
/*****************************************************************************/
/**
*
* @file xsecure_sha.h
* @addtogroup xsecure_sha_versal_2ve_2vm_apis XilSecure SHA Versal_2Ve_2Vm Server APIs
* @{
* @cond xsecure_internal
*
* This file Contains the function prototypes, defines and macros for
* the SHA2/3 hardware module.
*
* @note
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.4   kal  07/24/24 Initial release
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/
#ifndef XSECURE_SHA_H_
#define XSECURE_SHA_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xsecure_sss.h"
#include "xsecure_sha_common.h"
#include "xsecure_sha_hw.h"
#include "xpmcdma.h"

/************************** Constant Definitions ****************************/
/** @cond xsecure_internal
 * @{
 */
#define XSECURE_SHA_RESET_OFFSET            	(0x04U) /**< Reset Register */
#define XSECURE_SHA_MODE_OFFSET             	(0xA0U) /**< SHA Mode Register */
#define XSECURE_SHA_AUTO_PADDING_OFFSET     	(0xA4U) /**< SHA Auto Padding Register */
#define XSECURE_SHA_DONE_OFFSET             	(0x08U) /**< SHA Done Register */
#define XSECURE_SHA_RESET_ASSERT		(1U)	/**< SHA Reset Assert Register */
#define XSECURE_SHA_RESET_DEASSERT		(0U)	/**< SHA Reset Deassert Register */
#define	XSECURE_SHA_START_VALUE			(1U)	/**< SHA Start Value */
#define	XSECURE_SHA_DONE_VALUE			(1U)	/**< SHA Done Value */
#define	XSECURE_SHA_AUTO_MODE_ENABLE		(1U)	/**< SHA AUTO_MODE enable */
#define XSECURE_SHA_TIMEOUT_MAX			(0x1FFFFU) /**< SHA Timeout max */
#define XSECURE_HASH_SIZE_IN_BYTES		(48U) 	/**< SHA3-384 hash in size of bytes */
#define XSECURE_SHA_DIGEST_OFFSET		(0x10U) /**< SHA Digest Register */

/***************************** Type Definitions******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/***************************** Function Prototypes ******************************************/
int XSecure_ShaInitialize(XSecure_Sha* const InstancePtr, XPmcDma* DmaPtr);
int XSecure_ShaStart(XSecure_Sha* const InstancePtr, XSecure_ShaMode ShaMode);
int XSecure_ShaUpdate(XSecure_Sha* const InstancePtr, u64 DataAddr, const u32 Size);
int XSecure_ShaFinish(XSecure_Sha* const InstancePtr, u64 HashAddr, u32 HashBufSize);
int XSecure_ShaDigest(XSecure_Sha* const InstancePtr, XSecure_ShaMode ShaMode, const u64 DataAddr,
u32 DataSize, u64 HashAddr, u32 HashBufSize);
int XSecure_ShaLastUpdate(XSecure_Sha *InstancePtr);
void XSecure_ShaSetDataContext(XSecure_Sha *InstancePtr);
int XSecure_ShaValidateModeAndCfgInstance(XSecure_Sha * const InstancePtr,
	XSecure_ShaMode ShaMode);

#ifdef __cplusplus
}
#endif

#endif /** XSECURE_SHA_H_ */
