/******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
 *
 * @file xqspipsu.c
 * @addtogroup qspipsu_v1_12
 * @{
 *
 * This file implements the functions required to use the QSPIPSU hardware to
 * perform a transfer. These are accessible to the user via xqspipsu.h.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.0   hk  08/21/14 First release
 *       sk  03/13/15 Added IO mode support.
 *       hk  03/18/15 Switch to I/O mode before clearing RX FIFO.
 *                    Clear and disable DMA interrupts/status in abort.
 *                    Use DMA DONE bit instead of BUSY as recommended.
 *       sk  04/24/15 Modified the code according to MISRAC-2012.
 *       sk  06/17/15 Removed NULL checks for Rx/Tx buffers. As
 *                    writing/reading from 0x0 location is permitted.
 * 1.1   sk  04/12/16 Added debug message prints.
 * 1.2 nsk 07/01/16 Changed XQspiPsu_Select to support GQSPI and LQSPI
 *                  selection.
 *       rk  07/15/16 Added support for TapDelays at different frequencies.
 *     nsk 08/05/16 Added example support PollData and PollTimeout
 * 1.3 nsk 09/16/16 Update PollData and PollTimeout support for dual
 *                  parallel configurations, modified XQspiPsu_PollData()
 *                  and XQspiPsu_Create_PollConfigData()
 * 1,5 nsk 08/14/17 Added CCI support
 * 1.7 tjs 01/16/18 Removed the check for DMA MSB to be written. (CR#992560)
 * 1.7 tjs 01/17/18 Added a support to toggle WP pin of the flash.
 * 1.7 tjs 03/14/18 Added support in EL1 NS mode (CR#974882)
 * 1.8 tjs 06/26/18 Added an example for accessing 64bit dma within
 *                  32 bit application. CR#1004701
 * 1.8 tjs 06/26/18 Removed checkpatch warnings.
 * 1.8 tjs 07/09/18 Fixed cppcheck and doxygen warnings. (CR#1006336)
 * 1.8 tjs 07/18/18 Setup64BRxDma() should be called only if the RxAddress is
 *                  greater than 32 bit address space. (CR#1006862)
 * 1.8 tjs 09/06/18 Fixed the code in XQspiPsu_GenFifoEntryData() for data
 *                  transfer length up to 255 for reducing the extra loop.
 * 1.8  mus 11/05/18 Support 64 bit DMA addresses for Microblaze-X platform.
 * 1.9 tjs 11/22/17 Added the check for A72 and R5 processors (CR-987075)
 * 1.9 tjs 04/17/18 Updated register addresses as per the latest revision
 *		    of versal (CR#999610)
 * 1.9  aru 01/17/19 Fixes violations according to MISRAC-2012
 *                  in safety mode and modified the code such as
 *                  Added UNITPTR inplace of INTPTR,Declared the pointer param
 *		    as Pointer to const .
 * 1.9  nsk 02/01/19 Clear DMA_DST_ADDR_MSB register on 32bit machine, if the
 *		     address is of only 32bit (CR#1020031)
 * 1.9  nsk 02/01/19 Added QSPI idling support.
 * 1.9  rama 03/13/19 Fixed MISRA violations related to UR data anamoly,
 *					  expression is not a boolean
 * 1.9  nsk 03/27/19 Update 64bit dma support
 * 1.10 sk  08/20/19 Fixed issues in poll timeout feature.
 * 1.11 akm 02/19/20 Added XQspiPsu_StartDmaTransfer() and XQspiPsu_CheckDmaDone()
 * 		     APIs for non-blocking transfer.
 * 1.11 sd  01/02/20 Added clocking support
 * 1.11 akm 03/09/20 Reorganize the source code, enable qspi controller and
 *		     interrupts in XQspiPsu_CfgInitialize() API.
 * 1.11 akm 03/26/20 Fixed issue by updating XQspiPsu_CfgInitialize to return
 *		     XST_DEVICE_IS_STARTED instead of asserting, when the
 *		     instance is already configured.
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/

#include "xqspipsu.h"
#include "xqspipsu_control.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 *
 * Initializes a specific XQspiPsu instance as such the driver is ready to use.
 *
 *
 * @param	InstancePtr is a pointer to the XQspiPsu instance.
 * @param	ConfigPtr is a reference to a structure containing information
 *		about a specific QSPIPSU device. This function initializes an
 *		InstancePtr object for a specific device specified by the
 *		contents of Config.
 * @param	EffectiveAddr is the device base address in the virtual memory
 *		address space. The caller is responsible for keeping the address
 *		mapping from EffectiveAddr to the device physical base address
 *		unchanged once this function is invoked. Unexpected errors may
 *		occur if the address mapping changes after this function is
 *		called. If address translation is not used, use
 *		ConfigPtr->Config.BaseAddress for this device.
 *
 * @return
 *		- XST_SUCCESS if successful.
 *		- XST_DEVICE_IS_STARTED if the device is already started.
 *		It must be stopped to re-initialize.
 *
 * @note	None.
 *
 ******************************************************************************/
s32 XQspiPsu_CfgInitialize(XQspiPsu *InstancePtr,
			   const XQspiPsu_Config *ConfigPtr, u32 EffectiveAddr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);
	s32 Status;

	/*
	 * If the device is busy, disallow the initialize and return a status
	 * indicating it is already started. This allows the user to stop the
	 * device and re-initialize, but prevents a user from inadvertently
	 * initializing. This assumes the busy flag is cleared at startup.
	 */
	if (InstancePtr->IsBusy == TRUE ||
	    InstancePtr->IsReady == XIL_COMPONENT_IS_READY) {
		Status = (s32)XST_DEVICE_IS_STARTED;
	} else {
		/* Set some default values. */
		InstancePtr->IsBusy = FALSE;
		InstancePtr->Config.BaseAddress =
			EffectiveAddr + XQSPIPSU_OFFSET;
		InstancePtr->Config.ConnectionMode = ConfigPtr->ConnectionMode;
		InstancePtr->StatusHandler = StubStatusHandler;
		InstancePtr->Config.BusWidth = ConfigPtr->BusWidth;
		InstancePtr->Config.InputClockHz = ConfigPtr->InputClockHz;
#if defined  (XCLOCKING)
		InstancePtr->Config.RefClk = ConfigPtr->RefClk;
#endif
		InstancePtr->Config.IsCacheCoherent =
			ConfigPtr->IsCacheCoherent;
		/* Other instance variable initializations */
		InstancePtr->SendBufferPtr = NULL;
		InstancePtr->RecvBufferPtr = NULL;
		InstancePtr->GenFifoBufferPtr = NULL;
		InstancePtr->TxBytes = 0;
		InstancePtr->RxBytes = 0;
		InstancePtr->GenFifoEntries = 0;
		InstancePtr->ReadMode = XQSPIPSU_READMODE_DMA;
		InstancePtr->GenFifoCS = XQSPIPSU_GENFIFO_CS_LOWER;
		InstancePtr->GenFifoBus = XQSPIPSU_GENFIFO_BUS_LOWER;
		InstancePtr->IsUnaligned = 0;
		InstancePtr->IsManualstart = TRUE;

		/* Select QSPIPSU */
		XQspiPsu_Select(InstancePtr, XQSPIPSU_SEL_GQSPI_MASK);
		/*
		 * Reset the QSPIPSU device to get it into its initial state.
		 * It is expected that device configuration will take place
		 * after this initialization is done, but before the device
		 * is started.
		 */
		XQspiPsu_Reset(InstancePtr);
		/* Enable */
		XQspiPsu_Enable(InstancePtr);

		InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 *
 * Stops the transfer of data to internal DST FIFO from stream interface and
 * also stops the issuing of new write commands to memory.
 *
 * By calling this API, any ongoing Dma transfers will be paused and DMA will
 * not issue AXI write commands to memory
 *
 * @param	InstancePtr is a pointer to the XQspiPsu instance.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void XQspiPsu_Idle(const XQspiPsu *InstancePtr)
{
	u32 RegEn;
	u32 DmaStatus;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Check for QSPI enable */
	RegEn = XQspiPsu_ReadReg(InstancePtr->Config.BaseAddress,
			XQSPIPSU_EN_OFFSET);
	if ((RegEn & XQSPIPSU_EN_MASK) != 0U) {
		DmaStatus = XQspiPsu_ReadReg(InstancePtr->Config.BaseAddress,
				XQSPIPSU_QSPIDMA_DST_CTRL_OFFSET);
		DmaStatus |= XQSPIPSU_QSPIDMA_DST_CTRL_PAUSE_STRM_MASK;
		DmaStatus |= XQSPIPSU_QSPIDMA_DST_CTRL_PAUSE_MEM_MASK;
		XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress,
				XQSPIPSU_QSPIDMA_DST_CTRL_OFFSET, DmaStatus);
	}
#if defined  (XCLOCKING)
	Xil_ClockDisable(InstancePtr->Config.RefClk);
#endif
}

/*****************************************************************************/
/**
 *
 * Resets the QSPIPSU device. Reset must only be called after the driver has
 * been initialized. Any data transfer that is in progress is aborted.
 *
 * The upper layer software is responsible for re-configuring (if necessary)
 * and restarting the QSPIPSU device after the reset.
 *
 * @param	InstancePtr is a pointer to the XQspiPsu instance.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void XQspiPsu_Reset(XQspiPsu *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
#ifdef DEBUG
	xil_printf("\nXQspiPsu_Reset\r\n");
#endif

	/* Abort any transfer that is in progress */
	XQspiPsu_Abort(InstancePtr);

	/* Default value to config register */
	XQspiPsu_SetDefaultConfig(InstancePtr);

}

/*****************************************************************************/
/**
 *
 * Aborts a transfer in progress.
 *
 * @param	InstancePtr is a pointer to the XQspiPsu instance.
 *
 * @return	None.
 *
 * @note
 *
 ******************************************************************************/
void XQspiPsu_Abort(XQspiPsu *InstancePtr)
{
	u32 IntrStatus, ConfigReg;

	Xil_AssertVoid(InstancePtr != NULL);
#ifdef DEBUG
	xil_printf("\nXQspiPsu_Abort\r\n");
#endif
	IntrStatus = XQspiPsu_ReadReg(InstancePtr->Config.BaseAddress,
					XQSPIPSU_ISR_OFFSET);

	/* Clear and disable interrupts */
	XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress,
		XQSPIPSU_ISR_OFFSET, IntrStatus | XQSPIPSU_ISR_WR_TO_CLR_MASK);
	XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress,
			XQSPIPSU_QSPIDMA_DST_I_STS_OFFSET,
		XQspiPsu_ReadReg(InstancePtr->Config.BaseAddress,
				XQSPIPSU_QSPIDMA_DST_I_STS_OFFSET));
	XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress,
			XQSPIPSU_QSPIDMA_DST_STS_OFFSET,
			XQspiPsu_ReadReg(InstancePtr->Config.BaseAddress,
				XQSPIPSU_QSPIDMA_DST_STS_OFFSET) |
				XQSPIPSU_QSPIDMA_DST_STS_WTC);
	XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress,
		XQSPIPSU_IDR_OFFSET, XQSPIPSU_IDR_ALL_MASK);
	XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress,
			XQSPIPSU_QSPIDMA_DST_I_DIS_OFFSET,
			XQSPIPSU_QSPIDMA_DST_INTR_ALL_MASK);

	/* Clear FIFO */
	if ((XQspiPsu_ReadReg(InstancePtr->Config.BaseAddress,
		XQSPIPSU_ISR_OFFSET) & XQSPIPSU_ISR_RXEMPTY_MASK) != FALSE)
		XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress,
			XQSPIPSU_FIFO_CTRL_OFFSET, XQSPIPSU_FIFO_CTRL_RST_TX_FIFO_MASK |
			XQSPIPSU_FIFO_CTRL_RST_GEN_FIFO_MASK);
	/*
	 * Switch to IO mode to Clear RX FIFO. This is because of DMA behaviour
	 * where it waits on RX empty and goes busy assuming there is data
	 * to be transferred even if there is no request.
	 */
	if ((IntrStatus & XQSPIPSU_ISR_RXEMPTY_MASK) != 0U) {
		ConfigReg = XQspiPsu_ReadReg(InstancePtr->Config.BaseAddress,
					XQSPIPSU_CFG_OFFSET);
		ConfigReg &= ~XQSPIPSU_CFG_MODE_EN_MASK;
		XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress,
				XQSPIPSU_CFG_OFFSET, ConfigReg);

		XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress,
				XQSPIPSU_FIFO_CTRL_OFFSET,
				XQSPIPSU_FIFO_CTRL_RST_RX_FIFO_MASK);

		if (InstancePtr->ReadMode == XQSPIPSU_READMODE_DMA) {
			ConfigReg |= XQSPIPSU_CFG_MODE_EN_DMA_MASK;
			XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress,
					XQSPIPSU_CFG_OFFSET, ConfigReg);
		}
	}

	InstancePtr->TxBytes = 0;
	InstancePtr->RxBytes = 0;
	InstancePtr->GenFifoEntries = 0;
	InstancePtr->IsBusy = FALSE;
}

/*****************************************************************************/
/**
 * This is the handler for polling functionality of controller. It reads data
 * from RXFIFO, since when data from the flash device (status data) matched
 * with configured value in poll_cfg, then controller writes the matched data
 * into RXFIFO.
 *
 *
 * @param       InstancePtr is a pointer to the XQspiPsu instance.
 * @param       Msg is a pointer to the structure containing transfer data.l
 * @param       Index is the message number to be transferred.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void XQspiPsu_PollDataHandler(XQspiPsu *InstancePtr, u32 StatusReg)
{

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
#ifdef DEBUG
	xil_printf("\nXQspiPsu_PollDataHandler\r\n");
#endif

	if ((StatusReg & XQSPIPSU_ISR_RXNEMPTY_MASK) != FALSE) {
		/*
		 * Read data from RXFIFO, since when data from the
		 * flash device (status data) matched with configured
		 * value in poll_cfg, then controller writes the
		 * matched data into RXFIFO.
		 */
		(void)XQspiPsu_ReadReg(InstancePtr->Config.BaseAddress,
				XQSPIPSU_RXD_OFFSET);

		InstancePtr->StatusHandler(InstancePtr->StatusRef,
				XST_SPI_POLL_DONE, 0);
	}
	if ((StatusReg & XQSPIPSU_ISR_POLL_TIME_EXPIRE_MASK) != FALSE)
			InstancePtr->StatusHandler(InstancePtr->StatusRef,
							XST_FLASH_TIMEOUT_ERROR, 0);

	XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress, XQSPIPSU_IDR_OFFSET,
			(u32)XQSPIPSU_IER_RXNEMPTY_MASK |
			(u32)XQSPIPSU_IER_POLL_TIME_EXPIRE_MASK);
	InstancePtr->IsBusy = FALSE;
	if (InstancePtr->ReadMode == XQSPIPSU_READMODE_DMA)
			XQspiPsu_SetReadMode(InstancePtr, XQSPIPSU_READMODE_DMA);
	/* De-select slave */
	XQspiPsu_GenFifoEntryCSDeAssert(InstancePtr);
	XQspiPsu_ManualStartEnable(InstancePtr);
}

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
 *		- XST_DEVICE_BUSY if a transfer is already in progress.
 *
 * @note	None.
 *
 ******************************************************************************/
s32 XQspiPsu_PolledTransfer(XQspiPsu *InstancePtr, XQspiPsu_Msg *Msg,
				u32 NumMsg)
{
	s32 Index;
	u32 QspiPsuStatusReg;
	u32 IOPending = (u32)FALSE;
	u32 DmaIntrSts;
	s32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Msg != NULL);
	Xil_AssertNonvoid(NumMsg > 0);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	for (Index = 0; Index < (s32)NumMsg; Index++)
		Xil_AssertNonvoid(Msg[Index].ByteCount > 0U);
	/*
	 * Check whether there is another transfer in progress.
	 * Not thread-safe
	 */
	if (InstancePtr->IsBusy == TRUE) {
		Status = (s32)XST_DEVICE_BUSY;
		goto END;
	}
	/* Check for ByteCount upper limit - 2^28 for DMA */
	for (Index = 0; Index < (s32)NumMsg; Index++) {
		if ((Msg[Index].ByteCount > XQSPIPSU_DMA_BYTES_MAX) &&
				((Msg[Index].Flags & XQSPIPSU_MSG_FLAG_RX) != FALSE)) {
			Status = XST_FAILURE;
			goto END;
		}
	}
	/*
	 * Set the busy flag, which will be cleared when the transfer is
	 * entirely done.
	 */
	InstancePtr->IsBusy = TRUE;

#if defined  (XCLOCKING)
	Xil_ClockEnable(InstancePtr->Config.RefClk);
#endif
	/* Select slave */
	XQspiPsu_GenFifoEntryCSAssert(InstancePtr);

	/* list */
	Index = 0;
	while (Index < (s32)NumMsg) {
		XQspiPsu_GenFifoEntryData(InstancePtr, &Msg[Index]);
		XQspiPsu_ManualStartEnable(InstancePtr);
		/* Use thresholds here */
		/* If there is more data to be transmitted */
		do {
			QspiPsuStatusReg = XQspiPsu_ReadReg(InstancePtr->Config.BaseAddress,
						XQSPIPSU_ISR_OFFSET);
			/* Transmit more data if left */
			if (((QspiPsuStatusReg & XQSPIPSU_ISR_TXNOT_FULL_MASK) != FALSE) &&
				((Msg[Index].Flags & XQSPIPSU_MSG_FLAG_TX) != FALSE) &&
				(InstancePtr->TxBytes > 0))
				XQspiPsu_FillTxFifo(InstancePtr, &Msg[Index],
						(u32)XQSPIPSU_TXD_DEPTH);
			/* Check if DMA RX is complete and update RxBytes */
			if ((InstancePtr->ReadMode == XQSPIPSU_READMODE_DMA) &&
				((Msg[Index].Flags & XQSPIPSU_MSG_FLAG_RX) != FALSE)) {
				DmaIntrSts = XQspiPsu_ReadReg(InstancePtr->Config.BaseAddress,
					XQSPIPSU_QSPIDMA_DST_I_STS_OFFSET);
				if ((DmaIntrSts &
						XQSPIPSU_QSPIDMA_DST_I_STS_DONE_MASK) != FALSE) {
					XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress,
							XQSPIPSU_QSPIDMA_DST_I_STS_OFFSET, DmaIntrSts);

					IOPending = XQspiPsu_SetIOMode(InstancePtr, &Msg[Index]);
					InstancePtr->RxBytes = 0;
					if (IOPending == (u32)TRUE)
						break;
				}
			} else if ((Msg[Index].Flags & XQSPIPSU_MSG_FLAG_RX) != FALSE)
				XQspiPsu_IORead(InstancePtr, &Msg[Index], QspiPsuStatusReg);

		} while (((QspiPsuStatusReg &
			XQSPIPSU_ISR_GENFIFOEMPTY_MASK) == FALSE) ||
			(InstancePtr->TxBytes != 0) ||
			((QspiPsuStatusReg & XQSPIPSU_ISR_TXEMPTY_MASK) == FALSE) ||
			(InstancePtr->RxBytes != 0));

		if ((InstancePtr->IsUnaligned != 0) && (IOPending == (u32)FALSE)) {
			InstancePtr->IsUnaligned = 0;
			XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress, XQSPIPSU_CFG_OFFSET,
					(XQspiPsu_ReadReg(InstancePtr->Config.BaseAddress, XQSPIPSU_CFG_OFFSET) |
							XQSPIPSU_CFG_MODE_EN_DMA_MASK));
			InstancePtr->ReadMode = XQSPIPSU_READMODE_DMA;
		}
		if (IOPending == (u32)TRUE)
			IOPending = (u32)FALSE;
		else
			Index++;
	}
	/* De-select slave */
	XQspiPsu_GenFifoEntryCSDeAssert(InstancePtr);
	XQspiPsu_ManualStartEnable(InstancePtr);
	do
		QspiPsuStatusReg = XQspiPsu_ReadReg(InstancePtr->Config.BaseAddress, XQSPIPSU_ISR_OFFSET);
	while ((QspiPsuStatusReg & XQSPIPSU_ISR_GENFIFOEMPTY_MASK) == FALSE);

	/* Clear the busy flag. */
	InstancePtr->IsBusy = FALSE;

	Status = XST_SUCCESS;

#if defined  (XCLOCKING)
	Xil_ClockDisable(InstancePtr->Config.RefClk);
#endif
	END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * This function initiates a transfer on the bus and enables interrupts.
 * The transfer is completed by the interrupt handler. The messages passed are
 * all transferred on the bus between one CS assert and de-assert.
 *
 * @param	InstancePtr is a pointer to the XQspiPsu instance.
 * @param	Msg is a pointer to the structure containing transfer data.
 * @param	NumMsg is the number of messages to be transferred.
 *
 * @return
 *		- XST_SUCCESS if successful.
 *		- XST_FAILURE if transfer fails.
 *		- XST_DEVICE_BUSY if a transfer is already in progress.
 *
 * @note	None.
 *
 ******************************************************************************/
s32 XQspiPsu_InterruptTransfer(XQspiPsu *InstancePtr, XQspiPsu_Msg *Msg,
				u32 NumMsg)
{
	s32 Index;
	s32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	for (Index = 0; Index < (s32)NumMsg; Index++)
		Xil_AssertNonvoid(Msg[Index].ByteCount > 0U);
	/*
	 * Check whether there is another transfer in progress.
	 * Not thread-safe
	 */
	if (InstancePtr->IsBusy == TRUE) {
			Status = (s32)XST_DEVICE_BUSY;
			goto END;
	}
#if defined  (XCLOCKING)
	Xil_ClockEnable(InstancePtr->Config.RefClk);
#endif

	if ((Msg[0].Flags & XQSPIPSU_MSG_FLAG_POLL) != FALSE) {
		InstancePtr->IsBusy = TRUE;
		XQspiPsu_PollDataConfig(InstancePtr, Msg);
	} else {
		/* Check for ByteCount upper limit - 2^28 for DMA */
		for (Index = 0; Index < (s32)NumMsg; Index++) {
			if ((Msg[Index].ByteCount > XQSPIPSU_DMA_BYTES_MAX) &&
					((Msg[Index].Flags & XQSPIPSU_MSG_FLAG_RX) != FALSE)) {
				Status = XST_FAILURE;
				goto END;
			}
		}
		/*
		 * Set the busy flag, which will be cleared when the transfer is
		 * entirely done.
		 */
		InstancePtr->IsBusy = TRUE;

		InstancePtr->Msg = Msg;
		InstancePtr->NumMsg = (s32)NumMsg;
		InstancePtr->MsgCnt = 0;

		/* Select slave */
		XQspiPsu_GenFifoEntryCSAssert(InstancePtr);
		/* This might not work if not manual start */
		/* Put first message in FIFO along with the above slave select */
		XQspiPsu_GenFifoEntryData(InstancePtr, &Msg[0]);
		XQspiPsu_ManualStartEnable(InstancePtr);

		/* Enable interrupts */
		XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress, XQSPIPSU_IER_OFFSET,
			(u32)XQSPIPSU_IER_TXNOT_FULL_MASK |
			(u32)XQSPIPSU_IER_TXEMPTY_MASK |
			(u32)XQSPIPSU_IER_RXNEMPTY_MASK |
			(u32)XQSPIPSU_IER_GENFIFOEMPTY_MASK |
			(u32)XQSPIPSU_IER_RXEMPTY_MASK);

		if (InstancePtr->ReadMode == XQSPIPSU_READMODE_DMA)
			XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress, XQSPIPSU_QSPIDMA_DST_I_EN_OFFSET,
					XQSPIPSU_QSPIDMA_DST_I_EN_DONE_MASK);
	}
	Status = XST_SUCCESS;

	END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * Handles interrupt based transfers by acting on GENFIFO and DMA interurpts.
 *
 * @param	InstancePtr is a pointer to the XQspiPsu instance.
 *
 * @return
 *		- XST_SUCCESS if successful.
 *		- XST_FAILURE if transfer fails.
 *
 * @note	None.
 *
 ******************************************************************************/
s32 XQspiPsu_InterruptHandler(XQspiPsu *InstancePtr)
{
	u32 QspiPsuStatusReg, DmaIntrStatusReg = 0;
	XQspiPsu_Msg *Msg;
	s32 NumMsg;
	s32 MsgCnt;
	u8 DeltaMsgCnt = 0;
	u32 TxRxFlag;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->NumMsg > 0);
	Xil_AssertNonvoid(InstancePtr->Msg != NULL);

	Msg = InstancePtr->Msg;
	NumMsg = InstancePtr->NumMsg;
	MsgCnt = InstancePtr->MsgCnt;
	TxRxFlag = Msg[MsgCnt].Flags;

	/* QSPIPSU Intr cleared on read */
	QspiPsuStatusReg = XQspiPsu_ReadReg(InstancePtr->Config.BaseAddress, XQSPIPSU_ISR_OFFSET);
	if (InstancePtr->ReadMode == XQSPIPSU_READMODE_DMA) {
		/* DMA Intr write to clear */
		DmaIntrStatusReg = XQspiPsu_ReadReg(InstancePtr->Config.BaseAddress,
					XQSPIPSU_QSPIDMA_DST_I_STS_OFFSET);
		XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress, XQSPIPSU_QSPIDMA_DST_I_STS_OFFSET,
					DmaIntrStatusReg);
	}
	if (((DmaIntrStatusReg & XQSPIPSU_QSPIDMA_DST_INTR_ERR_MASK) != FALSE))
		/* Call status handler to indicate error */
		InstancePtr->StatusHandler(InstancePtr->StatusRef,
				XST_SPI_COMMAND_ERROR, 0);

	/* Fill more data to be txed if required */
	if ((MsgCnt < NumMsg) && ((TxRxFlag & XQSPIPSU_MSG_FLAG_TX) != FALSE) &&
		((QspiPsuStatusReg & XQSPIPSU_ISR_TXNOT_FULL_MASK) != FALSE) &&
		(InstancePtr->TxBytes > 0))
		XQspiPsu_FillTxFifo(InstancePtr, &Msg[MsgCnt], (u32)XQSPIPSU_TXD_DEPTH);
	/*
	 * Check if the entry is ONLY TX and increase MsgCnt.
	 * This is to allow TX and RX together in one entry - corner case.
	 */
	if ((MsgCnt < NumMsg) && ((TxRxFlag & XQSPIPSU_MSG_FLAG_TX) != FALSE) &&
		((QspiPsuStatusReg & XQSPIPSU_ISR_TXEMPTY_MASK) != FALSE) &&
		((QspiPsuStatusReg & XQSPIPSU_ISR_GENFIFOEMPTY_MASK) != FALSE) &&
		(InstancePtr->TxBytes == 0) &&
		((TxRxFlag & XQSPIPSU_MSG_FLAG_RX) == FALSE)) {
		MsgCnt += 1;
		DeltaMsgCnt = 1U;
	}
	if ((InstancePtr->ReadMode == XQSPIPSU_READMODE_DMA) &&
		(MsgCnt < NumMsg) && ((TxRxFlag & XQSPIPSU_MSG_FLAG_RX) != FALSE)) {
		if ((DmaIntrStatusReg &
			XQSPIPSU_QSPIDMA_DST_I_STS_DONE_MASK) != FALSE) {
			if (XQspiPsu_SetIOMode(InstancePtr, &Msg[MsgCnt])) {
				XQspiPsu_GenFifoEntryData(InstancePtr, &Msg[MsgCnt]);
				XQspiPsu_ManualStartEnable(InstancePtr);
			} else {
				InstancePtr->RxBytes = 0;
				MsgCnt += 1;
				DeltaMsgCnt = 1U;
			}
		}
	} else if ((MsgCnt < NumMsg) &&
			((TxRxFlag & XQSPIPSU_MSG_FLAG_RX) != FALSE)) {
		if (InstancePtr->RxBytes != 0) {
			XQspiPsu_IORead(InstancePtr, &Msg[MsgCnt], QspiPsuStatusReg);
			if (InstancePtr->RxBytes == 0) {
				MsgCnt += 1;
				DeltaMsgCnt = 1U;
			}
		}
	}
	/*
	 * Dummy byte transfer
	 * MsgCnt < NumMsg check is to ensure is it a valid dummy cycle message
	 * If one of the above conditions increased MsgCnt, then
	 * the new message is yet to be placed in the FIFO; hence !DeltaMsgCnt.
	 */
	if ((MsgCnt < NumMsg) && (DeltaMsgCnt == FALSE) &&
		((TxRxFlag & XQSPIPSU_MSG_FLAG_RX) == FALSE) &&
		((TxRxFlag & XQSPIPSU_MSG_FLAG_TX) == FALSE) &&
		((TxRxFlag & XQSPIPSU_MSG_FLAG_POLL) == FALSE) &&
		((QspiPsuStatusReg & XQSPIPSU_ISR_GENFIFOEMPTY_MASK) != FALSE)) {
		MsgCnt += 1;
		DeltaMsgCnt = 1U;
	}
	InstancePtr->MsgCnt = MsgCnt;
	/*
	 * DeltaMsgCnt is to handle conditions where genfifo empty can be set
	 * while tx is still not empty or rx dma is not yet done.
	 * MsgCnt > NumMsg indicates CS de-assert entry was also executed.
	 */
	if (((QspiPsuStatusReg & XQSPIPSU_ISR_GENFIFOEMPTY_MASK) != FALSE) &&
		((DeltaMsgCnt != FALSE) || (MsgCnt > NumMsg))) {
		if (MsgCnt < NumMsg) {
			if (InstancePtr->IsUnaligned != 0) {
				InstancePtr->IsUnaligned = 0;
				XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress,
					XQSPIPSU_CFG_OFFSET, (XQspiPsu_ReadReg(
					InstancePtr->Config.BaseAddress, XQSPIPSU_CFG_OFFSET) |
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
			XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress, XQSPIPSU_IDR_OFFSET,
					(u32)XQSPIPSU_IER_TXNOT_FULL_MASK |
					(u32)XQSPIPSU_IER_TXEMPTY_MASK |
					(u32)XQSPIPSU_IER_RXNEMPTY_MASK |
					(u32)XQSPIPSU_IER_GENFIFOEMPTY_MASK |
					(u32)XQSPIPSU_IER_RXEMPTY_MASK);
			if (InstancePtr->ReadMode == XQSPIPSU_READMODE_DMA)
				XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress,
					XQSPIPSU_QSPIDMA_DST_I_DIS_OFFSET,
					XQSPIPSU_QSPIDMA_DST_I_EN_DONE_MASK);

			/* Clear the busy flag. */
			InstancePtr->IsBusy = FALSE;
#if defined  (XCLOCKING)
			Xil_ClockDisable(InstancePtr->Config.RefClk);
#endif
			/* Call status handler to indicate completion */
			InstancePtr->StatusHandler(InstancePtr->StatusRef,
						XST_SPI_TRANSFER_DONE, 0);
		}
	}
	if ((TxRxFlag & XQSPIPSU_MSG_FLAG_POLL) != FALSE)
		XQspiPsu_PollDataHandler(InstancePtr, QspiPsuStatusReg);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * Sets the status callback function, the status handler, which the driver
 * calls when it encounters conditions that should be reported to upper
 * layer software. The handler executes in an interrupt context, so it must
 * minimize the amount of processing performed. One of the following status
 * events is passed to the status handler.
 *
 * <pre>
 *
 * XST_SPI_TRANSFER_DONE		The requested data transfer is done
 *
 * XST_SPI_TRANSMIT_UNDERRUN	As a slave device, the master clocked data
 *				but there were none available in the transmit
 *				register/FIFO. This typically means the slave
 *				application did not issue a transfer request
 *				fast enough, or the processor/driver could not
 *				fill the transmit register/FIFO fast enough.
 *
 * XST_SPI_RECEIVE_OVERRUN	The QSPIPSU device lost data. Data was received
 *				but the receive data register/FIFO was full.
 *
 * </pre>
 * @param	InstancePtr is a pointer to the XQspiPsu instance.
 * @param	CallBackRef is the upper layer callback reference passed back
 *		when the callback function is invoked.
 * @param	FuncPointer is the pointer to the callback function.
 *
 * @return	None.
 *
 * @note
 *
 * The handler is called within interrupt context, so it should do its work
 * quickly and queue potentially time-consuming work to a task-level thread.
 *
 ******************************************************************************/
void XQspiPsu_SetStatusHandler(XQspiPsu *InstancePtr, void *CallBackRef,
				XQspiPsu_StatusHandler FuncPointer)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(FuncPointer != NULL);
	Xil_AssertVoid(CallBackRef != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	InstancePtr->StatusHandler = FuncPointer;
	InstancePtr->StatusRef = CallBackRef;
}

/*****************************************************************************/
/**
 * @brief
 * This API enables/ disables Write Protect pin on the flash parts.
 *
 * @param	QspiPsuPtr is a pointer to the QSPIPSU driver component to use.
 *
 * @param	Toggle is a value of the GPIO pin
 *
 * @return	None
 *
 * @note	By default WP pin as per the QSPI controller is driven High
 *		which means no write protection. Calling this function once
 *		will enable the protection.
 *
 ******************************************************************************/
void XQspiPsu_WriteProtectToggle(const XQspiPsu *InstancePtr, u32 Toggle)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	/* For Single and Stacked flash configuration with x1 or x2 mode*/
	if (InstancePtr->Config.ConnectionMode ==
		XQSPIPSU_CONNECTION_MODE_SINGLE) {
		/* Select slave */
		XQspiPsu_GenFifoEntryCSAssert(InstancePtr);

		XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress,
			XQSPIPSU_GPIO_OFFSET, Toggle);

	} else {
#ifdef DEBUG
		xil_printf("Dual Parallel/Stacked configuration ");
		xil_printf("is not supported by this API\r\n");
#endif
	}
}

/*****************************************************************************/
/**
*
* This function start a DMA transfer.
*
 * @param	InstancePtr is a pointer to the XQspiPsu instance.
 * @param	Msg is a pointer to the structure containing transfer data.
 * @param	NumMsg is the number of messages to be transferred.
 *
 * @return
 *		- XST_SUCCESS if successful.
 *		- XST_FAILURE if ByteCount is greater than
 *		  XQSPIPSU_DMA_BYTES_MAX.
 *		- XST_DEVICE_BUSY if a transfer is already in progress.
 *
 * @note	None.
 *
*
******************************************************************************/
s32 XQspiPsu_StartDmaTransfer(XQspiPsu *InstancePtr, XQspiPsu_Msg *Msg,
				u32 NumMsg)
{
	s32 Index;
	u32 QspiPsuStatusReg = 0;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Msg != NULL);
	Xil_AssertNonvoid(NumMsg > 0);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	for (Index = 0; Index < (s32)NumMsg; Index++) {
		Xil_AssertNonvoid(Msg[Index].ByteCount > 0U);
	}

	/*
	 * Check whether there is another transfer in progress.
	 * Not thread-safe
	 */
	if (InstancePtr->IsBusy == TRUE) {
		return (s32)XST_DEVICE_BUSY;
	}

	/* Check for ByteCount upper limit - 2^28 for DMA */
	for (Index = 0; Index < (s32)NumMsg; Index++) {
		if ((Msg[Index].ByteCount > XQSPIPSU_DMA_BYTES_MAX) &&
		    ((Msg[Index].Flags & XQSPIPSU_MSG_FLAG_RX) != FALSE))
			return (s32)XST_FAILURE;
	}

	/*
	 * Set the busy flag, which will be cleared when the transfer is
	 * entirely done.
	 */
	InstancePtr->IsBusy = TRUE;

	/* Select slave */
	XQspiPsu_GenFifoEntryCSAssert(InstancePtr);
	/* list */
	Index = 0;
	while (Index < (s32)NumMsg) {
		XQspiPsu_GenFifoEntryData(InstancePtr, &Msg[Index]);
		if (InstancePtr->IsManualstart == TRUE) {
#ifdef DEBUG
			xil_printf("\nManual Start\r\n");
#endif
			XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress, XQSPIPSU_CFG_OFFSET,
					  XQspiPsu_ReadReg(InstancePtr->Config.BaseAddress,
					  XQSPIPSU_CFG_OFFSET) |
					  XQSPIPSU_CFG_START_GEN_FIFO_MASK);
		}
		do {
			if((InstancePtr->ReadMode == XQSPIPSU_READMODE_DMA) &&
			   ((Msg[Index].Flags & XQSPIPSU_MSG_FLAG_RX) != FALSE))
				break;

			QspiPsuStatusReg = XQspiPsu_ReadReg(InstancePtr->Config.BaseAddress, XQSPIPSU_ISR_OFFSET);

		}while (((QspiPsuStatusReg & XQSPIPSU_ISR_GENFIFOEMPTY_MASK) == FALSE) ||
			(InstancePtr->TxBytes != 0) ||
			((QspiPsuStatusReg & XQSPIPSU_ISR_TXEMPTY_MASK) == FALSE));

		if(InstancePtr->ReadMode == XQSPIPSU_READMODE_IO) {
			XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress,
					  XQSPIPSU_CFG_OFFSET, (XQspiPsu_ReadReg(
					  InstancePtr->Config.BaseAddress, XQSPIPSU_CFG_OFFSET) |
					  XQSPIPSU_CFG_MODE_EN_DMA_MASK));
			InstancePtr->ReadMode = XQSPIPSU_READMODE_DMA;
		}
		Index++;
	}
	return (s32)XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function check for DMA transfer complete.
*
* @param	InstancePtr is a pointer to the XQspiPsu instance.
*
* @return
*		- XST_SUCCESS if DMA transfer complete.
*		- XST_FAILURE if DMA transfer is not completed.
*
* @note		None.
*
******************************************************************************/
s32 XQspiPsu_CheckDmaDone(XQspiPsu *InstancePtr)
{
	u32 QspiPsuStatusReg;
	u32 DmaIntrSts;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	DmaIntrSts = XQspiPsu_ReadReg(InstancePtr->Config.BaseAddress, XQSPIPSU_QSPIDMA_DST_I_STS_OFFSET);
	if ((DmaIntrSts & XQSPIPSU_QSPIDMA_DST_I_STS_DONE_MASK)	!= FALSE) {
		XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress, XQSPIPSU_QSPIDMA_DST_I_STS_OFFSET, DmaIntrSts);
		/* De-select slave */
		XQspiPsu_GenFifoEntryCSDeAssert(InstancePtr);
		if (InstancePtr->IsManualstart == TRUE) {
#ifdef DEBUG
			xil_printf("\nManual Start\r\n");
#endif
			XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress, XQSPIPSU_CFG_OFFSET,
					  XQspiPsu_ReadReg(InstancePtr->Config.BaseAddress, XQSPIPSU_CFG_OFFSET) |
					  XQSPIPSU_CFG_START_GEN_FIFO_MASK);
		}
		do
			QspiPsuStatusReg = XQspiPsu_ReadReg(InstancePtr->Config.BaseAddress, XQSPIPSU_ISR_OFFSET);
		while ((QspiPsuStatusReg & XQSPIPSU_ISR_GENFIFOEMPTY_MASK) == FALSE);

		/* Clear the busy flag. */
		InstancePtr->IsBusy = FALSE;

		return (s32)XST_SUCCESS;
	}
	else {
		return (s32)XST_FAILURE;
	}

}
/** @} */
