/****************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*****************************************************************************/

/*****************************************************************************/
/**
* @file xospipsv_xfer.c
* @addtogroup ospipsv_api OSPIPSV APIs
* @{
*
* This file contaions the data transfer functions for the OSPIPSV driver,
* including polled and interrupt-based send/receive operations. These functions
* are used internally by the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.13  sb  02/07/25 First release
*
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/
#include "xospipsv_control.h"
#include "xospipsv_hw.h"

/*****************************************************************************/
/**
* @brief
* This function performs a polled data receive operation.
*
* @param	InstancePtr Pointer to the XOspiPsv instance.
* @param	Msg Pointer to the structure containing transfer data.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if transfer fails.
*
******************************************************************************/
u32 XOspiPsv_PollRecvData(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg){
	u32 Status;
	XOspiPsv_Setup_Dev_Read_Instr_Reg(InstancePtr, Msg);
	InstancePtr->RxBytes = Msg->ByteCount;
	InstancePtr->SendBufferPtr = NULL;
	InstancePtr->RecvBufferPtr = Msg->RxBfrPtr;
	if ((InstancePtr->OpMode == XOSPIPSV_IDAC_MODE) ||
			(Msg->Addrvalid == 0U)) {
		if ((Msg->Addrvalid == 0U) || ((Msg->ByteCount <= 8U) &&
					((Msg->Proto == XOSPIPSV_READ_1_1_1) || (Msg->Proto == XOSPIPSV_READ_8_8_8)))) {
			Status = XOspiPsv_Stig_Read(InstancePtr, Msg);
		} else {
			Status = XOspiPsv_Dma_Read(InstancePtr,Msg);
		}
	} else {
		Status = XOspiPsv_Dac_Read(InstancePtr, Msg);
	}
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This function performs a polled data send operation.
*
* @param	InstancePtr Pointer to the XOspiPsv instance.
* @param	Msg Pointer to the structure containing transfer data.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if transfer fails.
*
******************************************************************************/
u32 XOspiPsv_PollSendData(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg){
	u32 Status;
	XOspiPsv_Setup_Dev_Write_Instr_Reg(InstancePtr, Msg);
	InstancePtr->TxBytes = Msg->ByteCount;
	InstancePtr->SendBufferPtr = Msg->TxBfrPtr;
	InstancePtr->RecvBufferPtr = NULL;
	if((InstancePtr->OpMode == XOSPIPSV_DAC_MODE) &&
			(InstancePtr->TxBytes != 0U)) {
		Status = XOspiPsv_Dac_Write(InstancePtr, Msg);
	} else {
		if (InstancePtr->TxBytes > 8U) {
			XOspiPsv_ConfigureMux_Linear(InstancePtr);
			Status = XOspiPsv_IDac_Write(InstancePtr, Msg);
			XOspiPsv_ConfigureMux_Dma(InstancePtr);
		} else {
			Status = XOspiPsv_Stig_Write(InstancePtr, Msg);
		}
	}
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This function performs an interrupt-based data receive operation.
*
* @param	InstancePtr Pointer to the XOspiPsv instance.
* @param	Msg Pointer to the structure containing transfer data.
*
* @return	None.
*
******************************************************************************/
void XOspiPsv_IntrRecvData(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg){
	XOspiPsv_Setup_Dev_Read_Instr_Reg(InstancePtr, Msg);
	InstancePtr->RxBytes = Msg->ByteCount;
	InstancePtr->RecvBufferPtr = Msg->RxBfrPtr;
	InstancePtr->SendBufferPtr = NULL;
	if (Msg->Addrvalid == 0U) {
		XOspiPsv_Setup_Stig_Ctrl(InstancePtr, (u32)Msg->Opcode,
				1, (u32)InstancePtr->RxBytes - (u32)1, Msg->Addrvalid, 0,
				(u32)Msg->Addrsize - (u32)1, 0, 0, (u32)Msg->Dummy, 0);
		/* Execute the command */
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_FLASH_CMD_CTRL_REG,
				(XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
						  XOSPIPSV_FLASH_CMD_CTRL_REG) |
				 (u32)XOSPIPSV_FLASH_CMD_CTRL_REG_CMD_EXEC_FLD_MASK));
	} else {
		if (Msg->ByteCount >= (u32)4) {
			if ((Msg->ByteCount % 4U) != 0U) {
				InstancePtr->IsUnaligned = 1U;
			}
			Msg->ByteCount -= (Msg->ByteCount % 4U);
		} else {
			Msg->ByteCount = 4;
			Msg->RxBfrPtr = InstancePtr->UnalignReadBuffer;
			InstancePtr->IsUnaligned = 0U;
		}
		XOspiPsv_Config_Dma(InstancePtr,Msg);
		XOspiPsv_Config_IndirectAhb(InstancePtr,Msg);
		/* Start the transfer */
		XOspiPsv_Start_Indr_RdTransfer(InstancePtr);
	}
}

/*****************************************************************************/
/**
* @brief
* This function performs an interrupt-based data send operation.
*
* @param	InstancePtr Pointer to the XOspiPsv instance.
* @param	Msg Pointer to the structure containing transfer data.
*
* @return	None.
*
******************************************************************************/
void XOspiPsv_IntrSendData(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg){
	u32 ByteCount;
	u8 WriteDataEn;
	u32 Reqaddr;
	XOspiPsv_Setup_Dev_Write_Instr_Reg(InstancePtr, Msg);
	InstancePtr->TxBytes = Msg->ByteCount;
	InstancePtr->SendBufferPtr = Msg->TxBfrPtr;
	InstancePtr->RecvBufferPtr = NULL;
	if (Msg->Addrvalid != 0U) {
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_FLASH_CMD_ADDR_REG, Msg->Addr);
		Reqaddr = 1;
	} else {
		Reqaddr = 0U;
	}
	if (InstancePtr->TxBytes != 0U) {
		WriteDataEn = 1;
		ByteCount = InstancePtr->TxBytes;
		XOspiPsv_FifoWrite(InstancePtr, Msg);
	} else {
		WriteDataEn = 0U;
		ByteCount = 1;
	}
	XOspiPsv_Setup_Stig_Ctrl(InstancePtr, (u32)Msg->Opcode,
			0, 0, Reqaddr, 0, (u32)Msg->Addrsize - (u32)1,
			WriteDataEn, (u32)ByteCount - (u32)1, 0, 0);

	/* Execute the command */
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_FLASH_CMD_CTRL_REG,
			(XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
					  XOSPIPSV_FLASH_CMD_CTRL_REG) |
			 (u32)XOSPIPSV_FLASH_CMD_CTRL_REG_CMD_EXEC_FLD_MASK));
}
/** @} */
