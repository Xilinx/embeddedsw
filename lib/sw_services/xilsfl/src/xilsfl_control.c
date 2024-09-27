/******************************************************************************
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilsfl_control.c
 * @addtogroup xilsfl overview
 * @{
 *
 * The xilsfl_control.c file implements the low level functions used by the functions in
 * xilsfl.c file.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.0   sb  8/20/24  Initial release
 * 1.0   sb  9/25/24  Update XSfl_FlashReadProcess() to support unaligned bytes read and
 *                    add support for non-blocking read
 *
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xilsfl_control.h"

/************************** Constant Definitions *****************************/
#define XSFL_BUFFER_ARRAY_LEN 4

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
XSfl_Msg SflMsg= {0};

/*****************************************************************************/
/**
 *
 * Reads the flash ID and identifies the flash in FCT table.
 *
 * @param	SflInstancePtr is a pointer to the XSfl_Interface driver component to use.
 * @param	ChipSelNum is the chip select number.
 * @param	SflReadBuffer is pointer to the read buffer to store flash id.
 *
 * @return	XST_SUCCESS if successful, otherwise error code.
 *
 ******************************************************************************/
u32 XSfl_FlashIdRead(XSfl_Interface *SflInstancePtr, u8 ChipSelNum, u8 *SflReadBuffer ){
	u32 Status = 0;
	u32 ReadId = 0;
	u8 ReadIdBytes = 8;

	/* Validate the input arguments */
	Xil_AssertNonvoid(SflInstancePtr != NULL);

	SflMsg.Opcode= XSFL_READ_ID;
	SflMsg.RxBfrPtr = SflReadBuffer;
	SflMsg.TxBfrPtr = NULL;
	SflMsg.ByteCount = ReadIdBytes;
	SflMsg.Dummy = 0;
	SflMsg.Addrsize = 0;
	SflMsg.Addrvalid = 0;
	SflMsg.Proto = XSFL_FLASH_PROTO_1_1_1;

	Status = SflInstancePtr->CntrlInfo.SelectFlash(ChipSelNum);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = SflInstancePtr->CntrlInfo.Transfer(SflInstancePtr->CntrlInfo.DeviceId,&SflMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#ifdef XSFL_DEBUG
	xil_printf("FlashID=0x%x 0x%x 0x%x\n\r", SflReadBuffer[0], SflReadBuffer[1],
			SflReadBuffer[2]);
#endif

	SflInstancePtr->SflFlashInfo.FlashMake = SflReadBuffer[0];
	ReadId = ((SflReadBuffer[0] << 16) | (SflReadBuffer[1] << 8) | SflReadBuffer[2]);

	Status = XSfl_CalculateFCTIndex(ReadId, &SflInstancePtr->SflFlashInfo.FlashIndex);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	if(Flash_Config_Table[SflInstancePtr->SflFlashInfo.FlashIndex].FlashType ==  XSFL_OSPI_FLASH){
		*(SflInstancePtr->CntrlInfo.DeviceIdData) = (u32)((SflReadBuffer[3] << 24) | (SflReadBuffer[2] << 16) |
				(SflReadBuffer[1] << 8) | SflReadBuffer[0]);
	}
	return Status;
}

/*****************************************************************************/
/**
 * This API enters the flash device into 4 bytes addressing mode.
 * As per the Micron spec, before issuing the command to enter into 4 byte addr
 * mode, a write enable command is issued.
 *
 * @param	SflInstancePtr is a pointer to the XSfl_Interface driver component to use.
 * @param	Enable is a either 1 or 0 if 1 then enters 4 byte if 0 exits.
 * @param	ChipSelNum is the chip select number.
 *
 * @return	 - XST_SUCCESS if successful.
 * 		 - error code if it fails.
 *
 ******************************************************************************/
u32 XSfl_FlashEnterExit4BAddMode(XSfl_Interface *SflInstancePtr, int Enable, u8 ChipSelNum)
{
	u32 Status;
	u32 FlashMake;
	u8 FlashType;
	u8 Command;

	/* Validate the input arguments */
	Xil_AssertNonvoid(SflInstancePtr != NULL);

	FlashMake = SflInstancePtr->SflFlashInfo.FlashMake;
	FlashType = Flash_Config_Table[SflInstancePtr->SflFlashInfo.FlashIndex].FlashType;

	if (Enable) {
		Command = XSFL_ENTER_4B_ADDR_MODE;
	}
	else {
		Command = XSFL_EXIT_4B_ADDR_MODE;
	}

	/* Select Flash */
	Status = SflInstancePtr->CntrlInfo.SelectFlash(ChipSelNum);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XSfl_FlashCmdTransfer(SflInstancePtr, XSFL_WRITE_ENABLE_CMD);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Execute 4 byte command */
	Status = XSfl_FlashCmdTransfer(SflInstancePtr, Command);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XSfl_WaitforStatusDone(SflInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	switch (FlashMake) {
		case XSFL_MICRON_OCTAL_ID_BYTE0:
			Status = XSfl_FlashCmdTransfer(SflInstancePtr, XSFL_WRITE_DISABLE_CMD);
			if (Status != XST_SUCCESS) {
				Status = XST_FAILURE;
				break;
			}
			break;

		default:
			break;
	}

	if (SflInstancePtr->SflFlashInfo.ConnectionMode == XSFL_CONNECTION_MODE_STACKED){

		/* Select Flash */
		Status = SflInstancePtr->CntrlInfo.SelectFlash(XSFL_SELECT_FLASH_CS1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Status = XSfl_FlashCmdTransfer(SflInstancePtr,XSFL_WRITE_ENABLE_CMD);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/* Execute 4 byte command */
		Status = XSfl_FlashCmdTransfer(SflInstancePtr, Command);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Status = XSfl_WaitforStatusDone(SflInstancePtr);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		switch (FlashMake) {
			case XSFL_MICRON_OCTAL_ID_BYTE0:
				XSfl_FlashCmdTransfer(SflInstancePtr, XSFL_WRITE_DISABLE_CMD);
				if (Status != XST_SUCCESS) {
					Status = XST_FAILURE;
					break;
				}
				break;

			default:
				break;
		}
	}

	return Status;
}

/*****************************************************************************/
/**
 *
 * This function used to erase the sectors of the serial nor flash connected to the
 * specific interface.
 *
 * @param	Sfl_Handler is a pointer to the XSfl interface component to use.
 * @param	Address contains the address of the first sector which needs to
 *              be erased.
 *
 * @return	XST_SUCCESS if successful, else error code.
 *
 ******************************************************************************/
u32 XSfl_SectorErase(XSfl_Interface *SflInstancePtr, u32 Address)
{
	u32 Status;
	u32 RealAddr;
	u8 FCTIndex;

	/* Validate the input arguments */
	Xil_AssertNonvoid(SflInstancePtr != NULL);

	FCTIndex = SflInstancePtr->SflFlashInfo.FlashIndex;

#ifdef XSFL_DEBUG
	xil_printf("EraseCmd 0x%x\n\r", (u8)Flash_Config_Table[FCTIndex].EraseCmd);
#endif

	/*
	 * Translate address based on type of connection
	 * If stacked assert the slave select based on address
	 */
	RealAddr = XSfl_GetRealAddr(SflInstancePtr, Address);

	/*
	 * Send the write enable command to the Flash so that it can be
	 * written to, this needs to be sent as a separate transfer before
	 * the write
	 */

	Status = XSfl_FlashCmdTransfer(SflInstancePtr, XSFL_WRITE_ENABLE_CMD);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	SflMsg.Opcode = (u8)Flash_Config_Table[FCTIndex].EraseCmd;
	SflMsg.Addrsize = 4;
	SflMsg.Addrvalid = 1;
	SflMsg.TxBfrPtr = NULL;
	SflMsg.RxBfrPtr = NULL;
	SflMsg.ByteCount = 0;
	SflMsg.Addr = RealAddr;
	SflMsg.Proto = 0;
	SflMsg.Dummy = 0;
	SflMsg.Proto =  (u8)Flash_Config_Table[FCTIndex].Proto;
	if (SflInstancePtr->CntrlInfo.SdrDdrMode == XSFL_EDGE_MODE_DDR_PHY) {
		SflMsg.Proto = (u8)(Flash_Config_Table[FCTIndex].Proto >> 8);
	}

	SflMsg.DualByteOpCode = 0;
	if (Flash_Config_Table[FCTIndex].ExtOpCodeType == XSFL_DUAL_BYTE_OP_INVERT){
		SflMsg.DualByteOpCode = (u8)(~SflMsg.Opcode);
	} else if (Flash_Config_Table[FCTIndex].ExtOpCodeType == XSFL_DUAL_BYTE_OP_SAME){
		SflMsg.DualByteOpCode = (u8)(SflMsg.Opcode);
	}

	Status = SflInstancePtr->CntrlInfo.Transfer(SflInstancePtr->CntrlInfo.DeviceId, &SflMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XSfl_WaitforStatusDone(SflInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return 0;
}

/*****************************************************************************/
/**
 *
 * This function writes to the serial nor flash connected to the specific interface.
 *
 * @param	Sfl_Handler is a index to the Sfl interface component to use.
 * @param	Address contains the address of the first page which needs to
 *              be written.
 * @param	ByteCount contains the number of bytes to write.
 * @param	WriteBfrPtr is Pointer to the write buffer (which is to be transmitted)
 *
 * @return	XST_SUCCESS if successful, else Error code.
 *
 ******************************************************************************/
u32 XSfl_FlashPageWrite(XSfl_Interface *SflInstancePtr, u32 Address, u32 ByteCount,
		u8 *WriteBfrPtr)
{
	u32 Status;
	u32 Bytestowrite;
	u32 RealAddr;
	u8 FCTIndex;

	/* Validate the input arguments */
	Xil_AssertNonvoid(SflInstancePtr != NULL);

	FCTIndex = SflInstancePtr->SflFlashInfo.FlashIndex;

	while (ByteCount != 0) {
		/*
		 * Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		RealAddr = XSfl_GetRealAddr(SflInstancePtr, Address);

		/*
		 * Send the write enable command to the Flash so that it can be
		 * written to, this needs to be sent as a separate transfer before
		 * the write
		 */
		Status = XSfl_FlashCmdTransfer(SflInstancePtr, XSFL_WRITE_ENABLE_CMD);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		if (ByteCount <= 8) {
			Bytestowrite = ByteCount;
			ByteCount = 0;
		}
		else {
			Bytestowrite = 8;
			ByteCount -= 8;
		}

		SflMsg.Opcode = (u8)Flash_Config_Table[FCTIndex].WriteCmd;
		SflMsg.Addrvalid = 1;
		SflMsg.TxBfrPtr = WriteBfrPtr;
		SflMsg.RxBfrPtr = NULL;
		SflMsg.ByteCount = Bytestowrite;
		SflMsg.Proto =  (u8)Flash_Config_Table[FCTIndex].Proto;
		SflMsg.Dummy = 0;
		SflMsg.Addrsize = 4;
		SflMsg.Addr = RealAddr;
		SflMsg.DualByteOpCode = 0;

		if(Flash_Config_Table[FCTIndex].FlashType == XSFL_QSPI_FLASH) {
			SflMsg.Proto = (u8)(Flash_Config_Table[FCTIndex].Proto >> 16);
		}
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

		WriteBfrPtr += 8;
		Address += 8;

		Status = XSfl_WaitforStatusDone(SflInstancePtr);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

	}
	return 0;
}

/*****************************************************************************/
/**
 *
 * This function performs read from the serial nor flash connected to specific interface.
 *
 * @param	Sfl_Handler is a index to the XSfl interface component to use.
 * @param	Address contains the address of the flash that needs to be read.
 * @param	ByteCount contains the total size to be erased.
 * @param	ReadBfrPtr is the pointer to the read buffer to which valid received data should be
 * 			written
 * @param	RxAddr64bit is of the 64bit address of destination read buffer to which
 *              valid received data should be written.
 *
 * @return	XST_SUCCESS if successful, else Error code.
 *
 ******************************************************************************/
u32 XSfl_FlashReadProcess(XSfl_Interface *SflInstancePtr, u32 Address, u32 ByteCount,
		u8 *ReadBfrPtr, u64 RxAddr64bit){
	/* Validate the input arguments */
	Xil_AssertNonvoid(SflInstancePtr != NULL);

	u32 RealAddr;
	u32 BytesToRead;
	u32 ByteCnt = ByteCount;
	u32 FlashMake;
	u32 Addr = Address;
	u8 FCTIndex;
	u8 Status;

	FlashMake = SflInstancePtr->SflFlashInfo.FlashMake;
	FCTIndex = SflInstancePtr->SflFlashInfo.FlashIndex;

	if ((Address < Flash_Config_Table[FCTIndex].FlashDeviceSize) &&
			((Address + ByteCount) >= Flash_Config_Table[FCTIndex].FlashDeviceSize) &&
			(SflInstancePtr->SflFlashInfo.ConnectionMode)) {
		BytesToRead = (Flash_Config_Table[FCTIndex].FlashDeviceSize - Address);
	}
	else {
		BytesToRead = ByteCount;
	}

	while (ByteCount != 0) {
		/*
		 * Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		RealAddr = XSfl_GetRealAddr(SflInstancePtr, Address);

		SflMsg.Opcode = (u8)Flash_Config_Table[FCTIndex].ReadCmd;
		SflMsg.Addrsize = 4;
		SflMsg.Addrvalid = 1;
		SflMsg.TxBfrPtr = NULL;
		SflMsg.RxBfrPtr = ReadBfrPtr;
		SflMsg.ByteCount = BytesToRead;
		SflMsg.Xfer64bit = 0;
		if(RxAddr64bit >= XSFL_RXADDR_OVER_32BIT) {
			SflMsg.RxAddr64bit = RxAddr64bit;
			SflMsg.Xfer64bit = 1;
		}

		SflMsg.Addr = RealAddr;
		SflMsg.Proto =  (u8)Flash_Config_Table[FCTIndex].Proto;
		SflMsg.Dummy = (u8)Flash_Config_Table[FCTIndex].DummyCycles;
		SflMsg.DualByteOpCode = 0;

		if( Flash_Config_Table[FCTIndex].Proto >> 16){
			SflMsg.Proto = Flash_Config_Table[FCTIndex].Proto >> 16;
		}

		if (SflInstancePtr->CntrlInfo.SdrDdrMode == XSFL_EDGE_MODE_DDR_PHY) {
			SflMsg.Proto = Flash_Config_Table[FCTIndex].Proto >> 8;
			SflMsg.Dummy = Flash_Config_Table[FCTIndex].DummyCycles >> 8;
		}

#ifdef XSFL_DEBUG
		xil_printf("ReadCmd 0x%x\r\n", SflMsg.Opcode);
#endif

		if (Flash_Config_Table[FCTIndex].ExtOpCodeType == XSFL_DUAL_BYTE_OP_INVERT){
			SflMsg.DualByteOpCode = (u8)(~SflMsg.Opcode);
		} else if (Flash_Config_Table[FCTIndex].ExtOpCodeType == XSFL_DUAL_BYTE_OP_SAME){
			SflMsg.DualByteOpCode = (u8)(SflMsg.Opcode);
		}

		Status = SflInstancePtr->CntrlInfo.Transfer(SflInstancePtr->CntrlInfo.DeviceId, &SflMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		ByteCount -= BytesToRead;
		Address += BytesToRead;
		ReadBfrPtr += BytesToRead;
		BytesToRead = ByteCnt - BytesToRead;
	}

	return Status;
}

/*****************************************************************************/
/**
 *
 * This function performs non-blocking read from the serial nor flash connected to specific interface.
 *
 * @param	Sfl_Handler is a index to the XSfl interface component to use.
 * @param	Address contains the address of the flash that needs to be read.
 * @param	ByteCount contains the total size to be erased.
 * @param	ReadBfrPtr is the pointer to the read buffer to which valid received data should be
 * 			written
 * @param	RxAddr64bit is of the 64bit address of destination read buffer to which
 *              valid received data should be written.
 *
 * @return	XST_SUCCESS if successful, else Error code.
 *
 ******************************************************************************/
u32 XSfl_FlashNonBlockingReadProcess(XSfl_Interface *SflInstancePtr, u32 Address, u32 ByteCount,
		u8 *ReadBfrPtr, u64 RxAddr64bit){
	/* Validate the input arguments */
	Xil_AssertNonvoid(SflInstancePtr != NULL);

	u32 RealAddr;
	u32 BytesToRead;
	u32 ByteCnt = ByteCount;
	u32 FlashMake;
	u32 Addr = Address;
	u8 FCTIndex;
	u8 Status;

	FlashMake = SflInstancePtr->SflFlashInfo.FlashMake;
	FCTIndex = SflInstancePtr->SflFlashInfo.FlashIndex;

	if ((Address < Flash_Config_Table[FCTIndex].FlashDeviceSize) &&
			((Address + ByteCount) >= Flash_Config_Table[FCTIndex].FlashDeviceSize) &&
			(SflInstancePtr->SflFlashInfo.ConnectionMode)) {
		BytesToRead = (Flash_Config_Table[FCTIndex].FlashDeviceSize - Address);
	}
	else {
		BytesToRead = ByteCount;
	}

	while (ByteCount != 0) {
		/*
		 * Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		RealAddr = XSfl_GetRealAddr(SflInstancePtr, Address);

		SflMsg.Opcode = (u8)Flash_Config_Table[FCTIndex].ReadCmd;
		SflMsg.Addrsize = 4;
		SflMsg.Addrvalid = 1;
		SflMsg.TxBfrPtr = NULL;
		SflMsg.RxBfrPtr = ReadBfrPtr;
		SflMsg.ByteCount = BytesToRead;
		SflMsg.Xfer64bit = 0;
		if(RxAddr64bit >= XSFL_RXADDR_OVER_32BIT) {
			SflMsg.RxAddr64bit = RxAddr64bit;
			SflMsg.Xfer64bit = 1;
		}

		SflMsg.Addr = RealAddr;
		SflMsg.Proto =  (u8)Flash_Config_Table[FCTIndex].Proto;
		SflMsg.Dummy = (u8)Flash_Config_Table[FCTIndex].DummyCycles;
		SflMsg.DualByteOpCode = 0;

		if( Flash_Config_Table[FCTIndex].Proto >> 16){
			SflMsg.Proto = Flash_Config_Table[FCTIndex].Proto >> 16;
		}

		if (SflInstancePtr->CntrlInfo.SdrDdrMode == XSFL_EDGE_MODE_DDR_PHY) {
			SflMsg.Proto = Flash_Config_Table[FCTIndex].Proto >> 8;
			SflMsg.Dummy = Flash_Config_Table[FCTIndex].DummyCycles >> 8;
		}

#ifdef XSFL_DEBUG
		xil_printf("ReadCmd 0x%x\r\n", SflMsg.Opcode);
#endif

		if (Flash_Config_Table[FCTIndex].ExtOpCodeType == XSFL_DUAL_BYTE_OP_INVERT){
			SflMsg.DualByteOpCode = (u8)(~SflMsg.Opcode);
		} else if (Flash_Config_Table[FCTIndex].ExtOpCodeType == XSFL_DUAL_BYTE_OP_SAME){
			SflMsg.DualByteOpCode = (u8)(SflMsg.Opcode);
		}

		Status = SflInstancePtr->CntrlInfo.NonBlockingTransfer(SflInstancePtr->CntrlInfo.DeviceId, &SflMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		if ((SflInstancePtr->SflFlashInfo.ConnectionMode == XSFL_CONNECTION_MODE_STACKED) &&
				!(Address & Flash_Config_Table[FCTIndex].FlashDeviceSize) &&
				((Addr + ByteCount) > Flash_Config_Table[FCTIndex].FlashDeviceSize)) {
			Status = SflInstancePtr->CntrlInfo.TransferDone(SflInstancePtr->CntrlInfo.DeviceId);
			while (Status != XST_SUCCESS) {
				Status = SflInstancePtr->CntrlInfo.TransferDone(SflInstancePtr->CntrlInfo.DeviceId);
			}
		}

		ByteCount -= BytesToRead;
		Address += BytesToRead;
		ReadBfrPtr += BytesToRead;
		BytesToRead = ByteCnt - BytesToRead;
	}

	return Status;
}

/*****************************************************************************/
/**
 *
 * This function used to check  read completion status the serial Flash
 * connected to the Specific interface.
 *
 * @param	Sfl_Handler is a pointer to the XSfl interface driver component to use.
 *
 * @return	XST_SUCCESS if successful, else error code.
 *
 ******************************************************************************/
u32 XSfl_FlashTransferDone(XSfl_Interface *SflInstancePtr){
	/* Validate the input arguments */
	Xil_AssertNonvoid(SflInstancePtr != NULL);

	u32 Status;
	Status = SflInstancePtr->CntrlInfo.TransferDone(0);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************/
/**
 * This API enters the flash device into Octal DDR mode or exit from octal DDR
 * mode (switches to Extended SPI mode).
 *
 * @param	SflInstancePtr is a pointer to the XSfl_Interface driver component to use.
 * @param	Mode is either 1 or 0 if 1 then enter octal DDR mode if 0 exits.
 *
 * @return	 - XST_SUCCESS if successful.
 * 		 - error code if it fails.
 *
 ******************************************************************************/
u32 XSfl_FlashSetSDRDDRMode(XSfl_Interface *SflInstancePtr, int Mode,u8 *SflReadBuffer){
	/* Validate the input arguments */
	Xil_AssertNonvoid(SflInstancePtr != NULL);

	u32 Status;
	u32 Address = 0;
	/* By default disable dual byte opcode */
	u8 DualByteOpCode = XSFL_DUAL_BYTE_OP_DISABLE;
	u8 Write_Reg_Opcode = XSFL_WRITE_CONFIG_REG;
	u8 AddrSize = 3;
	u8 Addrvalid=0;
#ifdef __ICCARM__
#pragma data_alignment = 4
	u8 Data[2];
	u32 CmdBuffer[XSFLBUFFER_ARRAY_LEN];
#else
	u8 Data[2] __attribute__ ((aligned(4)));
	u32 CmdBuffer[XSFL_BUFFER_ARRAY_LEN] __attribute__ ((aligned(4)));
#endif
	u8 FCTIndex = SflInstancePtr->SflFlashInfo.FlashIndex;
	u8 FlashType = Flash_Config_Table[FCTIndex].FlashType;

	if (FlashType == XSFL_OSPI_FLASH) {
		if (Mode == XSFL_EDGE_MODE_DDR_PHY) {
			Data[0] = 0xE7;
			Data[1] = 0xE7;
		}
		else {
			Data[0] = 0xFF;
			Data[1] = 0xFF;
		}

		Status = SflInstancePtr->CntrlInfo.SelectFlash(XSFL_SELECT_FLASH_CS0);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/*
		 * Send the write enable command to the Flash so that it can be
		 * written to, this needs to be sent as a separate transfer before
		 * the write
		 */
		Status = XSfl_FlashCmdTransfer(SflInstancePtr, XSFL_WRITE_ENABLE_CMD);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/*  Fill Buffer with opcode, address and dummy */
		CmdBuffer[XSFL_COMMAND_OFFSET] = Write_Reg_Opcode;
		CmdBuffer[XSFL_ADDRESS_OFFSET] = Address;
		CmdBuffer[XSFL_ADDRESS_SIZE_OFFSET] = AddrSize;
		CmdBuffer[XSFL_DUMMY_OFFSET] = 0;

		Addrvalid = 1;
		/* Write Configuration register */
		Status = XSfl_FlashRegisterReadWrite(SflInstancePtr, NULL, Data,  CmdBuffer, Addrvalid);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	/* Check is Dual byte opcode is inverted/same/Disabled from flash config table */
	if (Mode == XSFL_EDGE_MODE_DDR_PHY) {
		if (Flash_Config_Table[SflInstancePtr->SflFlashInfo.FlashIndex].ExtOpCodeType
				== XSFL_DUAL_BYTE_OP_INVERT) {
			DualByteOpCode = XSFL_DUAL_BYTE_OP_INVERT;
		} else if (Flash_Config_Table[SflInstancePtr->SflFlashInfo.FlashIndex].ExtOpCodeType
				== XSFL_DUAL_BYTE_OP_SAME) {
			DualByteOpCode = XSFL_DUAL_BYTE_OP_SAME;
		}
	}

	/* Set Controller modes  and Configure dual byte*/
	SflInstancePtr->CntrlInfo.SetSdrDdr(Mode, DualByteOpCode);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Set Controller mode in sfl instance */
	SflInstancePtr->CntrlInfo.SdrDdrMode = Mode;

	if (SflInstancePtr->SflFlashInfo.ConnectionMode == XSFL_CONNECTION_MODE_STACKED &&
			FlashType == XSFL_OSPI_FLASH ){
		/* Reset the controller mode to NON-PHY */
		SflInstancePtr->CntrlInfo.SetSdrDdr(XSFL_EDGE_MODE_SDR_NON_PHY, DualByteOpCode);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/* Set Controller mode in sfl instance */
		SflInstancePtr->CntrlInfo.SdrDdrMode = XSFL_EDGE_MODE_SDR_NON_PHY;

		/* Select Flash */
		Status = SflInstancePtr->CntrlInfo.SelectFlash(XSFL_SELECT_FLASH_CS1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Status = XSfl_FlashCmdTransfer(SflInstancePtr, XSFL_WRITE_ENABLE_CMD);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/*  Fill Buffer with opcode, address and dummy */
		CmdBuffer[XSFL_COMMAND_OFFSET] = Write_Reg_Opcode;
		CmdBuffer[XSFL_ADDRESS_OFFSET] = Address;
		CmdBuffer[XSFL_ADDRESS_SIZE_OFFSET] = AddrSize;
		CmdBuffer[XSFL_DUMMY_OFFSET] = 0;

		Addrvalid = 1;
		/* Write Configuration register */
		Status = XSfl_FlashRegisterReadWrite(SflInstancePtr, NULL, Data,  CmdBuffer, Addrvalid);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		/* set the controller mode back to requested mode */
		SflInstancePtr->CntrlInfo.SetSdrDdr(Mode, DualByteOpCode);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/* Set Controller mode in sfl instance */
		SflInstancePtr->CntrlInfo.SdrDdrMode = Mode;
	}

	return Status;
}
/** @} */
