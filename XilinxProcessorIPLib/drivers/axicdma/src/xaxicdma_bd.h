/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XAXICDMA_BD_H_    /* prevent circular inclusions */
#define XAXICDMA_BD_H_

/*****************************************************************************/
/**
 *  @file xaxicdma_bd.h
* @addtogroup axicdma Overview
* @{
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
 * @brief Buffer descriptor shared by hardware and software.
 *
 * The user never directly accesses BD fields. The layout contains next
 * descriptor pointers, source and destination addresses, length, status,
 * and configuration metadata.
 */
typedef UINTPTR XAxiCdma_Bd[XAXICDMA_BD_NUM_WORDS];

/**************************** Macros (Inline Functions) Definitions **********/

/**
 * @brief Reads a word from a buffer descriptor.
 *
 * @param[in] BdPtr BD to operate on.
 * @param[in] Offset Byte offset of the word to read.
 *
 * @return The 32-bit value at the specified offset.
 *
 * @note
 * C-style signature:
 *   u32 XAxiCdma_BdRead(XAxiCdma_Bd* BdPtr, int Offset);
 */
#define XAxiCdma_BdRead(BdPtr, Offset)   \
	(*(u32 *)((UINTPTR)((void *)(BdPtr)) + (u32)(Offset)))

/**
 * @brief Writes a word to a buffer descriptor.
 *
 * @param[in] BdPtr BD to operate on.
 * @param[in] Offset Byte offset of the word to write.
 * @param[in] Value 32-bit value to write.
 *
 * @note
 * C-style signature:
 *    u32 XAxiCdma_BdWrite(XAxiCdma_Bd* BdPtr, int Offset, u32 Value )
 */
#define XAxiCdma_BdWrite(BdPtr, Offset, Value)   \
	(*(u32 *)((UINTPTR)(void *)(BdPtr) + (u32)(Offset))) = (u32)(Value)

/**
 * @brief Sets the address length field of a BD.
 *
 * @param[in] BdPtr BD to operate on.
 * @param[in] AddrLen Address width of the CDMA.
 *
 * @note
 * C-style signature:
 *	u32 XAxiCdma_BdSetAddrLen(XAxiCdma_Bd* BdPtr, int AddrLen)
 */
#define XAxiCdma_BdSetAddrLen(BdPtr, AddrLen)   \
	XAxiCdma_BdWrite(BdPtr, XAXICDMA_BD_ADDRLEN_OFFSET, AddrLen);

/**
 * @brief Reads the address length field of a BD.
 *
 * @param[in] BdPtr BD to operate on.
 *
 * @return Address length stored in the BD.
 *
 * @note
 * C-style signature:
 *	u32 XAxiCdma_BdGetAddrLength(XAxiCdma_Bd* BdPtr)
 */
#define XAxiCdma_BdGetAddrLength(BdPtr)   \
	XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_ADDRLEN_OFFSET);


/************************** Function Prototypes ******************************/

void XAxiCdma_BdClear(XAxiCdma_Bd *BdPtr);
void XAxiCdma_BdClone(XAxiCdma_Bd *BdPtr, XAxiCdma_Bd *TmpBd);
LONG XAxiCdma_BdGetNextPtr(XAxiCdma_Bd *BdPtr);
void XAxiCdma_BdSetNextPtr(XAxiCdma_Bd *BdPtr, UINTPTR NextBdPtr);
u32 XAxiCdma_BdGetSts(XAxiCdma_Bd *BdPtr);
void XAxiCdma_BdClearSts(XAxiCdma_Bd *BdPtr);
u32 XAxiCdma_BdSetSrcBufAddr(XAxiCdma_Bd *BdPtr, UINTPTR Addr);
LONG XAxiCdma_BdGetSrcBufAddr(XAxiCdma_Bd *BdPtr);
u32 XAxiCdma_BdSetDstBufAddr(XAxiCdma_Bd *BdPtr, UINTPTR Addr);
LONG XAxiCdma_BdGetDstBufAddr(XAxiCdma_Bd *BdPtr);
u32 XAxiCdma_BdSetLength(XAxiCdma_Bd *BdPtr, int LenBytes);
u32 XAxiCdma_BdGetLength(XAxiCdma_Bd *BdPtr);
void XAxiCdma_BdSetPhysAddr(XAxiCdma_Bd *BdPtr, UINTPTR PhysAddr);
void XAxiCdma_BdSetIsLite(XAxiCdma_Bd *BdPtr, int IsLite);
void XAxiCdma_BdSetHasDRE(XAxiCdma_Bd *BdPtr, int HasDRE);
void XAxiCdma_BdSetWordLen(XAxiCdma_Bd *BdPtr, int WordLen);
void XAxiCdma_BdSetMaxLen(XAxiCdma_Bd *BdPtr, int MaxLen);
LONG XAxiCdma_BdGetPhysAddr(XAxiCdma_Bd *BdPtr);

void XAxiCdma_DumpBd(XAxiCdma_Bd *BdPtr);

#ifdef __cplusplus
}
#endif

/** @} */
#endif    /* prevent circular inclusions */
