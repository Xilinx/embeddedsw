/******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
 *
 * @file xqspipsu_control.c
 * @addtogroup qspipsu Overview
 * @{
 *
 * The xqspipsu_control.c file contains intermediate control functions used by functions
 * in xqspipsu.c and xqspipsu_options.c files.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.11   akm  03/09/20 First release
 * 1.13   akm  01/04/21 Fix MISRA-C violations.
 * 1.15   akm  10/21/21 Fix MISRA-C violations.
 * 1.15   akm  03/03/22 Enable tapdelay settings for applications on
 * 			 Microblaze platform.
 * 1.18   sb   08/29/23 Added XQspiPsu_PolledMessageTransfer, XQspiPsu_IntrDataTransfer and
 *                      XQspiPsu_IntrDummyDataTransfer functions.
 * 1.18   sb   09/11/23 Fix MISRA-C violation 8.13.
 * 1.18   sb   09/11/23 Update XQspiPsu_PolledMessageTransfer api to fix MISRA-C warnings.
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/

#include "xqspipsu_control.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/

/*****************************************************************************/
/**
 *
 * This function writes the GENFIFO entries to transmit the messages requested.
 *
 * @param	InstancePtr is a pointer to the XQspiPsu instance.
 * @param	Msg is a pointer to the structure containing transfer data.
 *
 * @return
 *		- XST_SUCCESS if successful.
 *		- XST_FAILURE if transfer fails.
 *		- XST_DEVICE_BUSY if a transfer is already in progress.
 *
 * @note	None.
 *
 ******************************************************************************/
void XQspiPsu_GenFifoEntryData(XQspiPsu *InstancePtr, XQspiPsu_Msg *Msg)
{
	u32 GenFifoEntry;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Msg != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
#ifdef DEBUG
	xil_printf("\nXQspiPsu_GenFifoEntryData\r\n");
#endif

	GenFifoEntry = 0x0U;
	/* Bus width */
	GenFifoEntry &= ~(u32)XQSPIPSU_GENFIFO_MODE_MASK;
	GenFifoEntry |= XQspiPsu_SelectSpiMode((u8)Msg->BusWidth);

	GenFifoEntry |= InstancePtr->GenFifoCS;
	GenFifoEntry &= ~(u32)XQSPIPSU_GENFIFO_BUS_MASK;
	GenFifoEntry |= InstancePtr->GenFifoBus;

	/* Data */
	if (((Msg->Flags) & XQSPIPSU_MSG_FLAG_STRIPE) != (u32)FALSE) {
		GenFifoEntry |= XQSPIPSU_GENFIFO_STRIPE;
	} else {
		GenFifoEntry &= ~XQSPIPSU_GENFIFO_STRIPE;
	}
	/* If Byte Count is less than 8 bytes do the transfer in IO mode */
	if ((Msg->ByteCount < 8U) &&
	    (InstancePtr->ReadMode == XQSPIPSU_READMODE_DMA)) {
		InstancePtr->ReadMode = XQSPIPSU_READMODE_IO;
		XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress, XQSPIPSU_CFG_OFFSET,
				  (XQspiPsu_ReadReg(InstancePtr->Config.BaseAddress, XQSPIPSU_CFG_OFFSET) &
				   ~XQSPIPSU_CFG_MODE_EN_MASK));
		InstancePtr->IsUnaligned = 1;
	}

	XQspiPsu_TXRXSetup(InstancePtr, Msg, &GenFifoEntry);

	XQspiPsu_GenFifoEntryDataLen(InstancePtr, Msg, &GenFifoEntry);

	/* One dummy GenFifo entry in case of IO mode */
	if ((InstancePtr->ReadMode == XQSPIPSU_READMODE_IO) &&
	    ((Msg->Flags & XQSPIPSU_MSG_FLAG_RX) != (u32)FALSE)) {
		GenFifoEntry = 0x0U;
#ifdef DEBUG
		xil_printf("\nDummy FifoEntry=%08x\r\n", GenFifoEntry);
#endif
		XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress, XQSPIPSU_GEN_FIFO_OFFSET, GenFifoEntry);
	}
}

/*****************************************************************************/
/**
 *
 * This function enables the polling functionality of controller
 *
 * @param	InstancePtr is a pointer to the XQspiPsu instance.
 *
 *
 * @param	FlashMsg is a pointer to the structure containing transfer data
 *
 * @return	None
 *
 * @note	None.
 *
 ******************************************************************************/
void XQspiPsu_PollDataConfig(XQspiPsu *InstancePtr, XQspiPsu_Msg *FlashMsg)
{

	u32 GenFifoEntry;
	u32 Value;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(FlashMsg != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
#ifdef DEBUG
	xil_printf("\nXQspiPsu_PollDataConfig\r\n");
#endif

	Value = XQspiPsu_CreatePollDataConfig(InstancePtr, FlashMsg);
	XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress,
			  XQSPIPSU_POLL_CFG_OFFSET, Value);
	XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress,
			  XQSPIPSU_P_TO_OFFSET, FlashMsg->PollTimeout);

	XQspiPsu_GenFifoEntryCSAssert(InstancePtr);

	GenFifoEntry = (u32)0;
	GenFifoEntry |= (u32)XQSPIPSU_GENFIFO_TX;
	GenFifoEntry |= InstancePtr->GenFifoBus;
	GenFifoEntry |= InstancePtr->GenFifoCS;
	GenFifoEntry |= (u32)XQSPIPSU_GENFIFO_MODE_SPI;
	GenFifoEntry |= (u32)FlashMsg->PollStatusCmd;

	XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress,
			  XQSPIPSU_GEN_FIFO_OFFSET, GenFifoEntry);

	GenFifoEntry = (u32)0;
	GenFifoEntry |= (u32)XQSPIPSU_GENFIFO_POLL;
	GenFifoEntry |= (u32)XQSPIPSU_GENFIFO_RX;
	GenFifoEntry |= InstancePtr->GenFifoBus;
	GenFifoEntry |= InstancePtr->GenFifoCS;
	GenFifoEntry |= (u32)XQSPIPSU_GENFIFO_MODE_SPI;
	if (((FlashMsg->Flags) & XQSPIPSU_MSG_FLAG_STRIPE) != (u32)FALSE) {
		GenFifoEntry |= XQSPIPSU_GENFIFO_STRIPE;
	} else {
		GenFifoEntry &= ~XQSPIPSU_GENFIFO_STRIPE;
	}
	XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress, XQSPIPSU_GEN_FIFO_OFFSET,
			  GenFifoEntry);

	/* One Dummy entry required for IO mode */
	GenFifoEntry = 0x0U;
	XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress, XQSPIPSU_GEN_FIFO_OFFSET,
			  GenFifoEntry);

	InstancePtr->Msg = FlashMsg;
	InstancePtr->NumMsg = (s32)1;
	InstancePtr->MsgCnt = 0;

	Value = XQspiPsu_ReadReg(InstancePtr->Config.BaseAddress,
				 XQSPIPSU_CFG_OFFSET);
	Value &= ~XQSPIPSU_CFG_MODE_EN_MASK;
	Value |= (XQSPIPSU_CFG_START_GEN_FIFO_MASK |
		  XQSPIPSU_CFG_GEN_FIFO_START_MODE_MASK |
		  XQSPIPSU_CFG_EN_POLL_TO_MASK);
	XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress, XQSPIPSU_CFG_OFFSET,
			  Value);

	/* Enable interrupts */
	Value = ((u32)XQSPIPSU_IER_RXNEMPTY_MASK |
		 (u32)XQSPIPSU_IER_POLL_TIME_EXPIRE_MASK);

	XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress, XQSPIPSU_IER_OFFSET,
			  Value);

}

#if defined (ARMR5) || defined (__aarch64__) || defined (__MICROBLAZE__)
/*****************************************************************************/
/**
*
* Configures the clock according to the prescaler passed.
*
*
* @param	InstancePtr is a pointer to the XQspiPsu instance.
* @param	Prescaler - clock prescaler.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_DEVICE_BUSY if the device is currently transferring data.
*		The transfer must complete or be aborted before setting Tapdelay.
*
* @note		None.
*
******************************************************************************/
s32 XQspipsu_Calculate_Tapdelay(const XQspiPsu *InstancePtr, u8 Prescaler)
{
	u32 FreqDiv, Divider;
	u32 Tapdelay = 0;
	u32 LBkModeReg = 0;
	u32 delayReg = 0;
	s32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(Prescaler <= XQSPIPSU_CR_PRESC_MAXIMUM);

	/*
	 * Do not allow the slave select to change while a transfer is in
	 * progress. Not thread-safe.
	 */
	if (InstancePtr->IsBusy == (u32)TRUE) {
		Status = (s32)XST_DEVICE_BUSY;
		goto END;
	} else {

		Divider = (u32)1U << (Prescaler + 1U);

		FreqDiv = (InstancePtr->Config.InputClockHz) / Divider;

#if defined (versal)
		if (FreqDiv <= XQSPIPSU_FREQ_37_5MHZ) {
#else
		if (FreqDiv <= XQSPIPSU_FREQ_40MHZ) {
#endif
			Tapdelay |= (TAPDLY_BYPASS_VALVE_40MHZ <<
				     IOU_TAPDLY_BYPASS_LQSPI_RX_SHIFT);
		} else if (FreqDiv <= XQSPIPSU_FREQ_100MHZ) {
			Tapdelay |= (TAPDLY_BYPASS_VALVE_100MHZ <<
				     IOU_TAPDLY_BYPASS_LQSPI_RX_SHIFT);
			LBkModeReg |= (USE_DLY_LPBK << XQSPIPSU_LPBK_DLY_ADJ_USE_LPBK_SHIFT);
#if defined (versal)
			delayReg |= (u32)USE_DATA_DLY_ADJ  <<
				    XQSPIPSU_DATA_DLY_ADJ_USE_DATA_DLY_SHIFT;
#else
			delayReg |= ((u32)USE_DATA_DLY_ADJ  <<
				     XQSPIPSU_DATA_DLY_ADJ_USE_DATA_DLY_SHIFT) |
				    ((u32)DATA_DLY_ADJ_DLY  << XQSPIPSU_DATA_DLY_ADJ_DLY_SHIFT);
#endif
		} else if (FreqDiv <= XQSPIPSU_FREQ_150MHZ) {
#if defined (versal)
			LBkModeReg |= (USE_DLY_LPBK  << XQSPIPSU_LPBK_DLY_ADJ_USE_LPBK_SHIFT) |
				      (LPBK_DLY_ADJ_DLY1 << XQSPIPSU_LPBK_DLY_ADJ_DLY1_SHIFT);
#else
			LBkModeReg |= USE_DLY_LPBK  << XQSPIPSU_LPBK_DLY_ADJ_USE_LPBK_SHIFT;
#endif
		} else {
			Status = (s32)XST_FAILURE;
			goto END;
		}

		Status =  XQspipsu_Set_TapDelay(InstancePtr, Tapdelay, LBkModeReg, delayReg);
	}

END:
	return Status;
}
#endif

/*****************************************************************************/
/**
 *
 * This function performs a transfer on the bus in polled mode. The messages
 * passed are all transferred on the bus between one CS assert and de-assert.
 *
 * @param	InstancePtr is a pointer to the XQspiPsu instance.
 * @param	Msg is a pointer to the structure containing transfer data.
 * @param	NumMsg is the number of messages to be transferred.
 *
 * @return
 *		- XST_SUCCESS if successful.
 *		- XST_FAILURE if transfer fails.
 *
 * @note	None.
 *
 ******************************************************************************/
s32 XQspiPsu_PolledMessageTransfer(XQspiPsu *InstancePtr, XQspiPsu_Msg *Msg,
				   u32 NumMsg)
{
	s32 Index;
	u32 QspiPsuStatusReg;
	u32 IOPending = (u32)FALSE;
	s32 Status;

	/* list */
	Index = 0;

	while (Index < (s32)NumMsg) {
		XQspiPsu_GenFifoEntryData(InstancePtr, &Msg[Index]);
		XQspiPsu_ManualStartEnable(InstancePtr);
		/* Use thresholds here */
		/* If there is more data to be transmitted */
		do {
			if ((Msg[Index].Flags & XQSPIPSU_MSG_FLAG_TX) != (u32)FALSE) {
				Status = XQspiPsu_PolledSendData(InstancePtr, Msg, Index);
				if (Status != (s32)XST_SUCCESS) {
					goto END;
				}
			}
			if ((Msg[Index].Flags & XQSPIPSU_MSG_FLAG_RX) != (u32)FALSE) {
				Status = XQspiPsu_PolledRecvData(InstancePtr, Msg, Index, &IOPending);
				if (Status != (s32)XST_SUCCESS) {
					goto END;
				}
				if (IOPending == (u32)TRUE) {
					break;
				}
			}

			QspiPsuStatusReg = XQspiPsu_ReadReg(InstancePtr->Config.BaseAddress, XQSPIPSU_ISR_OFFSET);

		} while (((QspiPsuStatusReg &
			   XQSPIPSU_ISR_GENFIFOEMPTY_MASK) == (u32)FALSE) ||
			 (InstancePtr->TxBytes != 0) ||
			 ((QspiPsuStatusReg & XQSPIPSU_ISR_TXEMPTY_MASK) == (u32)FALSE) ||
			 (InstancePtr->RxBytes != 0));

		if ((InstancePtr->IsUnaligned != 0) && (IOPending == (u32)FALSE)) {
			InstancePtr->IsUnaligned = 0;
			XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress, XQSPIPSU_CFG_OFFSET,
					  (XQspiPsu_ReadReg(InstancePtr->Config.BaseAddress, XQSPIPSU_CFG_OFFSET) |
					   XQSPIPSU_CFG_MODE_EN_DMA_MASK));
			InstancePtr->ReadMode = XQSPIPSU_READMODE_DMA;
		}
		if (IOPending == (u32)TRUE) {
			IOPending = (u32)FALSE;
		} else {
			Index++;
		}
	}
	Status = (s32) XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * This function transfers Tx and Rx data.
 *
 * @param       InstancePtr is a pointer to the XQspiPsu instance.
 * @param       QspiPsuStatusReg is the status of QSPI status register.
 * @param       DeltaMsgCnt is the message count flag.
 *
 * @return      None.
 *
 * @note        None.
 *
******************************************************************************/
void XQspiPsu_IntrDataTransfer(XQspiPsu *InstancePtr,
			       u32 *QspiPsuStatusReg, u8 *DeltaMsgCnt)
{
	u32 DmaIntrStatusReg = 0;
	const XQspiPsu_Msg *Msg = InstancePtr->Msg;
	s32 NumMsg = InstancePtr->NumMsg;
	s32 MsgCnt = InstancePtr->MsgCnt;
	u32 TxRxFlag = Msg[MsgCnt].Flags;
	UINTPTR BaseAddr = InstancePtr->Config.BaseAddress;

	/* QSPIPSU Intr cleared on read */
	*QspiPsuStatusReg = XQspiPsu_ReadReg(BaseAddr, XQSPIPSU_ISR_OFFSET);
	if (InstancePtr->ReadMode == XQSPIPSU_READMODE_DMA) {
		/* DMA Intr write to clear */
		DmaIntrStatusReg = XQspiPsu_ReadReg(BaseAddr,
						    XQSPIPSU_QSPIDMA_DST_I_STS_OFFSET);
		XQspiPsu_WriteReg(BaseAddr, XQSPIPSU_QSPIDMA_DST_I_STS_OFFSET,
				  DmaIntrStatusReg);
	}
	if (((DmaIntrStatusReg & XQSPIPSU_QSPIDMA_DST_INTR_ERR_MASK) != (u32)FALSE)) {
		/* Call status handler to indicate error */
		InstancePtr->StatusHandler(InstancePtr->StatusRef,
					   XST_SPI_COMMAND_ERROR, 0);
	}
	/* Fill more data to be txed if required */
	if ((MsgCnt < NumMsg) && ((TxRxFlag & XQSPIPSU_MSG_FLAG_TX) != (u32)FALSE) ) {
		XQspiPsu_IntrSendData(InstancePtr, *QspiPsuStatusReg, DeltaMsgCnt);
	}
	MsgCnt = InstancePtr->MsgCnt ;
	if ((MsgCnt < NumMsg) &&
	    ((TxRxFlag & XQSPIPSU_MSG_FLAG_RX) != (u32)FALSE)) {
		XQspiPsu_IntrRecvData(InstancePtr, *QspiPsuStatusReg, DmaIntrStatusReg, DeltaMsgCnt);
	}
}

/*****************************************************************************/
/**
 *
 * This function transfers Dummy byte
 *
 * @param       InstancePtr is a pointer to the XQspiPsu instance.
 * @param       QspiPsuStatusReg is the status of QSPI status register.
 * @param       DeltaMsgCnt is the message count flag.
 *
 * @return      None.
 *
 * @note        None.
 *
******************************************************************************/
void XQspiPsu_IntrDummyDataTransfer(XQspiPsu *InstancePtr, u32 QspiPsuStatusReg,
				    u8 DeltaMsgCnt)
{
	XQspiPsu_Msg *Msg = InstancePtr->Msg;
	s32 NumMsg = InstancePtr->NumMsg;
	s32 MsgCnt = InstancePtr->MsgCnt;
	UINTPTR BaseAddr = InstancePtr->Config.BaseAddress;

	/*
	 * DeltaMsgCnt is to handle conditions where genfifo empty can be set
	 * while tx is still not empty or rx dma is not yet done.
	 * MsgCnt > NumMsg indicates CS de-assert entry was also executed.
	 */
	if (((QspiPsuStatusReg & XQSPIPSU_ISR_GENFIFOEMPTY_MASK) != (u32)FALSE) &&
	    ((DeltaMsgCnt != (u8)FALSE) || (MsgCnt > NumMsg))) {
		if (MsgCnt < NumMsg) {
			if (InstancePtr->IsUnaligned != 0) {
				InstancePtr->IsUnaligned = 0;
				XQspiPsu_WriteReg(BaseAddr,
						  XQSPIPSU_CFG_OFFSET, (XQspiPsu_ReadReg(
								  BaseAddr, XQSPIPSU_CFG_OFFSET) |
									XQSPIPSU_CFG_MODE_EN_DMA_MASK));
				InstancePtr->ReadMode = XQSPIPSU_READMODE_DMA;
			}
			/* This might not work if not manual start */
			XQspiPsu_GenFifoEntryData(InstancePtr, &Msg[MsgCnt]);
			XQspiPsu_ManualStartEnable(InstancePtr);
		} else if (MsgCnt == NumMsg) {
			/* This is just to keep track of the de-assert entry */
			MsgCnt += 1;
			InstancePtr->MsgCnt = MsgCnt;
			/* De-select slave */
			XQspiPsu_GenFifoEntryCSDeAssert(InstancePtr);
			XQspiPsu_ManualStartEnable(InstancePtr);
		} else {
			/* Disable interrupts */
			XQspiPsu_WriteReg(BaseAddr, XQSPIPSU_IDR_OFFSET,
					  (u32)XQSPIPSU_IER_TXNOT_FULL_MASK |
					  (u32)XQSPIPSU_IER_TXEMPTY_MASK |
					  (u32)XQSPIPSU_IER_RXNEMPTY_MASK |
					  (u32)XQSPIPSU_IER_GENFIFOEMPTY_MASK |
					  (u32)XQSPIPSU_IER_RXEMPTY_MASK);
#if ! defined (__MICROBLAZE__)
			dmb();
#endif
			if (InstancePtr->ReadMode == XQSPIPSU_READMODE_DMA) {
				XQspiPsu_WriteReg(BaseAddr,
						  XQSPIPSU_QSPIDMA_DST_I_DIS_OFFSET,
						  XQSPIPSU_QSPIDMA_DST_I_EN_DONE_MASK);
			}

			/* Clear the busy flag. */
			InstancePtr->IsBusy = (u32)FALSE;

#if defined  (XCLOCKING)
			Xil_ClockDisable(InstancePtr->Config.RefClk);
#endif
			/* Call status handler to indicate completion */
			InstancePtr->StatusHandler(InstancePtr->StatusRef,
						   XST_SPI_TRANSFER_DONE, 0);
		}
	}
}
/** @} */
