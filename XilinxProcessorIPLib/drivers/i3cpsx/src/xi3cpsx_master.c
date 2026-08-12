/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved
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
* 1.8   vn  07/30/26 Fix Doxygen warnings
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
/*
 * Forward declaration of XI3C_DynaAddrList so the size-check typedef below
 * can apply sizeof to it. The array is defined with its initializer in the
 * Variable Definitions section further down.
 */
extern const u32 XI3C_DynaAddrList[XI3CPSX_MAX_DAT_ENTRIES];

/*
 * Compile-time assertion: XI3C_DynaAddrList must have exactly
 * XI3CPSX_MAX_DAT_ENTRIES entries. Emits a "negative array size"
 * diagnostic at build time if the two ever get out of sync.
 */
typedef char XI3cPsx_DynaAddrListSizeCheck[
	(XI3CPSX_ARRAY_SIZE(XI3C_DynaAddrList) == XI3CPSX_MAX_DAT_ENTRIES) ? 1 : -1
];

/***************** Macros (Inline Functions) Definitions *********************/
#define TX_MAX_LOOPCNT 1000000U	/**< Used to wait in polled function */

#define XI3CPSX_IBI_DATA_SEGMENT_DWORDS 31U	/**< IBI data segment size in Dwords */

/************************** Function Prototypes ******************************/
static void XI3cPsx_ConfigIbi(XI3cPsx *InstancePtr);
static void XI3cPsx_CaptureBcr(XI3cPsx *InstancePtr);
static void XI3cPsx_HandleIbi(XI3cPsx *InstancePtr);
static void XI3cPsx_IbiDrain(XI3cPsx *InstancePtr, u32 Len);
static u32 XI3cPsx_IbiReadPayload(XI3cPsx *InstancePtr, u8 *Buf, u32 CopyLen,
				  u32 QueueLen);
static s32 XI3cPsx_SendEnecDisec(XI3cPsx *InstancePtr, u32 DevIndex, u8 Enable);
static u32 XI3cPsx_IsHotJoinRejected(XI3cPsx *InstancePtr);

/************************* Variable Definitions *****************************/
/**
 * Dynamic addresses available for I3C devices.
 */
const u32 XI3C_DynaAddrList[XI3CPSX_MAX_DAT_ENTRIES] = {
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12
};

/*****************************************************************************/
/**
* @brief
* This function sends dynamic Address Assignment for available devices.
*
* @param	InstancePtr is a pointer to the XI3cPsx instance.
* @param	Addr is an array of dynamic addresses.
* @param	Devices is the number of slave devices present.
*
* @return
*               - XST_SUCCESS if everything went well.
*               - XST_FAILURE if any error.
*
* @note         None.
*
****************************************************************************/
static s32 XI3cPsx_SetDynamicAddr(XI3cPsx *InstancePtr, const u32 *Addr, u32 Devices)
{
	u32 i;
	u32 Reg;
	u8 Da;
	u8 Encoded;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);
	if (Devices > 0U) {
		Xil_AssertNonvoid(Addr != NULL);
	}

	if (Devices > InstancePtr->MaxDevs) {
		return XST_FAILURE;
	}

	for (i = 0; i < Devices; i++) {
		Da = (u8)(Addr[i] & XI3CPSX_7BITS_MASK);
		Encoded = (u8)((XI3cPsx_OddParity7(Da) << XI3CPSX_DYNAMIC_ADDR_PARITY_SHIFT) | Da);

		Reg = (u32)Encoded
		      << XI3CPSX_DEV_ADDR_TABLE_LOC1_DEV_DYNAMIC_ADDR_SHIFT;

		XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress,
				 XI3cPsx_DatEntryOffset(InstancePtr, i), Reg);
		InstancePtr->Addr[i] = (u32)Da;
	}
	for (; i < XI3CPSX_MAX_DAT_ENTRIES; i++) {
		InstancePtr->Addr[i] = 0U;
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
* @note		SCL timing must be programmed before calling this function.
*		Call XI3cPsx_SetSClk() or XI3cPsx_SetSClkRate() first so that
*		the RSTDAA and ENTDAA bus transactions issued here run at the
*		intended clock rate.  XI3cPsx_CfgInitialize() handles this
*		sequence automatically; callers invoking XI3cPsx_BusInit()
*		directly are responsible for calling XI3cPsx_SetSClk() first.
*
****************************************************************************/
s32 XI3cPsx_BusInit(XI3cPsx *InstancePtr)
{
	u32 Reg = 0;
	s32 Status = XST_FAILURE;
	struct CmdInfo CmdCCC = {0};

	/*
	 * Write self address
	 */
	Reg = ((XI3CPSX_DEVICE_ADDR_SELF_DYNAMIC_ADDR << XI3CPSX_DEVICE_ADDR_DYNAMIC_ADDR_SHIFT) |
	       XI3CPSX_DEVICE_ADDR_DYNAMIC_ADDR_VALID_MASK);
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_DEVICE_ADDR, Reg);

	/*
	 * Initialize Device address table (DAT)
	 */
	Status = XI3cPsx_SetDynamicAddr(InstancePtr, XI3C_DynaAddrList,
					InstancePtr->Config.DeviceCount);
	if (Status != XST_SUCCESS) {
		return Status;
	}

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
			/* Empty bus: RSTDAA NACK is expected for a Hot-Join
			 * master. Clear the halt and continue to ENTDAA. */
			XI3cPsx_Resume(InstancePtr);
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
		/* The DCT is only populated once ENTDAA has completed. */
		XI3cPsx_CaptureBcr(InstancePtr);

		/* Targets are opted in individually by XI3cPsx_EnableIbi(). */
		XI3cPsx_ConfigIbi(InstancePtr);
	} else if ((InstancePtr->Error == XI3CPSX_RESPONSE_ERR_ADDR_NACK) ||
		   (InstancePtr->Error == XI3CPSX_RESPONSE_ERR_IBA_NACK)) {
		/* Empty bus (expected for Hot-Join): clear halt, arm IBI/HJ. */
		XI3cPsx_Resume(InstancePtr);
		XI3cPsx_ConfigIbi(InstancePtr);
		Status = XST_SUCCESS;
	}

	return Status;
}

#ifdef DEBUG
void XI3cPsx_PrintDCT(XI3cPsx *InstancePtr)
{
	u32 i;
	u32 j;
	u32 DctRead;

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
	u32 EnableIntr = 0U;

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
		/* Preserve the response error code so callers (e.g. BusInit) can
		 * tell an expected address NACK from a real bus/protocol error. */
		InstancePtr->Error = (u32)XI3cPsx_GetResponse(InstancePtr);
		if (InstancePtr->Error != XI3CPSX_RESPONSE_ERR_NONE)
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
	s32 RdByteCount;

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
		 * FIFO level is in words; clamp the converted byte count to the
		 * remaining receive count so a partial last word cannot overrun
		 * the user buffer.
		 */
		if (XI3CPSX_WORD_TO_BYTES(RdWordCount) > InstancePtr->RecvByteCount)
			RdByteCount = InstancePtr->RecvByteCount;
		else
			RdByteCount = XI3CPSX_WORD_TO_BYTES(RdWordCount);

		/*
		 * Read data from Rx FIFO
		 */
		if (RdByteCount)
			XI3cPsx_RdRxFifo(InstancePtr, (u32 *)MsgPtr, RdByteCount);

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
* @param        CallBackRef is the callback reference passed to the handler.
* @param        FunctionPtr is the pointer to the callback function.
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

	/* IBI (SIR) received: level-triggered, serviced by draining the queue. */
	if (IntrStatusReg & XI3CPSX_INTR_IBI_THLD) {
		XI3cPsx_HandleIbi(InstancePtr);
	}

	/*
	 * Tx empty
	 */
	if (IntrStatusReg & XI3CPSX_INTR_TX_THLD) {
		WrFifoSpace = XI3cPsx_WrFifoLevel(InstancePtr);

		if (XI3CPSX_WORD_TO_BYTES(WrFifoSpace) > InstancePtr->SendByteCount)
			ByteCount = InstancePtr->SendByteCount;
		else
			ByteCount = XI3CPSX_WORD_TO_BYTES(WrFifoSpace);
		/*
		 * Write data to tx FIFO
		 */
		if (ByteCount)
			XI3cPsx_WrTxFifo(InstancePtr, (u32 *)InstancePtr->SendBufferPtr, ByteCount);

		if (InstancePtr->SendByteCount == 0) {
			XI3cPsx_DisableInterrupts(InstancePtr->Config.BaseAddress,
						  XI3CPSX_INTR_TX_THLD);
		}
	}

	/*
	 * Rx
	 */
	if (IntrStatusReg & XI3CPSX_INTR_RX_THLD) {
		RxDataAvailable = XI3cPsx_RdFifoLevel(InstancePtr);

		/*
		 * FIFO level is in words; clamp the converted byte count to the
		 * remaining receive count so a partial last word cannot overrun
		 * the user buffer.
		 */
		if (XI3CPSX_WORD_TO_BYTES(RxDataAvailable) > InstancePtr->RecvByteCount)
			ByteCount = InstancePtr->RecvByteCount;
		else
			ByteCount = XI3CPSX_WORD_TO_BYTES(RxDataAvailable);

		if (ByteCount)
			XI3cPsx_RdRxFifo(InstancePtr, (u32 *)InstancePtr->RecvBufferPtr, ByteCount);
		if (InstancePtr->RecvByteCount == 0) {
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

			/*
			 * Clamp to the remaining receive count so a partial last
			 * word cannot overrun the user buffer.
			 */
			if (XI3CPSX_WORD_TO_BYTES(RxDataAvailable) > InstancePtr->RecvByteCount)
				ByteCount = InstancePtr->RecvByteCount;
			else
				ByteCount = XI3CPSX_WORD_TO_BYTES(RxDataAvailable);

			if (ByteCount)
				XI3cPsx_RdRxFifo(InstancePtr, (u32 *)InstancePtr->RecvBufferPtr, ByteCount);
		}

		ResponseData = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress,
					       XI3CPSX_RESPONSE_QUEUE_PORT);
		InstancePtr->Error = (ResponseData & XI3CPSX_RESPONSE_ERR_STS_MASK) >> XI3CPSX_RESPONSE_ERR_STS_SHIFT;

		XI3cPsx_DisableInterrupts(InstancePtr->Config.BaseAddress,
					  XI3CPSX_INTR_RESP_READY);

		/* Signal completion so a waiting caller (e.g. SendEnecDisec)
		 * need not read the response queue and race this ISR. */
		InstancePtr->RespDone = 1U;

		if (InstancePtr->StatusHandler != NULL) {
			InstancePtr->StatusHandler(InstancePtr->CallBackRef, InstancePtr->Error);
		}
	}
}

/*****************************************************************************/
/**
* @brief
* This function sets the IBI (SIR) handler, which the driver calls from
* interrupt context when an In-Band Interrupt is received from a target.
*
* @param	InstancePtr is a pointer to the XI3cPsx instance.
* @param	CallBackRef is the upper-layer callback reference, passed back
*		to the handler when it is invoked.
* @param	FunctionPtr is the pointer to the IBI callback function.
*
* @note		The handler runs in interrupt context and must copy the payload
*		out immediately; the payload buffer is reused on the next IBI.
*
******************************************************************************/
void XI3cPsx_SetIbiHandler(XI3cPsx *InstancePtr, void *CallBackRef,
			   XI3cPsx_IbiHandler FunctionPtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(FunctionPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

	InstancePtr->IbiHandler = FunctionPtr;
	InstancePtr->IbiCallBackRef = CallBackRef;
}

/*****************************************************************************/
/**
* @brief
* This function programs the IBI defaults. It rejects all Slave-Interrupt
* (SIR), Master-Request (MR) and Hot-Join events, and sets the IBI status
* queue threshold so that a single IBI generates an interrupt. Individual
* targets are later opted in with XI3cPsx_EnableIbi(); Hot-Join is opted in
* with XI3cPsx_EnableHotJoin().
*
* @param	InstancePtr is a pointer to the XI3cPsx instance.
*
* @return	None.
*
* @note		The IBI threshold interrupt itself is not enabled here; it is
*		enabled lazily when the first target's IBI (or Hot-Join) is
*		enabled.
*
******************************************************************************/
static void XI3cPsx_ConfigIbi(XI3cPsx *InstancePtr)
{
	u32 ThldReg;
	u32 DevCtrl;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

	InstancePtr->SirRejectMask = 0xFFFFFFFFU;
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress,
			 XI3CPSX_IBI_SIR_REQ_REJECT, 0xFFFFFFFFU);
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress,
			 XI3CPSX_IBI_MR_REQ_REJECT, 0xFFFFFFFFU);

	/* Reject Hot-Join until XI3cPsx_EnableHotJoin() opts in. */
	DevCtrl = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, XI3CPSX_DEVICE_CTRL);
	DevCtrl |= XI3CPSX_DEVICE_CTRL_HOT_JOIN_CTRL_MASK;
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_DEVICE_CTRL, DevCtrl);

	/*
	 * Program QUEUE_THLD_CTRL in one write: IBI status threshold 0
	 * (interrupt per IBI) and a non-zero IBI data segment size so
	 * IBI-with-data payload is buffered into the IBI queue.
	 */
	ThldReg = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress,
				  XI3CPSX_QUEUE_THLD_CTRL);
	ThldReg &= ~(XI3CPSX_QUEUE_THLD_CTRL_IBI_STATUS_THLD_MASK |
		     XI3CPSX_QUEUE_THLD_CTRL_IBI_DATA_THLD_MASK);
	ThldReg |= (XI3CPSX_IBI_DATA_SEGMENT_DWORDS <<
		    XI3CPSX_QUEUE_THLD_CTRL_IBI_DATA_THLD_SHIFT) &
		   XI3CPSX_QUEUE_THLD_CTRL_IBI_DATA_THLD_MASK;
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress,
			 XI3CPSX_QUEUE_THLD_CTRL, ThldReg);
}

/*****************************************************************************/
/**
* @brief
* This function captures the Bus Characteristic Register (BCR) of every
* enumerated target from the Device Characteristic Table (DCT) into the
* driver instance. The BCR is used to determine whether a target appends a
* mandatory data byte to its IBI.
*
* @param	InstancePtr is a pointer to the XI3cPsx instance.
*
* @return	None.
*
* @note		Call after a successful ENTDAA so the DCT is populated.
*
******************************************************************************/
static void XI3cPsx_CaptureBcr(XI3cPsx *InstancePtr)
{
	u32 Index;
	u32 DctWord;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

	for (Index = 0U; (Index < InstancePtr->Config.DeviceCount) &&
	     (Index < InstancePtr->MaxDevs); Index++) {
		DctWord = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress,
					  XI3CPSX_DEV_CHAR_TABLE1_LOC3 +
					  (Index * XI3CPSX_DCT_ENTRY_BYTES));
		InstancePtr->Bcr[Index] =
			(u8)((DctWord & XI3CPSX_DEV_CHAR_TABLE1_LOC3_BCR_MASK) >>
			     XI3CPSX_DEV_CHAR_TABLE1_LOC3_BCR_SHIFT);
	}
}

/*****************************************************************************/
/**
* @brief
* This function sends a direct ENEC or DISEC CCC to a single target to
* enable or disable its Slave-Interrupt (SIR) event generation.
*
* @param	InstancePtr is a pointer to the XI3cPsx instance.
* @param	DevIndex is the Device Address Table index of the target.
* @param	Enable is TRUE to send ENEC(SIR), FALSE to send DISEC(SIR).
*
* @return
*		- XST_SUCCESS if the CCC completed without error.
*		- XST_SEND_ERROR if the payload could not be sent.
*		- XST_TIMEOUT if the ISR never signaled completion.
*		- XST_FAILURE if the CCC completed with a bus-level error.
*
******************************************************************************/
static s32 XI3cPsx_SendEnecDisec(XI3cPsx *InstancePtr, u32 DevIndex, u8 Enable)
{
	XI3cPsx_Cmd Cmd = {0};
	u8 Event = (u8)I3C_CCC_EVENT_SIR;
	u8 Ccc;
	u32 Timeout;
	s32 Status;

	Ccc = (u8)(Enable ? I3C_CCC_ENEC(0) : I3C_CCC_DISEC(0));

	Cmd.TransArg = COMMAND_PORT_ARG_DATA_LEN(1U) | COMMAND_PORT_TRANSFER_ARG;
	Cmd.TransCmd = (COMMAND_PORT_SPEED(0) |
			COMMAND_PORT_DEV_INDEX(DevIndex) |
			COMMAND_PORT_CMD(Ccc) |
			COMMAND_PORT_CP |
			COMMAND_PORT_TOC |
			COMMAND_PORT_ROC);

	/*
	 * Send interrupt-driven so the ISR is the single response consumer and
	 * the caller only waits (RespDone); a polled send would race the live
	 * ISR, which clears the response first, and report a false send error.
	 */
	InstancePtr->Error = 0U;
	InstancePtr->RespDone = 0U;
	/* CCC send has no data-in phase; ensure the ISR does not drain RX. */
	InstancePtr->RecvByteCount = 0U;

	Status = XI3cPsx_MasterSend(InstancePtr, &Event, 1, Cmd);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* Wait for the ISR to consume the response (it sets RespDone). */
	Timeout = XI3CPSX_TIMEOUT_COUNTER;
	while ((InstancePtr->RespDone == 0U) && (--Timeout != 0U)) {
		/* Busy-wait for the response interrupt. */
	}

	if (Timeout == 0U) {
		XI3cPsx_DisableInterrupts(InstancePtr->Config.BaseAddress,
					  XI3CPSX_INTR_RESP_READY | XI3CPSX_INTR_TX_THLD);
		return XST_TIMEOUT;
	}

	/* The bus-level result was captured by the ISR into InstancePtr->Error. */
	Status = (InstancePtr->Error == 0U) ? XST_SUCCESS : XST_FAILURE;

	return Status;
}

/*****************************************************************************/
/**
* @brief
* This function enables In-Band Interrupt (SIR) reception for a single
* target. It clears the SIR reject in the target's DAT entry and in the
* global reject register, sets IBI-with-data if the target's BCR advertises
* a mandatory data byte, lazily enables the IBI threshold interrupt, and
* sends ENEC(SIR) to the target.
*
* @param	InstancePtr is a pointer to the XI3cPsx instance.
* @param	DevIndex is the Device Address Table index of the target
*		(same index used during dynamic address assignment).
*
* @return
*		- XST_SUCCESS if everything went well.
*		- XST_SEND_ERROR if the ENEC(SIR) payload could not be sent.
*		- XST_TIMEOUT if the IBI queue reset or the ENEC(SIR) response
*		  never completed.
*		- XST_FAILURE if any other error.
*
******************************************************************************/
s32 XI3cPsx_EnableIbi(XI3cPsx *InstancePtr, u32 DevIndex)
{
	u32 DatReg;
	u32 DatLoc;
	u32 Poll;
	u32 Timeout;
	u32 WasAllRejected;
	s32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

	if (DevIndex >= InstancePtr->MaxDevs) {
		return XST_FAILURE;
	}

	DatLoc = XI3cPsx_DatEntryOffset(InstancePtr, DevIndex);
	DatReg = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, DatLoc);

	/* No dynamic address means the target was never enumerated; reject. */
	if ((DatReg & XI3CPSX_DEV_ADDR_TABLE_LOC1_DEV_DYNAMIC_ADDR_MASK) == 0U) {
		return XST_FAILURE;
	}

	WasAllRejected = (InstancePtr->SirRejectMask == 0xFFFFFFFFU) ? 1U : 0U;

	/*
	 * Flush stale IBI queue entries (not cleared by the generic FIFO
	 * reset). Only while no target is armed, so arming a second target
	 * cannot discard IBIs the first one already queued.
	 */
	if (WasAllRejected != 0U) {
		Timeout = XI3CPSX_TIMEOUT_COUNTER;
		XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress,
				 XI3CPSX_RESET_CTRL,
				 XI3CPSX_RESET_CTRL_BI_QUEUE_RST_MASK);
		do {
			Poll = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress,
					       XI3CPSX_RESET_CTRL) &
			       XI3CPSX_RESET_CTRL_BI_QUEUE_RST_MASK;
		} while ((Poll != 0U) && (--Timeout != 0U));

		if (Poll != 0U) {
			return XST_TIMEOUT;
		}
	}

	/* Update the DAT entry: accept SIR, request IBI-with-data if applicable. */
	DatReg &= ~XI3CPSX_DEV_ADDR_TABLE_LOC1_SIR_REJECT_MASK;
	if (InstancePtr->Bcr[DevIndex] & XI3CPSX_I3C_BCR_IBI_PAYLOAD) {
		DatReg |= XI3CPSX_DEV_ADDR_TABLE_LOC1_IBI_WITH_DATA_MASK;
	} else {
		DatReg &= ~XI3CPSX_DEV_ADDR_TABLE_LOC1_IBI_WITH_DATA_MASK;
	}
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, DatLoc, DatReg);

	/* Clear the per-device bit in the global SIR reject register. */
	InstancePtr->SirRejectMask &= ~((u32)1U << DevIndex);
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress,
			 XI3CPSX_IBI_SIR_REQ_REJECT, InstancePtr->SirRejectMask);

	/* Tell the target to start generating SIRs; IBI_THLD stays disabled until ENEC completes. */
	Status = XI3cPsx_SendEnecDisec(InstancePtr, DevIndex, TRUE);
	if (Status != XST_SUCCESS) {
		/* Roll back on failure. */
		(void)XI3cPsx_DisableIbi(InstancePtr, DevIndex);
		return Status;
	}

	/* Enable the IBI threshold interrupt once the first target is armed. */
	if (WasAllRejected != 0U) {
		XI3cPsx_EnableInterrupts(InstancePtr->Config.BaseAddress,
					 XI3CPSX_INTR_IBI_THLD);
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief
* This function disables In-Band Interrupt (SIR) reception for a single
* target. It sends DISEC(SIR) to the target, sets the SIR reject in both the
* DAT entry and the global reject register, and disables the IBI threshold
* interrupt if no targets remain armed.
*
* @param	InstancePtr is a pointer to the XI3cPsx instance.
* @param	DevIndex is the Device Address Table index of the target.
*
* @return
*		- XST_SUCCESS if everything went well.
*		- XST_SEND_ERROR if the DISEC(SIR) payload could not be sent.
*		- XST_TIMEOUT if the DISEC(SIR) response never completed.
*		- XST_FAILURE if any other error.
*
******************************************************************************/
s32 XI3cPsx_DisableIbi(XI3cPsx *InstancePtr, u32 DevIndex)
{
	u32 DatReg;
	u32 DatLoc;
	s32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

	if (DevIndex >= InstancePtr->MaxDevs) {
		return XST_FAILURE;
	}

	/* Tell the target to stop generating SIRs first. */
	Status = XI3cPsx_SendEnecDisec(InstancePtr, DevIndex, FALSE);

	/* Set the SIR reject in the DAT entry. */
	DatLoc = XI3cPsx_DatEntryOffset(InstancePtr, DevIndex);
	DatReg = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, DatLoc);
	DatReg |= XI3CPSX_DEV_ADDR_TABLE_LOC1_SIR_REJECT_MASK;
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, DatLoc, DatReg);

	/* Set the per-device bit in the global SIR reject register. */
	InstancePtr->SirRejectMask |= ((u32)1U << DevIndex);
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress,
			 XI3CPSX_IBI_SIR_REQ_REJECT, InstancePtr->SirRejectMask);

	/*
	 * Disable the IBI threshold interrupt when no targets remain armed
	 * and Hot-Join is also rejected; either one alone still needs it.
	 */
	if ((InstancePtr->SirRejectMask == 0xFFFFFFFFU) &&
	    (XI3cPsx_IsHotJoinRejected(InstancePtr) != 0U)) {
		XI3cPsx_DisableInterrupts(InstancePtr->Config.BaseAddress,
					  XI3CPSX_INTR_IBI_THLD);
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief
* This function returns whether Hot-Join requests are currently rejected at
* the controller (DEVICE_CTRL.HOT_JOIN_CTRL).
*
* @param	InstancePtr is a pointer to the XI3cPsx instance.
*
* @return	1 if Hot-Join is rejected, 0 if it is accepted.
*
******************************************************************************/
static u32 XI3cPsx_IsHotJoinRejected(XI3cPsx *InstancePtr)
{
	return ((XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, XI3CPSX_DEVICE_CTRL) &
		 XI3CPSX_DEVICE_CTRL_HOT_JOIN_CTRL_MASK) != 0U) ? 1U : 0U;
}

/*****************************************************************************/
/**
* @brief
* This function enables Hot-Join request acceptance. It clears
* DEVICE_CTRL.HOT_JOIN_CTRL so the controller ACKs a Hot-Join request, and
* lazily enables the IBI threshold interrupt (shared with SIR) since
* Hot-Join requests are reported through the same IBI queue.
*
* @param	InstancePtr is a pointer to the XI3cPsx instance.
*
* @note		A Hot-Join sets HotJoinPending and invokes the Hot-Join
*		handler from the ISR. The blocking ENTDAA enrolment is deferred:
*		the application must call XI3cPsx_HotJoinDaa() from thread
*		context to assign the new target its dynamic address.
*
******************************************************************************/
void XI3cPsx_EnableHotJoin(XI3cPsx *InstancePtr)
{
	u32 DevCtrl;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

	DevCtrl = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, XI3CPSX_DEVICE_CTRL);
	DevCtrl &= ~XI3CPSX_DEVICE_CTRL_HOT_JOIN_CTRL_MASK;
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_DEVICE_CTRL, DevCtrl);

	XI3cPsx_EnableInterrupts(InstancePtr->Config.BaseAddress, XI3CPSX_INTR_IBI_THLD);
}

/*****************************************************************************/
/**
* @brief
* This function disables Hot-Join request acceptance. It sets
* DEVICE_CTRL.HOT_JOIN_CTRL so the controller NACKs a Hot-Join request, and
* disables the IBI threshold interrupt if no SIR target remains armed either.
*
* @param	InstancePtr is a pointer to the XI3cPsx instance.
*
******************************************************************************/
void XI3cPsx_DisableHotJoin(XI3cPsx *InstancePtr)
{
	u32 DevCtrl;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

	DevCtrl = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, XI3CPSX_DEVICE_CTRL);
	DevCtrl |= XI3CPSX_DEVICE_CTRL_HOT_JOIN_CTRL_MASK;
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_DEVICE_CTRL, DevCtrl);

	if (InstancePtr->SirRejectMask == 0xFFFFFFFFU) {
		XI3cPsx_DisableInterrupts(InstancePtr->Config.BaseAddress,
					  XI3CPSX_INTR_IBI_THLD);
	}
}

/*****************************************************************************/
/**
* @brief
* This function sets the Hot-Join notification handler, which the driver
* calls from interrupt context to flag a Hot-Join request. The joining
* target is NOT yet enrolled when the handler runs; the application must
* run enrolment (ENTDAA) via XI3cPsx_HotJoinDaa() from thread context.
*
* @param	InstancePtr is a pointer to the XI3cPsx instance.
* @param	CallBackRef is the upper-layer callback reference, passed back
*		to the handler when it is invoked.
* @param	FunctionPtr is the pointer to the Hot-Join callback function.
*
* @note		The handler runs in interrupt context, so its work should be
*		minimized (e.g. record the event and return).
*
******************************************************************************/
void XI3cPsx_SetHjHandler(XI3cPsx *InstancePtr, void *CallBackRef,
			  XI3cPsx_HjHandler FunctionPtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(FunctionPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

	InstancePtr->HjHandler = FunctionPtr;
	InstancePtr->HjCallBackRef = CallBackRef;
}

/*****************************************************************************/
/**
* @brief
* This function services a Hot-Join request by re-running ENTDAA (without
* RSTDAA, so existing targets keep their address) and refreshing the
* captured BCR of every enumerated target on success.
*
* @param	InstancePtr is a pointer to the XI3cPsx instance.
*
* @return
*		- XST_SUCCESS if everything went well.
*		- XST_FAILURE if any error.
*
* @note		Must be called from thread (non-interrupt) context - e.g.
*		from the application after a Hot-Join notification. The ISR
*		only flags the Hot-Join via HotJoinPending. The ENTDAA is
*		issued interrupt-driven so the live ISR is the single response
*		consumer; a polled send would race the ISR (which clears the
*		response first) and time out.
*
******************************************************************************/
s32 XI3cPsx_HotJoinDaa(XI3cPsx *InstancePtr)
{
	XI3cPsx_Cmd Cmd = {0};
	u32 Timeout;
	s32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

	InstancePtr->HotJoinPending = 0U;

	Cmd.TransArg = COMMAND_PORT_TRANSFER_ARG;
	Cmd.TransCmd = (COMMAND_PORT_DEV_COUNT(InstancePtr->Config.DeviceCount) |
			COMMAND_PORT_DEV_INDEX(0) |
			COMMAND_PORT_CMD(I3C_CCC_ENTDAA) |
			COMMAND_PORT_ADDR_ASSGN_CMD |
			COMMAND_PORT_TOC |
			COMMAND_PORT_ROC);

	InstancePtr->Error = 0U;
	InstancePtr->RespDone = 0U;
	InstancePtr->RecvByteCount = 0U;

	XI3cPsx_EnableInterrupts(InstancePtr->Config.BaseAddress,
				 XI3CPSX_INTR_RESP_READY);
	XI3cPsx_WrCmdFifo(InstancePtr, &Cmd);

	/* Wait for the ISR to consume the response (it sets RespDone). */
	Timeout = XI3CPSX_TIMEOUT_COUNTER;
	while ((InstancePtr->RespDone == 0U) && (--Timeout != 0U)) {
		/* Busy-wait for the response interrupt. */
	}

	if (Timeout == 0U) {
		XI3cPsx_DisableInterrupts(InstancePtr->Config.BaseAddress,
					  XI3CPSX_INTR_RESP_READY);
		return XST_TIMEOUT;
	}

	/* The bus-level result was captured by the ISR into InstancePtr->Error. */
	Status = (InstancePtr->Error == 0U) ? XST_SUCCESS : XST_FAILURE;
	if (Status == XST_SUCCESS) {
		XI3cPsx_CaptureBcr(InstancePtr);
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief
* This function returns the number of IBI status entries currently queued.
*
* @param	InstancePtr is a pointer to the XI3cPsx instance.
*
* @return	Number of queued IBI status entries.
*
******************************************************************************/
static u32 XI3cPsx_IbiStsLevel(XI3cPsx *InstancePtr)
{
	return (XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress,
				XI3CPSX_QUEUE_STATUS_LEVEL) &
		XI3CPSX_QUEUE_STATUS_LEVEL_IBI_STS_CNT_MASK) >>
	       XI3CPSX_QUEUE_STATUS_LEVEL_IBI_STS_CNT_SHIFT;
}

/*****************************************************************************/
/**
* @brief
* This function drains and discards Len bytes of IBI payload from the IBI
* queue port. Used for IBI types the driver does not handle.
*
* @param	InstancePtr is a pointer to the XI3cPsx instance.
* @param	Len is the payload length in bytes.
*
* @return	None.
*
******************************************************************************/
static void XI3cPsx_IbiDrain(XI3cPsx *InstancePtr, u32 Len)
{
	u32 Words = XI3CPSX_CEIL_DIV(Len, 4U);
	u32 Index;

	for (Index = 0U; Index < Words; Index++) {
		(void)XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress,
				      XI3CPSX_IBI_QUEUE_STATUS);
	}
}

/*****************************************************************************/
/**
* @brief
* This function reads an IBI payload from the IBI queue port. It pops
* ceil(QueueLen/4) words so the whole payload is removed from the queue, but
* copies at most CopyLen bytes into Buf. Reading the full queue length even
* when CopyLen is smaller keeps the IBI queue from desynchronising.
*
* @param	InstancePtr is a pointer to the XI3cPsx instance.
* @param	Buf is the destination buffer.
* @param	CopyLen is the maximum number of payload bytes to copy to Buf.
* @param	QueueLen is the number of payload bytes present in the queue.
*
* @return	The number of bytes copied into Buf.
*
******************************************************************************/
static u32 XI3cPsx_IbiReadPayload(XI3cPsx *InstancePtr, u8 *Buf, u32 CopyLen,
				  u32 QueueLen)
{
	u32 Words = XI3CPSX_CEIL_DIV(QueueLen, 4U);
	u32 Word;
	u32 ByteIdx = 0U;
	u32 Index;
	u32 Shift;

	for (Index = 0U; Index < Words; Index++) {
		Word = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress,
				       XI3CPSX_IBI_QUEUE_STATUS);
		for (Shift = 0U; Shift < 4U; Shift++) {
			if (ByteIdx < CopyLen) {
				Buf[ByteIdx] = (u8)((Word >> (8U * Shift)) & 0xFFU);
			}
			ByteIdx++;
		}
	}

	return (CopyLen < ByteIdx) ? CopyLen : ByteIdx;
}

/*****************************************************************************/
/**
* @brief
* This function services the IBI threshold interrupt. It drains every queued
* IBI status entry, decodes the IBI type, and for a Slave-Interrupt (SIR)
* reads the payload and dispatches it to the registered IBI handler.
*
* @param	InstancePtr is a pointer to the XI3cPsx instance.
*
* @return	None.
*
******************************************************************************/
static void XI3cPsx_HandleIbi(XI3cPsx *InstancePtr)
{
	u32 Count;
	u32 Index;
	u32 Status;
	u32 IbiId;
	u32 Len;
	u32 Addr;
	u32 Rnw;

	/* Read the queued IBI status count and process that many entries. */
	Count = XI3cPsx_IbiStsLevel(InstancePtr);

	for (Index = 0U; Index < Count; Index++) {
		Status = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress,
					 XI3CPSX_IBI_QUEUE_STATUS);

		IbiId = (Status & XI3CPSX_IBI_QUEUE_STATUS_IBI_ID_MASK) >>
			XI3CPSX_IBI_QUEUE_STATUS_IBI_ID_SHIFT;
		Len = (Status & XI3CPSX_IBI_QUEUE_STATUS_DATA_LENGTH_MASK) >>
		      XI3CPSX_IBI_QUEUE_STATUS_DATA_LENGTH_SHIFT;
		Addr = IbiId >> 1;
		Rnw = IbiId & 1U;

		/* SIR: non-Hot-Join address with the read/not-write bit set. */
		if ((Addr != XI3CPSX_I3C_HOT_JOIN_ADDR) && (Rnw != 0U)) {
			u32 CopyLen = (Len > XI3CPSX_IBI_MAX_PAYLOAD) ?
				      XI3CPSX_IBI_MAX_PAYLOAD : Len;

			if (Len > 0U) {
				CopyLen = XI3cPsx_IbiReadPayload(InstancePtr,
								 InstancePtr->IbiData,
								 CopyLen, Len);
			}

			/* IbiData is reused next iteration; the handler must copy it out now. */
			if (InstancePtr->IbiHandler != NULL) {
				InstancePtr->IbiHandler(InstancePtr->IbiCallBackRef,
							Addr,
							InstancePtr->IbiData,
							CopyLen);
			}
		} else if ((Addr == XI3CPSX_I3C_HOT_JOIN_ADDR) && (Rnw == 0U)) {
			InstancePtr->HotJoinPending = 1U;
			if (InstancePtr->HjHandler != NULL) {
				InstancePtr->HjHandler(InstancePtr->HjCallBackRef);
			}
		} else {
			/* Non-SIR, non-Hot-Join entry (e.g. Master-Request): drain to keep the IBI queue in sync. */
			XI3cPsx_IbiDrain(InstancePtr, Len);
		}
	}
}
/** @} */
