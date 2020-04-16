/******************************************************************************
* Copyright (c) 2012 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilisf_erase.c
 *
 * This file contains the library functions to Erase the Serial Flash.
 * Refer xilisf.h for a detailed description.
 *
 * <pre>
 *
 * MODIFICATION HISTORY:
 *
 * Ver   Who      Date     Changes
 * ----- -------  -------- -----------------------------------------------
 * 1.00a ksu/sdm  03/03/08 First release
 * 2.01a sdm      01/04/10 Added Support for Winbond W25QXX/W25XX devices
 * 2.04a sdm      08/17/10 Updated to support Numonyx (N25QXX) and Spansion
 *			  flash memories
 * 3.02a srt	 04/25/13 Added Bulk Erase command support for SST and
 *			  Spansion flashes.
 * 5.0   sb	 08/05/14 Updated support for > 128 MB flash for PSQSPI
 *			  interface.
 *			  New API:
 *				DieErase()
 *			  Changed API:
 *				SectorErase()
 *				BulkErase()
 * 5.2  asa   05/12/15 Added support for Micron (N25Q256A) flash part
 *                     which supports 4 byte addressing.
 * 5.3  sk   08/07/17 Added QSPIPSU flash interface support for ZynqMP.
 * 5.5  sk   01/14/16 Used 4byte erase command in 4 byte addressing mode.
 *      sk   03/02/16 Used 3byte command with 4 byte addressing for Micron.
 * 5.7  rk	27/07/16 Added the subsector erase command.
 * 5.9  nsk  07/11/17 Add Micron 4Byte addressing support in
 *                    SectorErase, CR#980169
 *      ms   08/03/17 Added tags and updated comment lines style for doxygen.
 * 5.12 tjs	05/21/18 Removed the check for address to be non zero CR#1002769
 * 5.12 tjs	 06/18/18 Removed checkpatch and gcc warnings.
 * 5.12 tjs	07/05/18 Removed the mentions of Spansion flash from
 *			 BlockErase API CR#1006247
 * 5.13 nsk  01/22/18 Make variable declaration to XQspiPsu_Msg as global
 *                    CR#1015808.
 *      sk   02/11/19 Added support for OSPI flash interface.
 *      sk   02/15/19 4B Sector erase command is not supported by all QSPI
 *                    Micron flashes hence used used 3B sector erase command.
 *
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/

#include "include/xilisf.h"

/************************** Constant Definitions *****************************/
#define SIXTEENMB	0x1000000	/**< Sixteen MB */
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

#ifndef XPAR_XISF_INTERFACE_OSPIPSV
#ifdef XPAR_XISF_INTERFACE_PSQSPI
extern int SendBankSelect(XIsf *InstancePtr, u32 BankSel);
#endif
#if (XPAR_XISF_FLASH_FAMILY == ATMEL)
static int PageErase(XIsf *InstancePtr, u32 Address);
#endif
#if ((XPAR_XISF_FLASH_FAMILY == ATMEL) || \
	(XPAR_XISF_FLASH_FAMILY == INTEL) || \
	(XPAR_XISF_FLASH_FAMILY == WINBOND))
static int BlockErase(XIsf *InstancePtr, u32 Address);
#endif
#ifdef	XPAR_XISF_INTERFACE_QSPIPSU
	static XQspiPsu_Msg FlashMsg[2];
#endif
#if ((XPAR_XISF_FLASH_FAMILY != WINBOND) && (XPAR_XISF_FLASH_FAMILY != SST))
static int SubSectorErase(XIsf *InstancePtr, u32 Address);
#endif
#endif
#ifdef XPAR_XISF_INTERFACE_OSPIPSV
	static XOspiPsv_Msg FlashMsg;
#endif
static int SectorErase(XIsf *InstancePtr, u32 Address);
static int BulkErase(XIsf *InstancePtr);
#if defined(XPAR_XISF_INTERFACE_PSQSPI) || \
	defined(XPAR_XISF_INTERFACE_QSPIPSU)
static int DieErase(XIsf *InstancePtr);
#endif
/************************** Variable Definitions *****************************/

/************************** Function Definitions ******************************/


/*****************************************************************************/
/**
 * @brief
 * This API erases the contents of the specified memory in the Serial Flash.
 *
 * @param	InstancePtr	Pointer to the XIsf instance.
 * @param	Operation	Type of Erase operation to be performed on
 *				the Serial Flash.
 *				The different operations are
 *				- XISF_PAGE_ERASE: Page Erase
 *				- XISF_BLOCK_ERASE: Block Erase
 *				- XISF_SECTOR_ERASE: Sector Erase
 *				- XISF_BULK_ERASE: Bulk Erase
 * @param	Address		Address of the Page/Block/Sector to be
 *				erased. The address can be either Page
 *				address, Block address or Sector address
 *				based on the Erase operation to be
 *				performed.
 *
 * @return
 *		- XST_SUCCESS if successful.
 *		- XST_FAILURE if it fails.
 *
 * @note
 *		- The erased bytes will read as 0xFF.
 *		- For Intel, STM, Winbond or Spansion Serial Flash the user
 *		application must call XIsf_WriteEnable() API by passing
 *		XISF_WRITE_ENABLE as an argument before calling XIsf_Erase()
 *		API.
 *		- Atmel Serial Flash support Page/Block/Sector Erase
 *		  operations.
 *		- Intel, Winbond, Numonyx (N25QXX) and Spansion Serial Flash
 *		  support Sector/Block/Bulk Erase operations.
 *		- STM (M25PXX) Serial Flash support Sector/Bulk Erase
 *		  operations.
 *
 ******************************************************************************/
int XIsf_Erase(XIsf *InstancePtr, XIsf_EraseOperation Operation, u32 Address)
{
	int Status = (int)(XST_FAILURE);
	u8 Mode;

	if (InstancePtr == NULL)
		return (int)(XST_FAILURE);

	if (InstancePtr->IsReady != TRUE)
		return (int)(XST_FAILURE);

	switch (Operation) {
#ifndef XPAR_XISF_INTERFACE_OSPIPSV
	case XISF_PAGE_ERASE:
#if (XPAR_XISF_FLASH_FAMILY == ATMEL)
		Status = PageErase(InstancePtr, Address);
#endif
		break;

	case XISF_BLOCK_ERASE:
#if ((XPAR_XISF_FLASH_FAMILY == ATMEL) || \
	(XPAR_XISF_FLASH_FAMILY == INTEL) || \
	(XPAR_XISF_FLASH_FAMILY == WINBOND))
		Status = BlockErase(InstancePtr, Address);
#endif
		break;

#if ((XPAR_XISF_FLASH_FAMILY != WINBOND) && (XPAR_XISF_FLASH_FAMILY != SST))
	case XISF_SUB_SECTOR_ERASE:
			Status = SubSectorErase(InstancePtr, Address);
			break;
#endif
#endif

	case XISF_SECTOR_ERASE:
		Status = SectorErase(InstancePtr, Address);
		break;

	case XISF_BULK_ERASE:
		Status = BulkErase(InstancePtr);
		break;

	default:
		break;
	}


	/*
	 * Get the Transfer Mode
	 */
	Mode = XIsf_GetTransferMode(InstancePtr);

	if (Mode == XISF_INTERRUPT_MODE) {
#ifndef XPAR_XISF_INTERFACE_OSPIPSV
		InstancePtr->StatusHandler(InstancePtr,
				XIsf_StatusEventInfo, XIsf_ByteCountInfo);
#else
	InstancePtr->StatusHandler(InstancePtr, XIsf_StatusEventInfo);
#endif
	}

	return Status;
}

#ifndef XPAR_XISF_INTERFACE_OSPIPSV
/*****************************************************************************/
/**
 *
 * This function erases the contents of the specified Page in the Serial Flash.
 *
 * @param	InstancePtr is a pointer to the XIsf instance.
 * @param	Address is the address of Page to be erased. This can be any
 *		address in the Page to be erased. The Byte address values in
 *		this address are ignored.
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE.
 *
 * @note
 *		- The erased bytes will read as 0xFF.
 *		- This operation is only supported for Atmel Serial Flash.
 *
 ******************************************************************************/
#if (XPAR_XISF_FLASH_FAMILY == ATMEL)
static int PageErase(XIsf *InstancePtr, u32 Address)
{
	int Status = (int)(XST_FAILURE);

	Xil_AssertNonvoid(InstancePtr != NULL);

	InstancePtr->WriteBufPtr[BYTE1] = XISF_CMD_PAGE_ERASE;
	InstancePtr->WriteBufPtr[BYTE2] = (u8) (Address >> XISF_ADDR_SHIFT16);
	InstancePtr->WriteBufPtr[BYTE3] = (u8) (Address >> XISF_ADDR_SHIFT8);
	InstancePtr->WriteBufPtr[BYTE4] = (u8) XISF_DUMMYBYTE;

	/*
	 * Initiate the Transfer.
	 */
	Status = XIsf_Transfer(InstancePtr, InstancePtr->WriteBufPtr,
				NULL, XISF_CMD_SEND_EXTRA_BYTES);
	if (Status != (int)(XST_SUCCESS))
		return (int)(XST_FAILURE);

	return Status;
}
#endif /* (XPAR_XISF_FLASH_FAMILY == ATMEL) */

/*****************************************************************************/
/**
 *
 * This function erases the contents of the specified Block in the Serial Flash.
 *
 * @param	InstancePtr is a pointer to the XIsf instance.
 * @param	Address is the address of the Block to be erased. This can be
 *		any address in the Block to be erased. The Page/Byte address
 *		values in this address are ignored.
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE.
 *
 * @note
 *		- The erased bytes will read as 0xFF.
 *		- This operation is supported for Atmel, Intel, Winbond and
 *		  Numonyx (N25QXX) Serial Flash.
 *
 ******************************************************************************/
#if ((XPAR_XISF_FLASH_FAMILY == ATMEL) || \
	(XPAR_XISF_FLASH_FAMILY == INTEL) || \
	(XPAR_XISF_FLASH_FAMILY == WINBOND))

static int BlockErase(XIsf *InstancePtr, u32 Address)
{
	int Status = (int)(XST_FAILURE);

	Xil_AssertNonvoid(InstancePtr != NULL);

#if ((XPAR_XISF_FLASH_FAMILY == ATMEL) || (XPAR_XISF_FLASH_FAMILY == WINBOND))
	InstancePtr->WriteBufPtr[BYTE1] = XISF_CMD_BLOCK_ERASE;

#else
	InstancePtr->WriteBufPtr[BYTE1] = XISF_CMD_PARAM_BLOCK_ERASE;

#endif /* (XPAR_XISF_FLASH_FAMILY == ATMEL) */

	InstancePtr->WriteBufPtr[BYTE2] = (u8) (Address >> XISF_ADDR_SHIFT16);
	InstancePtr->WriteBufPtr[BYTE3] = (u8) (Address >> XISF_ADDR_SHIFT8);
	InstancePtr->WriteBufPtr[BYTE4] = (u8) XISF_DUMMYBYTE;

	/*
	 * Initiate the Transfer.
	 */
	Status = XIsf_Transfer(InstancePtr, InstancePtr->WriteBufPtr,
				NULL, XISF_CMD_SEND_EXTRA_BYTES);
	if (Status != (int)(XST_SUCCESS))
		return (int)(XST_FAILURE);

	return Status;
}
#endif /* ((XPAR_XISF_FLASH_FAMILY==ATMEL)||(XPAR_XISF_FLASH_FAMILY==INTEL)) */

#if ((XPAR_XISF_FLASH_FAMILY != WINBOND) && (XPAR_XISF_FLASH_FAMILY != SST))
/*****************************************************************************/
/**
 *
 * This function erases the contents of the specified SubSector in Serial Flash.
 *
 * @param	InstancePtr is a pointer to the XIsf instance.
 * @param	Address is the address of the Sector to be erased. This can be
 *		any address in the Sector to be erased.
 *		The Block/Page/Byte address values in this address are ignored.
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE.
 *
 * @note
 *		- The erased bytes will read as 0xFF.
 *		- This operation is supported for Atmel, Intel, STM, Winbond
 *		  and Spansion Serial Flash.
 *
 ******************************************************************************/
static int SubSectorErase(XIsf *InstancePtr, u32 Address)
{
	int Status = (int)(XST_FAILURE);
#ifdef XPAR_XISF_INTERFACE_PSQSPI
	u8 Mode;
	u32 BankSel;
#endif
	u32 RealAddr;
	u8 *NULLPtr = NULL;
#ifndef XPAR_XISF_INTERFACE_QSPIPSU
	u8 FlagStatus[2] = {0};
	u8 ReadStatusCmdBuf[] = { READ_STATUS_CMD, 0 };
	u8 ReadFlagSRCmd[] = {READ_FLAG_STATUS_CMD, 0};
#endif
	u8 FlashStatus[2] = {0};
#ifdef XPAR_XISF_INTERFACE_QSPIPSU
	u8 FSRFlag, ReadStatusCmd;
#endif
#if ((XPAR_XISF_FLASH_FAMILY != ATMEL) && \
	(XPAR_XISF_FLASH_FAMILY != WINBOND))

#if defined(XPAR_XISF_INTERFACE_PSQSPI) || \
	defined(XPAR_XISF_INTERFACE_QSPIPSU)
	u32 FlashMake = InstancePtr->ManufacturerID;
#endif
	Xil_AssertNonvoid(NULLPtr == NULL);
	/*
	 * Translate address based on type of connection
	 * If stacked assert the slave select based on address
	 */
	RealAddr = GetRealAddr(InstancePtr->SpiInstPtr, Address);

#ifdef XPAR_XISF_INTERFACE_PSQSPI
	/*
	 * Initial bank selection
	 */
	if (InstancePtr->DeviceIDMemSize > 0x18U) {

		/*
		 * Get the Transfer Mode
		 */
		Mode = XIsf_GetTransferMode(InstancePtr);

		/*
		 * Setting the transfer mode to Polled Mode before
		 * performing the Bank Select operation.
		 */
		XIsf_SetTransferMode(InstancePtr, XISF_POLLING_MODE);

		/*
		 * Calculate initial bank
		 */
		BankSel = RealAddr/SIXTEENMB;
		/*
		 * Select bank
		 */
		Status = SendBankSelect(InstancePtr, BankSel);

		/*
		 * Restoring the transfer mode back
		 */
		XIsf_SetTransferMode(InstancePtr, Mode);

		if (Status != (int)(XST_SUCCESS))
			return (int)XST_FAILURE;
	}
#endif
#if ((XPAR_XISF_FLASH_FAMILY == SPANSION) && \
	(!defined(XPAR_XISF_INTERFACE_PSQSPI)))
	if (InstancePtr->FourByteAddrMode == TRUE) {

		if (InstancePtr->ManufacturerID ==
			XISF_MANUFACTURER_ID_SPANSION) {
			InstancePtr->WriteBufPtr[BYTE1] =
					XISF_CMD_4BYTE_SUB_SECTOR_ERASE;
		} else {
			InstancePtr->WriteBufPtr[BYTE1] =
					XISF_CMD_SUB_SECTOR_ERASE;
		}

		InstancePtr->WriteBufPtr[BYTE2] =
				(u8) (RealAddr >> XISF_ADDR_SHIFT24);
		InstancePtr->WriteBufPtr[BYTE3] =
				(u8) (RealAddr >> XISF_ADDR_SHIFT16);
		InstancePtr->WriteBufPtr[BYTE4] =
				(u8) (RealAddr >> XISF_ADDR_SHIFT8);
		InstancePtr->WriteBufPtr[BYTE5] = (u8) (RealAddr);
#ifdef XPAR_XISF_INTERFACE_QSPIPSU
		FlashMsg[0].ByteCount = 5;
#endif
	} else {
#endif
		InstancePtr->WriteBufPtr[BYTE1] = XISF_CMD_SUB_SECTOR_ERASE;
		InstancePtr->WriteBufPtr[BYTE2] =
				(u8) (RealAddr >> XISF_ADDR_SHIFT16);
		InstancePtr->WriteBufPtr[BYTE3] =
				(u8) (RealAddr >> XISF_ADDR_SHIFT8);
		InstancePtr->WriteBufPtr[BYTE4] = (u8) (RealAddr);
#ifdef XPAR_XISF_INTERFACE_QSPIPSU
		FlashMsg[0].ByteCount = 4;
#endif

#if ((XPAR_XISF_FLASH_FAMILY == SPANSION) && \
	(!defined(XPAR_XISF_INTERFACE_PSQSPI)))
	}
#endif

#if defined(XPAR_XISF_INTERFACE_PSQSPI) || \
	defined(XPAR_XISF_INTERFACE_QSPIPSU)
	/*
	 * Enable write before transfer
	 */
	Status = XIsf_WriteEnable(InstancePtr, XISF_WRITE_ENABLE);
	if (Status != (int)XST_SUCCESS)
		return (int)XST_FAILURE;
#endif

#ifdef XPAR_XISF_INTERFACE_QSPIPSU
	FlashMsg[0].TxBfrPtr = InstancePtr->WriteBufPtr;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;
	InstancePtr->SpiInstPtr->Msg = FlashMsg;

	Status = XIsf_Transfer(InstancePtr, NULLPtr, NULLPtr, 1);
#else
	if (InstancePtr->FourByteAddrMode == TRUE)
		Status = XIsf_Transfer(InstancePtr, InstancePtr->WriteBufPtr,
			NULLPtr, XISF_CMD_SEND_EXTRA_BYTES_4BYTE_MODE);
	else
		Status = XIsf_Transfer(InstancePtr, InstancePtr->WriteBufPtr,
				NULLPtr, XISF_CMD_SEND_EXTRA_BYTES);
#endif
	if (Status != (int)XST_SUCCESS)
		return (int)XST_FAILURE;

#if defined(XPAR_XISF_INTERFACE_PSQSPI) || \
	defined(XPAR_XISF_INTERFACE_QSPIPSU)

	if ((InstancePtr->NumDie > (u8)1) &&
		(FlashMake == (u32)XISF_MANUFACTURER_ID_MICRON)) {
#ifdef XPAR_XISF_INTERFACE_QSPIPSU
		ReadStatusCmd = READ_FLAG_STATUS_CMD;
		FSRFlag = 1;
#endif
#ifndef XPAR_XISF_INTERFACE_QSPIPSU
		Status = XIsf_Transfer(InstancePtr, ReadFlagSRCmd, FlagStatus,
					(u32)sizeof(ReadFlagSRCmd));
#endif
	}
#ifdef XPAR_XISF_INTERFACE_QSPIPSU
	else {
		ReadStatusCmd = READ_STATUS_CMD;
		FSRFlag = 0;
	}
#endif

	/*
	 * Wait for the sector erase command to the Flash to be completed
	 */
	while (1) {
#ifdef XPAR_XISF_INTERFACE_QSPIPSU
		FlashMsg[0].TxBfrPtr = &ReadStatusCmd;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		FlashMsg[1].TxBfrPtr = NULL;
		FlashMsg[1].RxBfrPtr = FlashStatus;
		FlashMsg[1].ByteCount = 2;
		FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[1].Flags = XQSPIPSU_MSG_FLAG_RX;

		if (InstancePtr->SpiInstPtr->Config.ConnectionMode ==
				XISF_QSPIPS_CONNECTION_MODE_PARALLEL)
			FlashMsg[1].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;

		InstancePtr->SpiInstPtr->Msg = FlashMsg;

		Status = XIsf_Transfer(InstancePtr, NULLPtr, NULLPtr, 2);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;

		if (InstancePtr->SpiInstPtr->Config.ConnectionMode ==
				XISF_QSPIPS_CONNECTION_MODE_PARALLEL){
			if (FSRFlag)
				FlashStatus[1] &= FlashStatus[0];
			else
				FlashStatus[1] |= FlashStatus[0];
		}

		if (FSRFlag) {
			if ((FlashStatus[1] & 0x80) != 0)
				break;
		} else {
			if ((FlashStatus[1] & 0x01) == 0)
				break;
		}
#else
		/*
		 * Poll the status register of the device to determine
		 * when it completes, by sending a read status command
		 * and receiving the status byte
		 */
		Status = XIsf_Transfer(InstancePtr, ReadStatusCmdBuf,
					FlashStatus,
					(u32)sizeof(ReadStatusCmdBuf));

		/*
		 * If the status indicates the write is done, then stop
		 * waiting, if a value of 0xFF in the status byte is
		 * read from the device and this loop never exits, the
		 * device slave select is possibly incorrect such that
		 * the device status is not being read
		 */
		if ((FlashStatus[1] & 0x01) == 0)
			break;
#endif
	}

#ifdef XPAR_XISF_INTERFACE_PSQSPI
	if ((InstancePtr->NumDie > (u8)1) &&
		(FlashMake == (u32)XISF_MANUFACTURER_ID_MICRON)) {
		Status =
			XIsf_Transfer(InstancePtr, ReadFlagSRCmd, FlagStatus,
				(u32)sizeof(ReadFlagSRCmd));
	}
#endif
#endif
#endif
	return Status;
}
#endif
#endif
/*****************************************************************************/
/**
 *
 * This function erases the contents of the specified Sector in Serial Flash.
 *
 * @param	InstancePtr is a pointer to the XIsf instance.
 * @param	Address is the address of the Sector to be erased. This can be
 *		any address in the Sector to be erased.
 *		The Block/Page/Byte address values in this address are ignored.
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE.
 *
 * @note
 *		- The erased bytes will read as 0xFF.
 *		- This operation is supported for Atmel, Intel, STM, Winbond
 *		  and Spansion Serial Flash.
 *
 ******************************************************************************/
static int SectorErase(XIsf *InstancePtr, u32 Address)
{
	int Status = (int)(XST_FAILURE);
#ifdef XPAR_XISF_INTERFACE_PSQSPI
	u8 Mode;
	u32 BankSel;
#endif
#ifndef XPAR_XISF_INTERFACE_OSPIPSV
	u32 RealAddr;
#endif
	u8 *NULLPtr = NULL;
#if ((!defined(XPAR_XISF_INTERFACE_QSPIPSU)) && \
		(!defined(XPAR_XISF_INTERFACE_OSPIPSV)) && \
		(!defined(XPAR_XISF_INTERFACE_PSSPI)))
	u8 FlagStatus[2] = {0};
	u8 ReadStatusCmdBuf[] = { READ_STATUS_CMD, 0 };
	u8 ReadFlagSRCmd[] = {READ_FLAG_STATUS_CMD, 0};
#endif
#if (!defined(XPAR_XISF_INTERFACE_PSSPI))
	u8 FlashStatus[2] __attribute__ ((aligned(4))) = {0};
#endif
#if ((!defined(XPAR_XISF_INTERFACE_PSQSPI)) && \
		(!defined(XPAR_XISF_INTERFACE_OSPIPSV))&& \
		(!defined(XPAR_XISF_INTERFACE_PSSPI)))
	u8 FSRFlag, ReadStatusCmd;
#endif

#if defined(XPAR_XISF_INTERFACE_PSQSPI) || \
	defined(XPAR_XISF_INTERFACE_QSPIPSU)
	u32 FlashMake = InstancePtr->ManufacturerID;
#endif

	Xil_AssertNonvoid(NULLPtr == NULL);

#ifndef XPAR_XISF_INTERFACE_OSPIPSV
	/*
	 * Translate address based on type of connection
	 * If stacked assert the slave select based on address
	 */
	RealAddr = GetRealAddr(InstancePtr->SpiInstPtr, Address);

#ifdef XPAR_XISF_INTERFACE_PSQSPI
	/*
	 * Initial bank selection
	 */
	if (InstancePtr->DeviceIDMemSize > 0x18U) {

		/*
		 * Get the Transfer Mode
		 */
		Mode = XIsf_GetTransferMode(InstancePtr);

		/*
		 * Setting the transfer mode to Polled Mode before
		 * performing the Bank Select operation.
		 */
		XIsf_SetTransferMode(InstancePtr, XISF_POLLING_MODE);

		/*
		 * Calculate initial bank
		 */
		BankSel = RealAddr/SIXTEENMB;
		/*
		 * Select bank
		 */
		Status = SendBankSelect(InstancePtr, BankSel);

		/*
		 * Restoring the transfer mode back
		 */
		XIsf_SetTransferMode(InstancePtr, Mode);

		if (Status != (int)(XST_SUCCESS))
			return (int)XST_FAILURE;
	}
#endif
#if ((XPAR_XISF_FLASH_FAMILY == SPANSION) && \
	(!defined(XPAR_XISF_INTERFACE_PSQSPI)))
	if (InstancePtr->FourByteAddrMode == TRUE) {
		if (InstancePtr->ManufacturerID ==
				XISF_MANUFACTURER_ID_SPANSION)
			InstancePtr->WriteBufPtr[BYTE1] =
					XISF_CMD_4BYTE_SECTOR_ERASE;
		else
			InstancePtr->WriteBufPtr[BYTE1] =
					XISF_CMD_SECTOR_ERASE;

		InstancePtr->WriteBufPtr[BYTE2] =
				(u8) (RealAddr >> XISF_ADDR_SHIFT24);
		InstancePtr->WriteBufPtr[BYTE3] =
				(u8) (RealAddr >> XISF_ADDR_SHIFT16);
		InstancePtr->WriteBufPtr[BYTE4] =
				(u8) (RealAddr >> XISF_ADDR_SHIFT8);
		InstancePtr->WriteBufPtr[BYTE5] = (u8) (RealAddr);
#ifdef XPAR_XISF_INTERFACE_QSPIPSU
		FlashMsg[0].ByteCount = 5;
#endif
	} else {
#endif
		InstancePtr->WriteBufPtr[BYTE1] = XISF_CMD_SECTOR_ERASE;
		InstancePtr->WriteBufPtr[BYTE2] =
				(u8) (RealAddr >> XISF_ADDR_SHIFT16);
		InstancePtr->WriteBufPtr[BYTE3] =
				(u8) (RealAddr >> XISF_ADDR_SHIFT8);
		InstancePtr->WriteBufPtr[BYTE4] = (u8) (RealAddr);
#ifdef XPAR_XISF_INTERFACE_QSPIPSU
		FlashMsg[0].ByteCount = 4;
#endif

#if ((XPAR_XISF_FLASH_FAMILY == SPANSION) && \
	(!defined(XPAR_XISF_INTERFACE_PSQSPI)))
	}
#endif

#if defined(XPAR_XISF_INTERFACE_PSQSPI) || \
	defined(XPAR_XISF_INTERFACE_QSPIPSU)
	/*
	 * Enable write before transfer
	 */
	Status = XIsf_WriteEnable(InstancePtr, XISF_WRITE_ENABLE);
	if (Status != (int)XST_SUCCESS)
		return (int)XST_FAILURE;
#endif

#ifdef XPAR_XISF_INTERFACE_QSPIPSU
	FlashMsg[0].TxBfrPtr = InstancePtr->WriteBufPtr;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;
	InstancePtr->SpiInstPtr->Msg = FlashMsg;

	Status = XIsf_Transfer(InstancePtr, NULLPtr, NULLPtr, 1);
#else
	if (InstancePtr->FourByteAddrMode == TRUE)
		Status = XIsf_Transfer(InstancePtr, InstancePtr->WriteBufPtr,
			NULLPtr, XISF_CMD_SEND_EXTRA_BYTES_4BYTE_MODE);
	else
		Status = XIsf_Transfer(InstancePtr, InstancePtr->WriteBufPtr,
				NULLPtr, XISF_CMD_SEND_EXTRA_BYTES);
#endif
	if (Status != (int)XST_SUCCESS)
		return (int)XST_FAILURE;

#if defined(XPAR_XISF_INTERFACE_PSQSPI) || \
	defined(XPAR_XISF_INTERFACE_QSPIPSU)

	if ((InstancePtr->NumDie > (u8)1) &&
		(FlashMake == (u32)XISF_MANUFACTURER_ID_MICRON)) {
#ifdef XPAR_XISF_INTERFACE_QSPIPSU
		ReadStatusCmd = READ_FLAG_STATUS_CMD;
		FSRFlag = 1;
#endif
#ifndef XPAR_XISF_INTERFACE_QSPIPSU
		Status = XIsf_Transfer(InstancePtr, ReadFlagSRCmd, FlagStatus,
					(u32)sizeof(ReadFlagSRCmd));
#endif
	}
#ifdef XPAR_XISF_INTERFACE_QSPIPSU
	else {
		ReadStatusCmd = READ_STATUS_CMD;
		FSRFlag = 0;
	}
#endif

	/*
	 * Wait for the sector erase command to the Flash to be completed
	 */
	while (1) {
#ifdef XPAR_XISF_INTERFACE_QSPIPSU
		FlashMsg[0].TxBfrPtr = &ReadStatusCmd;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		FlashMsg[1].TxBfrPtr = NULL;
		FlashMsg[1].RxBfrPtr = FlashStatus;
		FlashMsg[1].ByteCount = 2;
		FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[1].Flags = XQSPIPSU_MSG_FLAG_RX;

		if (InstancePtr->SpiInstPtr->Config.ConnectionMode ==
				XISF_QSPIPS_CONNECTION_MODE_PARALLEL){
			FlashMsg[1].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
		}
		InstancePtr->SpiInstPtr->Msg = FlashMsg;

		Status = XIsf_Transfer(InstancePtr, NULLPtr, NULLPtr, 2);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;

		if (InstancePtr->SpiInstPtr->Config.ConnectionMode ==
				XISF_QSPIPS_CONNECTION_MODE_PARALLEL){
			if (FSRFlag)
				FlashStatus[1] &= FlashStatus[0];
			else
				FlashStatus[1] |= FlashStatus[0];
		}

		if (FSRFlag) {
			if ((FlashStatus[1] & 0x80) != 0)
				break;
		} else {
			if ((FlashStatus[1] & 0x01) == 0)
				break;
		}
#else
		/*
		 * Poll the status register of the device to determine
		 * when it completes, by sending a read status command
		 * and receiving the status byte
		 */
		Status = XIsf_Transfer(InstancePtr, ReadStatusCmdBuf,
					FlashStatus,
					(u32)sizeof(ReadStatusCmdBuf));

		/*
		 * If the status indicates the write is done, then stop
		 * waiting, if a value of 0xFF in the status byte is
		 * read from the device and this loop never exits, the
		 * device slave select is possibly incorrect such that
		 * the device status is not being read
		 */
		if ((FlashStatus[1] & 0x01) == 0)
			break;
#endif
	}

#ifdef XPAR_XISF_INTERFACE_PSQSPI
	if ((InstancePtr->NumDie > (u8)1) &&
		(FlashMake == (u32)XISF_MANUFACTURER_ID_MICRON)) {
		Status =
			XIsf_Transfer(InstancePtr, ReadFlagSRCmd, FlagStatus,
				(u32)sizeof(ReadFlagSRCmd));
	}
#endif
#endif
#else
	Status = XIsf_WriteEnable(InstancePtr, XISF_WRITE_ENABLE);
	if (Status != (int)XST_SUCCESS)
		return (int)XST_FAILURE;

	FlashMsg.Opcode = XISF_CMD_4BYTE_SECTOR_ERASE;
	FlashMsg.Addrsize = 4;
	FlashMsg.Addrvalid = 1;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 0;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.Addr = Address;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	FlashMsg.Dummy = 0;
	if (InstancePtr->SpiInstPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_WRITE_8_8_0;
	}
	InstancePtr->SpiInstPtr->Msg = &FlashMsg;
	Status = XIsf_Transfer(InstancePtr, NULLPtr, NULLPtr, 0);
	if (Status != (int)XST_SUCCESS)
		return (int)XST_FAILURE;

	while (1) {
		FlashMsg.Opcode = READ_FLAG_STATUS_CMD;
		FlashMsg.Addrsize = 0;
		FlashMsg.Addrvalid = 0;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = FlashStatus;
		FlashMsg.ByteCount = 1;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
		FlashMsg.Dummy = 0;
		FlashMsg.IsDDROpCode = 0;
		FlashMsg.Proto = 0;
		if (InstancePtr->SpiInstPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Proto = XOSPIPSV_READ_8_0_8;
			FlashMsg.ByteCount = 2;
			FlashMsg.Dummy = 8;
		}
		InstancePtr->SpiInstPtr->Msg = &FlashMsg;
		Status = XIsf_Transfer(InstancePtr, NULLPtr, NULLPtr, FlashMsg.ByteCount);
		if (Status != (int)XST_SUCCESS)
			return (int)XST_FAILURE;

		if ((FlashStatus[0] & 0x80) != 0)
			break;
	}
#endif

	return Status;
}

/*****************************************************************************/
/**
 *
 * This function erases the content of an entire Serial Flash.
 *
 * @param	InstancePtr is a pointer to the XIsf instance.
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE.
 *
 * @note
 *		- The erased bytes will read as 0xFF.
 *		- This operation is supported for Intel, STM, Winbond,
 *		  Spansion and SST Serial Flashes.
 *
 ******************************************************************************/
static int BulkErase(XIsf *InstancePtr)
{
	int Status = (int)(XST_FAILURE);
	u8 *NULLPtr = NULL;

#if ((XPAR_XISF_FLASH_FAMILY == INTEL) || \
	(XPAR_XISF_FLASH_FAMILY == STM) || \
	(XPAR_XISF_FLASH_FAMILY == WINBOND) || \
	(XPAR_XISF_FLASH_FAMILY == SST) || \
	(XPAR_XISF_FLASH_FAMILY == SPANSION))

#if defined(XPAR_XISF_INTERFACE_PSQSPI) || \
	defined(XPAR_XISF_INTERFACE_QSPIPSU)
	/*
	 * If the number of die is greater than 1 call die erase
	 */
	if (InstancePtr->NumDie > 1)
		Status = DieErase(InstancePtr);

	else{
#endif
		InstancePtr->WriteBufPtr[BYTE1] = XISF_CMD_BULK_ERASE;

		/*
		 * Enable write before transfer
		 */
		Status = XIsf_WriteEnable(InstancePtr, XISF_WRITE_ENABLE);
		if (Status != (int)XST_SUCCESS)
			return (int)XST_FAILURE;

		Xil_AssertNonvoid(NULLPtr == NULL);
#ifdef XPAR_XISF_INTERFACE_QSPIPSU
		FlashMsg[0].TxBfrPtr = InstancePtr->WriteBufPtr;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;
		InstancePtr->SpiInstPtr->Msg = FlashMsg;

		Status = XIsf_Transfer(InstancePtr, NULLPtr, NULLPtr, 1);
#else
		/*
		 * Initiate the Transfer.
		 */
		Status = XIsf_Transfer(InstancePtr, InstancePtr->WriteBufPtr,
				NULLPtr, XISF_BULK_ERASE_BYTES);
#endif

#if defined(XPAR_XISF_INTERFACE_PSQSPI) || \
	defined(XPAR_XISF_INTERFACE_QSPIPSU)
	}
#endif
#endif
/**
 * ((XPAR_XISF_FLASH_FAMILY==INTEL) || \
 * (XPAR_XISF_FLASH_FAMILY==STM)) || \
 * (XPAR_XISF_FLASH_FAMILY == WINBOND) || \
 * (XPAR_XISF_FLASH_FAMILY == SST) || \
 * (XPAR_XISF_FLASH_FAMILY == SPANSION))
 */

	return Status;
}

#if defined(XPAR_XISF_INTERFACE_PSQSPI) || \
	defined(XPAR_XISF_INTERFACE_QSPIPSU)
/*****************************************************************************/
/**
 *
 * This function erases the content of a Die of Serial Flash.
 *
 * @param	InstancePtr is a pointer to the XIsf instance.
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE.
 *
 * @note
 *		- This operation is supported for Winbond,
 *		  Spansion and Micron Serial Flashes.
 *
 ******************************************************************************/
static int DieErase(XIsf *InstancePtr)
{
	int Status = (int)(XST_FAILURE);
	u8 *NULLPtr = NULL;
#ifdef	XPAR_XISF_INTERFACE_QSPIPSU
	u8 ReadStatusCmd, FSRFlag;
	u32 FlashMake = InstancePtr->ManufacturerID;
	u8 FlashStatus[2] = {0};
#endif

#if ((XPAR_XISF_FLASH_FAMILY == WINBOND) || \
	(XPAR_XISF_FLASH_FAMILY == SPANSION))

	u8 DieCnt;
#ifndef XPAR_XISF_INTERFACE_QSPIPSU
	u8 ReadFlagSRCmd[] = { READ_FLAG_STATUS_CMD, 0 };
	u8 FlagStatus[2] = {0};
#endif

	for (DieCnt = 0; DieCnt < InstancePtr->NumDie; DieCnt++) {
#ifndef XPAR_XISF_INTERFACE_QSPIPSU
		/*
		 * Select bank - the lower of the 2 banks in each die
		 * This is specific to Micron flash
		 */
		Status = SendBankSelect(InstancePtr, DieCnt*2);
#endif

		/*
		 * Setup the write command with the specified address and data
		 * for the Flash
		 */
		/*
		 * This ensures 3B address is sent to flash even with address
		 * greater than 128Mb.
		 * The address is the start address of die - MSB bits will be
		 * derived from bank select by the flash
		 */
		InstancePtr->WriteBufPtr[BYTE1] = DIE_ERASE_CMD;
		InstancePtr->WriteBufPtr[BYTE2] = 0x00;
		InstancePtr->WriteBufPtr[BYTE3] = 0x00;
		InstancePtr->WriteBufPtr[BYTE4] = 0x00;

		/*
		 * Enable write before transfer
		 */
		Status = XIsf_WriteEnable(InstancePtr, XISF_WRITE_ENABLE);
		if (Status != (int)XST_SUCCESS)
			return (int)XST_FAILURE;

		Xil_AssertNonvoid(NULLPtr == NULL);
#ifdef XPAR_XISF_INTERFACE_QSPIPSU
		FlashMsg[0].TxBfrPtr = InstancePtr->WriteBufPtr;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = BYTE5;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;
		InstancePtr->SpiInstPtr->Msg = FlashMsg;

		Status = XIsf_Transfer(InstancePtr, NULLPtr, NULLPtr, 1);
		if ((InstancePtr->NumDie > (u8)1) &&
			(FlashMake == (u32)XISF_MANUFACTURER_ID_MICRON)) {
			ReadStatusCmd = READ_FLAG_STATUS_CMD;
			FSRFlag = 1;
		} else {
			ReadStatusCmd = READ_STATUS_CMD;
			FSRFlag = 0;
		}
#else
		/*
		 * Send the sector erase command and address; no receive buffer
		 * is specified since there is nothing to receive
		 */
		Status =
		XIsf_Transfer(InstancePtr, InstancePtr->WriteBufPtr, NULLPtr,
				DIE_ERASE_SIZE);
#endif

		/*
		 * Wait for the sector erase command to Flash to be completed
		 */
		while (1) {
#ifdef XPAR_XISF_INTERFACE_QSPIPSU
			FlashMsg[0].TxBfrPtr = &ReadStatusCmd;
			FlashMsg[0].RxBfrPtr = NULL;
			FlashMsg[0].ByteCount = 1;
			FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
			FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

			FlashMsg[1].TxBfrPtr = NULL;
			FlashMsg[1].RxBfrPtr = FlashStatus;
			FlashMsg[1].ByteCount = 2;
			FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
			FlashMsg[1].Flags = XQSPIPSU_MSG_FLAG_RX;
			InstancePtr->SpiInstPtr->Msg = FlashMsg;

			if (InstancePtr->SpiInstPtr->Config.ConnectionMode ==
					XISF_QSPIPS_CONNECTION_MODE_PARALLEL)
				FlashMsg[1].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;

			Status = XIsf_Transfer(InstancePtr,
					NULLPtr, NULLPtr, 2);
			if (Status != XST_SUCCESS)
				return XST_FAILURE;

			if (InstancePtr->SpiInstPtr->Config.ConnectionMode ==
					XISF_QSPIPS_CONNECTION_MODE_PARALLEL) {
				if (FSRFlag)
					FlashStatus[1] &= FlashStatus[0];
				else
					FlashStatus[1] |= FlashStatus[0];
			}

			if (FSRFlag) {
				if ((FlashStatus[1] & 0x80) != 0)
					break;
			} else {
				if ((FlashStatus[1] & 0x01) == 0)
					break;
			}
#else

			/*
			 * Poll the status register of the device to determine
			 * when it completes, by sending a read status command
			 * and receiving the status byte
			 */
			Status =
			XIsf_Transfer(InstancePtr, ReadFlagSRCmd, FlagStatus,
					(u32)sizeof(ReadFlagSRCmd));

			/*
			 * If the status indicates the write is done, then stop
			 * waiting, if a value of 0xFF in the status byte is
			 * read from the device and this loop never exits, the
			 * device slave select is possibly incorrect such that
			 * the device status is not being read
			 */
			if ((FlagStatus[1] & (u8)0x80) == (u8)0x80)
				break;
#endif
		}

	}

#endif
/**
 * ((XPAR_XISF_FLASH_FAMILY == WINBOND) || \
 * (XPAR_XISF_FLASH_FAMILY == SPANSION))
 */

	return Status;
}
#endif
