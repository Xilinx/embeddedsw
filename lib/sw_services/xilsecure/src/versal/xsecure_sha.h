/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_sha.h
* @addtogroup xsecure_sha3_apis SHA-3
* @{
* @cond xsecure_internal
* This file Contains the function prototypes, defines and macros for
* the SHA-384 hardware module.
*
* This driver supports the following features:
*
* - SHA-3 hash calculation
*
* <b>Initialization & Configuration</b>
*
* The SHA-3 driver instance can be initialized
* in the following way:
*
*   - XSecure_Sha3Initialize(XSecure_Sha3 *InstancePtr, XPmcDma *DmaPtr)
*
* A pointer to XPmcDma instance has to be passed in initialization as PMC
* DMA will be used for data transfers to SHA module.
*
*
* @note
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 4.2   har  03/20/20 Initial release
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/
#ifndef XSECURE_SHA_H
#define XSECURE_SHA_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xpmcdma.h"
#include "xsecure_sss.h"

/************************** Constant Definitions ****************************/
/** @cond xsecure_internal
@{
*/

/**
* SHA3 Memory Map
*/

#define XSECURE_SHA3_START_START	(1U << 0) /**< SHA Start Message */

#define XSECURE_SHA3_RESET_RESET	(1U << 0) /**< SHA Reset Value */

#define XSECURE_SHA3_DONE_DONE	(1U << 0) /**< SHA Done */

#define XSECURE_SHA3_BLOCK_LEN		(104U) /**< SHA min block length */

#define XSECURE_SHA3_LAST_PACKET	(0x1U) /**< Last Data Packet */

#define XSECURE_PMC_DMA_MAX_TRANSFER	(0x1FFFFFFCU) /** < PMC DMA Max Transfer
							rate in bytes*/
#define XSECURE_SHA_TIMEOUT_MAX         (0x1FFFFU)

#define XSECURE_HASH_SIZE_IN_BYTES      (48U)
/**
* SHA3 padding type
*/
/***************************** Type Definitions******************************/

/* Sha3 driver states */
typedef enum {
	XSECURE_SHA3_UNINITIALIZED = 0,
	XSECURE_SHA3_INITIALIZED,
	XSECURE_SHA3_ENGINE_STARTED
} XSecure_Sha3State;

/**
 * The SHA-3 driver instance data structure. A pointer to an instance data
 * structure is passed around by functions to refer to a specific driver
 * instance.
 */
typedef struct {
	u32 BaseAddress;  /**< Device Base Address */
	XPmcDma *DmaPtr; /**< Pointer to PMC DMA Instance */
	u32 Sha3Len; /**< SHA3 Input Length */
	u32 PartialLen;
	u32 IsLastUpdate;
	u8 PartialData[XSECURE_SHA3_BLOCK_LEN];
	XSecure_Sss SssInstance;
	XSecure_Sha3State Sha3State;
} XSecure_Sha3;
/**
@}
@endcond */

/***************************** Function Prototypes ***************************/
/* Initialization */
u32 XSecure_Sha3Initialize(XSecure_Sha3 *InstancePtr, XPmcDma *DmaPtr);

void XSecure_Sha3Start(XSecure_Sha3 *InstancePtr);

/* Data Transfer */
u32 XSecure_Sha3Update(XSecure_Sha3 *InstancePtr, const u8 *Data,
						const u32 Size);
u32 XSecure_Sha3Finish(XSecure_Sha3 *InstancePtr, u8 *Hash);

/* Complete SHA digest calculation */
u32 XSecure_Sha3Digest(XSecure_Sha3 *InstancePtr, const u8 *In,
						const u32 Size, u8 *Out);

void XSecure_Sha3ReadHash(XSecure_Sha3 *InstancePtr, u8 *Hash);

u32 XSecure_Sha3LastUpdate(XSecure_Sha3 *InstancePtr);

u32 XSecure_Sha3WaitForDone(XSecure_Sha3 *InstancePtr);

u32 XSecure_Sha3Kat(XSecure_Sha3 *SecureSha3);

#ifdef __cplusplus
}
#endif

#endif /** XSECURE_SHA_H */
/* @} */
