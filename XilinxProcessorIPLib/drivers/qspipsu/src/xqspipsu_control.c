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

		Divider = (u32)1U << (Prescaler+1U);

		FreqDiv = (InstancePtr->Config.InputClockHz)/Divider;

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
/** @} */
