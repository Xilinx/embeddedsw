/******************************************************************************
*
* Copyright (C) 2014 - 18 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
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
#include "xsecure_hw.h"
#include "xcsudma.h"
#include "xil_assert.h"

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

/***************************** Type Definitions******************************/

/* SHA3 type selection */
typedef enum {
	XSECURE_CSU_NIST_SHA3, /**< NIST sha3 */
	XSECURE_CSU_KECCAK_SHA3 /**< Keccak sha3 */
}XSecure_Sha3PadType;

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
} XSecure_Sha3;
/**
@}
@endcond */
/***************************** Function Prototypes ***************************/
/* Initialization */
s32 XSecure_Sha3Initialize(XSecure_Sha3 *InstancePtr, XCsuDma *CsuDmaPtr);

void XSecure_Sha3Start(XSecure_Sha3 *InstancePtr);

/* Data Transfer */
void XSecure_Sha3Update(XSecure_Sha3 *InstancePtr, const u8 *Data,
						const u32 Size);
void XSecure_Sha3Finish(XSecure_Sha3 *InstancePtr, u8 *Hash);

/* Complete SHA digest calculation */
void XSecure_Sha3Digest(XSecure_Sha3 *InstancePtr, const u8 *In,
						const u32 Size, u8 *Out);
void XSecure_Sha3_ReadHash(XSecure_Sha3 *InstancePtr, u8 *Hash);
s32 XSecure_Sha3PadSelection(XSecure_Sha3 *InstancePtr,
		XSecure_Sha3PadType Sha3Type);

#ifdef __cplusplus
extern "C" }
#endif

#endif /** XSECURE_SHA_H */
/* @} */