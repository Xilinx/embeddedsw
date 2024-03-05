/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc. All Rights Reserved
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xi3c_master.c
* @addtogroup Overview
* @{
*
* Handles master mode transfers.
*
* <pre> MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---  -------- ---------------------------------------------
* 1.00  gm  02/09/24 First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_util.h"
#include "xi3c.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/*****************************************************************************/
/**
* @brief
* This function reads the response.
*
* It waits for the response from slave and reads the response.
*
* @param	InstancePtr is a pointer to the XI3c instance.
*
* @return
*		- Response code on sucess.
*		- XST_TIMEOUT on timeout.
*
* @note
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
* @brief
* This function sends the command.
*
* @param	InstancePtr is a pointer to the XI3c instance.
* @param	Cmd is a pointer to the XI3c_Cmd.
* @param	Data is the command value.
*
* @return
*		- XST_SUCCESS if everything went well.
*		- XST_FAILURE if any error.
*
* @note         None.
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
* @brief
* This function initiates a interrupt mode send in master mode.
*
* It sends data to the FIFO and waits for the slave to pick them up.
* If master fails to send data due arbitration lost or any other,
* will stop transfer with status.
*
* @param	InstancePtr is a pointer to the XI3c instance.
* @param	Cmd is a pointer to the XI3c_Cmd instance.
* @param	MsgPtr is the pointer to the send buffer.
* @param	ByteCount is the number of bytes to be sent.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
s32 XI3c_MasterSend(XI3c *InstancePtr, XI3c_Cmd *Cmd, u8 *MsgPtr, u16 ByteCount)
{
	u16 WrFifoSpace;
	u16 SpaceIndex;

	/*
         * Assert validates the input arguments.
         */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Cmd != NULL);

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
	 * Eanble response fifo not empty interrupt
	 */
	XI3c_EnableREInterrupts(InstancePtr->Config.BaseAddress,
				XI3C_INTR_RESP_NOT_EMPTY_MASK);

	/* Fill command to trigger transfer */
	XI3c_FillCmdFifo(InstancePtr, Cmd);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief
* This function initiates a interrupt mode receive in master mode.
*
* It sets the transfer size register so the slave can send data to us.
* The rest of the work is managed by interrupt handler.
*
* @param	InstancePtr is a pointer to the XI3c instance.
* @param	Cmd is a pointer to the XI3c_Cmd instance.
* @param	MsgPtr is the pointer to the recv buffer.
* @param	ByteCount is the number of bytes to be recv.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
s32 XI3c_MasterRecv(XI3c *InstancePtr, XI3c_Cmd *Cmd, u8 *MsgPtr, u16 ByteCount)
{
	/*
         * Assert validates the input arguments.
         */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Cmd != NULL);

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
* @brief
* This function initiates a polled mode send in master mode.
*
* It sends data to the FIFO and waits for the slave to pick them up.
* If master fails to send data due arbitration lost or any other error,
* will stop transfer status.
*
* @param	InstancePtr is a pointer to the XI3c instance.
* @param	Cmd is a pointer to the XI3c_Cmd instance.
* @param	MsgPtr is the pointer to the send buffer.
* @param	ByteCount is the number of bytes to be sent.
*
* @return
*		- XST_SUCCESS if everything went well.
*		- XST_SEND_ERROR if any error.
*
* @note		None.
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
* @brief
* This function initiates a polled mode receive in master mode.
*
* It repeatedly sets the transfer size register so the slave can
* send data to us. It polls the data register for data to come in.
* If master fails to read data due arbitration lost or any other error,
* will return with status.
*
* @param	InstancePtr is a pointer to the XI3c instance.
* @param	Cmd is a pointer to the XI3c_Cmd instance.
* @param	MsgPtr is the pointer to the send buffer.
* @param	ByteCount is the number of bytes to be sent.
*
* @return
*		- XST_SUCCESS if everything went well.
*		- XST_RECV_ERROR if any error.
*
* @note		None.
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
*
* @brief
* This function sets the status handler, which the driver calls when it
* encounters conditions that should be reported to the higher layer software.
* The handler executes in an interrupt context, so the amount of processing
* should be minimized
*
* Refer to the xi3c.h file for a list of events. The events are defined
* to start with XI3C_EVENT_*.
*
* @param        InstancePtr is a pointer to the XI3c instance.
* @param        FunctionPtr is the pointer to the callback function.
*
* @return       None.
*
* @note
*
* The handler is called within interrupt context, so it should finish its
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
* The interrupt handler for the master mode. It does the protocol handling for
* the interrupt-driven transfers.
*
* If the Master is receiving data then the data is read from the FIFO and
* the Master has to request for more data (if there is more data to
* receive). If all the data has been received then a completion event
* is signalled to the upper layer by calling the callback handler.
* It is an error if the amount of received data is more than expected.
*
* @param	InstancePtr is a pointer to the XI3c instance.
*
* @return	None.
*
* @note 	None.
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

/** @} */
