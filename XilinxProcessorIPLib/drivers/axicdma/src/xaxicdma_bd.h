/******************************************************************************
*
* (c) Copyright 2010-2013 Xilinx, Inc. All rights reserved.
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
******************************************************************************/
/*****************************************************************************/
/**
 *  @file xaxicdma_bd.h
 *
 * The API definition for the Buffer Descriptor (BD).
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00a jz   04/16/10 First release
 * </pre>
 *
 *****************************************************************************/

#ifndef XAXICDMA_BD_H_    /* prevent circular inclusions */
#define XAXICDMA_BD_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xstatus.h"
#include "xaxicdma_hw.h"
#include "xdebug.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
/**
 * XAxiCdma_Bd
 *
 * Buffer descriptor, shared by hardware and software
 * The structure of the BD is as the following, however, the user never
 * directly accesses the fields in the BD. The following shows the fields
 * inside a BD:
 *
 *  u32 NextBdPtr;
 *  u32 Pad1;  -- This is always 0
 *  u32 SrcAddr;
 *  u32 Pad2;  -- This is always 0
 *  u32 DstAddr;
 *  u32 Pad3;  -- This is always 0
 *  int Length;
 *  u32 Status;
 *  u32 PhysAddr;
 *  u32 IsLite;
 *  u32 HasDRE;
 *  u32 WordLen;
 */
typedef u32 XAxiCdma_Bd[XAXICDMA_BD_NUM_WORDS];

/**************************** Macros (Inline Functions) Definitions **********/

/*****************************************************************************/
/* Macros to read a word from a BD
 *
 * @note
 * c-style signature:
 *   u32 XAxiCdma_BdRead(XAxiCdma_Bd* BdPtr, int Offset);
 *****************************************************************************/
#define XAxiCdma_BdRead(BdPtr, Offset)   \
	*(u32 *)((u32)(BdPtr) + (u32)(Offset))

/*****************************************************************************/
/* Macros to write to a word in a BD
 *
 * @note
 * c-style signature:
 *    u32 XAxiCdma_BdWrite(XAxiCdma_Bd* BdPtr, int Offset, u32 Value )
 *****************************************************************************/
#define XAxiCdma_BdWrite(BdPtr, Offset, Value)   \
	*(u32 *)((u32)(BdPtr) + (u32)(Offset)) = (u32)(Value)


/************************** Function Prototypes ******************************/

void XAxiCdma_BdClear(XAxiCdma_Bd *BdPtr);
void XAxiCdma_BdClone(XAxiCdma_Bd *BdPtr, XAxiCdma_Bd *TmpBd);
u32 XAxiCdma_BdGetNextPtr(XAxiCdma_Bd *BdPtr);
void XAxiCdma_BdSetNextPtr(XAxiCdma_Bd *BdPtr, u32 NextBdPtr);
u32 XAxiCdma_BdGetSts(XAxiCdma_Bd *BdPtr);
void XAxiCdma_BdClearSts(XAxiCdma_Bd *BdPtr);
int XAxiCdma_BdSetSrcBufAddr(XAxiCdma_Bd *BdPtr, u32 Addr);
u32 XAxiCdma_BdGetSrcBufAddr(XAxiCdma_Bd *BdPtr);
int XAxiCdma_BdSetDstBufAddr(XAxiCdma_Bd *BdPtr, u32 Addr);
u32 XAxiCdma_BdGetDstBufAddr(XAxiCdma_Bd *BdPtr);
int XAxiCdma_BdSetLength(XAxiCdma_Bd *BdPtr, int LenBytes);
int XAxiCdma_BdGetLength(XAxiCdma_Bd *BdPtr);
void XAxiCdma_BdSetPhysAddr(XAxiCdma_Bd *BdPtr, u32 PhysAddr);
void XAxiCdma_BdSetIsLite(XAxiCdma_Bd *BdPtr, int IsLite);
void XAxiCdma_BdSetHasDRE(XAxiCdma_Bd *BdPtr, int HasDRE);
void XAxiCdma_BdSetWordLen(XAxiCdma_Bd *BdPtr, int WordLen);
void XAxiCdma_BdSetMaxLen(XAxiCdma_Bd *BdPtr, int MaxLen);
u32 XAxiCdma_BdGetPhysAddr(XAxiCdma_Bd *BdPtr);

void XAxiCdma_DumpBd(XAxiCdma_Bd* BdPtr);

#ifdef __cplusplus
}
#endif

#endif    /* prevent circular inclusions */
