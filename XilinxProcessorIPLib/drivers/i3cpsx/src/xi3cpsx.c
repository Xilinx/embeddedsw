
/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xi3cpsx.c
*
* @brief Implements I3C initialization and FIFO helpers.
*
* <pre> MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---  -------- ---------------------------------------------
* 1.00 	sd   11/21/21 First release
* 1.2   sd    2/12/23 Remove the hardcoding devices
*       	      Copy the input clock
* 1.3  sd   11/17/23 Added support for system device-tree flow.
*      sd   1/30/24  Moved prints under DEBUG flag.
* 1.8  vn   07/30/26 Fix Doxygen warnings
* 1.8  tb   09/22/26 Align Doxygen comments with AMD documentation standards
* </pre>
*
******************************************************************************/

/**
* @addtogroup i3cpsx_api I3CPSX APIs
* @{
*/

/***************************** Include Files *********************************/

#include "xi3cpsx.h"
#include "xi3cpsx_pr.h"
#include "sleep.h"

/*****************************************************************************/
/**
*
* @brief Initializes an XI3cPsx instance so the driver is ready to use.
*
* @param[in,out] InstancePtr Pointer to the XI3cPsx instance.
* @param[in] ConfigPtr Reference to a structure containing information
*            about a specific I3C device. This function initializes an
*            InstancePtr object for a specific device specified by the contents
*            of Config.
* @param[in] EffectiveAddr The device base address in the virtual memory
*            address space. The caller is responsible for keeping the address
*            mapping from EffectiveAddr to the device physical base address
*            unchanged once this function is invoked. Unexpected errors may
*            occur if the address mapping changes after this function is called.
*            If address translation is not used, use ConfigPtr->BaseAddress for
*            this parameter, passing the physical address instead.
*
* @return
*        - XST_SUCCESS if successful.
*        - XST_DEVICE_IS_STARTED if the instance is already initialized.
*        - XST_FAILURE if SCL clock programming or bus initialization fails
*          (master mode only).
*
******************************************************************************/
s32 XI3cPsx_CfgInitialize(XI3cPsx *InstancePtr, XI3cPsx_Config *ConfigPtr,
			  u32 EffectiveAddr)
{
	s32 Status;
	u32 Reg;
	u32 Index;

	/* Assert the arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);

	if (InstancePtr->IsReady == XIL_COMPONENT_IS_READY) {
		return XST_DEVICE_IS_STARTED;
	}

	/* Set the values read from the device config and the base address. */
#ifndef SDT
	InstancePtr->Config.DeviceId = ConfigPtr->DeviceId;
#endif
	InstancePtr->Config.BaseAddress = EffectiveAddr;
	InstancePtr->Config.DeviceCount = ConfigPtr->DeviceCount;
	InstancePtr->Config.InputClockHz = ConfigPtr->InputClockHz;

	/* Indicate the instance is now ready to use, initialized without error */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	/* Cleared before the first command, in case the memory was not. */
	InstancePtr->Error = 0U;
	InstancePtr->LastDatIndex = 0U;
	InstancePtr->LastTransCmd = 0U;
	InstancePtr->OvfPending = 0U;

	/* Discover the DAT geometry, capped by the driver's per-device arrays. */
	Reg = XI3cPsx_ReadReg(EffectiveAddr, XI3CPSX_DEVICE_ADDR_TABLE_POINTER);
	InstancePtr->DatStartAddr = XI3CPSX_BASEADDR +
				    ((Reg &
				      XI3CPSX_DEVICE_ADDR_TABLE_POINTER_P_DEV_ADDR_TABLE_START_ADDR_MASK) >>
				     XI3CPSX_DEVICE_ADDR_TABLE_POINTER_P_DEV_ADDR_TABLE_START_ADDR_SHIFT);
	InstancePtr->MaxDevs = (Reg &
				XI3CPSX_DEVICE_ADDR_TABLE_POINTER_DEV_ADDR_TABLE_DEPTH_MASK) >>
			       XI3CPSX_DEVICE_ADDR_TABLE_POINTER_DEV_ADDR_TABLE_DEPTH_SHIFT;
	if (InstancePtr->MaxDevs > XI3CPSX_MAX_DAT_ENTRIES) {
		InstancePtr->MaxDevs = XI3CPSX_MAX_DAT_ENTRIES;
	}

	/*
	 * Default every target to "no IBI payload" so XI3cPsx_EnableIbi()
	 * cannot act on an uncaptured BCR.
	 */
	for (Index = 0U; Index < XI3CPSX_MAX_DAT_ENTRIES; Index++) {
		InstancePtr->Bcr[Index] = 0U;
	}
	InstancePtr->SirRejectMask = 0xFFFFFFFFU;
	InstancePtr->IbiHandler = NULL;
	InstancePtr->IbiCallBackRef = NULL;
	InstancePtr->RespDone = 0U;
	InstancePtr->HjHandler = NULL;
	InstancePtr->HjCallBackRef = NULL;
	InstancePtr->HotJoinPending = 0U;

	/*
	 * Reset the I3C controller to get it into its initial state. It is expected
	 * that device configuration will take place after this initialization
	 * is done, but before the device is started.
	 */
	XI3cPsx_Reset(InstancePtr);
	XI3cPsx_ResetFifos(InstancePtr);

	/*
	 * Set Response Buffer Threshold Value to zero to get the response
	 * interrupt status for single entry in Response Buffer.
	 */
	XI3cPsx_SetRespThreshold(InstancePtr, 0);

	/*
	 * Set Rx Buffer Threshold Value to zero to get the Rx interrupt status
	 * for single entry in Rx Buffer.
	 */
	XI3cPsx_SetRxThreshold(InstancePtr, 0);

	/*
	 * Set Rx start threshold
	 */
	XI3cPsx_SetRxStartThreshold(InstancePtr, 0);

	/*
	 * Set Tx Buffer Threshold Value to zero to get the Tx interrupt status
	 * for single entry in Tx Buffer.
	 */
	XI3cPsx_SetTxThreshold(InstancePtr, 0);

	/*
	 * Set Tx start threshold
	 */
	XI3cPsx_SetTxStartThreshold(InstancePtr, 0);

	/*
	 * Set Cmd empty threshold
	 */
	XI3cPsx_SetCmdEmptyThreshold(InstancePtr, 0);

	/*
	 * For Slave mode, DeviceCount config parameter should be zero.
	 * SclkHz is always initialized to 0 so XI3cPsx_GetSClk() never
	 * returns garbage in slave mode.
	 */
	InstancePtr->SclkHz = 0U;

	if (ConfigPtr->DeviceCount != 0) {
		InstancePtr->SclkHz = I3C_BUS_TYP_I3C_SCL_RATE;

		Status = XI3cPsx_SetSClk(InstancePtr);
		if (Status != XST_SUCCESS) {
			InstancePtr->IsReady = 0U;
			return Status;
		}

		Status = XI3cPsx_BusInit(InstancePtr);
		if (Status != XST_SUCCESS) {
			InstancePtr->IsReady = 0U;
			return Status;
		}
	}

	return XST_SUCCESS;
}

/***************************************************************************/
/**
* @brief Writes data into the I3C transmit FIFO.
*
* @param[in,out] InstancePtr Pointer to the XI3cPsx instance.
* @param[in] TxBuf Pointer to the send buffer.
* @param[in] TxLen The number of bytes to be sent.
*
******************************************************************************/
void XI3cPsx_WrTxFifo(XI3cPsx *InstancePtr, u32 *TxBuf, u16 TxLen)
{
	u32 Data = 0;
	u16 Index;

	(void)TxBuf;

	while (TxLen > 0 && InstancePtr->SendByteCount > 0) {
		/*
		 * Clear the word accumulator every iteration. The partial
		 * last-word branch below builds the word with |=, so a stale
		 * value left over from the previous full word would bleed its
		 * bits into the trailing 1-3 bytes and corrupt them.
		 */
		Data = 0;

		if (InstancePtr->SendByteCount > 3) {
			Data = (u32)((*(InstancePtr->SendBufferPtr + 0) << 0) |
				     (*(InstancePtr->SendBufferPtr + 1) << 8) |
				     (*(InstancePtr->SendBufferPtr + 2) << 16)  |
				     (*(InstancePtr->SendBufferPtr + 3) << 24));

			InstancePtr->SendByteCount = InstancePtr->SendByteCount - 4;
			InstancePtr->SendBufferPtr = InstancePtr->SendBufferPtr + 4;
			TxLen -= 4;

		} else {
			for (Index = 0; Index < InstancePtr->SendByteCount; Index++) {
				Data |= (u32)(*(InstancePtr->SendBufferPtr + Index) << (8*Index));
			}
			InstancePtr->SendByteCount = 0;
			TxLen = 0;
		}

		XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_TX_RX_DATA_PORT, Data);
	}
}

/***************************************************************************/
/**
* @brief Writes a command into the I3C command FIFO.
*
* @param[in,out] InstancePtr Pointer to the XI3cPsx instance.
* @param[in] Cmd Pointer to the XI3cPsx_Cmd structure.
*
******************************************************************************/
void XI3cPsx_WrCmdFifo(XI3cPsx *InstancePtr, XI3cPsx_Cmd *Cmd)
{
	/*
	 * Remembered so an overflow reported later, possibly from the
	 * interrupt handler, can still be tied back to the target.
	 */
	InstancePtr->LastTransCmd = Cmd->TransCmd;
	InstancePtr->LastDatIndex = (u8)((Cmd->TransCmd >> 16) & 0x1FU);

	if (Cmd->TransArg)
		XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress,
				 XI3CPSX_COMMAND_QUEUE_PORT, Cmd->TransArg);

	if (Cmd->TransCmd)
		XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress,
				 XI3CPSX_COMMAND_QUEUE_PORT, Cmd->TransCmd);
}

/***************************************************************************/
/**
* @brief Reads data from the I3C receive FIFO.
*
* @param[in,out] InstancePtr Pointer to the XI3cPsx instance.
* @param[out] RxBuf Pointer to the recv buffer.
* @param[in] RxLen The number of bytes to be recv.
*
******************************************************************************/
void XI3cPsx_RdRxFifo(XI3cPsx *InstancePtr, u32 *RxBuf, u16 RxLen)
{
	u32 Data;
	u16 Index;

	(void)RxBuf;

	/*
	 * RxLen comes from the word based FIFO level, so it can exceed the
	 * bytes left in the buffer.
	 */
	if (RxLen > InstancePtr->RecvByteCount) {
		RxLen = InstancePtr->RecvByteCount;
	}

	/* FIFO is word base, so read the data accordingly */
	while (RxLen > 0 && InstancePtr->RecvByteCount > 0) {
		Data = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, XI3CPSX_TX_RX_DATA_PORT);

		if (RxLen > 3) {
			*(InstancePtr->RecvBufferPtr + 0) = (u8)((Data >> 0) & 0xFF);
			*(InstancePtr->RecvBufferPtr + 1) = (u8)((Data >> 8) & 0xFF);
			*(InstancePtr->RecvBufferPtr + 2) = (u8)((Data >> 16) & 0xFF);
			*(InstancePtr->RecvBufferPtr + 3) = (u8)((Data >> 24) & 0xFF);

			InstancePtr->RecvByteCount = InstancePtr->RecvByteCount - 4;
			InstancePtr->RecvBufferPtr = InstancePtr->RecvBufferPtr + 4;
			RxLen = RxLen - 4;
		} else {
			for (Index = 0; Index < RxLen; Index++) {
				*(InstancePtr->RecvBufferPtr + Index) = (u8)((Data >> (8*Index)) & 0xFF);
			}
			InstancePtr->RecvByteCount -= RxLen;
			RxLen = 0;
		}
	}
}

/*****************************************************************************/
/**
* @brief Sends the transfer command.
*
* @param[in,out] InstancePtr Pointer to the XI3cPsx instance.
* @param[in] CmdCCC Pointer to the Command Info.
*
* @return
*        - XST_SUCCESS if successful.
*        - XST_FAILURE otherwise.
*
******************************************************************************/
s32 XI3cPsx_SendTransferCmd(XI3cPsx *InstancePtr, struct CmdInfo *CmdCCC)
{
	s32 Status = XST_FAILURE;
	XI3cPsx_Cmd Cmd = {0};

	/*
	 * Transfer Argument
	 */
	Cmd.TransArg = COMMAND_PORT_ARG_DATA_LEN(CmdCCC->RxLen) | COMMAND_PORT_TRANSFER_ARG;

	/*
	 * Transfer Command
	 * Refer Transfer command data structure in DWC MIPI I3C Master and Slave controller Databook
	 */
	Cmd.TransCmd = (COMMAND_PORT_SPEED(0) |	/* 21 - 23 Selecting SDR0 */
			       COMMAND_PORT_DEV_INDEX(CmdCCC->SlaveAddr) | /* 16 - 20 */
			       COMMAND_PORT_CMD(CmdCCC->Cmd) |	/* 7 - 14 Selecting broadcast*/
			       COMMAND_PORT_CP |	/* 15 */
			       COMMAND_PORT_TOC |	/* 30 */
			       COMMAND_PORT_ROC);	/* 26 */;
	if (CmdCCC->RxLen) {
		Cmd.TransCmd |= COMMAND_PORT_READ_TRANSFER;	/* 28 - For read */
	}

	if (CmdCCC->RxLen) {
		Status = XI3cPsx_MasterRecvPolled(InstancePtr, CmdCCC->RxBuff, CmdCCC->RxLen, &Cmd);
	} else {
		Status = XI3cPsx_MasterSendPolled(InstancePtr, NULL, 0, Cmd);
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief Sends the Address Assignment command.
*
* @param[in,out] InstancePtr Pointer to the XI3cPsx instance.
* @param[in] CmdCCC Pointer to the Command Info.
*
* @return
*        - XST_SUCCESS if successful.
*        - XST_FAILURE otherwise.
*
******************************************************************************/
s32 XI3cPsx_SendAddrAssignCmd(XI3cPsx *InstancePtr, struct CmdInfo *CmdCCC)
{
	s32 Status = XST_FAILURE;
	XI3cPsx_Cmd Cmd = {0};

	/*
	 * Transfer Argument
	 */
	Cmd.TransArg = COMMAND_PORT_TRANSFER_ARG;

	/*
	 * Transfer Command
	 * Refer Address assignment command data structure in DWC MIPI I3C Master and Slave controller Databook
	 */
	Cmd.TransCmd = ((InstancePtr->Config.DeviceCount << 21) |
			       COMMAND_PORT_DEV_INDEX(CmdCCC->SlaveAddr) | /* 16 - 20 */
			       COMMAND_PORT_CMD(CmdCCC->Cmd) |	/* 7 - 14 */
			       COMMAND_PORT_ADDR_ASSGN_CMD |	/* 0 - 2 */
			       COMMAND_PORT_TOC |	/* 30 */
			       COMMAND_PORT_ROC);	/* 26 */

	Status = XI3cPsx_MasterSendPolled(InstancePtr, NULL, 0, Cmd);

	return Status;
}
/** @} */
