/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *  @file xaxicdma_bd.c
* @addtogroup axicdma Overview
* @{
 *
* @brief Implements buffer descriptor access helpers for AXI CDMA.
 * The implementation for the Buffer Descriptor (BD) API functions.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00a jz   04/08/10 First release
 * 2.01a rkv  01/25/11 Replaced with "\r\n" in place of "\n\r" in printf
 *		       statements
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files *********************************/
#include "xaxicdma.h"
#include "xaxicdma_bd.h"

/** @cond INTERNAL */
#define XAXICDMA_PAGE_SIZE  0x1000
/** @endcond */

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/*****************************************************************************/
/**
 * @brief Clears the content of the BD.
 *
 * @param[in] BdPtr is the pointer to the BD to be cleared
 *
 *****************************************************************************/
void XAxiCdma_BdClear(XAxiCdma_Bd *BdPtr)
{
	memset((void *)((UINTPTR)BdPtr + XAXICDMA_BD_START_CLEAR), 0,
	       XAXICDMA_BD_TO_CLEAR);

	return;
}

/*****************************************************************************/
/**
 * @brief Clones the content from the template BD.
 *
 * @param[in] BdPtr is the pointer to the BD to be cloned to
 * @param[in] TmpBd is the pointer to the BD to be cloned from
 *
 *****************************************************************************/
void XAxiCdma_BdClone(XAxiCdma_Bd *BdPtr, XAxiCdma_Bd *TmpBd)
{

	memcpy((void *)((UINTPTR)BdPtr + XAXICDMA_BD_START_CLEAR),
	       (void *)((UINTPTR)TmpBd + XAXICDMA_BD_START_CLEAR),
	       XAXICDMA_BD_TO_CLEAR);

	return;
}

/*****************************************************************************/
/**
 * @brief Reads the next BD pointer from the BD.
 *
 * @param[in] BdPtr is the pointer to the BD to read the next BD pointer.
 *
 * @return	The next BD pointer of the BD.
 *
 *****************************************************************************/
LONG XAxiCdma_BdGetNextPtr(XAxiCdma_Bd *BdPtr)
{
	u32 addrlen;
	addrlen = XAxiCdma_BdGetAddrLength(BdPtr);

	if (addrlen > 32) {
		return (u64)((XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_NDESC_OFFSET)) |
			     ((uint64_t)(XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_NDESC_MSB_OFFSET)) << 32U));
	} else {
		return (u32)(XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_NDESC_OFFSET));
	}
}

/*****************************************************************************/
/**
 * @brief Sets the next BD pointer in the BD.
 *
 * @param[in] BdPtr is the pointer to the BD to set the next BD pointer.
 * @param[in] NextBdPtr is the next BD pointer value.
 *
 *****************************************************************************/
void XAxiCdma_BdSetNextPtr(XAxiCdma_Bd *BdPtr, UINTPTR NextBdPtr)
{
	u32 addrlen;
	addrlen = XAxiCdma_BdGetAddrLength(BdPtr);

	XAxiCdma_BdWrite(BdPtr, XAXICDMA_BD_NDESC_OFFSET,
			 (NextBdPtr & XAXICDMA_DESC_LSB_MASK));
	if (addrlen > 32)
		XAxiCdma_BdWrite(BdPtr, XAXICDMA_BD_NDESC_MSB_OFFSET,
				 UPPER_32_BITS(NextBdPtr));
	return;
}

/*****************************************************************************/
/**
 * @brief Reads the completion status of the BD.
 *
 * @param[in] BdPtr is the pointer to the BD to get the status
 *
 * @return	The status bits of the BD.
 *
 *****************************************************************************/
u32 XAxiCdma_BdGetSts(XAxiCdma_Bd *BdPtr)
{

	return ((u32)(XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_STS_OFFSET)) &
		XAXICDMA_BD_STS_ALL_MASK);
}

/*****************************************************************************/
/**
 * @brief Clears the status bits of the BD. The status bits include
 * the completion bit as well as the error bits.
 *
 * This is the only function that needs to be called to clear a completed
 * BD. All other fields (except reserved words) need to be re-assigned before
 * re-submission.
 *
 * @param[in] BdPtr is the pointer to the BD to clear the status
 *
 *****************************************************************************/
void XAxiCdma_BdClearSts(XAxiCdma_Bd *BdPtr)
{

	XAxiCdma_BdWrite(BdPtr, XAXICDMA_BD_STS_OFFSET, 0);

	return;
}

/*****************************************************************************/
/**
 * @brief Sets the source address of the BD.
 *
 * @param[in] BdPtr is the pointer to the BD to set the source address
 * @param[in] Addr is the source address of the buffer
 *
 * @return
 *		- XST_SUCCESS if buffer address set successfully
 *		- XST_INVALID_PARAM if buffer address is not aligned and
 *		hardware build has no DRE or in lite mode
 *
 *****************************************************************************/
u32 XAxiCdma_BdSetSrcBufAddr(XAxiCdma_Bd *BdPtr, UINTPTR Addr)
{
	u32 addrlen;
	addrlen = XAxiCdma_BdGetAddrLength(BdPtr);

	if (XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_HASDRE_OFFSET) == 0) {

		if (Addr &
		    (XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_WORDLEN_OFFSET) - 1)) {

			xdbg_printf(XDBG_DEBUG_ERROR,
				    "Unaligned transfers not supported, address %x "
				    "not aligned\r\n", (unsigned int)Addr);

			return XST_INVALID_PARAM;
		}
	}

	XAxiCdma_BdWrite(BdPtr, XAXICDMA_BD_BUFSRC_OFFSET, LOWER_32_BITS(Addr));
	if (addrlen > 32)
		XAxiCdma_BdWrite(BdPtr, XAXICDMA_BD_BUFSRC_MSB_OFFSET,
				 UPPER_32_BITS(Addr));

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief Reads the source address of the BD.
 *
 * @param[in] BdPtr is the pointer to the BD to get the source address
 *
 * @return	The source address of the buffer that this BD describes
 *
 *****************************************************************************/
LONG XAxiCdma_BdGetSrcBufAddr(XAxiCdma_Bd *BdPtr)
{
	u32 addrlen;
	addrlen = XAxiCdma_BdGetAddrLength(BdPtr);

	if (addrlen > 32)
		return (u64)((XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_BUFSRC_OFFSET)) |
			     ((uint64_t)(XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_BUFSRC_MSB_OFFSET))
			      << 32U));
	else {
		return (u32)(XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_BUFSRC_OFFSET));
	}
}

/*****************************************************************************/
/**
 * @brief Sets the destination address of the BD.
 *
 * @param[in] BdPtr is the pointer to the BD to set the destination address
 * @param[in] Addr is the destination address of the buffer
 *
 * @return
 * 		- XST_SUCCESS if buffer address set successfully
 * 		- XST_INVALID_PARAM if buffer address is not aligned and
 *			hardware build has no DRE or in lite mode
 *
 *****************************************************************************/
u32 XAxiCdma_BdSetDstBufAddr(XAxiCdma_Bd *BdPtr, UINTPTR Addr)
{
	u32 addrlen;
	addrlen = XAxiCdma_BdGetAddrLength(BdPtr);

	if (XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_HASDRE_OFFSET) == 0) {

		if (Addr &
		    (XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_WORDLEN_OFFSET) - 1)) {

			xdbg_printf(XDBG_DEBUG_ERROR,
				    "Unaligned transfers not supported, address "
				    "not aligned %x\r\n", (unsigned int)Addr);
			return XST_INVALID_PARAM;
		}
	}

	XAxiCdma_BdWrite(BdPtr, XAXICDMA_BD_BUFDST_OFFSET, LOWER_32_BITS(Addr));
	if (addrlen > 32)
		XAxiCdma_BdWrite(BdPtr, XAXICDMA_BD_BUFDST_MSB_OFFSET,
				 UPPER_32_BITS(Addr));

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief Reads the destination address of the BD.
 *
 * @param[in] BdPtr is the pointer to the BD to get the destination address
 *
 * @return	The destination address of the buffer that this BD describes
 *
 *****************************************************************************/
LONG XAxiCdma_BdGetDstBufAddr(XAxiCdma_Bd *BdPtr)
{
	u32 addrlen;
	addrlen = XAxiCdma_BdGetAddrLength(BdPtr);

	if (addrlen > 32)
		return (u64) (XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_BUFDST_OFFSET) |
			      ((uint64_t)(XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_BUFDST_MSB_OFFSET)) << 32U));
	else {
		return (u32)(XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_BUFDST_OFFSET));
	}
}

/*****************************************************************************/
/**
 * @brief Sets the length of the BD.
 *
 * @param[in] BdPtr is the pointer to the BD to set the length
 * @param[in] LenBytes is the length of the buffer
 *
 * @return
 *		- XST_SUCCESS for success
 *		- XST_INVALID_PARAM if invalid length:
 *		Length value out of valid range
 *
 * @note	One case that will cause hardware failure is not covered here:
 *		If the start address is not at page offset 0, and the length
 *		plus the offset crosses the page boundary, this will cause
 *		slave error if the hardware is built in lite mode.
 *
 *****************************************************************************/
u32 XAxiCdma_BdSetLength(XAxiCdma_Bd *BdPtr, int LenBytes)
{
	int MaxLen;

	MaxLen = XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_MAX_LEN_OFFSET);

	if ((LenBytes < 1) || (LenBytes > MaxLen)) {
		return XST_INVALID_PARAM;
	}

	XAxiCdma_BdWrite(BdPtr, XAXICDMA_BD_CTRL_LEN_OFFSET, LenBytes);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief Reads the length of the BD.
 *
 * @param[in] BdPtr is the pointer to the BD to get the length
 *
 * @return	The length of the buffer that this BD describes
 *
 *****************************************************************************/
u32 XAxiCdma_BdGetLength(XAxiCdma_Bd *BdPtr)
{

	return (u32)(XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_CTRL_LEN_OFFSET));
}

/*****************************************************************************/
/**
 * @brief Sets the physical address of the BD.
 *
 * @param[in] BdPtr is the pointer to the BD to set the physical address
 * @param[in] PhysAddr is the physical address of the BD
 *
 *****************************************************************************/
void XAxiCdma_BdSetPhysAddr(XAxiCdma_Bd *BdPtr, UINTPTR PhysAddr)
{
	u32 addrlen;
	addrlen = XAxiCdma_BdGetAddrLength(BdPtr);

	XAxiCdma_BdWrite(BdPtr, XAXICDMA_BD_PHYS_ADDR_OFFSET, PhysAddr);
	if (addrlen > 32)
		XAxiCdma_BdWrite(BdPtr, XAXICDMA_BD_PHYS_ADDR_MSB_OFFSET,
				 UPPER_32_BITS(PhysAddr));

	return;
}

/*****************************************************************************/
/**
 * @brief Sets the lite mode field of the BD.
 *
 * @param[in] BdPtr is the pointer to the BD to work on
 * @param[in] IsLite is the value for whether hardware is in lite mode
 *
 *****************************************************************************/
void XAxiCdma_BdSetIsLite(XAxiCdma_Bd *BdPtr, int IsLite)
{

	XAxiCdma_BdWrite(BdPtr, XAXICDMA_BD_ISLITE_OFFSET, IsLite);

	return;
}

/*****************************************************************************/
/**
 * @brief Sets the DRE field of the BD.
 *
 * @param[in] BdPtr is the pointer to the BD to work on
 * @param[in] HasDRE is the value for whether hardware has DRE built in
 *
 *****************************************************************************/
void XAxiCdma_BdSetHasDRE(XAxiCdma_Bd *BdPtr, int HasDRE)
{

	XAxiCdma_BdWrite(BdPtr, XAXICDMA_BD_HASDRE_OFFSET, HasDRE);

	return;
}

/*****************************************************************************/
/**
 * @brief Sets the word length field of the BD.
 *
 * @param[in] BdPtr is the pointer to the BD to work on
 * @param[in] WordLen is the length of word in bytes
 *
 *****************************************************************************/
void XAxiCdma_BdSetWordLen(XAxiCdma_Bd *BdPtr, int WordLen)
{

	XAxiCdma_BdWrite(BdPtr, XAXICDMA_BD_WORDLEN_OFFSET, WordLen);

	return;
}

/*****************************************************************************/
/**
 * @brief Sets the maximum transfer length field of the BD.
 *
 * @param[in] BdPtr is the pointer to the BD to work on
 * @param[in] MaxLen is the maximum transfer length in bytes
 *
 *****************************************************************************/
void XAxiCdma_BdSetMaxLen(XAxiCdma_Bd *BdPtr, int MaxLen)
{

	XAxiCdma_BdWrite(BdPtr, XAXICDMA_BD_MAX_LEN_OFFSET, MaxLen);

	return;
}

/*****************************************************************************/
/**
 * @brief Reads the physical address of the BD.
 *
 * @param[in] BdPtr is the pointer to the BD to get the physical address
 *
 * @return	The physical address of the BD
 *
 *****************************************************************************/
LONG XAxiCdma_BdGetPhysAddr(XAxiCdma_Bd *BdPtr)
{
	u32 addrlen;
	addrlen = XAxiCdma_BdGetAddrLength(BdPtr);

	if (addrlen > 32)
		return (u64)((XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_PHYS_ADDR_OFFSET)) |
			     ((uint64_t)(XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_PHYS_ADDR_MSB_OFFSET))
			      << 32U));
	else {
		return (u32)(XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_PHYS_ADDR_OFFSET));
	}
}

/*****************************************************************************/
/**
 * @brief Dumps the BD for debug purposes.
 *
 * @param[in] BdPtr is the pointer to the BD to get the physical address
 *
 *****************************************************************************/
void XAxiCdma_DumpBd(XAxiCdma_Bd *BdPtr)
{
	xil_printf("\r\nDump BD %p:\r\n", BdPtr);

	xil_printf("Next BD ptr \t%x\r\n",
		   (unsigned int)XAxiCdma_BdGetNextPtr(BdPtr));
	xil_printf("Buffer srcaddr \t%x\r\n",
		   (unsigned int)XAxiCdma_BdGetSrcBufAddr(BdPtr));
	xil_printf("Buffer dstaddr \t%x\r\n",
		   (unsigned int)XAxiCdma_BdGetDstBufAddr(BdPtr));
	xil_printf("Buffer Length \t%x\r\n",
		   (unsigned int)XAxiCdma_BdGetLength(BdPtr));
	xil_printf("BD status \t%x\r\n",
		   (unsigned int)XAxiCdma_BdGetSts(BdPtr));
	xil_printf("BD phys Addr \t%x\r\n",
		   (unsigned int)XAxiCdma_BdGetPhysAddr(BdPtr));
	xil_printf("BD is Lite \t%x\r\n",
		   (unsigned int)XAxiCdma_BdRead(BdPtr,
				   XAXICDMA_BD_ISLITE_OFFSET));
	xil_printf("BD has DRE \t%x\r\n",
		   (unsigned int)XAxiCdma_BdRead(BdPtr,
				   XAXICDMA_BD_HASDRE_OFFSET));
	xil_printf("BD word length \t%x\r\n\r\n",
		   (unsigned int)XAxiCdma_BdRead(BdPtr,
				   XAXICDMA_BD_WORDLEN_OFFSET));
	xil_printf("BD max transfer length \t%x\r\n\r\n",
		   (unsigned int)XAxiCdma_BdRead(BdPtr,
				   XAXICDMA_BD_MAX_LEN_OFFSET));

	return;
}
/** @} */
