/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved
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
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xi3cpsx.h"
#include "xi3cpsx_pr.h"
#include "sleep.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define TX_MAX_LOOPCNT 1000000U	/**< Used to wait in polled function */

/************************** Function Prototypes ******************************/

/************************* Variable Definitions *****************************/

void XI3cPsx_SetRespThreshold(XI3cPsx *InstancePtr, u32 Val)
{
	u32 reg = 0;

	reg = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, XI3CPSX_QUEUE_THLD_CTRL);
	reg = reg & ~(XI3CPSX_QUEUE_THLD_CTRL_RESP_BUF_THLD_MASK);
	reg = reg | (Val << XI3CPSX_QUEUE_THLD_CTRL_RESP_BUF_THLD_SHIFT);
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_QUEUE_THLD_CTRL, reg);
	xil_printf("XI3CPSX_QUEUE_THLD_CTRL 0x%x\n", reg);
}

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
* The Bus initialisation code.
*
*
* @return	None.
*
* @note 	None.
*
****************************************************************************/
s32 XI3cPsx_BusInit(XI3cPsx *InstancePtr)
{
	u32 tmp = 0;
	s32 ret = 0;
	struct CmdInfo CmdCCC;

	/* Device Mode Selection. If configured as secondary master, settings required. */

	/* Initializing common registers */

	XI3cPsx_SetRespThreshold(InstancePtr, 0);

	tmp = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, XI3CPSX_DATA_BUFFER_THLD_CTRL);
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_DATA_BUFFER_THLD_CTRL, tmp);
	xil_printf("XI3CPSX_DATA_BUFFER_THLD_CTRL 0x%x\n", tmp);

	tmp = XI3CPSX_INTR_SIGNAL_EN_TRANSFER_ERR_SIGNAL_EN_MASK | XI3CPSX_INTR_SIGNAL_EN_RESP_READY_SIGNAL_EN_MASK;
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_INTR_STATUS_EN, tmp);
	xil_printf("XI3CPSX_INTR_STATUS_EN 0x%x\n", tmp);

	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_INTR_SIGNAL_EN, XI3CPSX_INTR_SIGNAL_EN_RESP_READY_SIGNAL_EN_MASK);
	xil_printf("XI3CPSX_INTR_SIGNAL_EN 0x%x\n", XI3CPSX_INTR_SIGNAL_EN_RESP_READY_SIGNAL_EN_MASK);

	/* Initializing master registers */

	/* Write self address */
	tmp = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, XI3CPSX_DEVICE_ADDR);
	tmp = ((0x45 << XI3CPSX_DEVICE_ADDR_DYNAMIC_ADDR_SHIFT) | XI3CPSX_DEVICE_ADDR_DYNAMIC_ADDR_VALID_MASK);
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_DEVICE_ADDR, tmp);
	xil_printf("XI3CPSX_DEVICE_ADDR 0x%x\n", tmp);

	/* Initialize Device address table (DAT) */
	tmp = 0x25 << XI3CPSX_DEV_ADDR_TABLE_LOC1_DEV_DYNAMIC_ADDR_SHIFT;
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress,
				       XI3CPSX_DEV_ADDR_TABLE_LOC1, tmp);
	tmp = 0x26 << XI3CPSX_DEV_ADDR_TABLE_LOC1_DEV_DYNAMIC_ADDR_SHIFT;
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress,
				       XI3CPSX_DEV_ADDR_TABLE_LOC2, tmp);

	xil_printf("DEV_ADDR_TABLE_LOC(0x2c0, 0) 0x%x\n", tmp);

	/* Enable the controller */
	tmp = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, XI3CPSX_DEVICE_CTRL);
	tmp = tmp | XI3CPSX_DEVICE_CTRL_ENABLE_MASK;
	xil_printf("Write 0x%x\n", tmp);
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_DEVICE_CTRL, tmp);
	tmp = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, XI3CPSX_DEVICE_CTRL);

	xil_printf("DEVICE CTRL reads 0x%x\n", tmp);

	/* Issue ENTDAA command */
	CmdCCC.SlaveAddr = 0;
	CmdCCC.Cmd = I3C_CCC_RSTDAA(1);
	CmdCCC.RxLen = 0;
	ret = XI3cPsx_SendTransferCmd(InstancePtr, &CmdCCC);
	/* ENTDAA */
	CmdCCC.Cmd = I3C_CCC_ENTDAA;
	ret = XI3cPsx_SendAddrAssignCmd(InstancePtr, &CmdCCC);

	if (ret == XST_SUCCESS) {
		XI3cPsx_PrintDCT(InstancePtr);
		/* Update applicable entries in DAT */
	}

	return ret;
}

void XI3cPsx_WrTxFifo(XI3cPsx *InstancePtr, u32 *TxBuf, u16 TxLen)
{
	u16 NoWords = TxLen / 4;
	u32 Val;
	u16 i;
	xil_printf("Writing to FIFO\n");
	/* FIFO is word based, so pack the data accordingly */
	for (i = 0; i < NoWords; i++) {
		Val = TxBuf[i];
		xil_printf("0x%x\t", Val);
		XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress,
				XI3CPSX_TX_RX_DATA_PORT, Val);
	}
	if (TxLen & 3) {
		memcpy(&Val, TxBuf + (TxLen & (~3)), TxLen & 3);
		xil_printf("0x%x\n", Val);
		XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress,
				XI3CPSX_TX_RX_DATA_PORT, Val);
	}
}

static void XI3cPsixl_WrCmdFifo(XI3cPsx *InstancePtr, XI3cPsx_Cmd *Cmd)
{
	xil_printf("Writing Command - Arg: 0x%x\t Cmd: 0x%x\n", Cmd->TransCmd, Cmd->TransArg);
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress,
			 XI3CPSX_COMMAND_QUEUE_PORT, Cmd->TransCmd);
	 XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress,
			 XI3CPSX_COMMAND_QUEUE_PORT, Cmd->TransArg);
}

static void XI3cPsx_RdRxFifo(XI3cPsx *InstancePtr, u32 *RxBuf, u16 RxLen)
{
	u16 NoWords = RxLen / 4;
	u32 Val;
	u16 i;

	/* FIFO is word base, so read the data accordingly */
	for (i = 0; i < NoWords; i++) {
		RxBuf[i] = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress,
					   XI3CPSX_TX_RX_DATA_PORT);
		xil_printf("Data word 0x%x\tData 0x%x\n", i, RxBuf[i]);
	}
	if (RxLen & 3) {
		Val = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress,
					   XI3CPSX_TX_RX_DATA_PORT);
		xil_printf("Data word 0x%x\tData 0x%x\n", i, Val);
		memcpy(RxBuf + (RxLen & (~3)), &Val, RxLen & 3);
	}
}

void XI3cPsx_PrintDCT(XI3cPsx *InstancePtr)
{
	u32 i = 0;
	u32 j = 0;
	u32 DctRead = 0;

	for (i = 0; i < InstancePtr->Config.DeviceCount; i++) {
		/* 4 words in DCT for 1 device */
		xil_printf("Device number 0x%x\n", i);
		for (j = 0; j < 4; j++) {
			DctRead = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, XI3CPSX_DEV_CHAR_TABLE1_LOC1 + (i * 16) + (j * 4));
			xil_printf("Word 0x%x\t DctRead 0x%x\n", j, DctRead);

		}
	}
}

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
* @param	SlaveAddr is the address of the slave we are sending to.
*
* @return	None.
*
* @note		This send routine is for interrupt-driven transfer only.
*
 ****************************************************************************/
XI3cPsx_Cmd * CmdsPtr;
s32 XI3cPsx_MasterSend(XI3cPsx *InstancePtr, u8 *MsgPtr,
		 s32 ByteCount, XI3cPsx_Cmd Cmds)
{
	u32 Rbuf_level = 0;

	CmdsPtr = &Cmds;
	XI3cPsx_EnableInterrupts(InstancePtr->Config.BaseAddress, (u32)0xB77F);
	InstancePtr->SendByteCount = ByteCount;
	InstancePtr->SendBufferPtr = MsgPtr;
	Rbuf_level = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress,
					XI3CPSX_QUEUE_STATUS_LEVEL);

	xil_printf("Rbuf_level before Cmd write %d\n", Rbuf_level);
	/* Send command part to controller. It triggers the transfer */

	XI3cPsixl_WrCmdFifo(InstancePtr, &Cmds);
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
* @param	SlaveAddr is the address of the slave we are receiving from.
*
* @return	None.
*
* @note		This receive routine is for interrupt-driven transfer only.
*
****************************************************************************/

s32 XI3cPsx_MasterRecv(XI3cPsx *InstancePtr, u8 *MsgPtr,
				s32 ByteCount, XI3cPsx_Cmd *Cmds)
{
	u32 Rbuf_level = 0;
	u32 Intr_status = 0;
	u32 IntrStatusReg;

	XI3cPsx_EnableInterrupts(InstancePtr->Config.BaseAddress, (u32)0xB77F);
	IntrStatusReg = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress,
			XI3CPSX_INTR_STATUS);

	Rbuf_level = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress,
					XI3CPSX_QUEUE_STATUS_LEVEL);
	InstancePtr->RecvByteCount  = ByteCount;

	xil_printf("Rbuf_level, Intr_status before Cmd write %x\t%x\n", Rbuf_level, Intr_status);

	/* Send command part to controller. It triggers the transfer */
	XI3cPsixl_WrCmdFifo(InstancePtr, Cmds);
	/* Wait until response buffer is filled up */
	Rbuf_level = (Rbuf_level & XI3CPSX_QUEUE_STATUS_LEVEL_RESP_BUF_BLR_MASK) >> XI3CPSX_QUEUE_STATUS_LEVEL_RESP_BUF_BLR_SHIFT;
	while(((s32)Rbuf_level  != InstancePtr->RecvByteCount) && !(IntrStatusReg)) {
		Rbuf_level = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress,
					XI3CPSX_QUEUE_STATUS_LEVEL);
		Rbuf_level = (Rbuf_level & XI3CPSX_QUEUE_STATUS_LEVEL_RESP_BUF_BLR_MASK) >> XI3CPSX_QUEUE_STATUS_LEVEL_RESP_BUF_BLR_SHIFT;
		IntrStatusReg = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress,
				XI3CPSX_INTR_STATUS);
	}

	InstancePtr->SendBufferPtr = MsgPtr;

	/* Check if any error in all commands */
	if (InstancePtr->Error != 0) {
		xil_printf("Error %d \n" , InstancePtr->Error );
		XI3cPsx_ResetFifos(InstancePtr);
		return XST_FAILURE;
	}
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
* @param	SlaveAddr is the address of the slave we are sending to.
*
* @return
*		- XST_SUCCESS if everything went well.
*		- XST_FAILURE if timed out.
*		- XST_IIC_ARB_LOST if arbitration lost
*
* @note		This send routine is for polled mode transfer only.
*
****************************************************************************/
s32 XI3cPsx_MasterSendPolled(XI3cPsx *InstancePtr, u8 *MsgPtr,
		 s32 ByteCount, XI3cPsx_Cmd Cmds)
{
	/* If tx command, first write it's data to tx FIFO */
	if (ByteCount) {
		XI3cPsx_WrTxFifo(InstancePtr, (u32 *)MsgPtr,
				  ByteCount);
	}

	/* Send command part to controller. It triggers the transfer */
	XI3cPsixl_WrCmdFifo(InstancePtr, &Cmds);

	return XST_SUCCESS;
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
* @param	Cmds is the address of the slave we are receiving from.
*
* @return
*		- XST_SUCCESS if everything went well.
*		- XST_FAILURE if timed out.
*		- XST_IIC_ARB_LOST if arbitration lost
*
* @note		This receive routine is for polled mode transfer only.
*
****************************************************************************/
s32 XI3cPsx_MasterRecvPolled(XI3cPsx *InstancePtr, u8 *MsgPtr,
				s32 ByteCount, XI3cPsx_Cmd *Cmds)
{
	u32 Resp = 0;
	u32 Rbuf_level = 0;
	u32 Intr_status = 0;
	u16 RxLen;

	XI3cPsx_EnableInterrupts(InstancePtr->Config.BaseAddress, (u32)0xB77F);
	Intr_status = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress,
			XI3CPSX_INTR_STATUS);

	Rbuf_level = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress,
					XI3CPSX_QUEUE_STATUS_LEVEL);

	/* Send command part to controller. It triggers the transfer */
	XI3cPsixl_WrCmdFifo(InstancePtr, Cmds);

	/* Wait until response buffer is filled up */
	Rbuf_level = (Rbuf_level & XI3CPSX_QUEUE_STATUS_LEVEL_RESP_BUF_BLR_MASK) >> XI3CPSX_QUEUE_STATUS_LEVEL_RESP_BUF_BLR_SHIFT;
	while(((s32)Rbuf_level  != ByteCount) && !(Intr_status)) {
		Rbuf_level = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress,
					XI3CPSX_QUEUE_STATUS_LEVEL);
		Rbuf_level = (Rbuf_level & XI3CPSX_QUEUE_STATUS_LEVEL_RESP_BUF_BLR_MASK) >> XI3CPSX_QUEUE_STATUS_LEVEL_RESP_BUF_BLR_SHIFT;
		Intr_status = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress,
				XI3CPSX_INTR_STATUS);
	}

	/* Read the response buffer */
	Resp = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress,
			      XI3CPSX_RESPONSE_QUEUE_PORT);
	RxLen = RESPONSE_PORT_DATA_LEN(Resp);
	Cmds->Error = RESPONSE_PORT_ERR_STATUS(Resp);

	/* If rx command, read data from FIFO */
	if ((RxLen) && !(Cmds->Error)) {
		XI3cPsx_RdRxFifo(InstancePtr, (u32 *)MsgPtr,
				  RxLen);
	}

	/* Check if any error in all commands */
	if (Cmds->Error != 0) {
		XI3cPsx_ResetFifos(InstancePtr);
		return XST_FAILURE;
	}
	return XST_SUCCESS;

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
	u32 Rbuf_level = 0;
	u32 Resp = 0;
	u16 RxLen;
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

	Rbuf_level = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress,
					XI3CPSX_QUEUE_STATUS_LEVEL);

	/* Wait until response buffer is filled up */
	Rbuf_level = (Rbuf_level & XI3CPSX_QUEUE_STATUS_LEVEL_RESP_BUF_BLR_MASK) >> XI3CPSX_QUEUE_STATUS_LEVEL_RESP_BUF_BLR_SHIFT;
	while((Rbuf_level  != (u32)InstancePtr->RecvByteCount) && !(IntrStatusReg)) {
		Rbuf_level = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress,
					XI3CPSX_QUEUE_STATUS_LEVEL);
		Rbuf_level = (Rbuf_level & XI3CPSX_QUEUE_STATUS_LEVEL_RESP_BUF_BLR_MASK) >> XI3CPSX_QUEUE_STATUS_LEVEL_RESP_BUF_BLR_SHIFT;
		IntrStatusReg = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress,
				XI3CPSX_INTR_STATUS);
	}

	/* Read the response buffer */
	Resp = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress,
			      XI3CPSX_RESPONSE_QUEUE_PORT);
	xil_printf("Resp 0x%x\n", Resp);
	RxLen = RESPONSE_PORT_DATA_LEN(Resp);
	InstancePtr->Error = RESPONSE_PORT_ERR_STATUS(Resp);

	/* If rx command, read data from FIFO */
	if ((RxLen) && !(InstancePtr->Error)) {
		XI3cPsx_RdRxFifo(InstancePtr, (u32 *)InstancePtr->RecvBufferPtr,
				  RxLen);
	}
	xil_printf("Rbuf_level, Intr_status after Cmd write %x\t%x\n", Rbuf_level, IntrStatusReg);
	if (InstancePtr->SendByteCount) {
		XI3cPsx_WrTxFifo(InstancePtr, (u32 *)InstancePtr->SendBufferPtr,
					InstancePtr->SendByteCount);
	}
}
/** @} */
