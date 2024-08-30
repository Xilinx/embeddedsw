/******************************************************************************
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/


/*****************************************************************************/
/**
 *
 * @file xilsfl_interface.c
 * @addtogroup xilsfl overview SFL APIs
 * @{
 *
 * This file implements the functions required to use the SFL interface hardware to
 * perform a transfer.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.0   sb  8/20/24  Initial release
 *
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xilsfl_control.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 *
 * This API can be used to Read/write to a flash register.
 *
 * @param	SflInstancePtr is a pointer to the interface driver component to use.
 * @param	RxBfrPtr is the pointer to store the read value from flash.
 * @param	TxBfrPtr is the pointer to value to be written into flash.
 * @param	CmdBufferPtr is the pointer that holds command, address and dummy.
 * @param	Addrvalid defines the specific read/write requires address or not.
 *
 * @return	XST_SUCCESS if successful, else error code.
 *
 * @note	This API can only be used for one flash at a time.
 *
 ******************************************************************************/
u32 XSfl_FlashRegisterReadWrite(XSfl_Interface *SflInstancePtr, u8 *RxBfrPtr,u8 *TxBfrPtr,
		u32 *CmdBufferPtr,u8 Addrvalid) {
	u32 Status;
	XSfl_Msg SflMsg = {0};
	u32 FCTIndex = SflInstancePtr->SflFlashInfo.FlashIndex;

	SflMsg.Opcode = (u8)CmdBufferPtr[XSFL_COMMAND_OFFSET];
	SflMsg.TxBfrPtr = TxBfrPtr;
	SflMsg.RxBfrPtr = RxBfrPtr;
	SflMsg.ByteCount = 1;
	SflMsg.Addrvalid = Addrvalid;
	SflMsg.DualByteOpCode = 0;
	SflMsg.Proto = (u8)Flash_Config_Table[FCTIndex].Proto;
	SflMsg.Addr = CmdBufferPtr[XSFL_ADDRESS_OFFSET];
	SflMsg.Addrsize = (u8)CmdBufferPtr[XSFL_ADDRESS_SIZE_OFFSET];
	SflMsg.Dummy = (u8)CmdBufferPtr[XSFL_DUMMY_OFFSET];

	if(SflInstancePtr->CntrlInfo.SdrDdrMode == XSFL_EDGE_MODE_DDR_PHY){
		SflMsg.Proto = (u8)Flash_Config_Table[FCTIndex].Proto >> 8;
		SflMsg.Addrsize = 4;
		SflMsg.ByteCount = 2;
	}

	if (Flash_Config_Table[FCTIndex].ExtOpCodeType == XSFL_DUAL_BYTE_OP_INVERT){
		SflMsg.DualByteOpCode = (u8)(~SflMsg.Opcode);
	}else if (Flash_Config_Table[FCTIndex].ExtOpCodeType == XSFL_DUAL_BYTE_OP_SAME){
		SflMsg.DualByteOpCode = (u8)(SflMsg.Opcode);
		SflMsg.Dummy += 8;
	}

	Status = SflInstancePtr->CntrlInfo.Transfer(SflInstancePtr->CntrlInfo.DeviceId, &SflMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************/
/**
 *
 * This API can be used to transfer the command to the flash.
 *
 * @param	SflInstancePtr is a pointer to the interface driver component to use.
 * @param	Cmd is the command to be transfered to the flash.
 *
 * @return	XST_SUCCESS if successful, else error code.
 *
 * @note	This API can only be used for one flash at a time.
 *
 ******************************************************************************/
u32 XSfl_FlashCmdTransfer(XSfl_Interface *SflInstancePtr,u8 Cmd){
	u32 Status;
	u32 FCTIndex = SflInstancePtr->SflFlashInfo.FlashIndex;
	XSfl_Msg SflMsg = {0};

	SflMsg.Opcode = Cmd;
	SflMsg.TxBfrPtr = NULL;
	SflMsg.RxBfrPtr = NULL;
	SflMsg.ByteCount = 0;
	SflMsg.Dummy = 0;
	SflMsg.Addr = 0;
	SflMsg.Addrsize = 0;
	SflMsg.Addrvalid = 0;
	SflMsg.DualByteOpCode = 0;
	SflMsg.Proto = (u8)Flash_Config_Table[FCTIndex].Proto;
	if (SflInstancePtr->CntrlInfo.SdrDdrMode == XSFL_EDGE_MODE_DDR_PHY) {
		SflMsg.Proto = (u8)(Flash_Config_Table[FCTIndex].Proto >> 8);
	}

	if (Flash_Config_Table[FCTIndex].ExtOpCodeType == XSFL_DUAL_BYTE_OP_INVERT){
		SflMsg.DualByteOpCode = (u8)(~SflMsg.Opcode);
	} else if (Flash_Config_Table[FCTIndex].ExtOpCodeType == XSFL_DUAL_BYTE_OP_SAME){
		SflMsg.DualByteOpCode = (u8)(SflMsg.Opcode);
	}

	Status = SflInstancePtr->CntrlInfo.Transfer(SflInstancePtr->CntrlInfo.DeviceId, &SflMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************/
/**
 *
 * This API can be used to read status of the flash status register.
 *
 * @param	SflInstancePtr is a pointer to the interface driver component to use.
 *
 * @return	XST_SUCCESS if successful, else error code.
 *
 * @note	This API can only be used for one flash at a time.
 *
 ******************************************************************************/
u32 XSfl_WaitforStatusDone(XSfl_Interface *SflInstancePtr){

	XSfl_Msg SflMsg = {0};
	u32 Status;
	u32 FCTIndex = SflInstancePtr->SflFlashInfo.FlashIndex;
#ifdef __ICCARM__
#pragma data_alignment = 4
	u8 FlashStatus[2];
#else
	u8 FlashStatus[2] __attribute__ ((aligned(4)));
#endif

	while (1) {
		SflMsg.Opcode = Flash_Config_Table[FCTIndex].StatusCmd;
		SflMsg.TxBfrPtr = NULL;
		SflMsg.RxBfrPtr = FlashStatus;
		SflMsg.ByteCount = 1;
		SflMsg.Dummy     = 0;
		SflMsg.Addrsize = 0;
		SflMsg.Addrvalid = 0;
		SflMsg.Addr = 0;
		SflMsg.DualByteOpCode = 0;
		SflMsg.Proto =  (u8)Flash_Config_Table[0].Proto;
		if (SflInstancePtr->CntrlInfo.SdrDdrMode == XSFL_EDGE_MODE_DDR_PHY) {
			SflMsg.Proto = (u8)(Flash_Config_Table[0].Proto >> 8);
			SflMsg.ByteCount = 2;
			SflMsg.Dummy += 8;
		}

		if (Flash_Config_Table[FCTIndex].ExtOpCodeType == XSFL_DUAL_BYTE_OP_INVERT){
			SflMsg.DualByteOpCode = (u8)(~SflMsg.Opcode);
		} else if (Flash_Config_Table[FCTIndex].ExtOpCodeType == XSFL_DUAL_BYTE_OP_SAME){
			SflMsg.DualByteOpCode = (u8)(SflMsg.Opcode);
		}

		Status = SflInstancePtr->CntrlInfo.Transfer(SflInstancePtr->CntrlInfo.DeviceId, &SflMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		if (Flash_Config_Table[FCTIndex].FSRFlag) {
			if ((FlashStatus[0] & 0x80) != 0) {
				break;
			}
		} else {
			if ((FlashStatus[0] & 0x01) == 0) {
				break;
			}
		}
	}

	return Status;
}

/*****************************************************************************/
/**
 *
 * This functions translates the address based on the type of interconnection.
 * In case of stacked, this function asserts the corresponding slave select.
 *
 * @param	SflInstancePtr is a pointer to the interface driver component to use.
 * @param	Address which is to be accessed (for erase, write or read)
 *
 * @return	RealAddr is the translated address - for single it is unchanged;
 *			for stacked, the lower flash size is subtracted;
 *
 * @note	In addition to get the actual address to work on flash this
 *			function also selects the CS based on the configuration detected.
 *
 ******************************************************************************/
u32 XSfl_GetRealAddr(XSfl_Interface *SflInstancePtr, u32 Address)
{
	u32 RealAddr = Address;
	u8 Chip_Sel = XSFL_SELECT_FLASH_CS0;
	u8 FCTIndex = SflInstancePtr->SflFlashInfo.FlashIndex;

	if ((SflInstancePtr->SflFlashInfo.ConnectionMode == XSFL_CONNECTION_MODE_STACKED) &&
			(Address & Flash_Config_Table[FCTIndex].FlashDeviceSize)) {
		Chip_Sel = XSFL_SELECT_FLASH_CS1;
		RealAddr = Address & (~Flash_Config_Table[FCTIndex].FlashDeviceSize);
	}

	/* Select Flash */
	(void)SflInstancePtr->CntrlInfo.SelectFlash(Chip_Sel);

	return RealAddr;
}
/** @} */
