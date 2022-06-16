/******************************************************************************
* Copyright (c) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_sha.h
* @addtogroup xsecure_sha3_versal_apis XilSecure SHA3 Versal APIs
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
* 1.0   har  03/20/20 Initial release
* 4.2   har  03/20/20 Updated file version to sync with library version
*       bvi  04/07/20 Renamed csudma as pmcdma
* 4.3   ana  06/04/20 created XSecure_Sha3Hash structure variable
*       kpt  08/26/20 Changed argument type from u8* to UINTPTR
*       rpo  09/10/20 Changed the return type of some prototypes
*       am   09/24/20 Resolved MISRA C violations
*       har  10/12/20 Addressed security review comments
*       ana  10/15/20 Updated doxygen tags
* 4.5   bm   01/13/21 Added support for 64-bit input data address
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
 * @{
 */

/**
* SHA3 Memory Map
*/

#define XSECURE_SHA3_START_START	(1U << 0) /**< SHA Start Message */

#define XSECURE_SHA3_DONE_DONE		(1U << 0) /**< SHA Done */

#define XSECURE_SHA3_BLOCK_LEN		(104U) /**< SHA min block length */

#define XSECURE_PMC_DMA_MAX_TRANSFER	(0x1FFFFFFCU) /** < PMC DMA Max Transfer
							  rate in bytes*/
#define XSECURE_SHA_TIMEOUT_MAX         (0x1FFFFU)

#define XSECURE_HASH_SIZE_IN_BYTES      (48U)
/**
* SHA3 padding type
*/
/***************************** Type Definitions******************************/

typedef struct {
	u8 Hash[XSECURE_HASH_SIZE_IN_BYTES];
} XSecure_Sha3Hash;

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
	u32 PartialLen; /**< Partial Length */
	u32 IsLastUpdate; /**< Last DMA block indication */
	u8 PartialData[XSECURE_SHA3_BLOCK_LEN]; /**< Partial Data */
	XSecure_Sss SssInstance; /**< SSS Instance */
	XSecure_Sha3State Sha3State; /**< SHA engine state */
} XSecure_Sha3;
/**
 * @}
 * @endcond
 */

/***************************** Function Prototypes ***************************/
/* Initialization */
int XSecure_Sha3Initialize(XSecure_Sha3 *InstancePtr, XPmcDma *DmaPtr);

int XSecure_Sha3Start(XSecure_Sha3 *InstancePtr);

/* Data Transfer */
int XSecure_Sha3Update(XSecure_Sha3 *InstancePtr, const UINTPTR InDataAddr,
		       const u32 Size);
int XSecure_Sha3Update64Bit(XSecure_Sha3 *InstancePtr, u64 InDataAddr,
			const u32 Size);
int XSecure_Sha3Finish(XSecure_Sha3 *InstancePtr, XSecure_Sha3Hash *Sha3Hash);


/* Complete SHA digest calculation */
int XSecure_Sha3Digest(XSecure_Sha3 *InstancePtr, const UINTPTR InDataAddr,
		       const u32 Size, XSecure_Sha3Hash *Sha3Hash);

int XSecure_Sha3ReadHash(const XSecure_Sha3 *InstancePtr,
			 XSecure_Sha3Hash *Sha3Hash);

int XSecure_Sha3LastUpdate(XSecure_Sha3 *InstancePtr);

int XSecure_Sha3Kat(XSecure_Sha3 *SecureSha3);

#ifdef __cplusplus
}
#endif

#endif /** XSECURE_SHA_H */

/* @} */
