/******************************************************************************
*
* (c) Copyright 2012 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
*******************************************************************************/
/*****************************************************************************/
/**
*
* @file xsecure_sha.h
*
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
*
* </pre>
*
* @note
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
/**
* CSU SHA3 Memory Map
*/
#define XSECURE_CSU_SHA3_START_START	(1U << 0) /**< SHA Start Message */

#define XSECURE_CSU_SHA3_RESET_RESET	(1U << 0) /**< SHA Reset Value */

#define XSECURE_CSU_SHA3_DONE_DONE	(1U << 0) /**< SHA Done */

#define XSECURE_SHA3_BLOCK_LEN		(104U) /**< SHA min block length */

#define XSECURE_SHA3_LAST_PACKET	(0x1U) /**< Last Data Packet */

/***************************** Type Definitions******************************/

/**
 * The SHA-3 driver instance data structure. A pointer to an instance data
 * structure is passed around by functions to refer to a specific driver
 * instance.
 */
typedef struct {
	u32 BaseAddress;  /**< Device Base Address */
	XCsuDma *CsuDmaPtr; /**< Pointer to CSU DMA Instance */
	u32 Sha3Len; /**< SHA3 Input Length */
} XSecure_Sha3;

/***************************** Function Prototypes ***************************/
/* Initialization */
s32 XSecure_Sha3Initialize(XSecure_Sha3 *InstancePtr, XCsuDma *CsuDmaPtr);

void XSecure_Sha3Start(XSecure_Sha3 *InstancePtr);

/* Data Transfer */
u32 XSecure_Sha3Update(XSecure_Sha3 *InstancePtr, const u8 *Data,
						const u32 Size, const u32 EndLast);
u32 XSecure_Sha3Finish(XSecure_Sha3 *InstancePtr, u8 *Hash);

/* Complete SHA digest calculation */
void XSecure_Sha3Digest(XSecure_Sha3 *InstancePtr, const u8 *In,
						const u32 Size, u8 *Out);

u32 XSecure_Sha3FinishPad(XSecure_Sha3 *InstancePtr, u8 *Hash);

void XSecure_Sha3_ReadHash(XSecure_Sha3 *InstancePtr, u8 *Hash);
#ifdef __cplusplus
extern "C" }
#endif

#endif /** XSECURE_SHA_H */
