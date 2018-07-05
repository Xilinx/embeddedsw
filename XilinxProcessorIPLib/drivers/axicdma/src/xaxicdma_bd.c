/******************************************************************************
*
* Copyright (C) 2010 - 2019 Xilinx, Inc.  All rights reserved.
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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
 *  @file xaxicdma_bd.c
* @addtogroup axicdma_v4_6
* @{
 *
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

#define XAXICDMA_PAGE_SIZE  0x1000

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/*****************************************************************************/
/**
 * This function clears the content of the BD.
 *
 * @param 	BdPtr is the pointer to the BD to be cleared
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void XAxiCdma_BdClear(XAxiCdma_Bd* BdPtr)
{
	memset((void *)((UINTPTR)BdPtr + XAXICDMA_BD_START_CLEAR), 0,
	    XAXICDMA_BD_TO_CLEAR);

	return;
}

/*****************************************************************************/
/**
 * This function clones the content from the template BD
 *
 * @param 	BdPtr is the pointer to the BD to be cloned to
 * @param 	TmpBd is the pointer to the BD to be cloned from
 *
 * @return	None.
 *
 * @note	None.
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
 * This function gets the next BD pointer of the BD.
 *
 * @param 	BdPtr is the pointer to the BD to get the next BD ptr
 *
 * @return	The next BD ptr of the BD.
 *
 * @note	None.
 *
 *****************************************************************************/
LONG XAxiCdma_BdGetNextPtr(XAxiCdma_Bd* BdPtr)
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
 * This function set the Next BD ptr of a BD
 *
 * @param 	BdPtr is the pointer to the BD to set the Next BD ptr
 * @param 	NextBdPtr is the valud of Next BD ptr
 *
 * @return	None
 *
 * @note	None.
 *
 *****************************************************************************/
void XAxiCdma_BdSetNextPtr(XAxiCdma_Bd* BdPtr, UINTPTR NextBdPtr)
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
 * This function gets the completion status of the BD.
 *
 * @param	BdPtr is the pointer to the BD to get the status
 *
 * @return	The status bits of the BD.
 *
 * @note	None.
 *
 *****************************************************************************/
u32 XAxiCdma_BdGetSts(XAxiCdma_Bd* BdPtr)
{

	return ((u32)(XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_STS_OFFSET)) &
		XAXICDMA_BD_STS_ALL_MASK);
}

/*****************************************************************************/
/**
 * This function clears the status bits of the BD. The status bits include
 * the completion bit as well as the error bits.
 *
 * This is the only function that needs to be called to clear a completed
 * BD. All other fields (except reserved words) need to be re-assigned before
 * re-submission.
 *
 * @param	BdPtr is the pointer to the BD to clear the status
 *
 * @return	None
 *
 * @note	None.
 *
 *****************************************************************************/
void XAxiCdma_BdClearSts(XAxiCdma_Bd* BdPtr)
{

	XAxiCdma_BdWrite(BdPtr, XAXICDMA_BD_STS_OFFSET, 0);

	return;
}

/*****************************************************************************/
/**
 * This function sets the source address of the BD.
 *
 * @param	BdPtr is the pointer to the BD to set the source address
 * @param	Addr is the source address of the buffer
 *
 * @return
 *		- XST_SUCCESS if buffer address set successfully
 *		- XST_INVALID_PARAM if buffer address is not aligned and
 *		hardware build has no DRE or in lite mode
 *
 * @note	None.
 *
 *****************************************************************************/
u32 XAxiCdma_BdSetSrcBufAddr(XAxiCdma_Bd* BdPtr, UINTPTR Addr)
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
 * This function gets the source address of the BD.
 *
 * @param	BdPtr is the pointer to the BD to get the source address
 *
 * @return	The source address of the buffer that this BD describes
 *
 * @note	None.
 *
 *****************************************************************************/
LONG XAxiCdma_BdGetSrcBufAddr(XAxiCdma_Bd* BdPtr)
{
	u32 addrlen;
        addrlen = XAxiCdma_BdGetAddrLength(BdPtr);

	if (addrlen > 32)
		return (u64)((XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_BUFSRC_OFFSET)) |
			((uint64_t)(XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_BUFSRC_MSB_OFFSET))
				<< 32U));
	else
		return (u32)(XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_BUFSRC_OFFSET));
}

/*****************************************************************************/
/**
 * This function sets the destination address of the BD.
 *
 * @param	BdPtr is the pointer to the BD to set the destination address
 * @param	Addr is the destination address of the buffer
 *
 * @return
 * 		- XST_SUCCESS if buffer address set successfully
 * 		- XST_INVALID_PARAM if buffer address is not aligned and
 *			hardware build has no DRE or in lite mode
 *
 * @note	None.
 *
 *****************************************************************************/
u32 XAxiCdma_BdSetDstBufAddr(XAxiCdma_Bd* BdPtr, UINTPTR Addr)
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
 * This function gets the destination address of the BD.
 *
 * @param	BdPtr is the pointer to the BD to get the destination address
 *
 * @return	The destination address of the buffer that this BD describes
 *
 * @note	None
 *
 *****************************************************************************/
LONG XAxiCdma_BdGetDstBufAddr(XAxiCdma_Bd* BdPtr)
{
	u32 addrlen;
    addrlen = XAxiCdma_BdGetAddrLength(BdPtr);

	if (addrlen > 32)
		return (u64) (XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_BUFDST_OFFSET) |
			((uint64_t)(XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_BUFDST_MSB_OFFSET)) << 32U));
	else
		return (u32)(XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_BUFDST_OFFSET));
}

/*****************************************************************************/
/**
 * This function sets the length of the BD.
 *
 * @param	BdPtr is the pointer to the BD to set the length
 * @param	LenBytes is the length of the buffer
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
u32 XAxiCdma_BdSetLength(XAxiCdma_Bd* BdPtr, int LenBytes)
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
 * This function gets the length of the BD.
 *
 * @param	BdPtr is the pointer to the BD to get the length
 *
 * @return	The length of the buffer that this BD describes
 *
 * @note	None
 *
 *****************************************************************************/
u32 XAxiCdma_BdGetLength(XAxiCdma_Bd* BdPtr)
{

	return (u32)(XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_CTRL_LEN_OFFSET));
}

/*****************************************************************************/
/**
 * This function sets the Physical address of the BD.
 *
 * @param	BdPtr is the pointer to the BD to set the physical address
 * @param	PhysAddr is the physical address of the BD
 *
 * @return	None
 *
 * @note	None
 *
 *****************************************************************************/
void XAxiCdma_BdSetPhysAddr(XAxiCdma_Bd* BdPtr, UINTPTR PhysAddr)
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
 * This function sets the is lite mode field of the BD.
 *
 * @param	BdPtr is the pointer to the BD to work on
 * @param	IsLite is the value for whether hardware is in lite mode
 *
 * @return	None
 *
 * @note	None
 *
 *****************************************************************************/
void XAxiCdma_BdSetIsLite(XAxiCdma_Bd* BdPtr, int IsLite)
{

	XAxiCdma_BdWrite(BdPtr, XAXICDMA_BD_ISLITE_OFFSET, IsLite);

	return;
}

/*****************************************************************************/
/**
 * This function sets the has DRE field of the BD.
 *
 * @param	BdPtr is the pointer to the BD to work on
 * @param	HasDRE is the value for whether hardware has DRE built in
 *
 * @return	None
 *
 * @note	None
 *
 *****************************************************************************/
void XAxiCdma_BdSetHasDRE(XAxiCdma_Bd* BdPtr, int HasDRE)
{

	XAxiCdma_BdWrite(BdPtr, XAXICDMA_BD_HASDRE_OFFSET, HasDRE);

	return;
}

/*****************************************************************************/
/**
 * This function sets the word length field of the BD.
 *
 * @param	BdPtr is the pointer to the BD to work on
 * @param	WordLen is the length of word in bytes
 *
 * @return	None
 *
 * @note	None
 *
 *****************************************************************************/
void XAxiCdma_BdSetWordLen(XAxiCdma_Bd* BdPtr, int WordLen)
{

	XAxiCdma_BdWrite(BdPtr, XAXICDMA_BD_WORDLEN_OFFSET, WordLen);

	return;
}

/*****************************************************************************/
/**
 * This function sets the maximum transfer length field of the BD.
 *
 * @param	BdPtr is the pointer to the BD to work on
 * @param	MaxLen is the maximum transfer length in bytes
 *
 * @return	None
 *
 * @note	None
 *
 *****************************************************************************/
void XAxiCdma_BdSetMaxLen(XAxiCdma_Bd* BdPtr, int MaxLen)
{

	XAxiCdma_BdWrite(BdPtr, XAXICDMA_BD_MAX_LEN_OFFSET, MaxLen);

	return;
}

/*****************************************************************************/
/**
 * This function gets the Physical address of the BD.
 *
 * @param	BdPtr is the pointer to the BD to get the physical address
 *
 * @return	The physical address of the BD
 *
 * @note	None
 *
 *****************************************************************************/
LONG XAxiCdma_BdGetPhysAddr(XAxiCdma_Bd* BdPtr)
{
	 u32 addrlen;
        addrlen = XAxiCdma_BdGetAddrLength(BdPtr);

	 if (addrlen > 32)
             return (u64)((XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_PHYS_ADDR_OFFSET)) |
                       ((uint64_t)(XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_PHYS_ADDR_MSB_OFFSET))
                                << 32U));
        else
             return (u32)(XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_PHYS_ADDR_OFFSET));
}

/*****************************************************************************/
/**
 * This function dumps the BD, it is a debug utility
 *
 * @param	BdPtr is the pointer to the BD to get the physical address
 *
 * @return	None
 *
 * @note	None
 *
 *****************************************************************************/
void XAxiCdma_DumpBd(XAxiCdma_Bd* BdPtr)
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
