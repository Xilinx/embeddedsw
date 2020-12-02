/******************************************************************************
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_sha.h
* @addtogroup xsecure_sha3_zynqmp_apis XilSecure SHA3 ZynqMP APIs
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
*   - XSecure_Sha3Initialize(XSecure_Sha3 *InstancePtr, XCsuDma *CsuDmaPtr)
*
* A pointer to CsuDma instance has to be passed in initialization as CSU
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
* 1.00  ba   11/05/14 Initial release
* 2.0   vns  01/28/17 Added API to read SHA3 hash.
* 2.2   vns  07/06/17 Added doxygen tags
* 3.0   vns  01/23/18 Added NIST SHA3 support.
* 4.0   arc  18/12/18 Fixed MISRA-C violations.
*       vns  03/12/19 Modified as part of XilSecure code re-arch.
        arc  03/20/19 Changed prototype of the functions return type
                      as void to u32.
*       psl  03/26/19 Fixed MISRA-C violation
* 4.1   mmd  07/05/19 Optimized the code
* 4.2   har  11/07/19 Typo correction to enable compilation in C++
*       har  03/23/20 Moved to zynqmp directory
*                     Replaced function like macro with inline function
*       ana  10/15/20 Updated doxygen tags

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
#include "xsecure_sha_hw.h"
#include "xcsudma.h"
#include "xil_assert.h"
#include "xsecure_sss.h"

/************************** Constant Definitions ****************************/
/** @cond xsecure_internal
@{
*/

/**
* CSU SHA3 Memory Map
*/
#define XSECURE_CSU_SHA3_START_START	(1U << 0) /**< SHA Start Message */

#define XSECURE_CSU_SHA3_RESET_RESET	(1U << 0) /**< SHA Reset Value */

#define XSECURE_CSU_SHA3_DONE_DONE	(1U << 0) /**< SHA Done */

#define XSECURE_SHA3_BLOCK_LEN		(104U) /**< SHA min block length */

#define XSECURE_SHA3_LAST_PACKET	(0x1U) /**< Last Data Packet */

#define XSECURE_CSU_DMA_MAX_TRANSFER	(0x1FFFFFFCU) /** < CSU DMA Max Transfer
							rate in bytes*/
#define XSECURE_SHA_TIMEOUT_MAX         (0x1FFFFU)

/***************************** Type Definitions******************************/

/* SHA3 type selection */
typedef enum {
	XSECURE_CSU_NIST_SHA3, /**< NIST sha3 */
	XSECURE_CSU_KECCAK_SHA3 /**< Keccak sha3 */
}XSecure_Sha3PadType;

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
	XCsuDma *CsuDmaPtr; /**< Pointer to CSU DMA Instance */
	u32 Sha3Len; /**< SHA3 Input Length */
	XSecure_Sha3PadType Sha3PadType; /** Selection for Sha3 */
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
s32 XSecure_Sha3Initialize(XSecure_Sha3 *InstancePtr, XCsuDma *CsuDmaPtr);

void XSecure_Sha3Start(XSecure_Sha3 *InstancePtr);

/* Data Transfer */
u32 XSecure_Sha3Update(XSecure_Sha3 *InstancePtr, const u8 *Data,
						const u32 Size);
u32 XSecure_Sha3Finish(XSecure_Sha3 *InstancePtr, u8 *Hash);

/* Complete SHA digest calculation */
u32 XSecure_Sha3Digest(XSecure_Sha3 *InstancePtr, const u8 *In,
						const u32 Size, u8 *Out);

void XSecure_Sha3_ReadHash(XSecure_Sha3 *InstancePtr, u8 *Hash);

s32 XSecure_Sha3PadSelection(XSecure_Sha3 *InstancePtr,
		XSecure_Sha3PadType Sha3PadType);

s32 XSecure_Sha3LastUpdate(XSecure_Sha3 *InstancePtr);

u32 XSecure_Sha3WaitForDone(XSecure_Sha3 *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /** XSECURE_SHA_H */

/* @} */