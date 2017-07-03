/******************************************************************************
* Copyright (C) 2005 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xllfifo.c
* @addtogroup llfifo_v5_4
* @{
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
 * </pre>
 ******************************************************************************/


/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xllfifo.h"
#include "xil_assert.h"

/************************** Constant Definitions *****************************/
#define FIFO_WIDTH_BYTES 4

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
 * The streamer driver will eventually make calls back into the routines (which
 * reside in this driver) given at initialization to perform the actual I/O.
 *
 * Interrupts
 * Interrupts are handled in the OS/Application layer above this driver.
 ******************************************************************************/

xdbg_stmnt(u32 _xllfifo_rr_value;)
xdbg_stmnt(u32 _xllfifo_ipie_value;)
xdbg_stmnt(u32 _xllfifo_ipis_value;)

/*****************************************************************************/
/**
*
* XLlFifo_iRxOccupancy returns the number of 32-bit words available (occupancy)
* to be read from the receive channel of the FIFO, specified by
* <i>InstancePtr</i>.
*
* @param    InstancePtr references the FIFO on which to operate.
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
*
* XLlFifo_iRxGetLen notifies the hardware that the program is ready to receive the
* next frame from the receive channel of the FIFO specified by <i>InstancePtr</i>.
*
* Note that the program must first call XLlFifo_iRxGetLen before pulling data
* out of the receive channel of the FIFO with XLlFifo_Read.
*
* @param    InstancePtr references the FIFO on which to operate.
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
*
* XLlFifo_iRead_Aligned reads, <i>WordCount</i>, words from the FIFO referenced by
* <i>InstancePtr</i> to the block of memory, referenced by <i>BufPtr</i>.
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
* @param    InstancePtr references the FIFO on which to operate.
*
* @param    BufPtr specifies the memory address to place the data read.
*
* @param    WordCount specifies the number of 32 bit words to read.
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
	Xil_AssertNonvoid(((unsigned)BufPtr & 0x3) == 0x0);
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
*
* XLlFifo_iTxVacancy returns the number of unused 32 bit words available
* (vacancy) in the send channel of the FIFO, specified by <i>InstancePtr</i>.
*
* @param    InstancePtr references the FIFO on which to operate.
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
*
* XLlFifo_iTxSetLen begins a hardware transfer of data out of the transmit
* channel of the FIFO, specified by <i>InstancePtr</i>. <i>Bytes</i> specifies the number
* of bytes in the frame to transmit.
*
* Note that <i>Bytes</i> (rounded up to the nearest whole 32 bit word) must be same
* number of words just written using one or more calls to
* XLlFifo_iWrite_Aligned()
*
* @param    InstancePtr references the FIFO on which to operate.
*
* @param    Bytes specifies the number of bytes to transmit.
*
* @return   N/A
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
*
* XLlFifo_iWrite_Aligned writes, <i>WordCount</i>, words to the FIFO referenced by
* <i>InstancePtr</i> from the block of memory, referenced by <i>BufPtr</i>.
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
* @param    InstancePtr references the FIFO on which to operate.
*
* @param    BufPtr specifies the memory address to place the data read.
*
* @param    WordCount specifies the number of 32 bit words to read.
*
* @return   XLlFifo_iWrite_Aligned always returns XST_SUCCESS. Error handling is
*           otherwise handled through hardware exceptions and interrupts.
*
* @note
*
* C Signature: int XLlFifo_iWrite_Aligned(XLlFifo *InstancePtr,
*                      void *BufPtr, unsigned WordCount);
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
	Xil_AssertNonvoid(((unsigned)BufPtr & 0x3) == 0x0);

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
*
* XLlFifo_CfgInitialize initializes an XPS_ll_Fifo device along with the
* <i>InstancePtr</i> that references it.
*
* @param       InstancePtr is a pointer to the Axi Streaming FIFO instance
*              to be worked on.
* @param	Config references the structure holding the hardware
*		configuration for the Axi Streaming FIFO core to initialize.
* @param 	EffectiveAddress is the device base address in the virtual memory
*		address	space. The caller is responsible for keeping the
*		address mapping from EffectiveAddr to the device physical base
*		address unchanged once this function is invoked. Unexpected
*		errors may occur if the address mapping changes after this
*		function is called. If address translation is not used,
*		use Config->BaseAddress for this parameters, passing the
*		physical address instead.
*
* @return   N/A
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
*
* XLlFifo_RxGetWord reads one 32 bit word from the FIFO specified by
* <i>InstancePtr</i>.
*
* XLlFifo_RxGetLen or XLlFifo_iRxGetLen must be called before calling
* XLlFifo_RxGetWord. Otherwise, the hardware will raise an <i>Over Read
* Exception</i>.
*
* @param    InstancePtr references the FIFO on which to operate.
*
* @return   XLlFifo_RxGetWord returns the 32 bit word read from the FIFO.
*
* @note
* C-style signature:
*    u32 XLlFifo_RxGetWord(XLlFifo *InstancePtr)
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
*
* XLlFifo_TxPutWord writes the 32 bit word, <i>Word</i> to the FIFO specified by
* <i>InstancePtr</i>.
*
* @param    InstancePtr references the FIFO on which to operate.
* @param    Word is the data word to be written to FIFO.
*
* @return   N/A
*
* @note
* C-style signature:
*    void XLlFifo_TxPutWord(XLlFifo *InstancePtr, u32 Word)
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
*
* XLlFifo_Initialize initializes an XPS_ll_Fifo device along with the
* <i>InstancePtr</i> that references it.
*
* @param    InstancePtr references the memory instance to be associated with
*           the FIFO device upon initialization.
*
* @param    BaseAddress is the processor address used to access the
*           base address of the Fifo device.
*
* @return   N/A
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
