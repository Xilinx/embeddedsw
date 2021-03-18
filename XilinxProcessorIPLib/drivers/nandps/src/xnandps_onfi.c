/******************************************************************************
* Copyright (C) 2009 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnandps_onfi.c
* @addtogroup nandps_v2_7
* @{
*
* This module implements the ONFI specific commands.
* See xnandps_onfi.h for more information.
*
* @note		None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date    	   Changes
* ----- ----   ----------  -----------------------------------------------
* 1.00a nm     12/10/2010  First release
* 1.01a nm     28/02/2012  Added support for 8Gb On-Die ECC NAND flash
*                          parts (CR 648463).
*                          Fixed 16-bit issue with ONFI commands like
*                          read, write and read status command.
* 1.03a nm     10/22/2012  Fixed CR# 673348.
* 1.04a nm     04/25/2013  Implemented PR# 699544. Added page cache read
*			   and program commands to ONFI command list.
*			   Reading the cache features during read param page.
* 2.7   sg     03/18/21    Added validation check for parameter page.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xnandps_onfi.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void Onfi_ReadData(XNandPs *InstancePtr, u8 *Buf, u32 Length);

static void Onfi_CmdReset(XNandPs *InstancePtr);

static void Onfi_CmdReadId(XNandPs *InstancePtr, u8 Address);

static void Onfi_CmdReadParamPage(XNandPs *InstancePtr);

static unsigned long Onfi_Crc16(u8 *Buf);

static int Onfi_ReadParamPage(XNandPs *InstancePtr, u8 *Buf);

extern void XNandPs_SendCommand(XNandPs *InstancePtr, XNandPs_CommandFormat
		*Command, int Page, int Column);
/************************** Variable Definitions *****************************/

/**
 * This structure defines the onfi command format sent to the flash.
 */
XNandPs_CommandFormat OnfiCommands[] = {
	{ONFI_CMD_READ1, ONFI_CMD_READ2, 5, XNANDPS_CMD_PHASE},
					/*<< Read command format */
	{ONFI_CMD_CHANGE_READ_COLUMN1, ONFI_CMD_CHANGE_READ_COLUMN2,
	2, XNANDPS_CMD_PHASE},		/*<< Change Read column format */
	{ONFI_CMD_BLOCK_ERASE1, ONFI_CMD_BLOCK_ERASE2, 3, XNANDPS_CMD_PHASE},
					/*<<Block Erase command format */
	{ONFI_CMD_READ_STATUS, XNANDPS_END_CMD_NONE, 0,
		XNANDPS_END_CMD_INVALID},
					/*<< Read Status command format */
	{ONFI_CMD_PAGE_PROG1, ONFI_CMD_PAGE_PROG2, 5, XNANDPS_DATA_PHASE},
					/*<< Page program command format */
	{ONFI_CMD_CHANGE_WRITE_COLUMN, XNANDPS_END_CMD_NONE, 2,
		XNANDPS_END_CMD_INVALID},	/*<< Change Write Column
						  command format */
	{ONFI_CMD_READ_ID, XNANDPS_END_CMD_NONE, 1, XNANDPS_END_CMD_INVALID},
					/*<< Read ID command format */
	{ONFI_CMD_READ_PARAM_PAGE, XNANDPS_END_CMD_NONE, 1,
		XNANDPS_END_CMD_INVALID},
					/*<< Read Param Page command format */
	{ONFI_CMD_RESET, XNANDPS_END_CMD_NONE, 0, XNANDPS_END_CMD_INVALID},
					/*<< Reset command format */
	{ONFI_CMD_GET_FEATURES, XNANDPS_END_CMD_NONE, 1,
		XNANDPS_END_CMD_INVALID},
					/*<< Get Features */
	{ONFI_CMD_SET_FEATURES, XNANDPS_END_CMD_NONE, 1,
		XNANDPS_END_CMD_INVALID},
					/*<< Set Features */
	{ONFI_CMD_READ_CACHE_ENHANCED1, ONFI_CMD_READ_CACHE_ENHANCED2, 5,
		XNANDPS_CMD_PHASE},
					/*<< Read page cache random */
	{ONFI_CMD_READ_CACHE_END, XNANDPS_END_CMD_NONE, 0,
		XNANDPS_END_CMD_INVALID},
					/*<< Read page cache end */
	{ONFI_CMD_PAGE_CACHE_PROGRAM1, ONFI_CMD_PAGE_CACHE_PROGRAM2, 5,
		XNANDPS_DATA_PHASE},
					/*<< Program page cache */
};

/**************************************************************************/
/**
*
* This function reads the data from flash. It is used for reading the control
* information from flash like ID and Parameter page.
*
* @param	InstancePtr is a pointer to the XNandPs instance.
* @param	Buf is the buffer pointer to read the data.
* @param	Length is the length of data to read.
*
* @return	None
*
* @note		None
*
***************************************************************************/
static void Onfi_ReadData(XNandPs *InstancePtr, u8 *Buf, u32 Length)
{
	u32 Index;

	/*
	 * 8-bit/16-bit access for basic read operations
	 */
	for(Index = 0; Index < Length; Index++) {

		if (InstancePtr->Config.FlashWidth == XNANDPS_FLASH_WIDTH_16)
			Buf[Index] = (u8)Xil_In16(InstancePtr->DataPhaseAddr);
		else
			Buf[Index] = Xil_In8(InstancePtr->DataPhaseAddr);
	}
}

/**************************************************************************/
/**
*
* This function writes command data to flash. It is used for writing the
* control information like set features.
*
* @param	InstancePtr is a pointer to the XNandPs instance.
* @param	Buf is the buffer pointer to write the data.
* @param	Length is the length of data to write.
*
* @return	None
*
* @note		None
*
***************************************************************************/
static void Onfi_WriteData(XNandPs *InstancePtr, u8 *Buf, u32 Length)
{
	u32 Index;

	/*
	 * 8-bit/16-bit access for basic write operations
	 */
	for(Index = 0; Index < Length; Index++) {
		if (InstancePtr->Config.FlashWidth == XNANDPS_FLASH_WIDTH_16)
			Xil_Out16(InstancePtr->DataPhaseAddr, Buf[Index]);
		else
			Xil_Out8(InstancePtr->DataPhaseAddr, Buf[Index]);
	}
}

/**************************************************************************/
/**
*
* This function sends read status command to the flash device.
*
* @param	InstancePtr is a pointer to the XNandPs instance.
*
* @return	flash status value read
*
* @note		None
*
***************************************************************************/
u8 Onfi_CmdReadStatus(XNandPs *InstancePtr)
{
	u8 Status;

	XNandPs_SendCommand(InstancePtr, &OnfiCommands[READ_STATUS],
			XNANDPS_PAGE_NOT_VALID, XNANDPS_COLUMN_NOT_VALID);

	if(InstancePtr->Config.FlashWidth == XNANDPS_FLASH_WIDTH_16)
		Status = (u8) Xil_In16(InstancePtr->DataPhaseAddr);
	else
		Status = Xil_In8(InstancePtr->DataPhaseAddr);

	return Status;
}

/**************************************************************************/
/**
*
* This function sends reset command to the flash device.
*
* @param	InstancePtr is a pointer to the XNandPs instance.
*
* @return	None
*
* @note		None
*
***************************************************************************/
static void Onfi_CmdReset(XNandPs *InstancePtr)
{
	u8 Status;

	XNandPs_SendCommand(InstancePtr, &OnfiCommands[RESET],
			XNANDPS_PAGE_NOT_VALID, XNANDPS_COLUMN_NOT_VALID);

	/*
	 * Check the Status Register SR[6]
	 */
	do {
		Status = Onfi_CmdReadStatus(InstancePtr);
	}while ((Status & ONFI_STATUS_RDY) != ONFI_STATUS_RDY);
}

/**************************************************************************/
/**
*
* This function sends read ID command to the flash device.
*
* @param	InstancePtr is a pointer to the XNandPs instance.
*
* @return	None
*
* @note		None
*
***************************************************************************/
static void Onfi_CmdReadId(XNandPs *InstancePtr, u8 Address)
{
	XNandPs_SendCommand(InstancePtr, &OnfiCommands[READ_ID],
			XNANDPS_PAGE_NOT_VALID, Address);
}

/**************************************************************************/
/**
*
* This function sends read parameter page command to the flash device.
*
* @param	InstancePtr is a pointer to the XNandPs instance.
*
* @return	None
*
* @note		None
*
***************************************************************************/
static void Onfi_CmdReadParamPage(XNandPs *InstancePtr)
{
	u8 Status;
	u32 ZeroCommand;

	XNandPs_SendCommand(InstancePtr, &OnfiCommands[READ_PARAM_PAGE],
			XNANDPS_PAGE_NOT_VALID, 0x00);
	/*
	 * Check the Status Register SR[6]
	 */
	do {
		Status = Onfi_CmdReadStatus(InstancePtr);
	}while ((Status & ONFI_STATUS_RDY) != ONFI_STATUS_RDY);

	/*
	 * ONFI : Reissue the 0x00 on the command line to start reading data
	 */
	ZeroCommand = InstancePtr->Config.FlashBase |
			(0 << XNANDPS_ADDR_CYCLES_SHIFT)|
			(0 << XNANDPS_END_CMD_VALID_SHIFT)|
			(XNANDPS_COMMAND_PHASE_MASK)|
			(0 << XNANDPS_END_CMD_SHIFT)|
			(0 << XNANDPS_START_CMD_SHIFT);

	/*
	 * Dummy AXI transaction for sending command 0x00 to the flash
	 */
	Xil_Out32(ZeroCommand, 0x0);
}

/**************************************************************************/
/**
*
* This function sends Get Feature command to the flash device.
*
* @param	InstancePtr is a pointer to the XNandPs instance.
* @param	Feature is the feature value to read.
*
* @return	None
*
* @note		None
*
***************************************************************************/
static void Onfi_GetFeature(XNandPs *InstancePtr, u8 Feature, u8 *Val)
{
	u8 Status;
	u32 ZeroCommand;

	XNandPs_SendCommand(InstancePtr, &OnfiCommands[GET_FEATURES],
			XNANDPS_PAGE_NOT_VALID, Feature);
	/*
	 * Check the Status Register SR[6]
	 */
	do {
		Status = Onfi_CmdReadStatus(InstancePtr);
	}while ((Status & ONFI_STATUS_RDY) != ONFI_STATUS_RDY);

	/*
	 * ONFI 2.3: Reissue the 0x00 on the command line to start reading
	 * data.
	 */
	ZeroCommand = InstancePtr->Config.FlashBase |
			(0 << XNANDPS_ADDR_CYCLES_SHIFT)|
			(0 << XNANDPS_END_CMD_VALID_SHIFT)|
			(XNANDPS_COMMAND_PHASE_MASK)|
			(0 << XNANDPS_END_CMD_SHIFT)|
			(0 << XNANDPS_START_CMD_SHIFT);

	/*
	 * Dummy AXI transaction for sending command 0x00 to the flash
	 */
	Xil_Out32(ZeroCommand, 0x00);

	/*
	 * Read the feature value
	 */
	Onfi_ReadData(InstancePtr, Val, 4);
}

/**************************************************************************/
/**
*
* This function sends Set Feature command to the flash device.
*
* @param	InstancePtr is a pointer to the XNandPs instance.
* @param	Feature is the feature value to Set.
*
* @return	None
*
* @note		None
*
***************************************************************************/
static void Onfi_SetFeature(XNandPs *InstancePtr, u8 Feature, u8 *Val)
{
	u8 Status;

	XNandPs_SendCommand(InstancePtr, &OnfiCommands[SET_FEATURES],
			XNANDPS_PAGE_NOT_VALID, Feature);

	Onfi_WriteData(InstancePtr, Val, 4);

	/*
	 * Check the Status Register SR[6]
	 */
	do {
		Status = Onfi_CmdReadStatus(InstancePtr);
	}while ((Status & ONFI_STATUS_RDY) != ONFI_STATUS_RDY);
}

/**************************************************************************/
/**
*
* This function calculates the CRC on the parameter page buffer. This is taken
* from the ONFI 1.0 specification.
*
* @param	Buf is the parameter page buffer.
*
* @return	CRC value calculated.
*
* @note		None
*
***************************************************************************/
static unsigned long Onfi_Crc16(u8 *Buf)
{
	const int Order = ONFI_CRC_ORDER;
	const unsigned long Polynom = ONFI_CRC_POLYNOM;
	u32 Crc = ONFI_CRC_INIT;
	u32 Index;
	u32 j;
	u32 c;
	u32 Bit;
	u32 DataIn;
	int DataByteCount = 0;
	u32 CrcMask = ((((u32)1 << (Order - 1)) -1) << 1) | 1;
	u32 CrcHighBit = (u32)1 << (Order - 1);

	/*
	 * CRC covers the data bytes between byte 0 and byte 253 (ONFI 1.0, sec
	 * 5.4.1.36)
	 */
	for(Index = 0; Index < ONFI_CRC_LEN; Index++)
	{
		DataIn = Buf[Index];
		c = (u32)DataIn;
		DataByteCount++;
		for(j = 0x80; j; j >>= 1) {
			Bit = Crc & CrcHighBit;
			Crc <<= 1;
			if (c & j) Bit ^= CrcHighBit;
			if (Bit) Crc ^= Polynom;
		}
		Crc &= CrcMask;
	}
	return Crc;
}

/**************************************************************************/
/**
*
* This function reads the NAND flash parameter page defined by ONFI 1.0
* specification.
*
* @param	InstancePtr is a pointer to the XNandPs instance.
* @param	Buf is a buffer pointer to fill the data.
*
* @return
* 		- XST_SUCCESS if parameter page read successfully.
* 		- XST_FAILURE if parameter page is not read successfully.
*
* @note		None
*
***************************************************************************/
static int Onfi_ReadParamPage(XNandPs *InstancePtr, u8 *Buf)
{
	u32 Index;
	u32 CrcCalc;
	OnfiNand_Geometry *Geometry;

	/*
	 * Read the first 256 bytes of parameter page
	 */
	Onfi_CmdReadParamPage(InstancePtr);
	/* Read the 3 mandatory parameter pages */
	for(Index = 0; Index < 3; Index++) {
		Onfi_ReadData(InstancePtr, Buf, ONFI_PARAM_PAGE_LEN);
		Geometry = (OnfiNand_Geometry *)Buf;
		/* Check the CRC */
		CrcCalc = Onfi_Crc16(Buf);
		if(CrcCalc == Geometry->Crc) {
			break;
		}
	}

	if (Index == 3) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
/**************************************************************************/
/**
*
* This function initializes the NAND flash and gets the geometry information.
*
* @param	InstancePtr is a pointer to the XNandPs instance.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note		None
*
***************************************************************************/
int Onfi_NandInit(XNandPs *InstancePtr)
{
	u32 Target;
	int Status;
	u8 Id[ONFI_ID_LEN];
	u8 JedecId[2];
	u8 EccSetFeature[4] = {0x08, 0x00, 0x00, 0x00};
	u8 EccGetFeature[4];
	OnfiNand_Geometry Nand_Geometry;

	Xil_AssertNonvoid(InstancePtr != NULL);

	for(Target=0; Target < XNANDPS_MAX_TARGETS; Target++) {
		/*
		 * Reset the target
		 */
		Onfi_CmdReset(InstancePtr);

		/*
		 * Read the ONFI ID
		 */
		Onfi_CmdReadId(InstancePtr, 0x20);
		Onfi_ReadData(InstancePtr, &Id[0], ONFI_ID_LEN);

		/*
		 * Check the ONFI signature to know that the target supports
		 * ONFI
		 */
		if (Id[0]=='O' && Id[1]=='N' && Id[2]=='F' && Id[3]=='I') {
			/* Read the parameter page structure */
			Status = Onfi_ReadParamPage(InstancePtr,
					(u8 *)&Nand_Geometry);
			if (Status != XST_FAILURE) {
				if (Nand_Geometry.BytesPerPage > XNANDPS_MAX_PAGE_SIZE ||
					Nand_Geometry.SpareBytesPerPage > XNANDPS_MAX_SPARE_SIZE ||
					Nand_Geometry.PagesPerBlock > XNANDPS_MAX_PAGES_PER_BLOCK ||
					Nand_Geometry.BlocksPerLun > XNANDPS_MAX_BLOCKS ||
					Nand_Geometry.NumLuns > XNANDPS_MAX_LUNS) {
					return XST_FAILURE;
				}

				InstancePtr->Geometry.NumLun =
					Nand_Geometry.NumLuns;
				InstancePtr->Geometry.PagesPerBlock =
					Nand_Geometry.PagesPerBlock;
				InstancePtr->Geometry.SpareBytesPerPage =
					Nand_Geometry.SpareBytesPerPage;
				InstancePtr->Geometry.BytesPerPage =
					Nand_Geometry.BytesPerPage;
				InstancePtr->Geometry.BlocksPerLun =
					Nand_Geometry.BlocksPerLun;
				InstancePtr->Geometry.NumBlocks =
					(Nand_Geometry.NumLuns *
					 InstancePtr->Geometry.BlocksPerLun);
				InstancePtr->Geometry.NumPages =
					(Nand_Geometry.NumLuns *
					 Nand_Geometry.BlocksPerLun *
					 Nand_Geometry.PagesPerBlock);
				InstancePtr->Geometry.BlockSize =
					 (Nand_Geometry.PagesPerBlock *
					 Nand_Geometry.BytesPerPage);
				InstancePtr->Geometry.DeviceSize =
					(InstancePtr->Geometry.NumBlocks *
					 InstancePtr->Geometry.PagesPerBlock *
					 InstancePtr->Geometry.BytesPerPage);
				/*
				 * Calculate the address cycles
				 */
				InstancePtr->Geometry.RowAddrCycles =
					(Nand_Geometry.AddrCycles & 0xf);
				InstancePtr->Geometry.ColAddrCycles =
					((Nand_Geometry.AddrCycles >> 4) & 0xf);

				OnfiCommands[READ].AddrCycles =
						(InstancePtr->Geometry.RowAddrCycles +
						InstancePtr->Geometry.ColAddrCycles);

				OnfiCommands[PAGE_PROGRAM].AddrCycles =
						(InstancePtr->Geometry.RowAddrCycles +
						InstancePtr->Geometry.ColAddrCycles);

				OnfiCommands[BLOCK_ERASE].AddrCycles =
						InstancePtr->Geometry.RowAddrCycles;

				OnfiCommands[CHANGE_READ_COLUMN].AddrCycles =
						InstancePtr->Geometry.ColAddrCycles;

				OnfiCommands[CHANGE_WRITE_COLUMN].AddrCycles =
						InstancePtr->Geometry.ColAddrCycles;
				/*
				 * Read JEDEC ID
				 */
				Onfi_CmdReadId(InstancePtr, 0x00);
				Onfi_ReadData(InstancePtr, &JedecId[0], 2);

				if ((JedecId[0] == 0x2C) &&
						/* 1 Gb flash devices */
						((JedecId[1] == 0xF1) ||
						(JedecId[1] == 0xA1) ||
						(JedecId[1] == 0xB1) ||
						/* 2 Gb flash devices */
						(JedecId[1] == 0xAA) ||
						(JedecId[1] == 0xBA) ||
						(JedecId[1] == 0xDA) ||
						(JedecId[1] == 0xCA) ||
						/* 4 Gb flash devices */
						(JedecId[1] == 0xAC) ||
						(JedecId[1] == 0xBC) ||
						(JedecId[1] == 0xDC) ||
						(JedecId[1] == 0xCC) ||
						/* 8 Gb flash devices */
						(JedecId[1] == 0xA3) ||
						(JedecId[1] == 0xB3) ||
						(JedecId[1] == 0xD3) ||
						(JedecId[1] == 0xC3))) {
					/*
					 * Check if this flash supports On-Die ECC.
					 * Micron Flash: MT29F1G08ABADA, MT29F1G08ABBDA
					 *		 MT29F1G16ABBDA,
					 *		 MT29F2G08ABBEA, MT29F2G16ABBEA,
					 *		 MT29F2G08ABAEA, MT29F2G16ABAEA,
					 *		 MT29F4G08ABBDA, MT29F4G16ABBDA,
					 *		 MT29F4G08ABADA, MT29F4G16ABADA,
					 *		 MT29F8G08ADBDA, MT29F8G16ADBDA,
					 *		 MT29F8G08ADADA, MT29F8G16ADADA
					 */

					Onfi_SetFeature(InstancePtr, 0x90,
								&EccSetFeature[0]);
					/* Check to see if ECC feature is set */
					Onfi_GetFeature(InstancePtr, 0x90,
								&EccGetFeature[0]);
					if (EccGetFeature[0] & 0x08) {
						InstancePtr->EccMode = XNANDPS_ECC_ONDIE;
					} else {
						InstancePtr->EccMode = XNANDPS_ECC_HW;
					}
				} else if (Nand_Geometry.BytesPerPage < 512 ||
					Nand_Geometry.BytesPerPage > 2048) {
					/*
					 * This controller doesn't support ECC for
					 * page size < 512 & > 2048 bytes.
					 */
					InstancePtr->EccMode = XNANDPS_ECC_NONE;
				} else {
					/* SMC controller ECC (1-bit correction) */
					InstancePtr->EccMode = XNANDPS_ECC_HW;
				}
				/*
				 * Updating the instance flash width after checking
				 * for on-die ECC
				 */
				InstancePtr->Geometry.FlashWidth =
					(Nand_Geometry.Features & 0x1) ?
						XNANDPS_FLASH_WIDTH_16 :
						XNANDPS_FLASH_WIDTH_8;
				/*
				 * Features and Optional commands supported.
				 * On-Die ECC flash doesn't support these
				 * commands when ECC is enabled.
				 */
				if (InstancePtr->EccMode != XNANDPS_ECC_ONDIE) {
					InstancePtr->Features.ProgramCache =
						(Nand_Geometry.OptionalCmds & 0x1) ? 1:0;
					InstancePtr->Features.ReadCache =
						(Nand_Geometry.OptionalCmds & 0x2) ? 1:0;
				}
			} else {
				return XST_FAILURE;
			}
		} else {
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}
/** @} */
