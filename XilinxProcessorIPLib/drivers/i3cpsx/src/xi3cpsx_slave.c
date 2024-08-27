/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xi3cpsx_slave.c
* @addtogroup Overview
* @{
*
* Handles init functions.
*
* <pre> MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---  -------- ---------------------------------------------
* 1.00  sd  06/10/22 First release
* 1.1   sd  14/12/22 Fix warnings
* 1.2   sd  1/2/23   Write to resume bit
* 1.3 	sd   1/30/24  Moved prints under DEBUG flag.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xi3cpsx.h"
#include "sleep.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************* Variable Definitions *****************************/

/*****************************************************************************/
/**
* @brief
* This function sets up the device to be a slave.
*
* @param	InstancePtr is a pointer to the XI3cPsx instance.
* @param	SlaveAddr is the address of the slave we are receiving from.
*
* @return	None.
*
* @note
*	Interrupt is always enabled no matter the transfer is interrupt-
*	driven or polled mode. Whether device will be interrupted or not
*	depends on whether the device is connected to an interrupt
*	controller and interrupt for the device is enabled.
*
****************************************************************************/
void XI3cPsx_SetupSlave(XI3cPsx *InstancePtr, u16 SlaveAddr)
{
	u32 reg = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

	reg = SlaveAddr | XI3CPSX_DEVICE_ADDR_STATIC_ADDR_VALID_MASK;
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_DEVICE_ADDR, reg);

	reg = XI3CPSX_QUEUE_THLD_CTRL_CMD_EMPTY_BUF_THLD_DEFVAL;
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_QUEUE_THLD_CTRL, reg);

	reg = (XI3CPSX_DATA_BUFFER_THLD_CTRL_RX_START_THLD_DEFVAL <<
	       XI3CPSX_DATA_BUFFER_THLD_CTRL_RX_START_THLD_SHIFT)	|
	      XI3CPSX_DATA_BUFFER_THLD_CTRL_TX_EMPTY_BUF_THLD_DEFVAL;
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_DATA_BUFFER_THLD_CTRL, reg);

	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_BUS_FREE_AVAIL_TIMING,
			 (XI3CPSX_BUS_FREE_AVAIL_TIMING_BUS_AVAILABLE_TIME_VAL <<
			  XI3CPSX_BUS_FREE_AVAIL_TIMING_BUS_AVAILABLE_TIME_SHIFT) |
			 XI3CPSX_BUS_FREE_AVAIL_TIMING_BUS_FREE_TIME_DEFVAL);

	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_DEVICE_CTRL,
			 XI3CPSX_DEVICE_CTRL_ENABLE_MASK);
	/* Set the mode to slave */
	reg = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, XI3CPSX_QUEUE_THLD_CTRL);
	reg = reg | XI3CPSX_DEVICE_CTRL_EXTENDED_DEV_OPERATION_MODE_SLAVE ;
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_DEVICE_CTRL_EXTENDED, reg);

	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_INTR_STATUS_EN,
			 XI3CPSX_INTR_STATUS_EN_FULLMASK);
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_INTR_SIGNAL_EN,
			 XI3CPSX_INTR_SIGNAL_EN_FULLMASK);

	reg = (XI3CPSX_SLV_CHAR_CTRL_HDR_CAP_DEFVAL << XI3CPSX_SLV_CHAR_CTRL_HDR_CAP_SHIFT)|
	      (XI3CPSX_SLV_CHAR_CTRL_DCR_VAL << XI3CPSX_SLV_CHAR_CTRL_DCR_SHIFT)	   |
	      XI3CPSX_SLV_CHAR_CTRL_HDR_CAPABLE_MASK					   |
	      XI3CPSX_SLV_CHAR_CTRL_IBI_REQUEST_CAPABLE_MASK				   |
	      XI3CPSX_SLV_CHAR_CTRL_MAX_DATA_SPEED_LIMIT_MASK;

	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_SLV_CHAR_CTRL, reg);

	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_SLV_MIPI_ID_VALUE,
			 XI3CPSX_SLV_MIPI_ID_VALUE_SLV_PROV_ID_SEL_VAL);
}

/*****************************************************************************/
/**
* @brief
* This function setup a slave interrupt-driven receive.
* Data processing for the receive is handled by the interrupt handler.
*
* @param	InstancePtr is a pointer to the XI3cPsx instance.
* @param	MsgPtr is the pointer to the receive buffer.
* @param	ByteCount is the number of bytes to be received.
*
* @return	None.
*
* @note		This routine is for interrupt-driven transfer only.
*
****************************************************************************/
void XI3cPsx_SlaveRecv(XI3cPsx *InstancePtr, u8 *MsgPtr, s32 ByteCount)
{
	/*
	 * Assert validates the input arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(MsgPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(ByteCount != 0);

}

/*****************************************************************************/
/**
* @brief
* This function sends  a buffer in polled mode as a slave.
*
* @param	InstancePtr is a pointer to the XI3cPsx instance.
* @param	MsgPtr is the pointer to the send buffer.
* @param	ByteCount is the number of bytes to be sent.
* @param	Cmds is the instance of XI3cPsx_Cmd.
*
* @return
*		- XST_SUCCESS if everything went well.
*		- XST_FAILURE if master sends us data or master terminates the
*		transfer before all data has sent out.
*
* @note		This send routine is for polled mode transfer only.
*
****************************************************************************/
s32 XI3cPsx_SlaveSendPolled(XI3cPsx *InstancePtr, u8 *MsgPtr,
			    s32 ByteCount, XI3cPsx_Cmd Cmds)
{
	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

	if (ByteCount)
		XI3cPsx_WrTxFifo(InstancePtr, (u32 *)MsgPtr, ByteCount);

	/* Send command part to controller. It triggers the transfer */
	XI3cPsx_WrCmdFifo(InstancePtr, &Cmds);

	return (s32)XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief
* This function receives a buffer in polled mode as a slave.
*
* @param	InstancePtr is a pointer to the XI3cPsx instance.
* @param	MsgPtr is the pointer to the receive buffer.
*
* @return
*		- XST_SUCCESS if everything went well.
*		- XST_FAILURE if timed out.
*
* @note		This receive routine is for polled mode transfer only.
*
****************************************************************************/
s32 XI3cPsx_SlaveRecvPolled(XI3cPsx *InstancePtr, u8 *MsgPtr)
{
	u32 Reg;
	u32 Err;

	/*
	 * Assert validates the input arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(MsgPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

	InstancePtr->RecvBufferPtr = MsgPtr;

	Reg = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress,
			      XI3CPSX_INTR_STATUS);
	while (!(Reg & XI3CPSX_INTR_STATUS_RESP_READY_STS_MASK)) {
		Reg = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress,
				      XI3CPSX_INTR_STATUS);
	}
	Reg = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress,
			      XI3CPSX_RESPONSE_QUEUE_PORT);
	Err = Reg & XI3CPSX_TRANSFER_ERROR;
	if (Err != 0) {
		Reg = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress,
				      XI3CPSX_DEVICE_CTRL);
		Reg = Reg | XI3CPSX_DEVICE_CTRL_RESUME_MASK;
		XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_DEVICE_CTRL, Reg);
		return (s32)XST_FAILURE;
	}

	Reg = Reg & XI3CPSX_DATA_LEN;

	XI3cPsx_RdRxFifo(InstancePtr, (u32 *)MsgPtr, Reg);

	return (s32)XST_SUCCESS;
}

/*****************************************************************************/
/**
* The interrupt handler for slave mode. It does the protocol handling for
* the interrupt-driven transfers.
*
* Completion events and errors are signaled to upper layer for proper
* handling.
*
* The interrupts that are handled are:
* - DATA
*	If the instance is sending, it means that the master wants to read more
*	data from us. Send more data, and check whether we are done with this
*	send.
*
*	If the instance is receiving, it means that the master has written
* 	more data to us. Receive more data, and check whether we are done with
*	with this receive.
*
* - COMP
*	This marks that stop sequence has been sent from the master, transfer
*	is about to terminate. However, for receiving, the master may have
*	written us some data, so receive that first.
*
*	It is an error if the amount of transferred data is less than expected.
*
* - NAK
*	This marks that master does not want our data. It is for send only.
*
* - Other interrupts
*	These interrupts are marked as error.
*
*
* @param	InstancePtr is a pointer to the XIicPs instance.
*
* @return	None.
*
* @note 	None.
*
****************************************************************************/
void XI3cPsx_SlaveInterruptHandler(XI3cPsx *InstancePtr)
{

	/*
	 * Assert validates the input arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

}
/** @} */
