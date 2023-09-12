/******************************************************************************
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
 *
 * @file xqspipsu_transfer.c
 * @addtogroup qspipsu Overview
 * @{
 *
 * The xqspipsu_transfer.c contains functions to receive and transfer data
 * in polled and interrupt mode.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.18   sb  08/29/2023 Restructured the code for more modularity
 * 1.18   sb  08/29/2023 Upadte XQspiPsu_PolledRecvData api to fix MISRA-C warnings.
 *
 * </pre>
 ******************************************************************************/

/***************************** Include Files *********************************/

#include "xqspipsu.h"
#include "xqspipsu_control.h"
#if defined (__aarch64__)
#include "xil_smc.h"
#endif
/************************** Constant Definitions *****************************/
#define MAX_DELAY_CNT	1000000000U	/**< Max delay count */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/

/*****************************************************************************/
/**
 *
 * This function transfers Tx data on the bus in polled mode.
 *
 * @param       InstancePtr is a pointer to the XQspiPsu instance.
 * @param       Msg is a pointer to the structure containing transfer data.
 * @param       Index is the Msg index to transfer.
 *
 * @return
 *		- XST_SUCCESS if successful.
 *		- XST_FAILURE if transfer fails.
 *
 * @note        None.
 *
******************************************************************************/
s32 XQspiPsu_PolledSendData(XQspiPsu *InstancePtr, XQspiPsu_Msg *Msg,
			    s32 Index)
{
	s32 Status;
	/* Check if TXFIFO is not empty */
	if (Xil_WaitForEvent((InstancePtr->Config.BaseAddress + XQSPIPSU_ISR_OFFSET),
			     XQSPIPSU_ISR_TXNOT_FULL_MASK,
			     XQSPIPSU_ISR_TXNOT_FULL_MASK,
			     MAX_DELAY_CNT) != (u32)XST_SUCCESS) {
		Status = (s32)XST_FAILURE;
		goto END;
	}
	if (InstancePtr->TxBytes > 0) {
		/* Transmit more data if left */
		XQspiPsu_FillTxFifo(InstancePtr, &Msg[Index], (u32)XQSPIPSU_TXD_DEPTH);
	}
	Status = (s32)XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * This function transfers Rx data on the bus in polled mode.
 *
 * @param       InstancePtr is a pointer to the XQspiPsu instance.
 * @param       Msg is a pointer to the structure containing transfer data.
 * @param       Index is the Msg index to transfer.
 * @param       IOPending is the  .
 *
 * @return
 *		- XST_SUCCESS if successful.
 *		- XST_FAILURE if transfer fails.
 *
 * @note        None.
 *
******************************************************************************/
s32 XQspiPsu_PolledRecvData(XQspiPsu *InstancePtr, XQspiPsu_Msg *Msg,
			    s32 Index, u32 *IOPending)
{
	s32 Status;
	u32 QspiPsuStatusReg;
	u32 DmaIntrSts;

	if (InstancePtr->ReadMode == XQSPIPSU_READMODE_DMA) {
		/* Check if DMA RX is complete */
		if (Xil_WaitForEvent((InstancePtr->Config.BaseAddress + XQSPIPSU_QSPIDMA_DST_I_STS_OFFSET),
				     XQSPIPSU_QSPIDMA_DST_I_STS_DONE_MASK,
				     XQSPIPSU_QSPIDMA_DST_I_STS_DONE_MASK,
				     MAX_DELAY_CNT) != (u32)XST_SUCCESS) {
			Status = (s32)XST_FAILURE;
			goto END;
		} else {
			/* DMA Intr write to clear */
			DmaIntrSts = XQspiPsu_ReadReg(InstancePtr->Config.BaseAddress,
						      XQSPIPSU_QSPIDMA_DST_I_STS_OFFSET);
			XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress,
					  XQSPIPSU_QSPIDMA_DST_I_STS_OFFSET, DmaIntrSts);
			/* DMA transfer done, Invalidate Data Cache */
			if (!((Msg[Index].RxAddr64bit >= XQSPIPSU_RXADDR_OVER_32BIT) ||
			      (Msg[Index].Xfer64bit != (u8)0U)) &&
			    (InstancePtr->Config.IsCacheCoherent == 0U)) {
				Xil_DCacheInvalidateRange((INTPTR)Msg[Index].RxBfrPtr,
							  (INTPTR)Msg[Index].ByteCount);
			}

			*IOPending = XQspiPsu_SetIOMode(InstancePtr, &Msg[Index]);
			InstancePtr->RxBytes = 0;
		}
	} else {
		QspiPsuStatusReg = XQspiPsu_ReadReg(InstancePtr->Config.BaseAddress, XQSPIPSU_ISR_OFFSET);
		XQspiPsu_IORead(InstancePtr, &Msg[Index], QspiPsuStatusReg);
	}
	Status = (s32)XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * This function performs a transfer of Tx data on the bus in interrupt mode.
 *
 * @param	InstancePtr is a pointer to the XQspiPsu instance.
 * @param	QspiPsuStatusReg is the status QSPI status register.
 * @param	DeltaMsgCnt is the message count flag.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void XQspiPsu_IntrSendData(XQspiPsu *InstancePtr,
			   u32 QspiPsuStatusReg, u8 *DeltaMsgCnt)
{
	s32 MsgCnt = InstancePtr->MsgCnt;
	XQspiPsu_Msg *Msg = InstancePtr->Msg;
	u32 TxRxFlag = Msg[MsgCnt].Flags;

	if ( ((QspiPsuStatusReg & XQSPIPSU_ISR_TXNOT_FULL_MASK) != (u32)FALSE) &&
	     (InstancePtr->TxBytes > 0)) {
		XQspiPsu_FillTxFifo(InstancePtr, &Msg[MsgCnt], (u32)XQSPIPSU_TXD_DEPTH);
	}
	/*
	 * Check if the entry is ONLY TX and increase MsgCnt.
	 * This is to allow TX and RX together in one entry - corner case.
	 */
	if (  ((QspiPsuStatusReg & XQSPIPSU_ISR_TXEMPTY_MASK) != (u32)FALSE) &&
	      ((QspiPsuStatusReg & XQSPIPSU_ISR_GENFIFOEMPTY_MASK) != (u32)FALSE) &&
	      (InstancePtr->TxBytes == 0) &&
	      ((TxRxFlag & XQSPIPSU_MSG_FLAG_RX) == (u32)FALSE)) {
		MsgCnt += 1;
		*DeltaMsgCnt = 1U;
	}
	InstancePtr->MsgCnt = MsgCnt;
}

/*****************************************************************************/
/**
 *
 * This function performs a transfer of Rx data on the busin interrupt mode.
 *
 * @param	InstancePtr is a pointer to the XQspiPsu instance.
 * @param	QspiPsuStatusReg is the status QSPI status register.
 * @param	DmaIntrStatusReg is the status DMA interrupt register.
 * @param	DeltaMsgCnt is the message count flag.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void XQspiPsu_IntrRecvData(XQspiPsu *InstancePtr,
			   u32 QspiPsuStatusReg, u32 DmaIntrStatusReg, u8 *DeltaMsgCnt)
{
	s32 MsgCnt = InstancePtr->MsgCnt;
	XQspiPsu_Msg *Msg = InstancePtr->Msg;

	if (InstancePtr->ReadMode == XQSPIPSU_READMODE_DMA) {
		if ((DmaIntrStatusReg &
		     XQSPIPSU_QSPIDMA_DST_I_STS_DONE_MASK) != (u32)FALSE) {
			/* DMA transfer done, Invalidate Data Cache */
			if (!((Msg[MsgCnt].RxAddr64bit >= XQSPIPSU_RXADDR_OVER_32BIT) ||
			      (Msg[MsgCnt].Xfer64bit != (u8)0U)) &&
			    (InstancePtr->Config.IsCacheCoherent == 0U)) {
				Xil_DCacheInvalidateRange((INTPTR)Msg[MsgCnt].RxBfrPtr, (INTPTR)Msg[MsgCnt].ByteCount);
			}
			if (XQspiPsu_SetIOMode(InstancePtr, &Msg[MsgCnt]) == (u32)TRUE) {
				XQspiPsu_GenFifoEntryData(InstancePtr, &Msg[MsgCnt]);
				XQspiPsu_ManualStartEnable(InstancePtr);
			} else {
				InstancePtr->RxBytes = 0;
				MsgCnt += 1;
				*DeltaMsgCnt = 1U;
			}
		}
	} else {
		if (InstancePtr->RxBytes != 0) {
			XQspiPsu_IORead(InstancePtr, &Msg[MsgCnt], QspiPsuStatusReg);
			if (InstancePtr->RxBytes == 0) {
				MsgCnt += 1;
				*DeltaMsgCnt = 1U;
			}
		}
	}
	InstancePtr->MsgCnt = MsgCnt;
}
/** @} */
