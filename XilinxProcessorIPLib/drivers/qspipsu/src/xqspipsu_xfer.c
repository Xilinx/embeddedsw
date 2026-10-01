/******************************************************************************
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
 * @file xqspipsu_xfer.c
 * @brief Implements the polled and interrupt mode data transfer functions.
 *
 * The xqspipsu_xfer.c file contains functions to receive and transfer data
 * in polled and interrupt mode.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.18   sb  08/29/2023 Restructured the code for more modularity
 * 1.18   sb  08/29/2023 Update XQspiPsu_PolledRecvData api to fix MISRA-C warnings.
 * 1.22   sb  10/24/2025 Added cache invalidate when EL1_NONSECURE is not defined.
 *        zm  06/18/2026 Queued whole transfer into GENFIFO to fix bus conflict.
 *        zm  06/25/2026 Skip IO read when RxBytes is zero.
 * 1.24   vn  08/26/26   Fix Doxygen warnings.
 * 1.24   tb  09/30/26   Align Doxygen comments with AMD standards
 *
 * </pre>
 ******************************************************************************/

/** @addtogroup qspipsu_api QSPIPSU APIs
 * @{
 */

/***************************** Include Files *********************************/

#include "xqspipsu.h"
#include "xqspipsu_control.h"
#if defined (__aarch64__)
#include "xil_smc.h"
#endif
/************************** Constant Definitions *****************************/
#define MAX_DELAY_CNT	1000000000U	/**< Maximum delay count. */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/

/*****************************************************************************/
/**
 *
 * @brief Transfers Tx data on the bus in polled mode.
 *
 * @param[in,out] InstancePtr Pointer to the XQspiPsu instance.
 * @param[in,out] Msg Pointer to the structure containing transfer data.
 * @param[in]     Index Msg index to transfer.
 *
 * @return
 *		  - XST_SUCCESS if successful.
 *		  - XST_FAILURE if transfer fails.
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
 * @brief Transfers Rx data on the bus in polled mode.
 *
 * @param[in,out] InstancePtr Pointer to the XQspiPsu instance.
 * @param[in,out] Msg Pointer to the structure containing transfer data.
 * @param[in]     Index Msg index to transfer.
 * @param[out]    IOPending I/O mode transfer status.
 *
 * @return
 *		  - XST_SUCCESS if successful.
 *		  - XST_FAILURE if transfer fails.
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
#if defined(EL1_NONSECURE) && (EL1_NONSECURE==1U)
			if (!((Msg[Index].RxAddr64bit >= XQSPIPSU_RXADDR_OVER_32BIT) ||
			      (Msg[Index].Xfer64bit != (u8)0U)) &&
			    (InstancePtr->Config.IsCacheCoherent == 0U)) {
				Xil_DCacheInvalidateRange((INTPTR)Msg[Index].RxBfrPtr,
							  (INTPTR)Msg[Index].ByteCount);
			}
#else
			Xil_DCacheInvalidateRange((INTPTR)Msg[Index].RxBfrPtr,
					(INTPTR)Msg[Index].ByteCount);
#endif

			*IOPending = XQspiPsu_SetIOMode(InstancePtr, &Msg[Index]);
			InstancePtr->RxBytes = 0;
		}
	} else {
		if (InstancePtr->RxBytes != 0) {
			QspiPsuStatusReg = XQspiPsu_ReadReg(InstancePtr->Config.BaseAddress, XQSPIPSU_ISR_OFFSET);
			XQspiPsu_IORead(InstancePtr, &Msg[Index], QspiPsuStatusReg);
		}
	}
	Status = (s32)XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief Performs a transfer of Tx data on the bus in interrupt mode.
 *
 * @param[in,out] InstancePtr Pointer to the XQspiPsu instance.
 * @param[in]     QspiPsuStatusReg Status QSPI status register.
 *
 ******************************************************************************/
void XQspiPsu_IntrSendData(XQspiPsu *InstancePtr, u32 QspiPsuStatusReg)
{
	s32 MsgCnt = InstancePtr->MsgCnt;
	XQspiPsu_Msg *Msg = InstancePtr->Msg;

	if ( ((QspiPsuStatusReg & XQSPIPSU_ISR_TXNOT_FULL_MASK) != (u32)FALSE) &&
	     (InstancePtr->TxBytes > 0)) {
		XQspiPsu_FillTxFifo(InstancePtr, &Msg[MsgCnt], (u32)XQSPIPSU_TXD_DEPTH);
	}
}

/*****************************************************************************/
/**
 *
 * @brief Performs a transfer of Rx data on the bus in interrupt mode.
 *
 * @param[in,out] InstancePtr Pointer to the XQspiPsu instance.
 * @param[in]     QspiPsuStatusReg Status QSPI status register.
 * @param[in]     DmaIntrStatusReg Status DMA interrupt register.
 *
 ******************************************************************************/
void XQspiPsu_IntrRecvData(XQspiPsu *InstancePtr,
			   u32 QspiPsuStatusReg, u32 DmaIntrStatusReg)
{
	s32 MsgCnt = InstancePtr->MsgCnt;
	XQspiPsu_Msg *Msg = InstancePtr->Msg;

	if (InstancePtr->ReadMode == XQSPIPSU_READMODE_DMA) {
		if ((DmaIntrStatusReg &
		     XQSPIPSU_QSPIDMA_DST_I_STS_DONE_MASK) != (u32)FALSE) {
			/* DMA transfer done, Invalidate Data Cache */
#if defined(EL1_NONSECURE) && (EL1_NONSECURE==1U)
			if (!((Msg[MsgCnt].RxAddr64bit >= XQSPIPSU_RXADDR_OVER_32BIT) ||
			      (Msg[MsgCnt].Xfer64bit != (u8)0U)) &&
			    (InstancePtr->Config.IsCacheCoherent == 0U)) {
				Xil_DCacheInvalidateRange((INTPTR)Msg[MsgCnt].RxBfrPtr, (INTPTR)Msg[MsgCnt].ByteCount);
			}
#else
			Xil_DCacheInvalidateRange((INTPTR)Msg[MsgCnt].RxBfrPtr, (INTPTR)Msg[MsgCnt].ByteCount);
#endif
			if (XQspiPsu_SetIOMode(InstancePtr, &Msg[MsgCnt])) {
				XQspiPsu_GenFifoEntryData(InstancePtr, &Msg[MsgCnt]);
				XQspiPsu_ManualStartEnable(InstancePtr);
			} else {
				InstancePtr->RxBytes = 0;
			}
		}
	} else {
		if (InstancePtr->RxBytes != 0) {
			XQspiPsu_IORead(InstancePtr, &Msg[MsgCnt], QspiPsuStatusReg);
		}
	}
}
/** @} */
