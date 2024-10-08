/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xi3cpsx_master.c
* @addtogroup Overview
* @{
*
* Handles master mode transfers.
*
* <pre> MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---  -------- ---------------------------------------------
* 1.00  sd  06/10/22 First release
* 1.01  sd  12/14/22 Fix warnings
* 1.3 	sd   1/30/24  Moved prints under DEBUG flag.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xi3cpsx.h"
#include "xi3cpsx_pr.h"
#include "sleep.h"
#include "stdbool.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define TX_MAX_LOOPCNT 1000000U	/**< Used to wait in polled function */

/************************** Function Prototypes ******************************/

/************************* Variable Definitions *****************************/

s32 XI3cPsx_SetDynamicAddr(XI3cPsx *InstancePtr, u32 *Addr, u32 Devices)
{
	u32 i;

	for (i = 0; i < Devices; i++) {
		InstancePtr->Addr[i] = Addr[i];
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief
* Initializes the I3c bus by configuring the device address table,
* resets and assigns the dynamic address to the slave devices.
*
* @param	InstancePtr is a pointer to the XI3cPsx instance.
*
* @return
*		- XST_SUCCESS if everything went well.
*		- XST_FAILURE if any error.
*
* @note 	None.
*
****************************************************************************/
s32 XI3cPsx_BusInit(XI3cPsx *InstancePtr)
{
	u32 Reg = 0;
	s32 Status = XST_FAILURE;
	struct CmdInfo CmdCCC;

	/*
	 * Write self address
	 */
	Reg = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, XI3CPSX_DEVICE_ADDR);
	Reg = ((XI3CPSX_DEVICE_ADDR_SELF_DYNAMIC_ADDR << XI3CPSX_DEVICE_ADDR_DYNAMIC_ADDR_SHIFT) |
	       XI3CPSX_DEVICE_ADDR_DYNAMIC_ADDR_VALID_MASK);
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_DEVICE_ADDR, Reg);

	/*
	 * Set SCL push-pull High count and Low count
	 */
	Reg = SCL_I3C_TIMING_HCNT(XI3CPSX_SCL_I3C_PP_TIMING_I3C_PP_HCNT_VAL)
		| (XI3CPSX_SCL_I3C_PP_TIMING_I3C_PP_LCNT_VAL & XI3CPSX_SCL_I3C_PP_TIMING_I3C_PP_LCNT_MASK);
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_SCL_I3C_PP_TIMING, Reg);

	/*
	 * Set Open Drain Low Count and High Count
	 */
	Reg = ((XI3CPSX_SCL_I3C_OD_TIMING_I3C_OD_HCNT_MASK | (XI3CPSX_SCL_I3C_OD_TIMING_I3C_OD_HCNT_VAL << XI3CPSX_SCL_I3C_OD_TIMING_I3C_OD_HCNT_SHIFT))
	       | (XI3CPSX_SCL_I3C_OD_TIMING_I3C_OD_LCNT_VAL & XI3CPSX_SCL_I3C_OD_TIMING_I3C_OD_LCNT_MASK));
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_SCL_I3C_OD_TIMING, Reg);

	/*
	 * Initialize Device address table (DAT)
	 */
	Reg = 0x25 << XI3CPSX_DEV_ADDR_TABLE_LOC1_DEV_DYNAMIC_ADDR_SHIFT;
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress,
			 XI3CPSX_DEV_ADDR_TABLE_LOC1, Reg);
	Reg = 0x26 << XI3CPSX_DEV_ADDR_TABLE_LOC1_DEV_DYNAMIC_ADDR_SHIFT;
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress,
			 XI3CPSX_DEV_ADDR_TABLE_LOC2, Reg);

	/*
	 * Enable the controller
	 */
	XI3cPsx_Enable(InstancePtr);

	/*
	 * Issue RSTDAA command
	 * If Slave is in disabled state, it is expected get NACK from slave.
	 * After generating SCL clock from master, slave will be enabled.
	 * So the initial command RSTDAA may fail, need to send again.
	 */

	CmdCCC.SlaveAddr = 0;
	CmdCCC.Cmd = I3C_CCC_RSTDAA(1);
	CmdCCC.RxLen = 0;
	Status = XI3cPsx_SendTransferCmd(InstancePtr, &CmdCCC);
	if (Status != XST_SUCCESS) {
		XI3cPsx_Resume(InstancePtr);
		Status = XI3cPsx_SendTransferCmd(InstancePtr, &CmdCCC);
		if (Status != XST_SUCCESS) {
#ifdef DEBUG
			xil_printf("RSTDAA failed\n");
#endif
			return XST_FAILURE;
		}
	}

	/*
	 * Issue ENTDAA command
	 */
	CmdCCC.SlaveAddr = 0;
	CmdCCC.Cmd = I3C_CCC_ENTDAA;
	Status = XI3cPsx_SendAddrAssignCmd(InstancePtr, &CmdCCC);
	if (Status == XST_SUCCESS) {
#ifdef DEBUG
		XI3cPsx_PrintDCT(InstancePtr);
#endif
		/* Update applicable entries in DAT */
	}

	return Status;
}

#ifdef DEBUG
void XI3cPsx_PrintDCT(XI3cPsx *InstancePtr)
{
	u32 i = 0;
	u32 j = 0;
	u32 DctRead = 0;

	for (i = 0; i < InstancePtr->Config.DeviceCount; i++) {
		/* 4 words in DCT for 1 device */
		for (j = 0; j < 4; j++) {

			DctRead = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, XI3CPSX_DEV_CHAR_TABLE1_LOC1 + (i * 16) + (j * 4));
			xil_printf("Word 0x%x\t DctRead 0x%x\n", j, DctRead);
		}
	}
}
#endif

/*****************************************************************************/
/**
* @brief
* This function initiates an interrupt-driven send in master mode.
*
* It tries to send the first FIFO-full of data, then lets the interrupt
* handler to handle the rest of the data if there is any.
*
* @param	InstancePtr is a pointer to the XI3cPsx instance.
* @param	MsgPtr is the pointer to the send buffer.
* @param	ByteCount is the number of bytes to be sent.
* @param	Cmds is the instance of XI3cPsx_Cmd.
*
* @return
*		- XST_SUCCESS if everything went well.
*
* @note		This send routine is for interrupt-driven transfer only.
*
 ****************************************************************************/
s32 XI3cPsx_MasterSend(XI3cPsx *InstancePtr, u8 *MsgPtr,
		       s32 ByteCount, XI3cPsx_Cmd Cmds)
{
	u16 WrFifoSpace;
	u32 EnableIntr;

	if (ByteCount) {
		if (ByteCount >= COMMAND_PORT_ARG_DATA_LEN_MAX)
			return XST_SEND_ERROR;

		InstancePtr->SendByteCount = (u16)ByteCount;
		InstancePtr->SendBufferPtr = MsgPtr;

		WrFifoSpace = XI3cPsx_WrFifoLevel(InstancePtr);

		if (WrFifoSpace > InstancePtr->SendByteCount)
			ByteCount = InstancePtr->SendByteCount;
		else
			ByteCount = WrFifoSpace;

		XI3cPsx_WrTxFifo(InstancePtr, (u32 *)MsgPtr, ByteCount);

		/*
		 * Enable interrupts
		 */
		if (InstancePtr->SendByteCount)
			EnableIntr = XI3CPSX_INTR_TX_THLD;
	}
	if (Cmds.TransCmd & COMMAND_PORT_ROC)
		EnableIntr |= XI3CPSX_INTR_RESP_READY;

	XI3cPsx_EnableInterrupts(InstancePtr->Config.BaseAddress, EnableIntr);

	/*
	 * Send command part to controller. It triggers the transfer
	 */
	XI3cPsx_WrCmdFifo(InstancePtr, &Cmds);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief
* This function initiates an interrupt-driven receive in master mode.
*
* It sets the transfer size register so the slave can send data to us.
* The rest of the work is managed by interrupt handler.
*
* @param	InstancePtr is a pointer to the XI3cPsx instance.
* @param	MsgPtr is the pointer to the receive buffer.
* @param	ByteCount is the number of bytes to be received.
* @param	Cmds is a pointer to the XI3cPsx_Cmd instance.
*
* @return
*		- XST_SUCCESS if everything went well.
*		- XST_FAILURE if any error.
*
* @note		This receive routine is for interrupt-driven transfer only.
*
****************************************************************************/

s32 XI3cPsx_MasterRecv(XI3cPsx *InstancePtr, u8 *MsgPtr,
		       s32 ByteCount, XI3cPsx_Cmd *Cmds)
{
	u32 EnableIntr;

	if (ByteCount >= COMMAND_PORT_ARG_DATA_LEN_MAX)
		return XST_RECV_ERROR;

	InstancePtr->RecvByteCount  = (u16)ByteCount;
	InstancePtr->RecvBufferPtr = MsgPtr;

	/*
	 * Enable interrupts
	 */
	EnableIntr = XI3CPSX_INTR_RX_THLD;

	if (Cmds->TransCmd & COMMAND_PORT_ROC)
		EnableIntr |= XI3CPSX_INTR_RESP_READY;

	XI3cPsx_EnableInterrupts(InstancePtr->Config.BaseAddress, EnableIntr);

	/* Send command part to controller. It triggers the transfer */
	XI3cPsx_WrCmdFifo(InstancePtr, Cmds);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief
* This function initiates a polled mode send in master mode.
*
* It sends data to the FIFO and waits for the slave to pick them up.
* If master fails to send data due arbitration lost, will stop transfer
* and with arbitration lost status
* If slave fails to remove data from FIFO, the send fails with
* time out.
*
* @param	InstancePtr is a pointer to the XI3cPsx instance.
* @param	MsgPtr is the pointer to the send buffer.
* @param	ByteCount is the number of bytes to be sent.
* @param	Cmds is a pointer to the XI3cPsx_Cmd instance.
*
* @return
*		- XST_SUCCESS if everything went well.
*
* @note		This send routine is for polled mode transfer only.
* 		All the FIFO operations are in terms of words.
*
****************************************************************************/
s32 XI3cPsx_MasterSendPolled(XI3cPsx *InstancePtr, u8 *MsgPtr,
			     s32 ByteCount, XI3cPsx_Cmd Cmds)
{
	bool Resp = FALSE;
	s32 Status = XST_FAILURE;
	u16 WrFifoSpace;

	if (Cmds.TransCmd & COMMAND_PORT_ROC)
		XI3cPsx_EnableInterrupts(InstancePtr->Config.BaseAddress, XI3CPSX_INTR_RESP_READY);

	if (ByteCount) {
		if (ByteCount >= COMMAND_PORT_ARG_DATA_LEN_MAX)
			return XST_SEND_ERROR;

		InstancePtr->SendByteCount = (u16)ByteCount;
		InstancePtr->SendBufferPtr = MsgPtr;

		WrFifoSpace = XI3cPsx_WrFifoLevel(InstancePtr);

		if (XI3CPSX_WORD_TO_BYTES(WrFifoSpace) > InstancePtr->SendByteCount)
			ByteCount = InstancePtr->SendByteCount;
		else
			ByteCount = XI3CPSX_WORD_TO_BYTES(WrFifoSpace);
		/*
		 * Write data to tx FIFO
		 */
		if (ByteCount)
			XI3cPsx_WrTxFifo(InstancePtr, (u32 *)MsgPtr, ByteCount);

		/*
		 * Send command part to controller. It triggers the transfer
		 */
		XI3cPsx_WrCmdFifo(InstancePtr, &Cmds);

		while (InstancePtr->SendByteCount > 0 && !Resp){
			WrFifoSpace = XI3cPsx_WrFifoLevel(InstancePtr);

			if (XI3CPSX_WORD_TO_BYTES(WrFifoSpace) > InstancePtr->SendByteCount)
				ByteCount = InstancePtr->SendByteCount;
			else
				ByteCount = XI3CPSX_WORD_TO_BYTES(WrFifoSpace);
			/*
			 * Write data to tx FIFO
			 */
			if (ByteCount)
				XI3cPsx_WrTxFifo(InstancePtr, (u32 *)MsgPtr, ByteCount);

			/*
			 * Response from intr status
			 */
			Resp = !!(XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, XI3CPSX_INTR_STATUS)
				      & XI3CPSX_INTR_RESP_READY);
		}
	} else {

		/* Send command part to controller. It triggers the transfer */
		XI3cPsx_WrCmdFifo(InstancePtr, &Cmds);
	}

	if (Cmds.TransCmd & COMMAND_PORT_ROC) {
		if (XI3cPsx_GetResponse(InstancePtr))
			Status = XST_SEND_ERROR;
		else
			Status = XST_SUCCESS;
		XI3cPsx_DisableInterrupts(InstancePtr->Config.BaseAddress,
					  XI3CPSX_INTR_RESP_READY);
	} else {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief
* This function initiates a polled mode receive in master mode.
*
* It repeatedly sets the transfer size register so the slave can
* send data to us. It polls the data register for data to come in.
* If master fails to read data due arbitration lost, will return
* with arbitration lost status.
* If slave fails to send us data, it fails with time out.
*
* @param	InstancePtr is a pointer to the XI3cPsx instance.
* @param	MsgPtr is the pointer to the receive buffer.
* @param	ByteCount is the number of bytes to be received.
* @param	Cmds is a pointer to the XI3cPsx_Cmd instance.
*
* @return
*		- XST_SUCCESS if everything went well.
*		- XST_FAILURE if any error.
*
* @note		This receive routine is for polled mode transfer only.
*
****************************************************************************/
s32 XI3cPsx_MasterRecvPolled(XI3cPsx *InstancePtr, u8 *MsgPtr,
			     s32 ByteCount, XI3cPsx_Cmd *Cmds)
{

	bool Resp = FALSE;
	s32 Status = XST_FAILURE;
	u16 RdWordCount;

	if (ByteCount >= COMMAND_PORT_ARG_DATA_LEN_MAX)
		return XST_RECV_ERROR;
	InstancePtr->RecvByteCount = (u16)ByteCount;
	InstancePtr->RecvBufferPtr = MsgPtr;

	if (Cmds->TransCmd & COMMAND_PORT_ROC)
		XI3cPsx_EnableInterrupts(InstancePtr->Config.BaseAddress, XI3CPSX_INTR_RESP_READY);

	/*
	 * Send command part to controller. It triggers the transfer
	 */
	XI3cPsx_WrCmdFifo(InstancePtr, Cmds);

	while (InstancePtr->RecvByteCount > 0 && !Resp){
		RdWordCount = XI3cPsx_RdFifoLevel(InstancePtr);

		/*
		 * Read data from Rx FIFO
		 */
		if (RdWordCount)
			XI3cPsx_RdRxFifo(InstancePtr, (u32 *)MsgPtr, XI3CPSX_WORD_TO_BYTES(RdWordCount));

		/*
		 * Response from intr status
		 */
		Resp = !!(XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, XI3CPSX_INTR_STATUS)
			      & XI3CPSX_INTR_RESP_READY);
	}

	if(InstancePtr->RecvByteCount > 0)
		XI3cPsx_RdRxFifo(InstancePtr, (u32 *)MsgPtr, InstancePtr->RecvByteCount);

	if (Cmds->TransCmd & COMMAND_PORT_ROC) {
		if (XI3cPsx_GetResponse(InstancePtr))
			Status = XST_RECV_ERROR;
		else
			Status = XST_SUCCESS;
		XI3cPsx_DisableInterrupts(InstancePtr->Config.BaseAddress, XI3CPSX_INTR_RESP_READY);
	} else {
		Status = XST_SUCCESS;
	}
	return Status;
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
void XI3cPsx_SetStatusHandler(XI3cPsx *InstancePtr, void *CallBackRef,
			      XI3cPsx_IntrHandler FunctionPtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(FunctionPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

	InstancePtr->StatusHandler = FunctionPtr;
	InstancePtr->CallBackRef = CallBackRef;
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
* @param	InstancePtr is a pointer to the XI3cPsx instance.
*
* @return	None.
*
* @note 	None.
*
****************************************************************************/
void XI3cPsx_MasterInterruptHandler(XI3cPsx *InstancePtr)
{
	u32 IntrStatusReg;
	u16 RxDataAvailable;
	u16 WrFifoSpace;
	s32 ByteCount;
	u32 ResponseData;
	/*
	 * Assert validates the input arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

	/*
	 * Read the Interrupt status register.
	 */
	IntrStatusReg = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, XI3CPSX_INTR_STATUS);

	/*
	 * Write the status back to clear the interrupts so no events are
	 * missed while processing this interrupt.
	 */
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, (u32)XI3CPSX_INTR_STATUS, IntrStatusReg);

	/*
	 * Tx empty
	 */
	if (IntrStatusReg & XI3CPSX_INTR_TX_THLD) {
		WrFifoSpace = XI3cPsx_WrFifoLevel(InstancePtr);

		if (WrFifoSpace > InstancePtr->SendByteCount)
			ByteCount = InstancePtr->SendByteCount;
		else
			ByteCount = WrFifoSpace;

		XI3cPsx_WrTxFifo(InstancePtr, (u32 *)InstancePtr->SendBufferPtr, ByteCount);

		if (InstancePtr->SendByteCount <= 0) {
			XI3cPsx_DisableInterrupts(InstancePtr->Config.BaseAddress,
						  XI3CPSX_INTR_TX_THLD);
		}
	}

	/*
	 * Rx
	 */
	if (IntrStatusReg & XI3CPSX_INTR_RX_THLD) {
		RxDataAvailable = XI3cPsx_RdFifoLevel(InstancePtr);

		XI3cPsx_RdRxFifo(InstancePtr, (u32 *)InstancePtr->RecvBufferPtr, RxDataAvailable);
		if (InstancePtr->RecvByteCount <= 0) {
			XI3cPsx_DisableInterrupts(InstancePtr->Config.BaseAddress,
						  XI3CPSX_INTR_RX_THLD);
		}
	}

	/*
	 * Response
	 */
	if (IntrStatusReg & XI3CPSX_INTR_RESP_READY) {
		if (InstancePtr->RecvByteCount > 0) {
			RxDataAvailable = XI3cPsx_RdFifoLevel(InstancePtr);

			XI3cPsx_RdRxFifo(InstancePtr, (u32 *)InstancePtr->RecvBufferPtr, RxDataAvailable);
		}

		ResponseData = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress,
					       XI3CPSX_RESPONSE_QUEUE_PORT);
		InstancePtr->Error = (ResponseData & XI3CPSX_RESPONSE_ERR_STS_MASK) >> XI3CPSX_RESPONSE_ERR_STS_SHIFT;

		XI3cPsx_DisableInterrupts(InstancePtr->Config.BaseAddress,
					  XI3CPSX_INTR_RESP_READY);

		InstancePtr->StatusHandler(InstancePtr->CallBackRef, InstancePtr->Error);
	}
}
/** @} */
