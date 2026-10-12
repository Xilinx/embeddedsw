/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xiicps_slave.c
* @brief Implements the slave mode transfers of the XIicPs driver.
*
* The xiicps_slave.c file handles slave transfers.
*
* <pre> MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --  -------- ---------------------------------------------
* 1.00a jz  01/30/10 First release
* 1.04a kpc 08/30/13 Avoid buffer overwrite in SlaveRecvData function
* 3.00	sk	01/31/15 Modified the code according to MISRAC 2012 Compliant.
* 3.3   kvn 05/05/16 Modified latest code for MISRA-C:2012 Compliance.
* 3.8   ask 08/01/18 Fix for Cppcheck and Doxygen warnings.
* 3.10 sg   06/24/19 Fix for Slave send polled and interruput transfers.
* 3.11  rna 12/20/19 Clear the ISR before enabling interrupts in Send/Receive.
*           12/23/19 Add 10 bit address support for Master/Slave
* 3.11  sd  02/06/20 Added clocking support.
* 3.11  rna 02/12/20 Moved static data transfer functions to xiicps_xfer.c file
*	    02/18/20 Modified latest code for MISRA-C:2012 Compliance.
*       rna 04/09/20 Added timeout as event in slave interrupt handler.
* 3.16  gm  05/10/22 Updated slave mode receive API's by removing byte count
* 		     dependency.
* 3.24  vn  08/26/26 Fix Doxygen warnings
* 3.24  tb  09/30/26 Align Doxygen comments with AMD standards
* 3.24  vlt 10/02/26 Refactor XIicPs_SlaveSendPolled() to reduce CCM.
* </pre>
*
******************************************************************************/

/** @addtogroup iicps_api IICPS APIs
 * @{
 */

/***************************** Include Files *********************************/
#include "xiicps.h"
#include "sleep.h"
#include "xiicps_xfer.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static s32 XIicPs_SlaveWaitForReadRequest(XIicPs *InstancePtr);
static s32 XIicPs_SlaveSendData(XIicPs *InstancePtr);
static s32 XIicPs_WaitForSlaveSendComplete(XIicPs *InstancePtr);

/************************* Variable Definitions *****************************/

/*****************************************************************************/
/**
* @brief
* Waits for the master to initiate a read from the slave.
*
* This function monitors the RXRW status bit until the master starts a read.
* If receive data becomes available, the master has initiated a write instead
* of a read.
*
* @param[in,out] InstancePtr Pointer to the XIicPs instance.
*
* @return
*		- XST_SUCCESS if the master initiates a read.
*		- XST_FAILURE if the master initiates a write.
*
****************************************************************************/
static s32 XIicPs_SlaveWaitForReadRequest(XIicPs *InstancePtr)
{
	u32 StatusReg;
	UINTPTR BaseAddr;
	s32 Error = 0;
	s32 Status = (s32)XST_SUCCESS;
	_Bool Result;

	BaseAddr = InstancePtr->Config.BaseAddress;

	/*
	 * Use RXRW bit in status register to wait for the master
	 * to start a read.
	 */
	StatusReg = XIicPs_ReadReg(BaseAddr, XIICPS_SR_OFFSET);
	Result = ((StatusReg & XIICPS_SR_RXRW_MASK) == (u32)0U);

	while (Result != FALSE) {
		/*
		 * If the master tries to send data to the slave,
		 * it is an error.
		 */
		if ((StatusReg & XIICPS_SR_RXDV_MASK) != (u32)0U) {
			Error = 1;
		}

		StatusReg = XIicPs_ReadReg(BaseAddr, XIICPS_SR_OFFSET);

		Result = (((StatusReg & XIICPS_SR_RXRW_MASK) ==
			   (u32)0U) &&
			  (Error == 0));
	}

	if (Error != 0) {
		Status = (s32)XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief
* Sends slave data through the transmit FIFO.
*
* This function fills the transmit FIFO, waits for the master to read the
* FIFO data, and refills the FIFO until all requested data is sent. It also
* detects if the master terminates the transfer before all data is sent.
*
* @param[in,out] InstancePtr Pointer to the XIicPs instance.
*
* @return
*		- XST_SUCCESS if all data is sent.
*		- XST_FAILURE if the master terminates the transfer early.
*
****************************************************************************/
static s32 XIicPs_SlaveSendData(XIicPs *InstancePtr)
{
	u32 IntrStatusReg;
	u32 StatusReg;
	UINTPTR BaseAddr;
	s32 Tmp;
	s32 BytesToSend;
	s32 Error = 0;
	s32 Status = (s32)XST_SUCCESS;
	_Bool Value;

	BaseAddr = InstancePtr->Config.BaseAddress;

	Value = (InstancePtr->SendByteCount > (s32)0);

	while (Value != FALSE) {
		/*
		 * Find out how many bytes can be sent.
		 */
		BytesToSend = InstancePtr->SendByteCount;

		if (BytesToSend > (s32)XIICPS_FIFO_DEPTH) {
			BytesToSend = (s32)XIICPS_FIFO_DEPTH;
		}

		/*
		 * Fill the transmit FIFO.
		 */
		for (Tmp = 0; Tmp < BytesToSend; Tmp++) {
			XIicPs_SendByte(InstancePtr);
		}

		StatusReg = XIicPs_ReadReg(BaseAddr, XIICPS_SR_OFFSET);

		/*
		 * Wait for the master to read the data from the FIFO.
		 */
		while (((StatusReg & XIICPS_SR_TXDV_MASK) != (u32)0U) &&
		       (Error == 0)) {
			/*
			 * If the master terminates the transfer before all
			 * data is sent, it is an error.
			 */
			IntrStatusReg = XIicPs_ReadReg(BaseAddr,
						      XIICPS_ISR_OFFSET);

			if ((IntrStatusReg & XIICPS_IXR_NACK_MASK) !=
			    (u32)0U) {
				Error = 1;
			}

			/*
			 * Clear the interrupt status register.
			 */
			XIicPs_WriteReg(BaseAddr, XIICPS_ISR_OFFSET,
					IntrStatusReg);

			StatusReg = XIicPs_ReadReg(BaseAddr,
						  XIICPS_SR_OFFSET);
		}

		Value = ((InstancePtr->SendByteCount > (s32)0) &&
			 (Error == 0));
	}

	if (Error != 0) {
		Status = (s32)XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief
* Waits for completion of a slave send transfer.
*
* This function waits for the transfer-complete status and clears the
* interrupt status register after the transfer. It returns failure if the
* completion status is not received before the timeout expires.
*
* @param[in,out] InstancePtr Pointer to the XIicPs instance.
*
* @return
*		- XST_SUCCESS if the transfer completes.
*		- XST_FAILURE if the transfer times out.
*
****************************************************************************/
static s32 XIicPs_WaitForSlaveSendComplete(XIicPs *InstancePtr)
{
	UINTPTR BaseAddr;
	volatile u32 RegValue = 0U;
	u32 Timeout = XIICPS_POLL_DEFAULT_TIMEOUT_VAL;
	s32 Status = (s32)XST_SUCCESS;

	BaseAddr = InstancePtr->Config.BaseAddress;

	/*
	 * Wait for transfer completion.
	 */
	while (Timeout != 0U) {
		RegValue = XIicPs_ReadReg(BaseAddr, XIICPS_ISR_OFFSET);

		if ((RegValue & XIICPS_IXR_COMP_MASK) ==
		    XIICPS_IXR_COMP_MASK) {
			break;
		}

		usleep(1000U);
		Timeout--;
	}

	if (Timeout == 0U) {
		Status = (s32)XST_FAILURE;
	}

	/*
	 * Clear the interrupt status register.
	 */
	XIicPs_WriteReg(BaseAddr, XIICPS_ISR_OFFSET, RegValue);

	return Status;
}

/*****************************************************************************/
/**
* @brief Sets up the device to be a slave.
*
* @param[in,out] InstancePtr Pointer to the XIicPs instance.
* @param[in]     SlaveAddr Address of the slave we are receiving from.
*
* @note
* Interrupt is always enabled whether the transfer is interrupt-driven
* or polled mode. The device interruption depends
* on whether the device is connected to an interrupt
* controller and interrupt for the device is enabled.
*
****************************************************************************/
void XIicPs_SetupSlave(XIicPs *InstancePtr, u16 SlaveAddr)
{
	u32 ControlReg;
	UINTPTR BaseAddr;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);
	Xil_AssertVoid((u16)XIICPS_ADDR_MASK >= SlaveAddr);

#if defined  (XCLOCKING)
	if (InstancePtr->IsClkEnabled == 0) {
		Xil_ClockEnable(InstancePtr->Config.RefClk);
		InstancePtr->IsClkEnabled = 1;
	}
#endif

	BaseAddr = InstancePtr->Config.BaseAddress;

	ControlReg = XIicPs_In32(BaseAddr + XIICPS_CR_OFFSET);

	/*
	 * Set up master, AckEn and also clear fifo.
	 */
	ControlReg |= (u32)XIICPS_CR_ACKEN_MASK | (u32)XIICPS_CR_CLR_FIFO_MASK;
	ControlReg &= ~((u32)XIICPS_CR_MS_MASK);

	XIicPs_WriteReg(BaseAddr, XIICPS_CR_OFFSET,
			  ControlReg);

	XIicPs_DisableAllInterrupts(BaseAddr);

	XIicPs_WriteReg(InstancePtr->Config.BaseAddress,
			  XIICPS_ADDR_OFFSET, (u32)SlaveAddr);

	return;
}

/*****************************************************************************/
/**
* @brief Sets up a slave interrupt-driven send.
*
* It sets the repeated start for the device with a transfer size larger than
* FIFO depth. The interrupt handler initiates data processing for the send.
*
* @param[in,out] InstancePtr Pointer to the XIicPs instance.
* @param[in]     MsgPtr Pointer to the send buffer.
* @param[in]     ByteCount Number of bytes to be sent.
*
* @note          This send routine is for interrupt-driven transfer only.
*
****************************************************************************/
void XIicPs_SlaveSend(XIicPs *InstancePtr, u8 *MsgPtr, s32 ByteCount)
{
	UINTPTR BaseAddr;

	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(MsgPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);


	BaseAddr = InstancePtr->Config.BaseAddress;
	InstancePtr->SendBufferPtr = MsgPtr;
	InstancePtr->SendByteCount = ByteCount;
	InstancePtr->RecvBufferPtr = NULL;

	/*
	 * Clear the interrupt status register.
	 */
	XIicPs_WriteReg(BaseAddr, XIICPS_ISR_OFFSET, XIICPS_IXR_ALL_INTR_MASK);

	XIicPs_EnableInterrupts(BaseAddr,
			(u32)XIICPS_IXR_DATA_MASK | (u32)XIICPS_IXR_COMP_MASK |
			(u32)XIICPS_IXR_TO_MASK | (u32)XIICPS_IXR_NACK_MASK |
			(u32)XIICPS_IXR_TX_OVR_MASK);
}

/*****************************************************************************/
/**
* @brief Sets up a slave interrupt-driven receive.
*
* The interrupt handler handles data processing for the receive.
*
* @param[in,out] InstancePtr Pointer to the XIicPs instance.
* @param[out]    MsgPtr Pointer to the receive buffer.
* @param[in]     ByteCount Number of bytes to be received.
*
* @note          This routine is for interrupt-driven transfer only.
*
****************************************************************************/
void XIicPs_SlaveRecv(XIicPs *InstancePtr, u8 *MsgPtr, s32 ByteCount)
{
	UINTPTR BaseAddr;

	(void)ByteCount;

	/*
	 * Assert validates the input arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(MsgPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);


	BaseAddr = InstancePtr->Config.BaseAddress;
	InstancePtr->RecvBufferPtr = MsgPtr;
	InstancePtr->SendBufferPtr = NULL;

	/*
	 * Clear the interrupt status register.
	 */
	XIicPs_WriteReg(BaseAddr, XIICPS_ISR_OFFSET, XIICPS_IXR_ALL_INTR_MASK);

	XIicPs_EnableInterrupts(BaseAddr,
		(u32)XIICPS_IXR_DATA_MASK | (u32)XIICPS_IXR_COMP_MASK |
		(u32)XIICPS_IXR_NACK_MASK | (u32)XIICPS_IXR_TO_MASK |
		(u32)XIICPS_IXR_RX_OVR_MASK | (u32)XIICPS_IXR_RX_UNF_MASK);

}

/*****************************************************************************/
/**
* @brief Sends a buffer in polled mode as a slave.
*
* @param[in,out] InstancePtr Pointer to the XIicPs instance.
* @param[in]     MsgPtr Pointer to the send buffer.
* @param[in]     ByteCount Number of bytes to be sent.
*
* @return
*                - XST_SUCCESS if everything went well.
*                - XST_FAILURE if master sends us data or master terminates
*                  the transfer before all data has been sent out.
*
* @note          This send routine is for polled mode transfer only.
*
****************************************************************************/
s32 XIicPs_SlaveSendPolled(XIicPs *InstancePtr, u8 *MsgPtr, s32 ByteCount)
{
	u32 IntrStatusReg;
	UINTPTR BaseAddr;
	s32 SendStatus = (s32)XST_SUCCESS;
	s32 Status;

	/*
	 * Assert validates the input arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(MsgPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

	BaseAddr = InstancePtr->Config.BaseAddress;
	InstancePtr->SendBufferPtr = MsgPtr;
	InstancePtr->SendByteCount = ByteCount;

	/*
	 * Wait for the master to initiate a read.
	 */
	Status = XIicPs_SlaveWaitForReadRequest(InstancePtr);

	if (Status == (s32)XST_SUCCESS) {

		/*
		 * Clear the interrupt status register.
		 */
		IntrStatusReg = XIicPs_ReadReg(BaseAddr, XIICPS_ISR_OFFSET);
		XIicPs_WriteReg(BaseAddr, XIICPS_ISR_OFFSET, IntrStatusReg);

		/*
		 * Send data through the transmit FIFO.
		 */
		SendStatus = XIicPs_SlaveSendData(InstancePtr);

		/*
		 * Wait for transfer completion and clear the status, even
		 * if sending the data failed.
		 */
		Status = XIicPs_WaitForSlaveSendComplete(InstancePtr);

		/*
		 * A failure while sending the data is reported as failure.
		 */
		if (SendStatus != (s32)XST_SUCCESS) {
			Status = (s32)XST_FAILURE;
		}
	}

	return Status;
}
/*****************************************************************************/
/**
* @brief Receives a buffer in polled mode as a slave.
*
* @param[in,out] InstancePtr Pointer to the XIicPs instance.
* @param[out]    MsgPtr Pointer to the receive buffer.
* @param[in]     ByteCount Number of bytes to be received.
*
* @return
*                - XST_SUCCESS if everything went well.
*                - XST_FAILURE if timed out.
*
* @note          This receive routine is for polled mode transfer only.
*
****************************************************************************/
s32 XIicPs_SlaveRecvPolled(XIicPs *InstancePtr, u8 *MsgPtr, s32 ByteCount)
{
	u32 IntrStatusReg;
	u32 StatusReg;
	UINTPTR BaseAddr;
	u32 RecvCompleteState=0;

	(void)ByteCount;

	/*
	 * Assert validates the input arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(MsgPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

	BaseAddr = InstancePtr->Config.BaseAddress;
	InstancePtr->RecvBufferPtr = MsgPtr;

	/*
	 * Clear the interrupt status register.
	 */
	IntrStatusReg = XIicPs_ReadReg(BaseAddr, XIICPS_ISR_OFFSET);
	XIicPs_WriteReg(BaseAddr, XIICPS_ISR_OFFSET, IntrStatusReg);

	/*
	 * Clear the status register.
	 */
	StatusReg = XIicPs_ReadReg(BaseAddr, XIICPS_SR_OFFSET);
	XIicPs_WriteReg(BaseAddr, XIICPS_SR_OFFSET, StatusReg);

	StatusReg = XIicPs_ReadReg(BaseAddr, XIICPS_SR_OFFSET);

	while (RecvCompleteState == 0U) {

		/* Wait for master to put data */
		IntrStatusReg = XIicPs_ReadReg(BaseAddr,XIICPS_ISR_OFFSET);

		while((IntrStatusReg & (XIICPS_IXR_DATA_MASK | XIICPS_IXR_COMP_MASK)) == 0x0U){

			/*
			 * If master terminates the transfer before we get all
			 * the data or the master tries to read from us,
			 * it is an error.
			 */
			IntrStatusReg = XIicPs_ReadReg(BaseAddr,
						XIICPS_ISR_OFFSET);
			StatusReg = XIicPs_ReadReg(BaseAddr, XIICPS_SR_OFFSET);

			if (((IntrStatusReg & (XIICPS_IXR_DATA_MASK | XIICPS_IXR_COMP_MASK)) != 0x0U) &&
				((StatusReg & XIICPS_SR_RXDV_MASK) == 0U)) {

				return (s32)XST_FAILURE;
			}

			/*
			 * Clear the interrupt status register.
			 */
			XIicPs_WriteReg(BaseAddr, XIICPS_ISR_OFFSET,
			IntrStatusReg);
		}

		/*
		 * Read all data from FIFO.
		 */
		while ((StatusReg & XIICPS_SR_RXDV_MASK) != 0x0U) {

			if ((IntrStatusReg & XIICPS_IXR_COMP_MASK) !=0x0U){
				RecvCompleteState = 1;
			}

			XIicPs_RecvByte(InstancePtr);

			StatusReg = XIicPs_ReadReg(BaseAddr,
				XIICPS_SR_OFFSET);
		}
	}

	return (s32)XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Handles interrupts for slave mode.
*
* The interrupt handler for slave mode does the protocol handling for
* the interrupt-driven transfers.
*
* Completion events and errors are signaled to upper layer for proper
* handling.
*
* The interrupts that are handled are:
* - DATA:
*	If the instance is sending, it means that the master wants to read more
*	data from us. Send more data, and check whether the send is complete.
*
*	If the instance is receiving, it means that the master has written
*	more data to us. Receive more data, and check whether the receive is
*	complete.
*
* - COMP:
*	This marks that stop sequence has been sent from the master, transfer
*	is about to terminate. However, for receiving, the master may have
*	written us some data, so receive that first.
*
*	It is an error if the amount of transferred data is less than expected.
*
* - NAK:
*	This marks that master does not want our data. It is for send only.
*
* - Other interrupts:
*	These interrupts are marked as error.
*
* @param[in,out] InstancePtr Pointer to the XIicPs instance.
*
****************************************************************************/
void XIicPs_SlaveInterruptHandler(XIicPs *InstancePtr)
{
	u32 IntrStatusReg;
	u32 IsSend = 0U;
	u32 StatusEvent = 0U;
	UINTPTR BaseAddr;

	/*
	 * Assert validates the input arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

	BaseAddr = InstancePtr->Config.BaseAddress;

	/*
	 * Read the Interrupt status register.
	 */
	IntrStatusReg = XIicPs_ReadReg(BaseAddr, XIICPS_ISR_OFFSET);

	/*
	 * Write the status back to clear the interrupts so no events are missed
	 * while processing this interrupt.
	 */
	XIicPs_WriteReg(BaseAddr, XIICPS_ISR_OFFSET, IntrStatusReg);

	/*
	 * Use the Mask register AND with the Interrupt Status register so
	 * disabled interrupts are not processed.
	 */
	IntrStatusReg &= ~(XIicPs_ReadReg(BaseAddr, XIICPS_IMR_OFFSET));

	/*
	 * Determine whether the device is sending.
	 */
	if (InstancePtr->RecvBufferPtr == NULL) {
		IsSend = 1U;
	}

	/* Data interrupt
	 *
	 * This means master wants to do more data transfers.
	 * Also check for completion of transfer, signal upper layer if done.
	 */
	if ((u32)0U != (IntrStatusReg & XIICPS_IXR_DATA_MASK)) {
		if (IsSend != 0x0U) {
			(void)TransmitFifoFill(InstancePtr);
		} else {
			XIicPs_WriteReg(BaseAddr, (u32)XIICPS_CR_OFFSET,
						XIicPs_ReadReg(BaseAddr, (u32)XIICPS_CR_OFFSET) |
									(u32)XIICPS_CR_HOLD_MASK);

			(void)SlaveRecvData(InstancePtr);
		}
	}

	/*
	 * Complete interrupt.
	 *
	 * In slave mode, it means the master has done with this transfer, so
	 * we signal the application using completion event.
	 */
	if (0U != (IntrStatusReg & XIICPS_IXR_COMP_MASK)) {
		if (IsSend != 0x0U) {
			if (InstancePtr->SendByteCount > 0) {
				StatusEvent |= XIICPS_EVENT_ERROR;
			}else {
				StatusEvent |= XIICPS_EVENT_COMPLETE_SEND;
			}
		} else {
			(void)SlaveRecvData(InstancePtr);
			if ((InstancePtr)->RecvByteCount == 0) {
				StatusEvent |= XIICPS_EVENT_ERROR;
			} else {
				StatusEvent |= XIICPS_EVENT_COMPLETE_RECV;
			}
		}
	}

	/*
	 * Nack interrupt, pass this information to application.
	 */
	if (0U != (IntrStatusReg & XIICPS_IXR_NACK_MASK)) {
		StatusEvent |= XIICPS_EVENT_NACK;
	}

	/*
	 * Timeout interrupt, pass this information to application.
	 */
	if (0U != (IntrStatusReg & XIICPS_IXR_TO_MASK)) {
		XIicPs_DisableInterrupts(BaseAddr, XIICPS_IXR_TO_MASK);
		StatusEvent |= XIICPS_EVENT_TIME_OUT;
	}

	/*
	 * All other interrupts are treated as error.
	 */
	if (0U != (IntrStatusReg & (XIICPS_IXR_RX_UNF_MASK |
				XIICPS_IXR_TX_OVR_MASK |
				XIICPS_IXR_RX_OVR_MASK))){

		StatusEvent |= XIICPS_EVENT_ERROR;
	}

	/*
	 * Signal application if there are any events.
	 */
	if ((u32)0U != StatusEvent) {
		InstancePtr->StatusHandler(InstancePtr->CallBackRef,
					   StatusEvent);
	}
}
/** @} */
