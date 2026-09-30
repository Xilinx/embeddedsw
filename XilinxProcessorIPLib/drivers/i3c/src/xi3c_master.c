/******************************************************************************
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xi3c_master.c
*
* @brief Implements I3C master transfer APIs.
*
* <pre> MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---  -------- ---------------------------------------------
* 1.00  gm  02/09/24 First release
* 1.3   vlt 01/30/26 Fixed codespell errors.
* 1.4   vn  07/30/26 Fix Doxygen warnings
* 1.4   tb  09/21/26 Align Doxygen comments with AMD documentation standards
* </pre>
*
******************************************************************************/

/**
* @addtogroup i3c_api I3C APIs
* @{
*/

/***************************** Include Files *********************************/

#include "xil_util.h"
#include "xi3c.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/*****************************************************************************/
/**
* @brief Reads the response.
*
* It waits for the response from slave and reads the response.
*
* @param[in]	InstancePtr Pointer to the XI3c instance.
*
* @return
*		- Response code on success.
*		- XST_TIMEOUT on timeout.
*
****************************************************************************/
static s32 XI3c_GetResponse(XI3c *InstancePtr)
{
	s32 Status;
	u32 ResponseData;

	Status = (int)Xil_WaitForEvent(((InstancePtr->Config.BaseAddress) +
					XI3C_SR_OFFSET),
				       XI3C_SR_RESP_NOT_EMPTY_MASK, XI3C_SR_RESP_NOT_EMPTY_MASK,
				       TIMEOUT_COUNTER);
	if (Status != XST_SUCCESS) {
		return XST_TIMEOUT;
	}

	ResponseData = XI3c_ReadReg(InstancePtr->Config.BaseAddress,
				    XI3C_RESP_STATUS_FIFO_OFFSET);

	/*
	 * Return response code
	 */
	return  (ResponseData & XI3C_RESP_CODE_MASK) >> XI3C_RESP_CODE_SHIFT;
}

/*****************************************************************************/
/**
* @brief Sends the command.
*
* @param[in,out]	InstancePtr Pointer to the XI3c instance.
* @param[in,out]	Cmd Pointer to the XI3c_Cmd.
* @param[in]	Data The command value.
*
* @return
*		- XST_SUCCESS if everything went well.
*		- XST_FAILURE if any error.
*
******************************************************************************/
s32 XI3c_SendTransferCmd(XI3c *InstancePtr, XI3c_Cmd *Cmd, u8 Data)
{
	/* Assert the arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Cmd != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

	/* Write command value to WR FIFO */
	InstancePtr->SendBufferPtr = &Data;
	InstancePtr->SendByteCount = 1;
	XI3c_WriteTxFifo(InstancePtr);

	/* Fill Command FIFO */
	Cmd->SlaveAddr = XI3C_BROADCAST_ADDRESS;	/**< Broadcast address */
	Cmd->Rw = 0;					/**< Write operation */
	Cmd->ByteCount = 1;				/**< SIze of the command is 8 bits */

	XI3c_FillCmdFifo(InstancePtr, Cmd);

	if (XI3c_GetResponse(InstancePtr))
		return XST_SEND_ERROR;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Initiates an interrupt mode send in master mode.
*
* It sends data to the FIFO and waits for the slave to pick them up.
* If master fails to send data due arbitration lost or any other,
* stops the transfer with status.
*
* @param[in,out]	InstancePtr Pointer to the XI3c instance.
* @param[in,out]	Cmd Pointer to the XI3c_Cmd instance.
* @param[in]	MsgPtr Pointer to the send buffer.
* @param[in]	ByteCount The number of bytes to be sent.
*
* @return
*		- XST_SUCCESS if everything went well.
*		- XST_SEND_ERROR if byte count exceeds max.
*		- XST_NO_DATA if MsgPtr is NULL.
*
****************************************************************************/
s32 XI3c_MasterSend(XI3c *InstancePtr, XI3c_Cmd *Cmd, u8 *MsgPtr, u16 ByteCount)
{
	u16 WrFifoSpace;
	u16 SpaceIndex;

	/*
         * Assert validates the input arguments.
         */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Cmd != NULL);

	if (MsgPtr == NULL)
		return XST_NO_DATA;

	if (ByteCount > XI3C_MAXDATA_LENGTH)
		return XST_SEND_ERROR;

	InstancePtr->SendBufferPtr = MsgPtr;
	InstancePtr->SendByteCount = ByteCount;

	Cmd->ByteCount = ByteCount;	/**< Controller uses byte count during bus transaction */
	Cmd->Rw = 0;			/**< Write operation */

	WrFifoSpace = XI3c_WrFifoLevel(InstancePtr);

	for (SpaceIndex = 0; (SpaceIndex < WrFifoSpace) && (InstancePtr->SendByteCount > 0); SpaceIndex++) {
		/* Fill data */
		XI3c_WriteTxFifo(InstancePtr);
	}

	/*
	 * Enable TX Falling edge interrupt to get TX empty interrupt
	 */
	if (InstancePtr->Config.WrThreshold < ByteCount) {
		XI3c_EnableFEInterrupts(InstancePtr->Config.BaseAddress,
					XI3C_INTR_WR_FIFO_ALMOST_FULL_MASK);
	}

	/*
	 * Enable response fifo not empty interrupt
	 */
	XI3c_EnableREInterrupts(InstancePtr->Config.BaseAddress,
				XI3C_INTR_RESP_NOT_EMPTY_MASK);

	/* Fill command to trigger transfer */
	XI3c_FillCmdFifo(InstancePtr, Cmd);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Initiates an interrupt mode receive in master mode.
*
* It sets the transfer size register so the slave can send data to us.
* The rest of the work is managed by interrupt handler.
*
* @param[in,out]	InstancePtr Pointer to the XI3c instance.
* @param[in,out]	Cmd Pointer to the XI3c_Cmd instance.
* @param[out]	MsgPtr Pointer to the recv buffer.
* @param[in]	ByteCount The number of bytes to be received.
*
* @return
*		- XST_SUCCESS if everything went well.
*		- XST_RECV_ERROR if byte count exceeds max.
*		- XST_NO_DATA if MsgPtr is NULL.
*
****************************************************************************/
s32 XI3c_MasterRecv(XI3c *InstancePtr, XI3c_Cmd *Cmd, u8 *MsgPtr, u16 ByteCount)
{
	/*
         * Assert validates the input arguments.
         */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Cmd != NULL);

	if (MsgPtr == NULL)
		return XST_NO_DATA;

	if (ByteCount > XI3C_MAXDATA_LENGTH)
		return XST_RECV_ERROR;

	InstancePtr->RecvBufferPtr = MsgPtr;
	InstancePtr->RecvByteCount = ByteCount;
	Cmd->ByteCount = ByteCount;		/**< Controller uses byte count during bus transaction */
	Cmd->Rw = 1;	/**< Read operation */

	/*
	 * Enable RX FULL and response fifo not empty raising edge interrupts
	 */
	XI3c_EnableREInterrupts(InstancePtr->Config.BaseAddress,
				XI3C_INTR_RD_FULL_MASK |
				XI3C_INTR_RESP_NOT_EMPTY_MASK);

	/*
	 * Fill command fifo
	 */
	XI3c_FillCmdFifo(InstancePtr, Cmd);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Initiates a polled mode send in master mode.
*
* It sends data to the FIFO and waits for the slave to pick them up.
* If master fails to send data due arbitration lost or any other error,
* stops the transfer with status.
*
* @param[in,out]	InstancePtr Pointer to the XI3c instance.
* @param[in,out]	Cmd Pointer to the XI3c_Cmd instance.
* @param[in]	MsgPtr Pointer to the send buffer.
* @param[in]	ByteCount The number of bytes to be sent.
*
* @return
*		- XST_SUCCESS if everything went well.
*		- XST_SEND_ERROR if any error.
*
****************************************************************************/
s32 XI3c_MasterSendPolled(XI3c *InstancePtr, XI3c_Cmd *Cmd, u8 *MsgPtr, u16 ByteCount)
{
	u16 WrFifoSpace;
	u16 SpaceIndex;

	/*
         * Assert validates the input arguments.
         */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(Cmd != NULL);

	if (MsgPtr == NULL)
		return XST_NO_DATA;

	if (ByteCount > XI3C_MAXDATA_LENGTH)
		return XST_SEND_ERROR;

	InstancePtr->SendBufferPtr = MsgPtr;
	InstancePtr->SendByteCount = ByteCount;

	Cmd->ByteCount = ByteCount;	/**< Controller uses byte count during bus transaction */
	Cmd->Rw = 0;			/**< Write operation */

	/*
	 * Fill command fifo
	 */

	XI3c_FillCmdFifo(InstancePtr, Cmd);

	while (InstancePtr->SendByteCount > 0){
		WrFifoSpace = XI3c_WrFifoLevel(InstancePtr);

		for (SpaceIndex = 0; (SpaceIndex < WrFifoSpace) && (InstancePtr->SendByteCount > 0); SpaceIndex++) {
			XI3c_WriteTxFifo(InstancePtr);
		}
	}

	if (XI3c_GetResponse(InstancePtr))
		return XST_SEND_ERROR;
	else
		return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Initiates a polled mode receive in master mode.
*
* It repeatedly sets the transfer size register so the slave can
* send data to us. It polls the data register for data to come in.
* If master fails to read data due arbitration lost or any other error,
* returns with status.
*
* @param[in,out]	InstancePtr Pointer to the XI3c instance.
* @param[in,out]	Cmd Pointer to the XI3c_Cmd instance.
* @param[out]	MsgPtr Pointer to the recv buffer.
* @param[in]	ByteCount The number of bytes to be received.
*
* @return
*		- XST_SUCCESS if everything went well.
*		- XST_RECV_ERROR if any error.
*
****************************************************************************/
s32 XI3c_MasterRecvPolled(XI3c *InstancePtr, XI3c_Cmd *Cmd, u8 *MsgPtr, u16 ByteCount)
{
	u16 DataIndex;
	u16 RxDataAvailable;

	/*
         * Assert validates the input arguments.
         */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(Cmd != NULL);

	if (MsgPtr == NULL)
		return XST_NO_DATA;

	if (ByteCount > XI3C_MAXDATA_LENGTH)
		return XST_RECV_ERROR;

	InstancePtr->RecvBufferPtr = MsgPtr;

	if (Cmd->SlaveAddr == XI3C_BROADCAST_ADDRESS)	/**< DAA case */
		InstancePtr->RecvByteCount = ByteCount-1;
	else
		InstancePtr->RecvByteCount = ByteCount;

	Cmd->ByteCount = ByteCount;		/**< Controller uses byte count during bus transaction */
	Cmd->Rw = 1;				/**< Read operation */

	/*
	 * Fill command fifo
	 */
	XI3c_FillCmdFifo(InstancePtr, Cmd);

	while (InstancePtr->RecvByteCount > 0) {
		RxDataAvailable = XI3c_RdFifoLevel(InstancePtr);

		for (DataIndex = 0; (DataIndex < RxDataAvailable) && (InstancePtr->RecvByteCount > 0); DataIndex++) {
			XI3c_ReadRxFifo(InstancePtr);
		}
	}

	if (XI3c_GetResponse(InstancePtr))
		return XST_RECV_ERROR;
	else
		return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Reads the Rx Fifo during IBI.
*
* @param[in,out]	InstancePtr Pointer to the XI3c instance.
*
****************************************************************************/
static void XI3c_IbiReadRxFifo(XI3c *InstancePtr)
{
	u16 DataIndex;
	u16 RxDataAvailable;

	RxDataAvailable = XI3c_RdFifoLevel(InstancePtr);

	for (DataIndex = 0; DataIndex < RxDataAvailable; DataIndex++) {
		InstancePtr->RecvByteCount = 4;
		XI3c_ReadRxFifo(InstancePtr);
	}
}

/*****************************************************************************/
/**
*
* @brief Sets the status handler for the driver.
*
* The driver calls the handler when it encounters conditions that should be
* reported to the higher layer software. The handler executes in an interrupt
* context, so the amount of processing should be minimized.
*
* Refer to the xi3c.h file for a list of events. The events are defined
* to start with XI3C_EVENT_*.
*
* @param[in,out]	InstancePtr Pointer to the XI3c instance.
* @param[in]	FunctionPtr Pointer to the callback function.
*
* @note The handler is called within interrupt context, so it should finish its
* work quickly.
*
******************************************************************************/
void XI3c_SetStatusHandler(XI3c *InstancePtr, XI3c_IntrHandler FunctionPtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(FunctionPtr != NULL);
        Xil_AssertVoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

	InstancePtr->StatusHandler = FunctionPtr;
}

/*****************************************************************************/
/**
* @brief	Handles interrupts in master mode.
*
* It does the protocol handling for the interrupt-driven transfers.
*
* If the Master is receiving data then the data is read from the FIFO and
* the Master has to request for more data (if there is more data to
* receive). If all the data has been received then a completion event
* is signalled to the upper layer by calling the callback handler.
* It is an error if the amount of received data is more than expected.
*
* @param[in,out]	InstancePtr Pointer to the XI3c instance.
*
****************************************************************************/
void XI3c_MasterInterruptHandler(XI3c *InstancePtr)
{
	u32 IntrStatusReg;
	u16 WrFifoSpace;
        u16 SpaceIndex;
	u16 DataIndex;
	u16 RxDataAvailable;
	u32 ResponseData;
	u8 DynaAddr[1];

	/*
	 * Assert validates the input arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);

	/*
	 * Read the Raising edge and faling edge Interrupt status registers.
	 */
	IntrStatusReg = XI3c_ReadReg(InstancePtr->Config.BaseAddress, XI3C_INTR_STATUS_OFFSET);

	/*
	 * Write the status back to clear the interrupts so no events are
	 * missed while processing this interrupt.
	 */
	XI3c_WriteReg(InstancePtr->Config.BaseAddress, (u32)XI3C_INTR_STATUS_OFFSET, IntrStatusReg);

	/*
	 * Hot Join
	 */
	if (IntrStatusReg & XI3C_INTR_HJ_MASK) {
		/* Drain stale FIFO data and resume controller before DAA */
		XI3c_ResetFifos(InstancePtr);
		XI3c_Resume(InstancePtr);

		if(InstancePtr->CurDeviceCount <= XI3C_MAXDAACOUNT) {
			DynaAddr[0] = XI3C_DynaAddrList[InstancePtr->CurDeviceCount];
			XI3c_DynaAddrAssign(InstancePtr, DynaAddr, 1);
			XI3c_UpdateAddrBcr(InstancePtr, InstancePtr->CurDeviceCount-1);
		}
		/*
		 * Clear fifos, as data in fifos unused in hot join case.
		 */
		XI3c_ResetFifos(InstancePtr);
	}

	/*
	 * IBI
	 */
	if (IntrStatusReg & XI3C_INTR_IBI_MASK) {
		while (XI3c_RxFifoNotEmpty(InstancePtr) || !XI3c_RespFifoNotEmpty(InstancePtr))
			XI3c_IbiReadRxFifo(InstancePtr);

		XI3c_DisableREInterrupts(InstancePtr->Config.BaseAddress, XI3C_INTR_IBI_MASK);
	}


	/* Tx empty */
	if (IntrStatusReg & XI3C_INTR_WR_FIFO_ALMOST_FULL_MASK) {
		WrFifoSpace = XI3c_WrFifoLevel(InstancePtr);

		for (SpaceIndex = 0; (SpaceIndex < WrFifoSpace) && (InstancePtr->SendByteCount > 0); SpaceIndex++) {
			XI3c_WriteTxFifo(InstancePtr);
		}

		if (InstancePtr->SendByteCount <= 0)
			XI3c_DisableFEInterrupts(InstancePtr->Config.BaseAddress,
						 XI3C_INTR_WR_FIFO_ALMOST_FULL_MASK);
	}

	/*Rx full */
	if (IntrStatusReg & XI3C_INTR_RD_FULL_MASK) {
		RxDataAvailable = XI3c_RdFifoLevel(InstancePtr);

		for (DataIndex = 0; (DataIndex < RxDataAvailable) && (InstancePtr->RecvByteCount > 0); DataIndex++) {
			XI3c_ReadRxFifo(InstancePtr);
		}

		if (InstancePtr->RecvByteCount <= 0)
			XI3c_DisableREInterrupts(InstancePtr->Config.BaseAddress,
						 XI3C_INTR_RD_FULL_MASK);
	}

	/* Response */
	if (IntrStatusReg & XI3C_INTR_RESP_NOT_EMPTY_MASK) {

		if (InstancePtr->RecvByteCount > 0) {
			RxDataAvailable = XI3c_RdFifoLevel(InstancePtr);

			for (DataIndex = 0; (DataIndex < RxDataAvailable) && (InstancePtr->RecvByteCount > 0); DataIndex++) {
				XI3c_ReadRxFifo(InstancePtr);
			}
		}
		/*
		 * Check and read if Rx FIFO has any data during IBI
		 */
		if (InstancePtr->Config.IbiCapable)
			XI3c_IbiReadRxFifo(InstancePtr);

		ResponseData = XI3c_ReadReg(InstancePtr->Config.BaseAddress, XI3C_RESP_STATUS_FIFO_OFFSET);
		InstancePtr->Error = ((ResponseData & XI3C_RESP_CODE_MASK) >> XI3C_RESP_CODE_SHIFT);

		XI3c_DisableREInterrupts(InstancePtr->Config.BaseAddress,
					 XI3C_INTR_RESP_NOT_EMPTY_MASK |
					 XI3C_INTR_RD_FULL_MASK);

		XI3c_DisableFEInterrupts(InstancePtr->Config.BaseAddress,
					 XI3C_INTR_WR_FIFO_ALMOST_FULL_MASK);

		InstancePtr->StatusHandler(InstancePtr->Error);
	}
}

/*****************************************************************************/
/**
* @brief Sets up a receive during IBI in interrupt mode.
*
* It enables the required interrupts for performing read operation during IBI.
*
* @param[in,out]	InstancePtr Pointer to the XI3c instance.
* @param[out]	MsgPtr Pointer to the recv buffer.
*
* @return
*		- XST_SUCCESS if everything went well.
*		- XST_NO_DATA if message buffer is NULL.
*
****************************************************************************/
s32 XI3c_IbiRecv(XI3c *InstancePtr, u8 *MsgPtr)
{
	/*
         * Assert validates the input arguments.
         */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(MsgPtr != NULL);

	if (MsgPtr == NULL)
		return XST_NO_DATA;

	InstancePtr->RecvBufferPtr = MsgPtr;

	/*
	 * Enable response fifo not empty and read fifo not empty
	 * raising edge interrupts.
	 */
	XI3c_EnableREInterrupts(InstancePtr->Config.BaseAddress,
				XI3C_INTR_IBI_MASK		|
				XI3C_INTR_RESP_NOT_EMPTY_MASK);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Receives data during IBI in polled mode.
*
* It polls the data register for data to come in during IBI.
* If master fails to read data due to any error, it returns with status.
*
* @param[in,out]	InstancePtr Pointer to the XI3c instance.
* @param[out]	MsgPtr Pointer to the recv buffer.
*
* @return
*		- XST_SUCCESS if everything went well.
*		- XST_RECV_ERROR if any error.
*
****************************************************************************/
s32 XI3c_IbiRecvPolled(XI3c *InstancePtr, u8 *MsgPtr)
{
	s32 Status;
	u16 DataIndex;
	u16 RxDataAvailable;

	/*
         * Assert validates the input arguments.
         */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(MsgPtr != NULL);

	if (MsgPtr == NULL)
		return XST_NO_DATA;

	InstancePtr->RecvBufferPtr = MsgPtr;

	/*
	 * Wait till Rx Fifo has data with timeout
	 */
	Status = (int)Xil_WaitForEvent(((InstancePtr->Config.BaseAddress) +
					XI3C_SR_OFFSET),
				       XI3C_SR_RD_FIFO_NOT_EMPTY_MASK, XI3C_SR_RD_FIFO_NOT_EMPTY_MASK,
				       TIMEOUT_COUNTER*10);
	if (Status != XST_SUCCESS) {
		goto out;
	}

	while (XI3c_RxFifoNotEmpty(InstancePtr) || !XI3c_RespFifoNotEmpty(InstancePtr)) {
		RxDataAvailable = XI3c_RdFifoLevel(InstancePtr);

		for (DataIndex = 0; DataIndex < RxDataAvailable; DataIndex++) {
			InstancePtr->RecvByteCount = 4;
			XI3c_ReadRxFifo(InstancePtr);
		}
	}

	/*
	 * Check and read if Rx FIFO has any data during response
	 */
	RxDataAvailable = XI3c_RdFifoLevel(InstancePtr);

	for (DataIndex = 0; DataIndex < RxDataAvailable; DataIndex++) {
		InstancePtr->RecvByteCount = 4;
		XI3c_ReadRxFifo(InstancePtr);
	}

out:
	if (XI3c_GetResponse(InstancePtr))
		return XST_RECV_ERROR;
	else
		return XST_SUCCESS;
}

/** @} */
