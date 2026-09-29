/******************************************************************************
* Copyright (C) 2005 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xllfifo.c
* @brief Implements XLlFifo interface functions.
*
* The Xilinx local link FIFO driver component. This driver supports the
* Xilinx xps_ll_fifo core.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00a jvb  10/13/06 First release
 * 1.00a xd   12/17/07 Added type casting to fix CR #456850
 * 1.02a jz   12/04/09  Hal phase 1 support
 * 2.00a hbm  01/20/10  Hal phase 1 support, bump up major release
 * 2.01a asa  09/17/10  Added code for resetting Streaming interface
 *		        for CR574868
 * 3.00a adk  08/10/13  Added support for AXI4 Datainterface.Updated
 * 		        XLlFifo_RxGetWord, XLlFifo_TxPutword inorder to
 * 		        handle AXI4 Datainterface. Added Config
 * 		        initialization for the driver.
 * 5.1   sk   11/10/15 Used UINTPTR instead of u32 for Baseaddress CR# 867425.
 *                     Changed the prototypes of XLlFifo_CfgInitialize,
 *                     XLlFifo_Initialize APIs.
 * 5.5   sk   06/15/20 In XLlFifo_iRead_Aligned and XLlFifo_iWrite_Aligned add
 *		       type casting to fix gcc warnings.
 * 5.6   sd   07/7/23  Add system devicetree support.
 * 6.2   vn   08/26/26 Fix Doxygen warnings.
 * 6.2   tb  09/15/26 Align Doxygen comments with AMD standards.
 * </pre>
 ******************************************************************************/

/**
 * @addtogroup llfifo_api LLFIFO APIs
 * @{
 */


/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xllfifo.h"
#include "xil_assert.h"

/************************** Constant Definitions *****************************/
#define FIFO_WIDTH_BYTES 4 /**< FIFO keyhole width in bytes. */

/*
 * Implementation Notes:
 *
 * This Fifo driver makes use of a byte streamer driver (xstreamer.h). The code
 * is structured like so:
 *
 * +--------------------+
 * |     llfifo        |
 * |   +----------------+
 * |   | +--------------+
 * |   | |  xstreamer   |
 * |   | +--------------+
 * |   +----------------+
 * |                    |
 * +--------------------+
 *
 * Initialization
 * At initialization time this driver (llfifo) sets up the streamer objects to
 * use routines in this driver (llfifo) to perform the actual I/O to the H/W
 * FIFO core.
 *
 * Operation
 * Once the streamer objects are set up, the API routines in this driver, just
 * call through to the streamer driver to perform the read/write operations.
 * The streamer driver makes calls back into the routines (which
 * reside in this driver) given at initialization to perform the actual I/O.
 *
 * Interrupts
 * Interrupts are handled in the OS/Application layer above this driver.
 ******************************************************************************/

/** @cond INTERNAL */
xdbg_stmnt(u32 _xllfifo_rr_value;)
xdbg_stmnt(u32 _xllfifo_ipie_value;)
xdbg_stmnt(u32 _xllfifo_ipis_value;)
/** @endcond */

/*****************************************************************************/
/**
* @brief Returns receive occupancy in 32-bit words.
*
* XLlFifo_iRxOccupancy returns the number of 32-bit words available (occupancy)
* to be read from the receive channel of the FIFO, specified by
* <i>InstancePtr</i>.
*
* @param[in]    InstancePtr References the FIFO on which to operate.
*
* @return   XLlFifo_iRxOccupancy returns the occupancy count in 32-bit words for
*           the specified FIFO.
*
******************************************************************************/
u32 XLlFifo_iRxOccupancy(XLlFifo *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr);

	return XLlFifo_ReadReg(InstancePtr->BaseAddress,
			XLLF_RDFO_OFFSET);
}

/*****************************************************************************/
/**
* @brief Returns the next receive frame length in bytes.
*
* XLlFifo_iRxGetLen notifies the hardware that the program is ready to
* receive the next frame from the receive channel of the FIFO specified by
* <i>InstancePtr</i>.
*
* Note that the program must first call XLlFifo_iRxGetLen before pulling data
* out of the receive channel of the FIFO with XLlFifo_Read.
*
* @param[in]    InstancePtr References the FIFO on which to operate.
*
* @return   XLlFifo_iRxGetLen returns the number of bytes available in the next
*           frame.
*
******************************************************************************/
u32 XLlFifo_iRxGetLen(XLlFifo *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr);

	return XLlFifo_ReadReg(InstancePtr->BaseAddress,
		XLLF_RLF_OFFSET);
}

/*****************************************************************************/
/**
* @brief Reads aligned 32-bit words from the receive FIFO.
*
* XLlFifo_iRead_Aligned reads <i>WordCount</i> words from the FIFO referenced
* by <i>InstancePtr</i> to the block of memory referenced by <i>BufPtr</i>.
*
* XLlFifo_iRead_Aligned assumes that <i>BufPtr</i> is already aligned according
* to the following hardware limitations:
*    ppc        - aligned on 32 bit boundaries to avoid performance penalties
*                 from unaligned exception handling.
*    microblaze - aligned on 32 bit boundaries as microblaze does not handle
*                 unaligned transfers.
*
* Care must be taken to ensure that the number of words read with one or more
* calls to XLlFifo_Read() does not exceed the number of bytes (rounded up to
* the nearest whole 32 bit word) available given from the last call to
* XLlFifo_RxGetLen().
*
* @param[in]    InstancePtr References the FIFO on which to operate.
* @param[out]    BufPtr Specifies the memory address to place the data read.
* @param[in]    WordCount Specifies the number of 32 bit words to read.
*
* @return   XLlFifo_iRead_Aligned always returns XST_SUCCESS. Error handling is
*           otherwise handled through hardware exceptions and interrupts.
*
******************************************************************************/
int XLlFifo_iRead_Aligned(XLlFifo *InstancePtr, void *BufPtr,
			     unsigned WordCount)
{
	unsigned WordsRemaining = WordCount;
	u32 *BufPtrIdx = (u32 *)BufPtr;

	xdbg_printf(XDBG_DEBUG_FIFO_RX, "XLlFifo_iRead_Aligned: start\n");
	Xil_AssertNonvoid(InstancePtr);
	Xil_AssertNonvoid(BufPtr);
	/* assert buffer is 32 bit aligned */
	Xil_AssertNonvoid(((UINTPTR)BufPtr & 0x3) == 0x0);
	xdbg_printf(XDBG_DEBUG_FIFO_RX, "XLlFifo_iRead_Aligned: after asserts\n");

	while (WordsRemaining) {
/*		xdbg_printf(XDBG_DEBUG_FIFO_RX,
			    "XLlFifo_iRead_Aligned: WordsRemaining: %d\n",
			    WordsRemaining);
*/
		*BufPtrIdx = XLlFifo_RxGetWord(InstancePtr);
		BufPtrIdx++;
		WordsRemaining--;
	}
	xdbg_printf(XDBG_DEBUG_FIFO_RX,
		    "XLlFifo_iRead_Aligned: returning SUCCESS\n");
	return XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief Returns transmit vacancy in 32-bit words.
*
* XLlFifo_iTxVacancy returns the number of unused 32 bit words available
* (vacancy) in the send channel of the FIFO, specified by <i>InstancePtr</i>.
*
* @param[in]    InstancePtr References the FIFO on which to operate.
*
* @return   XLlFifo_iTxVacancy returns the vacancy count in 32-bit words for
*           the specified FIFO.
*
*****************************************************************************/
u32 XLlFifo_iTxVacancy(XLlFifo *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr);

	return XLlFifo_ReadReg(InstancePtr->BaseAddress,
			XLLF_TDFV_OFFSET);
}

/*****************************************************************************/
/**
* @brief Starts a transmit of the given byte length.
*
* XLlFifo_iTxSetLen begins a hardware transfer of data out of the transmit
* channel of the FIFO specified by <i>InstancePtr</i>. <i>Bytes</i> specifies
* the number of bytes in the frame to transmit.
*
* Note that <i>Bytes</i> (rounded up to the nearest whole 32-bit word) must
* match the number of words just written using one or more calls to
* XLlFifo_iWrite_Aligned().
*
* @param[in]    InstancePtr References the FIFO on which to operate.
* @param[in]    Bytes Specifies the number of bytes to transmit.
*
******************************************************************************/
void XLlFifo_iTxSetLen(XLlFifo *InstancePtr, u32 Bytes)
{
	Xil_AssertVoid(InstancePtr);

	XLlFifo_WriteReg(InstancePtr->BaseAddress, XLLF_TLF_OFFSET,
			Bytes);
}

/*****************************************************************************/
/**
* @brief Writes aligned 32-bit words to the transmit FIFO.
*
* XLlFifo_iWrite_Aligned writes <i>WordCount</i> words to the FIFO referenced
* by <i>InstancePtr</i> from the block of memory referenced by <i>BufPtr</i>.
*
* XLlFifo_iWrite_Aligned assumes that <i>BufPtr</i> is already aligned according
* to the following hardware limitations:
*    ppc        - aligned on 32 bit boundaries to avoid performance penalties
*                 from unaligned exception handling.
*    microblaze - aligned on 32 bit boundaries as microblaze does not handle
*                 unaligned transfers.
*
* Care must be taken to ensure that the number of words written with one or
* more calls to XLlFifo_iWrite_Aligned() matches the number of bytes (rounded up
* to the nearest whole 32 bit word) given in the next call to
* XLlFifo_iTxSetLen().
*
* @param[in]    InstancePtr References the FIFO on which to operate.
* @param[in]    BufPtr Specifies the memory address of the data to write.
* @param[in]    WordCount Specifies the number of 32-bit words to write.
*
* @return   XLlFifo_iWrite_Aligned always returns XST_SUCCESS. Error handling is
*           otherwise handled through hardware exceptions and interrupts.
*
******************************************************************************/
int XLlFifo_iWrite_Aligned(XLlFifo *InstancePtr, void *BufPtr,
			      unsigned WordCount)
{
	unsigned WordsRemaining = WordCount;
	u32 *BufPtrIdx = (u32 *)BufPtr;

	xdbg_printf(XDBG_DEBUG_FIFO_TX,
		    "XLlFifo_iWrite_Aligned: Inst: %p; Buff: %p; Count: %d\n",
		    InstancePtr, BufPtr, WordCount);
	Xil_AssertNonvoid(InstancePtr);
	Xil_AssertNonvoid(BufPtr);
	/* assert buffer is 32 bit aligned */
	Xil_AssertNonvoid(((UINTPTR)BufPtr & 0x3) == 0x0);

	xdbg_printf(XDBG_DEBUG_FIFO_TX,
		    "XLlFifo_iWrite_Aligned: WordsRemaining: %d\n",
		    WordsRemaining);
	while (WordsRemaining) {
		XLlFifo_TxPutWord(InstancePtr, *BufPtrIdx);
		BufPtrIdx++;
		WordsRemaining--;
	}

	xdbg_printf(XDBG_DEBUG_FIFO_TX,
		    "XLlFifo_iWrite_Aligned: returning SUCCESS\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Initializes the XLlFifo driver from a config.
*
* XLlFifo_CfgInitialize initializes an XPS_ll_Fifo device along with the
* <i>InstancePtr</i> that references it.
*
* @param[out]  InstancePtr Pointer to the AXI Streaming FIFO instance
*   to be worked on.
* @param[in]  Config Hardware configuration for the core.
* @param[in]  EffectiveAddress Device base address in the virtual
*   memory address space. The caller must keep the mapping from
*   EffectiveAddr to the physical base address unchanged after this
*   call. If address translation is not used, pass
*   Config->BaseAddress as the physical address.
*
* @return
*              - XST_SUCCESS if initialization succeeds.
*              - XST_INVALID_PARAM if Config is NULL.
*
******************************************************************************/
int XLlFifo_CfgInitialize(XLlFifo *InstancePtr,
			XLlFifo_Config *Config, UINTPTR EffectiveAddress)
{
	Xil_AssertNonvoid(InstancePtr);

	if(!Config) {
		return XST_INVALID_PARAM;
	}


	/* Clear instance memory */
	memset(InstancePtr, 0, sizeof(XLlFifo));

	/*
	 * We don't care about the physical base address, just copy the
	 * processor address over it.
	 */
	InstancePtr->BaseAddress = EffectiveAddress;

	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	InstancePtr->Datainterface = Config->Datainterface;
	InstancePtr->Axi4BaseAddress = Config->Axi4BaseAddress;
	if (InstancePtr->Datainterface == 0)
		InstancePtr->Axi4BaseAddress = EffectiveAddress;

#ifdef SDT
	InstancePtr->IntId = Config->IntId;
	InstancePtr->IntrParent = Config->IntrParent;
#endif

	XLlFifo_TxReset(InstancePtr);
	XLlFifo_RxReset(InstancePtr);

	/*
	 * Reset the core and generate the external reset by writing to
	 * the Local Link/AXI Streaming Reset Register.
	 */
	XLlFifo_WriteReg((InstancePtr)->BaseAddress, XLLF_LLR_OFFSET,
				XLLF_RDFR_RESET_MASK);

	XStrm_RxInitialize(&(InstancePtr->RxStreamer), FIFO_WIDTH_BYTES,
			(void *)InstancePtr,
			(XStrm_XferFnType)XLlFifo_iRead_Aligned,
			(XStrm_GetLenFnType)XLlFifo_iRxGetLen,
			(XStrm_GetOccupancyFnType)XLlFifo_iRxOccupancy);

	XStrm_TxInitialize(&(InstancePtr->TxStreamer), FIFO_WIDTH_BYTES,
			(void *)InstancePtr,
			(XStrm_XferFnType)XLlFifo_iWrite_Aligned,
			(XStrm_SetLenFnType)XLlFifo_iTxSetLen,
			(XStrm_GetVacancyFnType)XLlFifo_iTxVacancy);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief Reads one 32-bit word from the receive FIFO.
*
* XLlFifo_RxGetWord reads one 32 bit word from the FIFO specified by
* <i>InstancePtr</i>.
*
* XLlFifo_RxGetLen or XLlFifo_iRxGetLen must be called before calling
* XLlFifo_RxGetWord. Otherwise, the hardware raises an <i>Over Read
* Exception</i>.
*
* @param[in]    InstancePtr References the FIFO on which to operate.
*
* @return   XLlFifo_RxGetWord returns the 32 bit word read from the FIFO.
*
*****************************************************************************/
u32 XLlFifo_RxGetWord(XLlFifo *InstancePtr)
{
	if (InstancePtr->Datainterface)
		return XLlFifo_ReadReg((InstancePtr)->Axi4BaseAddress,
				XLLF_AXI4_RDFD_OFFSET);
	else
		return XLlFifo_ReadReg((InstancePtr)->Axi4BaseAddress,
				XLLF_RDFD_OFFSET);
}

/****************************************************************************/
/**
* @brief Writes one 32-bit word to the transmit FIFO.
*
* XLlFifo_TxPutWord writes the 32 bit word, <i>Word</i> to the FIFO specified by
* <i>InstancePtr</i>.
*
* @param[in]    InstancePtr References the FIFO on which to operate.
* @param[in]    Word Data word to be written to FIFO.
*
*****************************************************************************/
void XLlFifo_TxPutWord(XLlFifo *InstancePtr, u32 Word)
{
	if (InstancePtr->Datainterface)
		XLlFifo_WriteReg((InstancePtr)->Axi4BaseAddress,
					XLLF_AXI4_TDFD_OFFSET, (Word));
	else
		XLlFifo_WriteReg((InstancePtr)->Axi4BaseAddress,
					XLLF_TDFD_OFFSET, (Word));

}

/*****************************************************************************/
/**
* @brief Initializes the XLlFifo driver from a base address.
*
* XLlFifo_Initialize initializes an XPS_ll_Fifo device along with the
* <i>InstancePtr</i> that references it.
*
* @param[out]   InstancePtr References the memory instance to be associated with
*           the FIFO device upon initialization.
* @param[in]    BaseAddress Processor address used to access the
*   base address of the Fifo device.
*
******************************************************************************/
void XLlFifo_Initialize(XLlFifo *InstancePtr, UINTPTR BaseAddress)
{
	Xil_AssertVoid(InstancePtr);
	Xil_AssertVoid(BaseAddress);

	/* Clear instance memory */
	memset(InstancePtr, 0, sizeof(XLlFifo));

	/*
	 * We don't care about the physical base address, just copy the
	 * processor address over it.
	 */
	InstancePtr->BaseAddress = BaseAddress;

	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;
	InstancePtr->Axi4BaseAddress = BaseAddress;

	XLlFifo_TxReset(InstancePtr);
	XLlFifo_RxReset(InstancePtr);

	/*
	 * Reset the core and generate the external reset by writing to
	 * the Local Link/AXI Streaming Reset Register.
	 */
	XLlFifo_WriteReg((InstancePtr)->BaseAddress, XLLF_LLR_OFFSET,
				XLLF_RDFR_RESET_MASK);

	XStrm_RxInitialize(&(InstancePtr->RxStreamer), FIFO_WIDTH_BYTES,
			(void *)InstancePtr,
			(XStrm_XferFnType)XLlFifo_iRead_Aligned,
			(XStrm_GetLenFnType)XLlFifo_iRxGetLen,
			(XStrm_GetOccupancyFnType)XLlFifo_iRxOccupancy);

	XStrm_TxInitialize(&(InstancePtr->TxStreamer), FIFO_WIDTH_BYTES,
			(void *)InstancePtr,
			(XStrm_XferFnType)XLlFifo_iWrite_Aligned,
			(XStrm_SetLenFnType)XLlFifo_iTxSetLen,
			(XStrm_GetVacancyFnType)XLlFifo_iTxVacancy);
}

/** @} */
