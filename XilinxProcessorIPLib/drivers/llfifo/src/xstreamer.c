/******************************************************************************
* Copyright (C) 2005 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xstreamer.c
* @brief Implements XLlFifo byte-streamer functions.
*
* See xstreamer.h for a description on how to use this driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a jvb  10/13/06 First release - based on Robert McGee's streaming packet
*                     fifo driver.
* 1.02a jz   12/04/09  Hal phase 1 support
* 2.00a hbm  01/20/10  Hal phase 1 support, bump up major release
* 2.02a asa  12/28/11  The function XStrm_Read is changed to reset HeadIndex
*		       to zero when all the bytes are read.
* 5.5   sk   06/15/20  In XStrm_Read and XStrm_Write add type casting to fix
*		       gcc warnings.
* 6.2   vn   08/26/26  Fix Doxygen warnings.
* 6.2   tb  09/15/26 Align Doxygen comments with AMD standards.
* </pre>
******************************************************************************/

/**
* @addtogroup llfifo_api LLFIFO APIs
* @{
*/

/***************************** Include Files *********************************/

#include "xstreamer.h"
#include "xil_assert.h"

/*
 * Implementation Notes
 *
 * --- Receive ---
 *
 * The basic algorithm for receiving bytes through this byte streamer copies a
 * fifo key-hole width chunk from the fifo into a holding buffer and then doles
 * out the bytes from the holding buffer. In some cases, when the buffer given
 * happens to already be aligned in memory, this algorithm bypasses the
 * holding buffer.
 *
 * Here is a picture to depict this process:
 *
 * Initial state:                 holding buffer
 *                                +--------------+
 *                                |   <empty>    |
 *                                +--------------+
 *                                               ^
 *                                               |
 *                                             index
 *
 * during XStrm_Read():
 * first holding buffer fill:     holding buffer
 *                                +--------------+
 *                                |////<full>////|
 *                                +--------------+
 *                                ^
 *                                |
 *                              index
 *
 * first holding buffer read:     holding buffer
 *                                 read   unread
 *                                +--------------+
 *                                |      |///////|
 *                                +--------------+
 *                                       ^
 *                                       |
 *                                     index
 *
 * ...
 *
 * last holding buffer read:     holding buffer
 *                                +--------------+
 *                                |   <empty>    |
 *                                +--------------+
 *                                               ^
 *                                               |
 *                                             index
 *
 * repeat this process ^^^
 *
 * --- Transmit ---
 *
 * The basic algorithm for transmitting bytes through this byte streamer copies
 * bytes into a holding buffer and then writes the holding buffer into the fifo
 * when it is full.  In some cases, when the buffer given happens to already be
 * aligned in memory, this algorithm bypasses the holding buffer.
 *
 * Here is a picture to depict this process:
 *
 * Initial state:                 holding buffer
 *                                +--------------+
 *                                |   <empty>    |
 *                                +--------------+
 *                                ^
 *                                |
 *                              index
 *
 * during XStrm_Write():
 * first holding buffer write:    holding buffer
 *                                 written  empty
 *                                +--------------+
 *                                |//////|       |
 *                                +--------------+
 *                                       ^
 *                                       |
 *                                     index
 *
 * ...
 * last holding buffer write:     holding buffer
 *                                +--------------+
 *                                |////<full>////|
 *                                +--------------+
 *                                               ^
 *                                               |
 *                                             index
 *
 * holding buffer flush:          holding buffer
 *                                +--------------+
 *                                |   <empty>    |
 *                                +--------------+
 *                                ^
 *                                |
 *                              index
 *
 * repeat this process ^^^
 */

#ifndef min
/**
* @brief Returns the smaller of two values.
*/
#define min(x, y) (((x) < (y)) ? (x) : (y))
#endif

/** @cond INTERNAL */
xdbg_stmnt(u32 _xstrm_ro_value;)
xdbg_stmnt(u32 _xstrm_buffered;)
/** @endcond */

/*****************************************************************************/
/**
* @brief Initializes a receive byte-streamer instance.
*
* XStrm_RxInitialize initializes the XStrm_RxFifoStreamer object referenced by
* <i>InstancePtr</i>.
*
* @param[out]   InstancePtr References the receive streamer to initialize.
* @param[in]    FifoWidth Specifies the FIFO keyhole size in bytes.
* @param[in]    FifoInstance FIFO driver instance this streamer uses
*   to transfer data into the actual FIFO.
* @param[in]    ReadFn Reads data from the FIFO. The routine must handle only
*               aligned buffers.
* @param[in]    GetLenFn Starts a receive on the actual FIFO.
* @param[in]    GetOccupancyFn Returns occupancy of the actual FIFO. The true
*               occupancy comes through this streamer because it holds some
*               of the bytes.
*
******************************************************************************/
void XStrm_RxInitialize(XStrm_RxFifoStreamer *InstancePtr, unsigned FifoWidth,
			void *FifoInstance, XStrm_XferFnType ReadFn,
			XStrm_GetLenFnType GetLenFn,
			XStrm_GetOccupancyFnType GetOccupancyFn)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	InstancePtr->HeadIndex = FifoWidth;
	InstancePtr->FifoWidth = FifoWidth;
	InstancePtr->FifoInstance = FifoInstance;
	InstancePtr->ReadFn = ReadFn;
	InstancePtr->GetLenFn = GetLenFn;
	InstancePtr->GetOccupancyFn = GetOccupancyFn;
}

/*****************************************************************************/
/**
* @brief Initializes a transmit byte-streamer instance.
*
* XStrm_TxInitialize initializes the XStrm_TxFifoStreamer object referenced by
* <i>InstancePtr</i>.
*
* @param[out]   InstancePtr References the transmit streamer to initialize.
* @param[in]    FifoWidth Specifies the FIFO keyhole size in bytes.
* @param[in]    FifoInstance FIFO driver instance this streamer uses
*   to transfer data into the actual FIFO.
* @param[in]    WriteFn Writes data into the FIFO. The routine must handle only
*               aligned buffers.
* @param[in]    SetLenFn Starts a transmit on the actual FIFO.
* @param[in]    GetVacancyFn Returns vacancy of the actual FIFO. The true
*               vacancy comes through this streamer because it holds some
*               of the bytes.
*
******************************************************************************/
void XStrm_TxInitialize(XStrm_TxFifoStreamer *InstancePtr, unsigned FifoWidth,
			void *FifoInstance, XStrm_XferFnType WriteFn,
			XStrm_SetLenFnType SetLenFn,
			XStrm_GetVacancyFnType GetVacancyFn)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	InstancePtr->TailIndex = 0;
	InstancePtr->FifoWidth = FifoWidth;
	InstancePtr->FifoInstance = FifoInstance;
	InstancePtr->WriteFn = WriteFn;
	InstancePtr->SetLenFn = SetLenFn;
	InstancePtr->GetVacancyFn = GetVacancyFn;
}

/*****************************************************************************/
/**
* @brief Returns the next receive frame length in bytes.
*
* XStrm_RxGetLen notifies the hardware that the program is ready to receive the
* next frame from the receive channel of the FIFO, specified by
* <i>InstancePtr</i>.
*
* Note that the program must first call XStrm_RxGetLen before pulling data
* out of the receive channel of the FIFO with XStrm_Read.
*
* @param[in,out]    InstancePtr References the FIFO on which to operate.
*
* @return   XStrm_RxGetLen returns the number of bytes available in the next
*           frame.
*
******************************************************************************/
u32 XStrm_RxGetLen(XStrm_RxFifoStreamer *InstancePtr)
{
	u32 len;

	InstancePtr->HeadIndex = InstancePtr->FifoWidth;
	len = (*InstancePtr->GetLenFn)(InstancePtr->FifoInstance);
	InstancePtr->FrmByteCnt = len;
	return len;
}

/*****************************************************************************/
/**
* @brief Reads bytes from the receive streamer.
*
* XStrm_Read reads <i>Bytes</i> bytes from the FIFO specified by
* <i>InstancePtr</i> to the block of memory, referenced by <i>BufPtr</i>.
*
* Care must be taken to ensure that the number of bytes read with one or more
* calls to XStrm_Read() does not exceed the number of bytes available given
* from the last call to XStrm_RxGetLen().
*
* @param[in,out]    InstancePtr References the FIFO on which to operate.
* @param[out]    BufPtr Specifies the memory address to place the data read.
* @param[in]    Bytes Specifies the number of bytes to read.
*
******************************************************************************/
void XStrm_Read(XStrm_RxFifoStreamer *InstancePtr, void *BufPtr,
		   unsigned Bytes)
{
	u8 *DestPtr = (u8 *) BufPtr;
	unsigned BytesRemaining = Bytes;
	unsigned FifoWordsToXfer;
	unsigned PartialBytes;
	unsigned i;

	while (BytesRemaining) {
		xdbg_printf(XDBG_DEBUG_FIFO_RX, "XStrm_Read: BytesRemaining: %d\n", BytesRemaining);
		/* Case 1: There are bytes in the holding buffer
		 *
		 *   1) Read the bytes from the holding buffer to the target
		 *   buffer.
		 *   2) Loop back around and handle the rest of the transfer.
		 */
		if (InstancePtr->HeadIndex != InstancePtr->FifoWidth) {
			xdbg_printf(XDBG_DEBUG_FIFO_RX, "XStrm_Read: Case 1: InstancePtr->HeadIndex [%d] != InstancePtr->FifoWidth [%d]\n",
				    InstancePtr->HeadIndex, InstancePtr->FifoWidth);
			i = InstancePtr->HeadIndex;

			PartialBytes = min(BytesRemaining,
				InstancePtr->FifoWidth -
				InstancePtr->HeadIndex);
			InstancePtr->HeadIndex += PartialBytes;
			BytesRemaining -= PartialBytes;
			InstancePtr->FrmByteCnt -= PartialBytes;
			while (PartialBytes--) {
				*DestPtr = InstancePtr->AlignedBuffer.bytes[i];
				i++;
				DestPtr++;
			}
		}
		/* Case 2: There are no more bytes in the holding buffer and
		 *         the target buffer is 32 bit aligned and
		 *         the number of bytes remaining to transfer is greater
		 *         than or equal to the fifo width.
		 *
		 *   1) We can go fast by reading a long string of fifo words
		 *   right out
		 *      of the fifo into the target buffer.
		 *   2) Loop back around to transfer the last few bytes.
		 */
		else if ((((UINTPTR)DestPtr & 3) == 0) &&
			 (BytesRemaining >= InstancePtr->FifoWidth)) {
			xdbg_printf(XDBG_DEBUG_FIFO_RX, "XStrm_Read: Case 2: DestPtr: %p, BytesRemaining: %d, InstancePtr->FifoWidth: %d\n",
				    DestPtr, BytesRemaining, InstancePtr->FifoWidth);
			FifoWordsToXfer =
			    BytesRemaining / InstancePtr->FifoWidth;

			(*(InstancePtr->ReadFn)) (InstancePtr->FifoInstance,
						DestPtr, FifoWordsToXfer);
			DestPtr += FifoWordsToXfer * InstancePtr->FifoWidth;
			BytesRemaining -=
				FifoWordsToXfer * InstancePtr->FifoWidth;
			InstancePtr->FrmByteCnt -=
				FifoWordsToXfer * InstancePtr->FifoWidth;
		}
		/* Case 3: There are no more bytes in the holding buffer and
		 *         the number of bytes remaining to transfer is less
		 *         than
		 *         the fifo width or
		 *         things just don't line up.
		 *
		 *   1) Fill the holding buffer.
		 *   2) Loop back around and handle the rest of the transfer.
		 */
		else {
			xdbg_printf(XDBG_DEBUG_FIFO_RX, "XStrm_Read: Case 3\n");
			/*
			 * At the tail end, read one fifo word into the local
			 * holding
			 * buffer and loop back around to take care of the
			 * transfer.
			 */
			(*InstancePtr->ReadFn) (InstancePtr->FifoInstance,
						&(InstancePtr->AlignedBuffer.
						  bytes[0]), 1);
			InstancePtr->HeadIndex = 0;
		}
	}
	if ((InstancePtr->FrmByteCnt) == 0) {
		InstancePtr->HeadIndex = 0;
	}
}

/*****************************************************************************/
/**
* @brief Flushes the streamer and starts a transmit.
*
* XStrm_TxSetLen flushes to the FIFO, specified by <i>InstancePtr</i>, any
* bytes remaining in internal buffers and begins a hardware transfer of data
* out of the transmit channel of the FIFO. <i>Bytes</i> specifies the number
* of bytes in the frame to transmit.
*
* @param[in,out] InstancePtr References the FIFO Streamer on which to operate.
* @param[in]    Bytes Specifies the frame length in bytes.
*
******************************************************************************/
void XStrm_TxSetLen(XStrm_TxFifoStreamer *InstancePtr, u32 Bytes)
{
	/*
	 * First flush what's in the holding buffer
	 */
	if (InstancePtr->TailIndex != 0) {
		(*InstancePtr->WriteFn) (InstancePtr->FifoInstance,
					 &(InstancePtr->AlignedBuffer.bytes[0]),
					 1);
		InstancePtr->TailIndex = 0;
	}

	/*
	 * Kick off the hw write
	 */
	(*(InstancePtr)->SetLenFn) (InstancePtr->FifoInstance, Bytes);
}

/*****************************************************************************/
/**
* @brief Writes bytes to the transmit streamer.
*
* XStrm_Write writes <i>Bytes</i> bytes of the block of memory, referenced by
* <i>BufPtr</i>, to the transmit channel of the FIFO referenced by
* <i>InstancePtr</i>.
*
* Care must be taken to ensure that the number of bytes written with one or
* more calls to XStrm_Write() matches the number of bytes given in the next
* call to XStrm_TxSetLen().
*
* @param[in,out]    InstancePtr References the FIFO on which to operate.
* @param[in]    BufPtr Specifies the memory address of data to write.
* @param[in]    Bytes Specifies the number of bytes to write.
*
******************************************************************************/
void XStrm_Write(XStrm_TxFifoStreamer *InstancePtr, void *BufPtr,
		    unsigned Bytes)
{
	u8 *SrcPtr = (u8 *) BufPtr;
	unsigned BytesRemaining = Bytes;
	unsigned FifoWordsToXfer;
	unsigned PartialBytes;
	unsigned i;

	while (BytesRemaining) {
		xdbg_printf(XDBG_DEBUG_FIFO_TX,
			    "XStrm_Write: BytesRemaining: %d\n",
			    BytesRemaining);
		/* Case 1: The holding buffer is full
		 *
		 *   1) Write it to the fifo.
		 *   2) Fall through to transfer more bytes in this iteration.
		 */
		if (InstancePtr->TailIndex == InstancePtr->FifoWidth) {
			xdbg_printf(XDBG_DEBUG_FIFO_TX,
				    "XStrm_Write: (case 1) TailIndex: %d; FifoWidth: %d; WriteFn: %p\n",
				    InstancePtr->TailIndex, InstancePtr->FifoWidth,
				    InstancePtr->WriteFn);
			(*InstancePtr->WriteFn) (InstancePtr->FifoInstance,
						 &(InstancePtr->AlignedBuffer.
						   bytes[0]), 1);
			InstancePtr->TailIndex = 0;
		}
		/* Case 2: There are no bytes in the holding buffer and
		 *         the target buffer is 32 bit aligned and
		 *         the number of bytes remaining to transfer is greater
		 *         than or equal to the fifo width.
		 *
		 *   1) We can go fast by writing a long string of fifo words
		 *   right out
		 *      of the source buffer into the fifo.
		 *   2) Loop back around to transfer the last few bytes.
		 */
		if ((InstancePtr->TailIndex == 0) &&
		    (BytesRemaining >= InstancePtr->FifoWidth) &&
		    (((UINTPTR)SrcPtr & 3) == 0)) {
			FifoWordsToXfer =
				BytesRemaining / InstancePtr->FifoWidth;

			xdbg_printf(XDBG_DEBUG_FIFO_TX, "XStrm_Write: (case 2) TailIndex: %d; BytesRemaining: %d; FifoWidth: %d; SrcPtr: %p;\n InstancePtr: %p; WriteFn: %p ,\nFifoWordsToXfer: %d (BytesRemaining: %d)\n",
				    InstancePtr->TailIndex, BytesRemaining,
				    InstancePtr->FifoWidth, SrcPtr,
				    InstancePtr, InstancePtr->WriteFn,
				    FifoWordsToXfer,
				    BytesRemaining);

			(*InstancePtr->WriteFn) (InstancePtr->FifoInstance,
						 SrcPtr, FifoWordsToXfer);
			SrcPtr += FifoWordsToXfer * InstancePtr->FifoWidth;
			BytesRemaining -=
				FifoWordsToXfer * InstancePtr->FifoWidth;
			xdbg_printf(XDBG_DEBUG_FIFO_TX, "XStrm_Write: (end case 2) TailIndex: %d; BytesRemaining: %d; SrcPtr: %p\n",
				    InstancePtr->TailIndex, BytesRemaining, SrcPtr);
		}
		/* Case 3: The alignment of the "galaxies" didn't occur in
		 *         Case 2 above, so we must pump the bytes through the
		 *         holding buffer.
		 *
		 *   1) Write bytes from the source buffer to the holding buffer
		 *   2) Loop back around and handle the rest of the transfer.
		 */
		else {
			i = InstancePtr->TailIndex;

			PartialBytes =
			    min(BytesRemaining,
				InstancePtr->FifoWidth -
				InstancePtr->TailIndex);
			BytesRemaining -= PartialBytes;
			InstancePtr->TailIndex += PartialBytes;
			while (PartialBytes--) {
				xdbg_printf(XDBG_DEBUG_FIFO_TX, "XStrm_Write: (case 3) PartialBytes: %d\n",
					    PartialBytes);
				InstancePtr->AlignedBuffer.bytes[i] = *SrcPtr;
				i++;
				SrcPtr++;
			}
		}
	}
}

/** @} */
