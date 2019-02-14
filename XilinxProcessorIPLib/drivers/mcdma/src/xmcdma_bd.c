/******************************************************************************
*
* Copyright (C) 2017 - 2019 Xilinx, Inc.  All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xmcdma_bd.c
* @addtogroup mcdma_v1_3
* @{
*
* This file implements all the Scatter/Gather handling for the MCDMA Core,
* please see xmcdma.h for more details.
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
*  1.0  adk  18/07/17 Initial Version.
*  1.2  mus  11/05/18 Support 64 bit DMA addresses for Microblaze-X platform.
*  1.3  rsp  02/11/19 Add top level submit XMcDma_Chan_Sideband_Submit() API
*                     to program BD control and sideband information.
******************************************************************************/

#include "xmcdma.h"
#include "xmcdma_hw.h"
#include "xmcdma_bd.h"


/******************************************************************************
 * Move the BdPtr argument ahead an arbitrary number of BDs wrapping around
 * to the beginning of the ring if needed.
 *
 * We know if a wraparound should occur if the new BdPtr is greater than
 * the high address in the ring OR if the new BdPtr crosses the 0xFFFFFFFF
 * to 0 boundary.
 *
 * @param	Chan is the mcdma chan BdPtr appears in
 * @param	BdPtr on input is the starting BD position and on output is the
 *		final BD position
 * @param	NumBd is the number of BD spaces to increment
 *
 * @returns	None
 *****************************************************************************/
#define XMCDMA_CHAN_SEEKAHEAD(Chan, BdPtr, NumBd)                       \
    {                                                                   \
        UINTPTR Addr = (UINTPTR)(void *)(BdPtr);                        \
                                                                        \
        Addr += (Chan->Separation * (NumBd));                           \
        if ((Addr > (Chan)->LastBdAddr) || ((UINTPTR)(BdPtr) > Addr))   \
        {                                                               \
            Addr -= (Chan)->Length;                              \
        }                                                               \
                                                                        \
        (BdPtr) = (XMcdma_Bd *)(void *)Addr;                            \
    }


/*****************************************************************************/
/**
* Update Current Descriptor for the MCDMA Channel
*
* @param	Chan is the Channel instance to be worked on
*
* @return
*		- XST_SUCCESS upon success
*
* @note		None.
*
*****************************************************************************/
int XMcdma_UpdateChanCDesc(XMcdma_ChanCtrl *Chan)
{
	UINTPTR BdPtr;
	u32 Offset;

	if (Chan->ChanState == XMCDMA_CHAN_BUSY) {
		return XST_SUCCESS;
	}

	Offset = (Chan->Chan_id - 1) * XMCDMA_NXTCHAN_OFFSET;
	BdPtr = (UINTPTR)(void *)Chan->BdHead;
	XMCDMA_CACHE_FLUSH((UINTPTR)(BdPtr));

	XMcdma_WriteReg(Chan->ChanBase,
				(XMCDMA_CDESC_OFFSET + Offset),
				(u32)BdPtr);
	if (Chan->ext_addr) {
		XMcdma_WriteReg(Chan->ChanBase,
				(XMCDMA_CDESC_MSB_OFFSET + Offset),
			UPPER_32_BITS(BdPtr));
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* Update Tail Desc for the Channel
*
* @param	Chan is the MCDMA Channel to be worked on.
*
* @return
*		- XST_SUCCESS upon success
*		- XST_DMA_ERROR if no valid BD available to put into current
*		BD register
*
*
*****************************************************************************/
int XMcdma_UpdateChanTDesc(XMcdma_ChanCtrl *Chan)
{
	u32 RegBase;
	u32 Offset;
	u32 Chan_id = Chan->Chan_id;
	u32 Status;

	RegBase = Chan->ChanBase;
	Offset = (Chan->Chan_id - 1) * XMCDMA_NXTCHAN_OFFSET;

	if (!XMcdma_HwIsStarted(Chan)) {
		Status = XMcdma_Start(Chan);
		if (Status != XST_SUCCESS) {
			xil_printf("Failed to start the DMA\n\r");
			return XST_DMA_ERROR;
		}
	}

	if (!XMcdma_ChanHwIsStarted(Chan, Chan_id)) {
		XMcdma_WriteReg(RegBase, XMCDMA_CR_OFFSET + Offset,
				XMcdma_ReadReg(RegBase,
					       (XMCDMA_CR_OFFSET + Offset)) |
					       XMCDMA_CCR_RUNSTOP_MASK);
	}

	/* Note as started */
	Chan->ChanState = XMCDMA_CHAN_BUSY;

	if (Chan->BdPendingCnt > 0) {
		XMCDMA_CACHE_INVALIDATE(Chan->BdTail);
		XMcdma_WriteReg(Chan->ChanBase,
				(XMCDMA_TDESC_OFFSET + Offset),
				LOWER_32_BITS((UINTPTR)Chan->BdTail));
		if (Chan->ext_addr) {
#if !defined (__MICROBLAZE__)
			dsb();
#endif
			XMcdma_WriteReg(Chan->ChanBase,
				  (XMCDMA_TDESC_MSB_OFFSET + Offset),
				  UPPER_32_BITS((UINTPTR)Chan->BdTail));
		}
		Chan->BdSubmitCnt += Chan->BdPendingCnt;
		Chan->BdPendingCnt = 0;

		return XST_SUCCESS;
	}

	return XST_DMA_ERROR;
}

/*****************************************************************************/
/**
* Using a memory segment allocated by the caller, This function creates and
* setup the BD Chain for the MCDMA Channel.
*
* @param	Chan is the MCDMA Channel to be worked on..
* @param	Addr is the address of the application memory region.
* @param	Count is the number of BDs to setup in the application memory
*		region.
*
* @return
*		- XST_SUCCESS if initialization was successful
*		- XST_INVALID_PARAM if Count is not positive.
*
*****************************************************************************/
u32 XMcDma_ChanBdCreate(XMcdma_ChanCtrl *Chan, UINTPTR Addr, u32 Count)
{
	UINTPTR BdStartAddr;
	UINTPTR NxtBdAddr;
	u32 i;

	if (Count <= 0)
		return XST_INVALID_PARAM;

	Chan->BdPendingCnt = 0;
	Chan->BdSubmitCnt = 0;
	Chan->BdCnt = 0;
	Chan->BdDoneCnt = 0;
	Chan->Separation = sizeof(XMcdma_Bd);

	memset((void *)Addr, 0, sizeof(XMcdma_Bd) * Count);

	BdStartAddr = Addr;
	NxtBdAddr = Addr + Chan->Separation;
	for (i = 1; i < Count; i++) {
		XMcdma_BdWrite(BdStartAddr, XMCDMA_BD_NDESC_OFFSET,
			       (u32)NxtBdAddr);
		XMcdma_BdWrite(BdStartAddr, XMCDMA_BD_NDESC_MSB_OFFSET,
			       UPPER_32_BITS(NxtBdAddr));
		if (Chan->IsRxChan) {
			XMcdma_BdWrite(BdStartAddr, XMCDMA_BD_HAS_DRE_OFFSET,
				       (((u32)(Chan->Has_Rxdre)) <<
					XMCDMA_BD_HAS_DRE_SHIFT) |
					    Chan->RxDataWidth);
		} else {
			XMcdma_BdWrite(BdStartAddr, XMCDMA_BD_HAS_DRE_OFFSET,
				       (((u32)(Chan->Has_Txdre)) <<
				       XMCDMA_BD_HAS_DRE_SHIFT) |
					   Chan->TxDataWidth);
		}
		XMcdma_BdWrite(BdStartAddr, XMCDMA_BD_HAS_CTRLSTS_OFFSET,
			       Chan->HasStsCntrlStrm);

		XMCDMA_CACHE_FLUSH(BdStartAddr);
		BdStartAddr += Chan->Separation;
		NxtBdAddr   += Chan->Separation;
	}

	/* Link the last Bd to the first Bd */
	XMCDMA_CACHE_FLUSH(BdStartAddr);
	XMcdma_BdWrite(BdStartAddr, XMCDMA_BD_NDESC_OFFSET, (u32)Addr);
	XMcdma_BdWrite(BdStartAddr, XMCDMA_BD_NDESC_MSB_OFFSET,
		       UPPER_32_BITS(Addr));
	if (Chan->IsRxChan) {
		XMcdma_BdWrite(BdStartAddr, XMCDMA_BD_HAS_DRE_OFFSET,
			       (((u32)(Chan->Has_Rxdre)) <<
				XMCDMA_BD_HAS_DRE_SHIFT) |
				    Chan->RxDataWidth);
	} else {
		XMcdma_BdWrite(BdStartAddr, XMCDMA_BD_HAS_DRE_OFFSET,
			       (((u32)(Chan->Has_Txdre)) <<
			       XMCDMA_BD_HAS_DRE_SHIFT) |
				   Chan->TxDataWidth);
	}
	XMcdma_BdWrite(BdStartAddr, XMCDMA_BD_HAS_CTRLSTS_OFFSET,
		       Chan->HasStsCntrlStrm);


	Chan->ChanState = XMCDMA_CHAN_IDLE;
	Chan->BdCnt = Count;
	Chan->FirstBdAddr = Addr;
	Chan->LastBdAddr = BdStartAddr;
	Chan->Length = Chan->LastBdAddr - Chan->FirstBdAddr +
			       Chan->Separation;
	Chan->BdHead = (XMcdma_Bd *) Addr;
	Chan->BdTail = (XMcdma_Bd *) Addr;
	Chan->BdRestart = (XMcdma_Bd *) Addr;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function Populates the BD Chain with the required buffer address and
* length fields.
*
* User can submit multiple buffers/buffer descriptors by calling this function
* multiple times
*
* @param	Chan is the MCDMA Channel to be worked on..
* @param	BufAddr is the buffer address to which data should send/recv.
* @param	len is the Amount of data user requested to send/recv.
*
* @return
*		- XST_SUCCESS if initialization was successful
*		- XST_FAILURE if the BdStartAddress or requested bdlen
*		  doesn't match for the driver requirements.
*
*****************************************************************************/
u32 XMcDma_ChanSubmit(XMcdma_ChanCtrl *Chan, UINTPTR BufAddr, u32 len)
{
	u32 BdCount = 1;
	XMcdma_Bd *BdCurPtr = Chan->BdRestart;
	u32 i;
	u32 Bdlen = len;

	/* Calculate the Number of BD's required for transferring user
	 * requested Data */
	if (len > Chan->MaxTransferLen) {
		BdCount = (len + (Chan->MaxTransferLen - 1)) / Chan->MaxTransferLen;
		Bdlen = Chan->MaxTransferLen;
	}

	if (BdCount > Chan->BdCnt) {
		xil_printf("for transferring len bytes required Bd's is %x\n\r", BdCount);
		xil_printf("User requested only this %x many Bd's\n\r", Chan->BdCnt);

		return XST_FAILURE;
	}

	for (i = 0; i < BdCount; i++) {
		XMcdma_BdClear(BdCurPtr);

		XMcdma_BdSetBufAddr(BdCurPtr, BufAddr);

		if (len < Chan->MaxTransferLen)
			Bdlen = len;

		XMcdma_BdWrite(BdCurPtr, XMCDMA_BD_CTRL_OFFSET, Bdlen);

		Chan->BdTail = BdCurPtr;
		XMCDMA_CACHE_FLUSH((UINTPTR)(BdCurPtr));
		BdCurPtr = (XMcdma_Bd *)XMcdma_BdChainNextBd(Chan, BdCurPtr);
		BufAddr += Bdlen;
		len -= Bdlen;
	}

	XMCDMA_CACHE_FLUSH((UINTPTR)(BdCurPtr));
	DATA_SYNC;
	Chan->BdRestart = BdCurPtr;
	Chan->BdPendingCnt += BdCount;
	Chan->BdCnt -= BdCount;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function populates the BD Chain with the required buffer address, length
* APP(user application), TUSER and TID fields.
*
* User can submit multiple buffers/buffer descriptors by calling this function
* multiple times
*
* @param	ChanPtr is the MCDMA Channel to be worked on..
* @param	BufAddr is the buffer address to which data should send/recv.
* @param	Len is the Amount of data user requested to send/recv.
* @param	AppPtr is the point to APP fields.
* @param	Tuser is the  value presented on the beat that has TLAST.
* @param	Tid is the value value presented on the beat that has TLAST.
*
* @return
*		- XST_SUCCESS if initialization was successful
*		- XST_FAILURE if the BdStartAddress or requested bdlen
*		  doesn't match for the driver requirements.
*
*****************************************************************************/
u32 XMcDma_Chan_Sideband_Submit(XMcdma_ChanCtrl *ChanPtr, UINTPTR BufAddr,
				u32 Len, u32 *AppPtr, u16 Tuser, u16 Tid)
{
	u32 BdCount = 1;
	XMcdma_Bd *BdCurPtr = ChanPtr->BdRestart;
	u32 i;
	u32 k;
	u32 Bdlen = Len;

	/* Calculate the Number of BD's required for transferring user
	 * requested Data */
	if (Len > ChanPtr->MaxTransferLen) {
		BdCount = (Len + (ChanPtr->MaxTransferLen - 1)) /
			   ChanPtr->MaxTransferLen;
		Bdlen = ChanPtr->MaxTransferLen;
	}

	if (BdCount > ChanPtr->BdCnt) {
		xil_printf("for transferring len bytes required Bd's is %x\n\r", BdCount);
		xil_printf("User requested only this %x many Bd's\n\r", ChanPtr->BdCnt);

		return XST_FAILURE;
	}

	for (i = 0; i < BdCount; i++) {
		XMcdma_BdClear(BdCurPtr);

		XMcdma_BdSetBufAddr(BdCurPtr, BufAddr);

		if (Len < ChanPtr->MaxTransferLen) {
			Bdlen = Len;
		}

		XMcdma_BdWrite(BdCurPtr, XMCDMA_BD_CTRL_OFFSET, Bdlen);

		/* Program BD sideband and app fields */
		XMcDma_BdSetCtrlSideBand(BdCurPtr, Tid, Tuser);

		if (!ChanPtr->IsRxChan || ChanPtr->HasRxLength) {
			for (k = 0; k <= XMCDMA_LAST_APPWORD; k++) {
				XMcdma_BdWrite(BdCurPtr,
					       XMCDMA_BD_USR0_OFFSET +
					       (k * 4), AppPtr[k]);
			}
		}

		ChanPtr->BdTail = BdCurPtr;
		XMCDMA_CACHE_FLUSH((UINTPTR)(BdCurPtr));
		BdCurPtr = (XMcdma_Bd *)XMcdma_BdChainNextBd(ChanPtr, BdCurPtr);
		BufAddr += Bdlen;
		Len -= Bdlen;
	}

	XMCDMA_CACHE_FLUSH((UINTPTR)(BdCurPtr));
	DATA_SYNC;
	ChanPtr->BdRestart = BdCurPtr;
	ChanPtr->BdPendingCnt += BdCount;
	ChanPtr->BdCnt -= BdCount;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function triggers/Starts the h/w by programming the Current and Tail
* Descriptors and enabling the particular channel.
*
* @param	Chan is the MCDMA Channel to be worked on..
*
* @return
*		- XST_SUCCESS if initialization was successful
*		- XST_FAILURE In case of failure
*
*****************************************************************************/
u32 XMcDma_ChanToHw(XMcdma_ChanCtrl *Chan)
{
	int Status = XST_SUCCESS;

	/* Update Current Descriptor */
	Status = XMcdma_UpdateChanCDesc(Chan);
	if (Status != XST_SUCCESS) {
		xil_printf("Update CUR DESC failed %x", Status);
		return Status;
	}

	/* Update Tail Descriptor */
	Status = XMcdma_UpdateChanTDesc(Chan);
	if (Status != XST_SUCCESS) {
		xil_printf("Update TAIL DESC failed %x", Status);
		return Status;
	}

	/* Enable the Channel */
	 XMcdma_EnableCh(Chan);

	return Status;
}

/*****************************************************************************/
/**
* Returns a set of BD(s) that have been processed by hardware. The returned
* BDs may be examined by the application to determine the outcome of the DMA
* transactions. Once the BDs have been examined, the application must call
* XMcdma_BdChainFree() in the same order which they were retrieved here.
*
* If hardware has partially completed a packet spanning multiple BDs, then
* none of the BDs for that packet will be included in the results.
*
* @param	Chan is the MCDMA Channel to be worked on..
* @param	BdLimit is the maximum number of BDs to return in the set.
* @param	BdSetPtr is an output parameter, it points to the first BD
*		available for examination.
*
* @return	The number of BDs processed by hardware. A value of 0 indicates
*		that no data is available. No more than BdLimit BDs will be
*		returned.
*
* @note	Treat BDs returned by this function as read-only.
*
*****************************************************************************/
int XMcdma_BdChainFromHW(XMcdma_ChanCtrl *Chan, u32 BdLimit, XMcdma_Bd **BdSetPtr)
{
	XMcdma_Bd *CurBdPtr;
	u32 BdCount;
	int BdPartialCount;
	volatile u32 BdSts;
	volatile u32 BdCr;

	CurBdPtr = Chan->BdHead;
	BdCount = 0;
	BdPartialCount = 0;
	BdSts = 0;
	BdCr = 0;

	if (Chan->BdSubmitCnt == 0) {
		*BdSetPtr = NULL;

		return XST_SUCCESS;
	}

	if (BdLimit > Chan->BdSubmitCnt) {
		BdLimit = Chan->BdSubmitCnt;
	}

	while (BdCount < BdLimit) {
		XMCDMA_CACHE_INVALIDATE((UINTPTR)(CurBdPtr));

		if(!(Chan->IsRxChan)) {
			BdSts = XMcdma_BdRead(CurBdPtr, XMCDMA_BD_SIDEBAND_STS_OFFSET);
			BdCr = XMcdma_BdRead(CurBdPtr, XMCDMA_BD_CTRL_OFFSET);
		} else {
			BdSts = XMcdma_BdRead(CurBdPtr, XMCDMA_BD_STS_OFFSET);
		}

		if (!(BdSts & XMCDMA_BD_STS_COMPLETE_MASK))
			break;

		BdCount++;

		/* Need to handle Tx case as well */
		if ((!(Chan->IsRxChan) && (BdCr & XMCDMA_BD_CTRL_EOF_MASK)) ||
		    (Chan->IsRxChan && (BdSts & XMCDMA_BD_STS_RXEOF_MASK))) {
			BdPartialCount = 0;
		} else {
			BdPartialCount++;
		}

		if (CurBdPtr == Chan->BdTail) {
			break;
		}

		CurBdPtr = (XMcdma_Bd *)(XMcdma_BdChainNextBd(Chan, CurBdPtr));
	}

	BdCount -= BdPartialCount;

	if (BdCount) {
		*BdSetPtr = Chan->BdHead;
		Chan->BdSubmitCnt -= BdCount;
		Chan->BdCnt += BdCount;
		Chan->BdDoneCnt += BdCount;
		XMCDMA_CHAN_SEEKAHEAD(Chan, Chan->BdHead, BdCount);

		return BdCount;
	} else {
		*BdSetPtr = (XMcdma_Bd *)NULL;
		return 0;
	}
}

/*****************************************************************************/
/**
 * Frees a set of BDs that had been previously retrieved with
 * XMcdma_BdChainFromHW().
 *
 * @param	Chan is the MCDMA Channel to be worked on..
 * @param	BdCount is the number of BDs to free.
 * @param	BdSetPtr is the head of a list of BDs returned by
 *		XMcdma_BdChainFromHW().
 *
 * @return
 *		- XST_SUCCESS if the set of BDs was freed.
 *		- XST_INVALID_PARAM if NumBd is negative
 *
 * @note	This function should not be preempted by another XAxiMcDma
 *		function call that modifies the BD space. It is the caller's
 *		responsibility to ensure mutual exclusion.
 *
 *****************************************************************************/
int XMcdma_BdChainFree(XMcdma_ChanCtrl *Chan, int BdCount, XMcdma_Bd *BdSetPtr)
{
	int i;

	if (BdCount < 0) {
		xil_printf("Invalid BD count\n\r");

		return XST_INVALID_PARAM;
	}

	if (BdCount == 0)
		return XST_SUCCESS;


	for (i = 0; i < BdCount; i++) {
		XMcdma_BdClear(BdSetPtr);
		BdSetPtr = (XMcdma_Bd *)(XMcdma_BdChainNextBd(Chan, BdSetPtr));
	}

	Chan->BdDoneCnt -= BdCount;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* Set the Buffer descriptor buffer address field.
*
* @param	BdPtr is the BD to operate on
* @param	Addr is the address to set
*
* @return
*		- XST_SUCCESS if buffer address set successfully
*		- XST_INVALID_PARAM if hardware has no DRE and address is not
*		aligned
*
*****************************************************************************/
u32 XMcdma_BdSetBufAddr(XMcdma_Bd *BdPtr, UINTPTR Addr)
{
	u32 HasDRE;
	u8 WordLen;

	HasDRE = XMcdma_BdRead(BdPtr, XMCDMA_BD_HAS_DRE_OFFSET);
	WordLen = HasDRE & XMCDMA_BD_WORDLEN_MASK;

	if (Addr & (WordLen - 1)) {
		if ((HasDRE & XMCDMA_BD_HAS_DRE_MASK) == 0) {
			xil_printf("Error set buf addr %x with %x and %x,"
			" %x\r\n",Addr, HasDRE, (WordLen - 1),
			Addr & (WordLen - 1));

			return XST_INVALID_PARAM;
		}
	}

#if defined(__aarch64__) || defined(__arch64__)
	XMcdma_BdWrite64(BdPtr, XMCDMA_BD_BUFA_OFFSET, Addr);
#else
	XMcdma_BdWrite(BdPtr, XMCDMA_BD_BUFA_OFFSET, Addr);
#endif

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * Set the control bits for a BD.
 *
 * @param       BdPtr is the BD to operate on.
 * @param       Data is the bit value to set
 *
 * @return      None
 *
 * @note        None
 *
 *****************************************************************************/
void XMcDma_BdSetCtrl(XMcdma_Bd *BdPtr, u32 Data)
{
        u32 RegValue = XMcdma_BdRead(BdPtr, XMCDMA_BD_CTRL_OFFSET);

        RegValue &= ~XMCDMA_BD_CTRL_ALL_MASK;

        RegValue |= (Data & XMCDMA_BD_CTRL_ALL_MASK);

        XMcdma_BdWrite((BdPtr), XMCDMA_BD_CTRL_OFFSET, RegValue);

        return;
}

/*****************************************************************************/
/**
 * Set the APP word at the specified APP word offset for a BD.
 *
 * @param	BdPtr is the BD to operate on
 * @param	Offset is the offset inside the APP word, it is valid from
 *		0 to 4
 * @param	Word is the value to set
 *
 * @returns
 *		- XST_SUCCESS for success
 *		- XST_INVALID_PARAM under following error conditions.
 *		1) StsCntrlStrm is not built in hardware
 *		2) Offset is not in valid range
 *
 *****************************************************************************/
int XMcDma_BdSetAppWord(XMcdma_Bd* BdPtr, int Offset, u32 Word)
{
	if (XMcdma_BdRead(BdPtr, XMCDMA_BD_HAS_CTRLSTS_OFFSET) == 0) {

		xil_printf("BdRingSetAppWord: no sts cntrl"
			   "stream in hardware build, cannot set app word\r\n");

		return XST_INVALID_PARAM;
	}

	if ((Offset < 0) || (Offset > XMCDMA_LAST_APPWORD)) {

		xil_printf("BdRingSetAppWord: invalid offset");

		return XST_INVALID_PARAM;
	}

	XMcdma_BdWrite(BdPtr, XMCDMA_BD_USR0_OFFSET + Offset * 4, Word);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * Get the APP word at the specified APP word offset for a BD.
 *
 * @param	BdPtr is the BD to operate on.
 * @param	Offset is the offset inside the APP word, it is valid from
 *		0 to 4
 * @param	Valid is to tell the caller whether parameters are valid
 *
 * @returns
 *		The APP word. Passed in parameter Valid holds 0 for failure,
 *		and 1 for success.
 *
 *****************************************************************************/
u32 XMcDma_BdGetAppWord(XMcdma_Bd* BdPtr, int Offset, int *Valid)
{
	*Valid = 0;

	if (XMcdma_BdRead(BdPtr, XMCDMA_BD_HAS_CTRLSTS_OFFSET) == 0) {

		xil_printf("BdRingGetAppWord: no sts cntrl"
			"stream in hardware build, no app word available\r\n");

		return (u32)0;
	}

	if((Offset < 0) || (Offset > XMCDMA_LAST_APPWORD)) {

		xil_printf("BdRingGetAppWord: invalid offset");

		return (u32)0;
	}

	*Valid = 1;

	return XMcdma_BdRead(BdPtr, XMCDMA_BD_USR0_OFFSET + Offset * 4);
}

/*****************************************************************************/
/**
* Set interrupt coalescing parameters for a particular channel.
*
* @param	Chan is the MCDMA Channel to be worked on..
* @param	IrqCoalesce is the Irq Threshold value valid ranges are 1 to 255
* @param	IrqDelay is the Irq Delay value valid ranges are 1 to 255
*
* @return
*		- XST_SUCCESS if Interrupt Coalescing settings updated
*		- XST_FAILURE if IrqCoalesce and IrqDelay parameters are out of range
*
*****************************************************************************/
u32 XMcdma_SetChanCoalesceDelay(XMcdma_ChanCtrl *Chan, u32 IrqCoalesce, u32 IrqDelay)
{
	u32 Cr;
	u32 Offset;

	Offset = (Chan->Chan_id - 1) * XMCDMA_NXTCHAN_OFFSET;

	Cr = XMcdma_ReadReg(Chan->ChanBase, XMCDMA_CR_OFFSET + Offset);

	if (IrqCoalesce == 0 || IrqCoalesce > 0xFF) {
		xil_printf("Invalid IrqCoalesce Value\n\r");
		return XST_FAILURE;
	}

	Cr = (Cr & ~XMCDMA_COALESCE_MASK) |
		(IrqCoalesce << XMCDMA_COALESCE_SHIFT);

	if (IrqDelay == 0 || IrqDelay > 0xFF) {
		xil_printf("Invalid IrqDelay Value\n\r");
		return XST_FAILURE;
	}

	Cr = (Cr & ~XMCDMA_DELAY_MASK) |
		(IrqDelay << XMCDMA_DELAY_SHIFT);

	XMcdma_WriteReg(Chan->ChanBase, XMCDMA_CR_OFFSET + Offset, Cr);

	return XST_SUCCESS;
}


/*****************************************************************************/
/**
* Dump the fields of a BD.
*
* @param	BdPtr is the BD to operate on.
*
* @return	None
*
*
*****************************************************************************/
void XMcDma_DumpBd(XMcdma_Bd* BdPtr)
{

	xil_printf("Dump BD %x:\r\n", (UINTPTR)BdPtr);
	xil_printf("\tNext Bd Ptr: %x\r\n",
	    (unsigned int)XMcdma_BdRead(BdPtr, XMCDMA_BD_NDESC_OFFSET));
	xil_printf("\tBuff addr: %x\r\n",
	    (unsigned int)XMcdma_BdRead(BdPtr, XMCDMA_BD_BUFA_OFFSET));
	xil_printf("\tContrl Contents: %x\r\n",
	    (unsigned int)XMcdma_BdRead(BdPtr, XMCDMA_BD_CTRL_OFFSET));
	xil_printf("\tStatus: %x\r\n",
	    (unsigned int)XMcdma_BdRead(BdPtr, XMCDMA_BD_STS_OFFSET));
	xil_printf("\tSideband Status: %x\r\n",
	    (unsigned int)XMcdma_BdRead(BdPtr, XMCDMA_BD_SIDEBAND_STS_OFFSET));

	xil_printf("\tAPP 0: %x\r\n",
	    (unsigned int)XMcdma_BdRead(BdPtr, XMCDMA_BD_USR0_OFFSET));
	xil_printf("\tAPP 1: %x\r\n",
	    (unsigned int)XMcdma_BdRead(BdPtr, XMCDMA_BD_USR1_OFFSET));
	xil_printf("\tAPP 2: %x\r\n",
	    (unsigned int)XMcdma_BdRead(BdPtr, XMCDMA_BD_USR2_OFFSET));
	xil_printf("\tAPP 3: %x\r\n",
	    (unsigned int)XMcdma_BdRead(BdPtr, XMCDMA_BD_USR3_OFFSET));
	xil_printf("\tAPP 4: %x\r\n",
	    (unsigned int)XMcdma_BdRead(BdPtr, XMCDMA_BD_USR4_OFFSET));

	xil_printf("\r\n");
}
