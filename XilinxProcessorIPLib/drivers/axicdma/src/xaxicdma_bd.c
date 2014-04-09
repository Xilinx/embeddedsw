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
 *  @file xaxicdma_bd.c
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
	memset((void *)((u32)BdPtr + XAXICDMA_BD_START_CLEAR), 0,
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

	memcpy((void *)((u32)BdPtr + XAXICDMA_BD_START_CLEAR),
	    (void *)((u32)TmpBd + XAXICDMA_BD_START_CLEAR),
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
u32 XAxiCdma_BdGetNextPtr(XAxiCdma_Bd* BdPtr)
{

	return (u32)(XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_NDESC_OFFSET));
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
void XAxiCdma_BdSetNextPtr(XAxiCdma_Bd* BdPtr, u32 NextBdPtr)
{

	XAxiCdma_BdWrite(BdPtr, XAXICDMA_BD_NDESC_OFFSET, NextBdPtr);

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
int XAxiCdma_BdSetSrcBufAddr(XAxiCdma_Bd* BdPtr, u32 Addr)
{

	if (XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_HASDRE_OFFSET) == 0) {

		if (Addr &
		    (XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_WORDLEN_OFFSET) - 1)) {

			xdbg_printf(XDBG_DEBUG_ERROR,
			    "Unaligned transfers not supported, address %x "
			    "not aligned\r\n", (unsigned int)Addr);

			return XST_INVALID_PARAM;
		}
	}

	XAxiCdma_BdWrite(BdPtr, XAXICDMA_BD_BUFSRC_OFFSET, Addr);

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
u32 XAxiCdma_BdGetSrcBufAddr(XAxiCdma_Bd* BdPtr)
{

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
int XAxiCdma_BdSetDstBufAddr(XAxiCdma_Bd* BdPtr, u32 Addr)
{
	if (XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_HASDRE_OFFSET) == 0) {

		if (Addr &
		    (XAxiCdma_BdRead(BdPtr, XAXICDMA_BD_WORDLEN_OFFSET) - 1)) {

			xdbg_printf(XDBG_DEBUG_ERROR,
			    "Unaligned transfers not supported, address "
			    "not aligned %x\r\n", (unsigned int)Addr);
			return XST_INVALID_PARAM;
		}
	}

	XAxiCdma_BdWrite(BdPtr, XAXICDMA_BD_BUFDST_OFFSET, Addr);

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
u32 XAxiCdma_BdGetDstBufAddr(XAxiCdma_Bd* BdPtr)
{

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
int XAxiCdma_BdSetLength(XAxiCdma_Bd* BdPtr, int LenBytes)
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
int XAxiCdma_BdGetLength(XAxiCdma_Bd* BdPtr)
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
void XAxiCdma_BdSetPhysAddr(XAxiCdma_Bd* BdPtr, u32 PhysAddr)
{

	XAxiCdma_BdWrite(BdPtr, XAXICDMA_BD_PHYS_ADDR_OFFSET, PhysAddr);

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
u32 XAxiCdma_BdGetPhysAddr(XAxiCdma_Bd* BdPtr)
{

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
	xil_printf("\r\nDump BD %x:\r\n", (unsigned int)BdPtr);

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
