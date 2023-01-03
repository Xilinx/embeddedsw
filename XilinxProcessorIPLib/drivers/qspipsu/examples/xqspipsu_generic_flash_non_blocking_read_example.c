/******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xqspipsu_generic_flash_non_blocking_read_example.c
*
*
** This file contains a design example using the QSPIPSU driver (XQspiPsu)
* The example writes to flash in IO mode and reads it back using non-blocking
* APIs in DMA mode.
* This examples runs with GENFIFO Manual start. It runs in polled mode.
* This example illustrates single, parallel and stacked modes.
* Both the flash devices have to be of the same make and size.
* The hardware which this example runs on, must have a serial Flash (Micron
* N25Q) for it to run. In order to test in single,
* parallel or stacked flash configurations the necessary HW must be present
* and QSPI_MODE (also reflected in ConnectionMode in the instance) has
* to be in sync with HW flash configuration being tested.
*
* This example has been tested with the Micron Serial Flash (N25Q512) and
* ISSI Serial Flash parts of IS25WP and IS25LP series flashes in
* single and parallel modes using A53 and R5 processors.
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.11   akm  02/19/20 First release
* 1.13   akm  11/30/20 Removed unwanted header files.
* 1.13   akm  12/10/20 Set Read command as per the qspi bus width.
* 1.14   akm  07/16/21 Enable Quad Mode for Winbond flashes.
* 1.15   akm  11/19/21 Fix read/write failures on Spansion flash parts.
* 1.15   akm  12/22/21 Initialize variables before use.
* 1.16   akm  08/16/22 Fix logical error in NumSect calculation.
* 1.17   akm  12/16/22 Add timeout in QSPIPSU driver examples.
*
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xqspipsu_flash_config.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define QSPIPSU_DEVICE_ID		XPAR_XQSPIPSU_0_DEVICE_ID

/*
 * Number of flash pages to be written.
 */
#define PAGE_COUNT		32

/*
 * Max page size to initialize write and read buffer
 */
#define MAX_PAGE_SIZE 1024

/*
 * Flash address to which data is to be written.
 */
#define TEST_ADDRESS		0x000000


#define UNIQUE_VALUE		0x20

#define ENTER_4B	1
#define EXIT_4B		0

/**************************** Type Definitions *******************************/

u8 ReadCmd;
u8 WriteCmd;
u8 StatusCmd;
u8 SectorEraseCmd;
u8 FSRFlag;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int QspiPsuFlashNonBlockingReadExample(XQspiPsu *QspiPsuInstancePtr, u16 QspiPsuDeviceId);

int FlashReadID(XQspiPsu *QspiPsuPtr);
int FlashErase(XQspiPsu *QspiPsuPtr, u32 Address, u32 ByteCount, u8 *WriteBfrPtr);
int FlashWrite(XQspiPsu *QspiPsuPtr, u32 Address, u32 ByteCount, u8 Command,
				u8 *WriteBfrPtr);
int FlashRead(XQspiPsu *QspiPsuPtr, u32 Address, u32 ByteCount, u8 Command,
				u8 *WriteBfrPtr, u8 *ReadBfrPtr);
int MultiDieRead(XQspiPsu *QspiPsuPtr, u32 Address, u32 ByteCount, u8 Command,
                 u8 *WriteBfrPtr, u8 *ReadBfrPtr);
u32 GetRealAddr(XQspiPsu *QspiPsuPtr, u32 Address);
int BulkErase(XQspiPsu *QspiPsuPtr, u8 *WriteBfrPtr);
int DieErase(XQspiPsu *QspiPsuPtr, u8 *WriteBfrPtr);
int FlashRegisterRead(XQspiPsu *QspiPsuPtr, u32 ByteCount, u8 Command, u8 *ReadBfrPtr);
int FlashRegisterWrite(XQspiPsu *QspiPsuPtr, u32 ByteCount, u8 Command,
					u8 *WriteBfrPtr, u8 WrEn);
int FlashEnterExit4BAddMode(XQspiPsu *QspiPsuPtr,unsigned int Enable);
int FlashEnableQuadMode(XQspiPsu *QspiPsuPtr);
/************************** Variable Definitions *****************************/
u8 TxBfrPtr;
u8 ReadBfrPtr[3];

u32 FlashMake;
u32 FCTIndex;	/* Flash configuration table index */

/*
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs. They could be local
 * but should at least be static so they are zeroed.
 */
static XQspiPsu QspiPsuInstance;

static XQspiPsu_Msg FlashMsg[5];

/*
 * The following variable allows a test value to be added to the values that
 * are written to the Flash such that unique values can be generated to
 * guarantee the writes to the Flash were successful
 */
int Test = 7;

/*
 * The following variables are used to read and write to the flash and they
 * are global to avoid having large buffers on the stack
 * The buffer size accounts for maximum page size and maximum banks -
 * for each bank separate read will be performed leading to that many
 * (overhead+dummy) bytes
 */
#ifdef __ICCARM__
#pragma data_alignment = 32
u8 ReadBuffer[(PAGE_COUNT * MAX_PAGE_SIZE) + (DATA_OFFSET + DUMMY_SIZE)*8];
#else
u8 ReadBuffer[(PAGE_COUNT * MAX_PAGE_SIZE) + (DATA_OFFSET + DUMMY_SIZE)*8] __attribute__ ((aligned(64)));
#endif
u8 WriteBuffer[(PAGE_COUNT * MAX_PAGE_SIZE) + DATA_OFFSET];
u8 CmdBfr[8];

/*
 * The following constants specify the max amount of data and the size of the
 * the buffer required to hold the data and overhead to transfer the data to
 * and from the Flash. Initialized to single flash page size.
 */
u32 MaxData = PAGE_COUNT*256;

/*****************************************************************************/
/**
 *
 * Main function to call the QSPIPSU Flash Polled example.
 *
 *
 * @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
 *
 * @note	None
 *
 ******************************************************************************/
int main(void)
{
	int Status;

	xil_printf("QSPIPSU Generic Flash Non Blocking Read Example Test \r\n");

	/*
	 * Run the QspiPsu non blocking read example.
	 */
	Status = QspiPsuFlashNonBlockingReadExample(&QspiPsuInstance, QSPIPSU_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("QSPIPSU Generic Flash Non Blocking Read Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran QSPIPSU Generic Flash Non Blocking Read Example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * The purpose of this function is to illustrate how to use the XQspiPsu
 * device driver in single, parallel and stacked modes using
 * flash devices greater than or equal to 128Mb.
 * This function reads data in DMA mode.
 *
 * @param       QspiPsuInstancePtr is a pointer to the instance of the QspiPsu
 *              device.
 * @param        QspiPsuDeviceId is the Device ID of the Qspi Device and is the
 *               XPAR_<QSPI_instance>_DEVICE_ID value from xparameters.h.
 *
 * @return	XST_SUCCESS if successful, else XST_FAILURE.
 *
 * @note	None.
 *
 *****************************************************************************/
int QspiPsuFlashNonBlockingReadExample(XQspiPsu *QspiPsuInstancePtr, u16 QspiPsuDeviceId)
{
	int Status;
	u8 UniqueValue;
	int Count;
	int Page;
	XQspiPsu_Config *QspiPsuConfig;
	int ReadBfrSize;
	u32 PageSize = 0;

	ReadBfrSize = (PAGE_COUNT * MAX_PAGE_SIZE) +
			(DATA_OFFSET + DUMMY_SIZE)*8;

	/*
	 * Initialize the QSPIPSU driver so that it's ready to use
	 */
	QspiPsuConfig = XQspiPsu_LookupConfig(QspiPsuDeviceId);
	if (QspiPsuConfig == NULL) {
		return XST_FAILURE;
	}

	/* To test, change connection mode here if not obtained from HDF */

	Status = XQspiPsu_CfgInitialize(QspiPsuInstancePtr, QspiPsuConfig,
					QspiPsuConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set Manual Start
	 */
	XQspiPsu_SetOptions(QspiPsuInstancePtr, XQSPIPSU_MANUAL_START_OPTION);

	/*
	 * Set the prescaler for QSPIPSU clock
	 */
	XQspiPsu_SetClkPrescaler(QspiPsuInstancePtr, XQSPIPSU_CLK_PRESCALE_8);

	XQspiPsu_SelectFlash(QspiPsuInstancePtr,
		XQSPIPSU_SELECT_FLASH_CS_LOWER,
		XQSPIPSU_SELECT_FLASH_BUS_LOWER);

	/*
	 * Read flash ID and obtain all flash related information
	 * It is important to call the read id function before
	 * performing proceeding to any operation, including
	 * preparing the WriteBuffer
	 */
	Status = FlashReadID(QspiPsuInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	xil_printf("Flash connection mode : %d\n\r",
			QspiPsuConfig->ConnectionMode);
	xil_printf("where 0 - Single; 1 - Stacked; 2 - Parallel\n\r");
	xil_printf("FCTIndex: %d\n\r", FCTIndex);

	/*
	 * Initialize MaxData according to page size.
	 */
	if(QspiPsuInstancePtr->Config.ConnectionMode == XQSPIPSU_CONNECTION_MODE_PARALLEL)
		PageSize = Flash_Config_Table[FCTIndex].PageSize * 2;
	else
		PageSize = Flash_Config_Table[FCTIndex].PageSize;

	MaxData = PAGE_COUNT * PageSize;

	/*
	 * Some flash needs to enable Quad mode before using
	 * quad commands.
	 */
	Status = FlashEnableQuadMode(QspiPsuInstancePtr);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	/*
	 * Address size and read command selection
	 * Micron flash on REMUS doesn't support this 4B write/erase cmd
	 */
	if(QspiPsuInstancePtr->Config.BusWidth == BUSWIDTH_SINGLE)
		ReadCmd = FAST_READ_CMD;
	else if(QspiPsuInstancePtr->Config.BusWidth == BUSWIDTH_DOUBLE)
		ReadCmd = DUAL_READ_CMD;
	else
		ReadCmd = QUAD_READ_CMD;

	WriteCmd = WRITE_CMD;
	SectorEraseCmd = SEC_ERASE_CMD;

	/* Status cmd - SR or FSR selection */
	if ((Flash_Config_Table[FCTIndex].NumDie > 1) &&
			(FlashMake == MICRON_ID_BYTE0)) {
		StatusCmd = READ_FLAG_STATUS_CMD;
		FSRFlag = 1;
	} else {
		StatusCmd = READ_STATUS_CMD;
		FSRFlag = 0;
	}

	xil_printf("ReadCmd: 0x%x, WriteCmd: 0x%x,"
		   " StatusCmd: 0x%x, FSRFlag: %d\n\r",
		ReadCmd, WriteCmd, StatusCmd, FSRFlag);

	if (Flash_Config_Table[FCTIndex].FlashDeviceSize > SIXTEENMB) {
		Status = FlashEnterExit4BAddMode(QspiPsuInstancePtr, ENTER_4B);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		if (FlashMake == SPANSION_ID_BYTE0) {
			if(QspiPsuInstancePtr->Config.BusWidth == BUSWIDTH_SINGLE)
				ReadCmd = FAST_READ_CMD_4B;
			else if(QspiPsuInstancePtr->Config.BusWidth == BUSWIDTH_DOUBLE)
				ReadCmd = DUAL_READ_CMD_4B;
			else
				ReadCmd = QUAD_READ_CMD_4B;

			WriteCmd = WRITE_CMD_4B;
			SectorEraseCmd = SEC_ERASE_CMD_4B;
		}
	}

	for (UniqueValue = UNIQUE_VALUE, Count = 0;
			Count < PageSize;
			Count++, UniqueValue++) {
		WriteBuffer[Count] = (u8)(UniqueValue + Test);
	}

	for (Count = 0; Count < ReadBfrSize; Count++) {
		ReadBuffer[Count] = 0;
	}
	Status = FlashErase(QspiPsuInstancePtr, TEST_ADDRESS, MaxData, CmdBfr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	for (Page = 0; Page < PAGE_COUNT; Page++) {
		Status = FlashWrite(QspiPsuInstancePtr,
				(Page * PageSize) +
				TEST_ADDRESS,
				PageSize,
				WriteCmd, WriteBuffer);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	Status = FlashRead(QspiPsuInstancePtr, TEST_ADDRESS, MaxData, ReadCmd,
				CmdBfr, ReadBuffer);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;
	/*
	 * Setup a pointer to the start of the data that was read into the read
	 * buffer and verify the data read is the data that was written
	 */
	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < MaxData;
	     Count++, UniqueValue++) {
		if (ReadBuffer[Count] != (u8)(UniqueValue + Test)) {
			return XST_FAILURE;
		}
	}

	if (Flash_Config_Table[FCTIndex].FlashDeviceSize > SIXTEENMB) {
		Status = FlashEnterExit4BAddMode(QspiPsuInstancePtr, EXIT_4B);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * Reads the flash ID and identifies the flash in FCT table.
 *
 * @param       QspiPsuPtr is a pointer to the instance of the QspiPsu
 *              device.
 *
 * @return	XST_SUCCESS if successful, else XST_FAILURE.
 *
 * @note	None.
 *
 *****************************************************************************/
int FlashReadID(XQspiPsu *QspiPsuPtr)
{
	int Status;
	u32 ReadId = 0;

	/*
	 * Read ID
	 */
	TxBfrPtr = READ_ID;
	FlashMsg[0].TxBfrPtr = &TxBfrPtr;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].ByteCount = 1;
	FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

	FlashMsg[1].TxBfrPtr = NULL;
	FlashMsg[1].RxBfrPtr = ReadBfrPtr;
	FlashMsg[1].ByteCount = 3;
	FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[1].Flags = XQSPIPSU_MSG_FLAG_RX;

	Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 2);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	xil_printf("FlashID=0x%x 0x%x 0x%x\n\r", ReadBfrPtr[0], ReadBfrPtr[1],
		   ReadBfrPtr[2]);

	/* In case of dual, read both and ensure they are same make/size */

	/*
	 * Deduce flash make
	 */
	FlashMake = ReadBfrPtr[0];

	ReadId = ((ReadBfrPtr[0] << 16) | (ReadBfrPtr[1] << 8) | ReadBfrPtr[2]);
	/*
	 * Assign corresponding index in the Flash configuration table
	 */
	Status = CalculateFCTIndex(ReadId, &FCTIndex);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This function writes to the  serial Flash connected to the QSPIPSU interface.
 * All the data put into the buffer must be in the same page of the device with
 * page boundaries being on 256 byte boundaries.
 *
 * @param	QspiPsuPtr is a pointer to the QSPIPSU driver component to use.
 * @param	Address contains the address to write data to in the Flash.
 * @param	ByteCount contains the number of bytes to write.
 * @param	Command is the command used to write data to the flash. QSPIPSU
 *		device supports only Page Program command to write data to the
 *		flash.
 * @param	WriteBfrPtr is Pointer to the write buffer (which is to be transmitted)
 *
 * @return	XST_SUCCESS if successful, else XST_FAILURE.
 *
 * @note	None.
 *
 ******************************************************************************/
int FlashWrite(XQspiPsu *QspiPsuPtr, u32 Address, u32 ByteCount, u8 Command,
				u8 *WriteBfrPtr)
{
	u8 WriteEnableCmd;
	u8 ReadStatusCmd;
	u8 FlashStatus[2];
	u8 WriteCmd[5];
	u32 RealAddr;
	u32 CmdByteCount;
	int Status;
	u32 DelayCount;

	WriteEnableCmd = WRITE_ENABLE_CMD;
	/*
	 * Translate address based on type of connection
	 * If stacked assert the slave select based on address
	 */
	RealAddr = GetRealAddr(QspiPsuPtr, Address);

	/*
	 * Send the write enable command to the Flash so that it can be
	 * written to, this needs to be sent as a separate transfer before
	 * the write
	 */
	FlashMsg[0].TxBfrPtr = &WriteEnableCmd;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].ByteCount = 1;
	FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

	Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	WriteCmd[COMMAND_OFFSET]   = Command;

	/* To be used only if 4B address program cmd is supported by flash */
	if (Flash_Config_Table[FCTIndex].FlashDeviceSize > SIXTEENMB) {
		WriteCmd[ADDRESS_1_OFFSET] =
				(u8)((RealAddr & 0xFF000000) >> 24);
		WriteCmd[ADDRESS_2_OFFSET] =
				(u8)((RealAddr & 0xFF0000) >> 16);
		WriteCmd[ADDRESS_3_OFFSET] =
				(u8)((RealAddr & 0xFF00) >> 8);
		WriteCmd[ADDRESS_4_OFFSET] =
				(u8)(RealAddr & 0xFF);
		CmdByteCount = 5;
	} else {
		WriteCmd[ADDRESS_1_OFFSET] =
				(u8)((RealAddr & 0xFF0000) >> 16);
		WriteCmd[ADDRESS_2_OFFSET] =
				(u8)((RealAddr & 0xFF00) >> 8);
		WriteCmd[ADDRESS_3_OFFSET] =
				(u8)(RealAddr & 0xFF);
		CmdByteCount = 4;
	}

	FlashMsg[0].TxBfrPtr = WriteCmd;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].ByteCount = CmdByteCount;
	FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

	FlashMsg[1].TxBfrPtr = WriteBfrPtr;
	FlashMsg[1].RxBfrPtr = NULL;
	FlashMsg[1].ByteCount = ByteCount;
	FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[1].Flags = XQSPIPSU_MSG_FLAG_TX;
	if (QspiPsuPtr->Config.ConnectionMode ==
			XQSPIPSU_CONNECTION_MODE_PARALLEL) {
		FlashMsg[1].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
	}

	Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 2);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait for the write command to the Flash to be completed, it takes
	 * some time for the data to be written
	 */
	DelayCount = 0;
	while (1) {
		if (DelayCount == MAX_DELAY_CNT) {
			Status = XST_FAILURE;
			goto ERROR_PATH;
		} else {
			ReadStatusCmd = StatusCmd;
			ReadStatusCmd = StatusCmd;
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
			if (QspiPsuPtr->Config.ConnectionMode ==
					XQSPIPSU_CONNECTION_MODE_PARALLEL) {
				FlashMsg[1].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
			}

			Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 2);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}

			if (QspiPsuPtr->Config.ConnectionMode ==
					XQSPIPSU_CONNECTION_MODE_PARALLEL) {
				if (FSRFlag) {
					FlashStatus[1] &= FlashStatus[0];
				} else {
					FlashStatus[1] |= FlashStatus[0];
				}
			}

			if (FSRFlag) {
				if ((FlashStatus[1] & 0x80) != 0) {
					break;
				}
			} else {
				if ((FlashStatus[1] & 0x01) == 0) {
					break;
				}
			}

			/* Wait for 1 usec */
			usleep(1);
			DelayCount++;
		}
	}
ERROR_PATH:
	return Status;
}

/*****************************************************************************/
/**
 *
 * This function erases the sectors in the  serial Flash connected to the
 * QSPIPSU interface.
 *
 * @param	QspiPsuPtr is a pointer to the QSPIPSU driver component to use.
 * @param	Address contains the address of the first sector which needs to
 *		be erased.
 * @param	ByteCount contains the total size to be erased.
 * @param	WriteBfrPtr is pointer to the write buffer (which is to be transmitted)
 *
 * @return	XST_SUCCESS if successful, else XST_FAILURE.
 *
 * @note	None.
 *
 ******************************************************************************/
int FlashErase(XQspiPsu *QspiPsuPtr, u32 Address, u32 ByteCount,
		u8 *WriteBfrPtr)
{
	u8 WriteEnableCmd;
	u8 ReadStatusCmd;
	u8 FlashStatus[2];
	int Sector;
	u32 RealAddr;
	u32 NumSect;
	int Status;
	u32 SectSize;
	u32 SectMask;
	u32 DelayCount;

	WriteEnableCmd = WRITE_ENABLE_CMD;

	if(QspiPsuPtr->Config.ConnectionMode == XQSPIPSU_CONNECTION_MODE_PARALLEL) {
		SectMask = (Flash_Config_Table[FCTIndex]).SectMask - (Flash_Config_Table[FCTIndex]).SectSize;
		SectSize = (Flash_Config_Table[FCTIndex]).SectSize * 2;
		NumSect = (Flash_Config_Table[FCTIndex]).NumSect;
	} else if (QspiPsuPtr->Config.ConnectionMode == XQSPIPSU_CONNECTION_MODE_STACKED) {
		NumSect = (Flash_Config_Table[FCTIndex]).NumSect * 2;
		SectMask = (Flash_Config_Table[FCTIndex]).SectMask;
		SectSize = (Flash_Config_Table[FCTIndex]).SectSize;
	} else {
		SectSize = (Flash_Config_Table[FCTIndex]).SectSize;
		NumSect = (Flash_Config_Table[FCTIndex]).NumSect;
		SectMask = (Flash_Config_Table[FCTIndex]).SectMask;
	}

	/*
	 * If erase size is same as the total size of the flash, use bulk erase
	 * command or die erase command multiple times as required
	 */
	if (ByteCount == NumSect * SectSize) {

		if (QspiPsuPtr->Config.ConnectionMode ==
				XQSPIPSU_CONNECTION_MODE_STACKED) {
			XQspiPsu_SelectFlash(QspiPsuPtr,
				XQSPIPSU_SELECT_FLASH_CS_LOWER,
				XQSPIPSU_SELECT_FLASH_BUS_LOWER);
		}

		if (Flash_Config_Table[FCTIndex].NumDie == 1) {
			/*
			 * Call Bulk erase
			 */
			BulkErase(QspiPsuPtr, WriteBfrPtr);
		}

		if (Flash_Config_Table[FCTIndex].NumDie > 1) {
			/*
			 * Call Die erase
			 */
			DieErase(QspiPsuPtr, WriteBfrPtr);
		}
		/*
		 * If stacked mode, bulk erase second flash
		 */
		if (QspiPsuPtr->Config.ConnectionMode ==
				XQSPIPSU_CONNECTION_MODE_STACKED) {

			XQspiPsu_SelectFlash(QspiPsuPtr,
				XQSPIPSU_SELECT_FLASH_CS_UPPER,
				XQSPIPSU_SELECT_FLASH_BUS_LOWER);

			if (Flash_Config_Table[FCTIndex].NumDie == 1) {
				/*
				 * Call Bulk erase
				 */
				BulkErase(QspiPsuPtr, WriteBfrPtr);
			}

			if (Flash_Config_Table[FCTIndex].NumDie > 1) {
				/*
				 * Call Die erase
				 */
				DieErase(QspiPsuPtr, WriteBfrPtr);
			}
		}

		return 0;
	}

	/*
	 * If the erase size is less than the total size of the flash, use
	 * sector erase command
	 */

	/*
	 * Calculate no. of sectors to erase based on byte count
	 */
	if (ByteCount % SectSize)
		NumSect = ByteCount / SectSize + 1;
	else
		NumSect = ByteCount / SectSize;

	/*
	 * If ByteCount to k sectors,
	 * but the address range spans from N to N+k+1 sectors, then
	 * increment no. of sectors to be erased
	 */

	if (((Address + ByteCount) & SectMask) ==
		((Address + (NumSect * SectSize)) & SectMask)) {
		NumSect++;
	}

	for (Sector = 0; Sector < NumSect; Sector++) {

		/*
		 * Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		RealAddr = GetRealAddr(QspiPsuPtr, Address);

		/*
		 * Send the write enable command to the Flash so that it can be
		 * written to, this needs to be sent as a separate
		 * transfer before the write
		 */
		FlashMsg[0].TxBfrPtr = &WriteEnableCmd;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		WriteBfrPtr[COMMAND_OFFSET]   = SectorEraseCmd;

		/*
		 * To be used only if 4B address sector erase cmd is
		 * supported by flash
		 */
		if (Flash_Config_Table[FCTIndex].FlashDeviceSize > SIXTEENMB) {
			WriteBfrPtr[ADDRESS_1_OFFSET] =
					(u8)((RealAddr & 0xFF000000) >> 24);
			WriteBfrPtr[ADDRESS_2_OFFSET] =
					(u8)((RealAddr & 0xFF0000) >> 16);
			WriteBfrPtr[ADDRESS_3_OFFSET] =
					(u8)((RealAddr & 0xFF00) >> 8);
			WriteBfrPtr[ADDRESS_4_OFFSET] =
					(u8)(RealAddr & 0xFF);
			FlashMsg[0].ByteCount = 5;
		} else {
			WriteBfrPtr[ADDRESS_1_OFFSET] =
					(u8)((RealAddr & 0xFF0000) >> 16);
			WriteBfrPtr[ADDRESS_2_OFFSET] =
					(u8)((RealAddr & 0xFF00) >> 8);
			WriteBfrPtr[ADDRESS_3_OFFSET] =
					(u8)(RealAddr & 0xFF);
			FlashMsg[0].ByteCount = 4;
		}

		FlashMsg[0].TxBfrPtr = WriteBfrPtr;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/*
		 * Wait for the erase command to be completed
		 */
		DelayCount = 0;
		while (1) {
			if (DelayCount == MAX_DELAY_CNT) {
				Status = XST_FAILURE;
				goto ERROR_PATH;
			} else {
				ReadStatusCmd = StatusCmd;
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
				if (QspiPsuPtr->Config.ConnectionMode ==
						XQSPIPSU_CONNECTION_MODE_PARALLEL) {
					FlashMsg[1].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
				}

				Status = XQspiPsu_PolledTransfer(QspiPsuPtr,
						FlashMsg, 2);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}

				if (QspiPsuPtr->Config.ConnectionMode ==
						XQSPIPSU_CONNECTION_MODE_PARALLEL) {
					if (FSRFlag) {
						FlashStatus[1] &= FlashStatus[0];
					} else {
						FlashStatus[1] |= FlashStatus[0];
					}
				}

				if (FSRFlag) {
					if ((FlashStatus[1] & 0x80) != 0) {
						break;
					}
				} else {
					if ((FlashStatus[1] & 0x01) == 0) {
						break;
					}
				}

				/* Wait for 1 usec */
				usleep(1);
				DelayCount++;
			}
		}
		Address += SectSize;
	}

ERROR_PATH:
	return Status;
}


/*****************************************************************************/
/**
 *
 * This function performs read. DMA is the default setting.
 *
 * @param	QspiPsuPtr is a pointer to the QSPIPSU driver component to use.
 * @param	Address contains the address of the first sector which needs to
 *			be erased.
 * @param	ByteCount contains the total size to be erased.
 * @param	Command is the command used to read data from the flash.
 *		Supports normal, fast, dual and quad read commands.
 * @param	WriteBfrPtr is Pointer to the write buffer which contains data
 *		to be transmitted
 * @param	ReadBfrPtr is Pointer to the read buffer to which valid received
 *		data should be written
 *
 * @return	XST_SUCCESS if successful, else XST_FAILURE.
 *
 * @note	None.
 *
 ******************************************************************************/
int FlashRead(XQspiPsu *QspiPsuPtr, u32 Address, u32 ByteCount, u8 Command,
				u8 *WriteBfrPtr, u8 *ReadBfrPtr)
{
	u32 RealAddr;
	u32 DiscardByteCnt;
	u32 FlashMsgCnt;
	int Status;

	/* Check die boundary conditions if required for any flash */
	if (Flash_Config_Table[FCTIndex].NumDie > 1) {
		Status = MultiDieRead(QspiPsuPtr, Address, ByteCount, Command,
				      WriteBfrPtr, ReadBfrPtr);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;
	} else {
		/* For Dual Stacked, split and read for boundary crossing */
		/*
		 * Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		RealAddr = GetRealAddr(QspiPsuPtr, Address);

		WriteBfrPtr[COMMAND_OFFSET]   = Command;
		if (Flash_Config_Table[FCTIndex].FlashDeviceSize > SIXTEENMB) {
			WriteBfrPtr[ADDRESS_1_OFFSET] =
					(u8)((RealAddr & 0xFF000000) >> 24);
			WriteBfrPtr[ADDRESS_2_OFFSET] =
					(u8)((RealAddr & 0xFF0000) >> 16);
			WriteBfrPtr[ADDRESS_3_OFFSET] =
					(u8)((RealAddr & 0xFF00) >> 8);
			WriteBfrPtr[ADDRESS_4_OFFSET] =
					(u8)(RealAddr & 0xFF);
			DiscardByteCnt = 5;
		} else {
			WriteBfrPtr[ADDRESS_1_OFFSET] =
					(u8)((RealAddr & 0xFF0000) >> 16);
			WriteBfrPtr[ADDRESS_2_OFFSET] =
					(u8)((RealAddr & 0xFF00) >> 8);
			WriteBfrPtr[ADDRESS_3_OFFSET] =
					(u8)(RealAddr & 0xFF);
			DiscardByteCnt = 4;
		}

		FlashMsg[0].TxBfrPtr = WriteBfrPtr;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = DiscardByteCnt;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		FlashMsgCnt = 1;

		/* It is recommended to have a separate entry for dummy */
		if (Command == FAST_READ_CMD || Command == DUAL_READ_CMD ||
		    Command == QUAD_READ_CMD || Command == FAST_READ_CMD_4B ||
		    Command == DUAL_READ_CMD_4B ||
		    Command == QUAD_READ_CMD_4B) {
			/* Update Dummy cycles as per flash specs for QUAD IO */

			/*
			 * It is recommended that Bus width value during dummy
			 * phase should be same as data phase
			 */
			if (Command == FAST_READ_CMD ||
			    Command == FAST_READ_CMD_4B)
				FlashMsg[1].BusWidth =
						XQSPIPSU_SELECT_MODE_SPI;

			if (Command == DUAL_READ_CMD ||
			    Command == DUAL_READ_CMD_4B)
				FlashMsg[1].BusWidth =
						XQSPIPSU_SELECT_MODE_DUALSPI;

			if (Command == QUAD_READ_CMD ||
			    Command == QUAD_READ_CMD_4B)
				FlashMsg[1].BusWidth =
						XQSPIPSU_SELECT_MODE_QUADSPI;

			FlashMsg[1].TxBfrPtr = NULL;
			FlashMsg[1].RxBfrPtr = NULL;
			FlashMsg[1].ByteCount = DUMMY_CLOCKS;
			FlashMsg[1].Flags = 0;

			FlashMsgCnt++;
		}

		if (Command == FAST_READ_CMD ||
		    Command == FAST_READ_CMD_4B)
			FlashMsg[FlashMsgCnt].BusWidth =
					XQSPIPSU_SELECT_MODE_SPI;

		if (Command == DUAL_READ_CMD ||
		    Command == DUAL_READ_CMD_4B)
			FlashMsg[FlashMsgCnt].BusWidth =
					XQSPIPSU_SELECT_MODE_DUALSPI;

		if (Command == QUAD_READ_CMD ||
		    Command == QUAD_READ_CMD_4B)
			FlashMsg[FlashMsgCnt].BusWidth =
					XQSPIPSU_SELECT_MODE_QUADSPI;

		FlashMsg[FlashMsgCnt].TxBfrPtr = NULL;
		FlashMsg[FlashMsgCnt].RxBfrPtr = ReadBfrPtr;
		FlashMsg[FlashMsgCnt].ByteCount = ByteCount;
		FlashMsg[FlashMsgCnt].Flags = XQSPIPSU_MSG_FLAG_RX;

		if (QspiPsuPtr->Config.ConnectionMode ==
				XQSPIPSU_CONNECTION_MODE_PARALLEL)
			FlashMsg[FlashMsgCnt].Flags |=
					XQSPIPSU_MSG_FLAG_STRIPE;

		Status = XQspiPsu_StartDmaTransfer(QspiPsuPtr, FlashMsg,
						 FlashMsgCnt + 1);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;

		do
			Status = XQspiPsu_CheckDmaDone(QspiPsuPtr);
		while(Status != XST_SUCCESS);
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This functions performs a read operation for multi die flash devices.
 *
 * @param	QspiPsuPtr is a pointer to the QSPIPSU driver component to use.
 * @param	Address contains the address of the first sector which needs to
 *			be erased.
 * @param	ByteCount contains the total size to be erased.
 * @param	Command is the command used to read data from the flash.
 *			Supports normal, fast, dual and quad read commands.
 * @param	WriteBfrPtr is pointer to the write buffer which contains data to be
 *			transmitted
 * @param	ReadBfrPtr is pointer to the read buffer to which valid received data
 *			should be written.
 *
 * @return	XST_SUCCESS if successful, else XST_FAILURE.
 *
 * @note	None.
 *
 ******************************************************************************/
int MultiDieRead(XQspiPsu *QspiPsuPtr, u32 Address, u32 ByteCount, u8 Command,
		 u8 *WriteBfrPtr, u8 *ReadBfrPtr)
{
	u32 RealAddr;
	u32 DiscardByteCnt;
	u32 FlashMsgCnt;
	int Status;
	u32 cur_bank = 0;
	u32 nxt_bank = 0;
	u32 bank_size;
	u32 remain_len = ByteCount;
	u32 data_len;
	u32 transfer_len;
	u8 *ReadBuffer = ReadBfrPtr;

	/*
	 * Some flash devices like N25Q512 have multiple dies
	 * in it. Read operation in these devices is bounded
	 * by its die segment. In a continuous read, across
	 * multiple dies, when the last byte of the selected
	 * die segment is read, the next byte read is the
	 * first byte of the same die segment. This is Die
	 * cross over issue. So to handle this issue, split
	 * a read transaction, that spans across multiple
	 * banks, into one read per bank. Bank size is 16MB
	 * for single and dual stacked mode and 32MB for dual
	 * parallel mode.
	 */
	if (QspiPsuPtr->Config.ConnectionMode ==
			XQSPIPSU_CONNECTION_MODE_PARALLEL)

		bank_size = SIXTEENMB << 1;

	else if (QspiPsuPtr->Config.ConnectionMode ==
			XQSPIPSU_CONNECTION_MODE_SINGLE)

		bank_size = SIXTEENMB;

	while (remain_len) {
		cur_bank = Address / bank_size;
		nxt_bank = (Address + remain_len) / bank_size;

		if (cur_bank != nxt_bank) {
			transfer_len = (bank_size * (cur_bank  + 1)) - Address;
			if (remain_len < transfer_len)
				data_len = remain_len;
			else
				data_len = transfer_len;
		} else {
			data_len = remain_len;
		}
		/*
		 * Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		RealAddr = GetRealAddr(QspiPsuPtr, Address);

		WriteBfrPtr[COMMAND_OFFSET]   = Command;
		if (Flash_Config_Table[FCTIndex].FlashDeviceSize > SIXTEENMB) {
			WriteBfrPtr[ADDRESS_1_OFFSET] =
					(u8)((RealAddr & 0xFF000000) >> 24);
			WriteBfrPtr[ADDRESS_2_OFFSET] =
					(u8)((RealAddr & 0xFF0000) >> 16);
			WriteBfrPtr[ADDRESS_3_OFFSET] =
					(u8)((RealAddr & 0xFF00) >> 8);
			WriteBfrPtr[ADDRESS_4_OFFSET] =
					(u8)(RealAddr & 0xFF);
			DiscardByteCnt = 5;
		} else {
			WriteBfrPtr[ADDRESS_1_OFFSET] =
					(u8)((RealAddr & 0xFF0000) >> 16);
			WriteBfrPtr[ADDRESS_2_OFFSET] =
					(u8)((RealAddr & 0xFF00) >> 8);
			WriteBfrPtr[ADDRESS_3_OFFSET] =
					(u8)(RealAddr & 0xFF);
			DiscardByteCnt = 4;
		}

		FlashMsg[0].TxBfrPtr = WriteBfrPtr;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = DiscardByteCnt;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		FlashMsgCnt = 1;

		/* It is recommended to have a separate entry for dummy */
		if (Command == FAST_READ_CMD || Command == DUAL_READ_CMD ||
		    Command == QUAD_READ_CMD || Command == FAST_READ_CMD_4B ||
		    Command == DUAL_READ_CMD_4B ||
		    Command == QUAD_READ_CMD_4B) {
			/* Update Dummy cycles as per flash specs for QUAD IO */

			/*
			 * It is recommended that Bus width value during dummy
			 * phase should be same as data phase
			 */
			if (Command == FAST_READ_CMD ||
			    Command == FAST_READ_CMD_4B)
				FlashMsg[1].BusWidth =
						XQSPIPSU_SELECT_MODE_SPI;

			if (Command == DUAL_READ_CMD ||
			    Command == DUAL_READ_CMD_4B)
				FlashMsg[1].BusWidth =
						XQSPIPSU_SELECT_MODE_DUALSPI;

			if (Command == QUAD_READ_CMD ||
			    Command == QUAD_READ_CMD_4B)
				FlashMsg[1].BusWidth =
						XQSPIPSU_SELECT_MODE_QUADSPI;

			FlashMsg[1].TxBfrPtr = NULL;
			FlashMsg[1].RxBfrPtr = NULL;
			FlashMsg[1].ByteCount = DUMMY_CLOCKS;
			FlashMsg[1].Flags = 0;

			FlashMsgCnt++;
		}

		if (Command == FAST_READ_CMD ||
		    Command == FAST_READ_CMD_4B)
			FlashMsg[FlashMsgCnt].BusWidth =
					XQSPIPSU_SELECT_MODE_SPI;

		if (Command == DUAL_READ_CMD ||
		    Command == DUAL_READ_CMD_4B)
			FlashMsg[FlashMsgCnt].BusWidth =
					XQSPIPSU_SELECT_MODE_DUALSPI;

		if (Command == QUAD_READ_CMD ||
		    Command == QUAD_READ_CMD_4B)
			FlashMsg[FlashMsgCnt].BusWidth =
					XQSPIPSU_SELECT_MODE_QUADSPI;

		FlashMsg[FlashMsgCnt].TxBfrPtr = NULL;
		FlashMsg[FlashMsgCnt].RxBfrPtr = ReadBuffer;
		FlashMsg[FlashMsgCnt].ByteCount = data_len;
		FlashMsg[FlashMsgCnt].Flags = XQSPIPSU_MSG_FLAG_RX;

		if (QspiPsuPtr->Config.ConnectionMode ==
				XQSPIPSU_CONNECTION_MODE_PARALLEL)
			FlashMsg[FlashMsgCnt].Flags |=
					XQSPIPSU_MSG_FLAG_STRIPE;

		Status = XQspiPsu_StartDmaTransfer(QspiPsuPtr, FlashMsg,
						 FlashMsgCnt + 1);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;
		do
			Status = XQspiPsu_CheckDmaDone(QspiPsuPtr);
		while(Status != XST_SUCCESS);

		ReadBuffer += data_len;
		Address += data_len;
		remain_len -= data_len;
	}

	return XST_SUCCESS;
}


/*****************************************************************************/
/**
 *
 * This API can be used to write to a flash register.
 *
 * @param	QspiPsuPtr is a pointer to the QSPIPSU driver component to use.
 * @param	ByteCount is the number of bytes to write.
 * @param	Command is specific register write command.
 * @param	WriteBfrPtr is the pointer to value to be written.
 * @param	WrEn is a flag to mention if WREN has to be sent before write.
 *
 * @return	XST_SUCCESS if successful, else XST_FAILURE.
 *
 * @note	This API can only be used for one flash at a time.
 *
 ******************************************************************************/
int FlashRegisterWrite(XQspiPsu *QspiPsuPtr, u32 ByteCount, u8 Command,
					u8 *WriteBfrPtr, u8 WrEn)
{
	u8 WriteEnableCmd;
	u8 ReadStatusCmd;
	u8 FlashStatus[2];
	int Status;
	u32 DelayCount;

	if (WrEn) {
		WriteEnableCmd = WRITE_ENABLE_CMD;

		/*
		 * Send the write enable command to the Flash so that it can be
		 * written to, this needs to be sent as a separate
		 * transfer before the write
		 */
		FlashMsg[0].TxBfrPtr = &WriteEnableCmd;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	WriteBfrPtr[COMMAND_OFFSET]   = Command;
	/*
	 * Value(s) is(are) expected to be written to the
	 * write buffer by calling API ByteCount is the count
	 * of the value(s) excluding the command.
	 */

	FlashMsg[0].TxBfrPtr = WriteBfrPtr;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].ByteCount = ByteCount + 1;
	FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

	Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait for the register write command to the Flash to be completed.
	 */
	DelayCount = 0;
	while (1) {
		if (DelayCount == MAX_DELAY_CNT) {
			Status = XST_FAILURE;
			goto ERROR_PATH;
		} else {
			ReadStatusCmd = StatusCmd;
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

			Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 2);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}

			if (FSRFlag) {
				if ((FlashStatus[1] & 0x80) != 0) {
					break;
				}
			} else {
				if ((FlashStatus[1] & 0x01) == 0) {
					break;
				}
			}
			/* Wait for 1 usec */
			usleep(1);
			DelayCount++;
		}
	}

ERROR_PATH:
	return Status;
}

/*****************************************************************************/
/**
 *
 * This API can be used to write to a flash register.
 *
 * @param	QspiPsuPtr is a pointer to the QSPIPSU driver component to use.
 * @param	ByteCount is the number of bytes to write.
 * @param	Command is specific register write command.
 * @param	ReadBfrPtr is Pointer to the read buffer to which valid received
 *		data should be written
 *
 * @return	XST_SUCCESS if successful, else XST_FAILURE.
 *
 * @note	This API can only be used for one flash at a time.
 *
 ******************************************************************************/
int FlashRegisterRead(XQspiPsu *QspiPsuPtr, u32 ByteCount,
		u8 Command, u8 *ReadBfrPtr)
{
	u8 WriteCmd;
	int Status;

	WriteCmd = Command;
	FlashMsg[0].TxBfrPtr = &WriteCmd;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].ByteCount = 1;
	FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

	FlashMsg[1].TxBfrPtr = NULL;
	FlashMsg[1].RxBfrPtr = ReadBfrPtr;
	//FlashMsg[1].ByteCount = ByteCount;
	/* This is for DMA reasons; to be changed shortly */
	FlashMsg[1].ByteCount = 4;
	FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[1].Flags = XQSPIPSU_MSG_FLAG_RX;

	Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 2);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return 0;
}

/*****************************************************************************/
/**
 *
 * This functions performs a bulk erase operation when the
 * flash device has a single die. Works for both Spansion and Micron
 *
 * @param	QspiPsuPtr is a pointer to the QSPIPSU driver component to use.
 * @param	WriteBfrPtr is the pointer to command+address to be sent
 *
 * @return	XST_SUCCESS if successful, else XST_FAILURE.
 *
 * @note	None.
 *
 ******************************************************************************/
int BulkErase(XQspiPsu *QspiPsuPtr, u8 *WriteBfrPtr)
{
	u8 WriteEnableCmd;
	u8 ReadStatusCmd;
	u8 FlashStatus[2];
	int Status;
	u32 DelayCount;

	WriteEnableCmd = WRITE_ENABLE_CMD;
	/*
	 * Send the write enable command to the Flash so that it can be
	 * written to, this needs to be sent as a separate transfer before
	 * the write
	 */
	FlashMsg[0].TxBfrPtr = &WriteEnableCmd;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].ByteCount = 1;
	FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

	Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	WriteBfrPtr[COMMAND_OFFSET]   = BULK_ERASE_CMD;
	FlashMsg[0].TxBfrPtr = WriteBfrPtr;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].ByteCount = 1;
	FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

	Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait for the write command to the Flash to be completed, it takes
	 * some time for the data to be written
	 */
	DelayCount = 0;
	while (1) {
		if (DelayCount == MAX_DELAY_CNT) {
			Status = XST_FAILURE;
			goto ERROR_PATH;
		} else {
			ReadStatusCmd = StatusCmd;
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
			if (QspiPsuPtr->Config.ConnectionMode ==
					XQSPIPSU_CONNECTION_MODE_PARALLEL) {
				FlashMsg[1].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
			}

			Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 2);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
			if (QspiPsuPtr->Config.ConnectionMode ==
					XQSPIPSU_CONNECTION_MODE_PARALLEL) {
				if (FSRFlag) {
					FlashStatus[1] &= FlashStatus[0];
				} else {
					FlashStatus[1] |= FlashStatus[0];
				}
			}

			if (FSRFlag) {
				if ((FlashStatus[1] & 0x80) != 0) {
					break;
				}
			} else {
				if ((FlashStatus[1] & 0x01) == 0) {
					break;
				}
			}
			/* Wait for 1 usec */
			usleep(1);
			DelayCount++;
		}
	}

ERROR_PATH:
	return Status;
}

/*****************************************************************************/
/**
 *
 * This functions performs a die erase operation on all the die in
 * the flash device. This function uses the die erase command for
 * Micron 512Mbit and 1Gbit
 *
 * @param	QspiPsuPtr is a pointer to the QSPIPSU driver component to use.
 * @param	WriteBfrPtr is the pointer to command+address to be sent
 *
 * @return	XST_SUCCESS if successful, else XST_FAILURE.
 *
 * @note	None.
 *
 ******************************************************************************/
int DieErase(XQspiPsu *QspiPsuPtr, u8 *WriteBfrPtr)
{
	u8 WriteEnableCmd;
	u8 DieCnt;
	u8 ReadStatusCmd;
	u8 FlashStatus[2];
	int Status;
	u32 DieSize = 0;
	u32 Address;
	u32 RealAddr;
	u32 SectSize = 0;
	u32 NumSect = 0;
	u32 DelayCount;

	WriteEnableCmd = WRITE_ENABLE_CMD;

	if (QspiPsuPtr->Config.ConnectionMode == XQSPIPSU_CONNECTION_MODE_PARALLEL) {
		SectSize = (Flash_Config_Table[FCTIndex]).SectSize * 2;
	} else if (QspiPsuPtr->Config.ConnectionMode == XQSPIPSU_CONNECTION_MODE_STACKED) {
		NumSect = (Flash_Config_Table[FCTIndex]).NumSect * 2;
	} else {
		SectSize = (Flash_Config_Table[FCTIndex]).SectSize;
		NumSect = (Flash_Config_Table[FCTIndex]).NumSect;
	}
	DieSize = (NumSect * SectSize) / Flash_Config_Table[FCTIndex].NumDie;
	for (DieCnt = 0;
		DieCnt < Flash_Config_Table[FCTIndex].NumDie;
		DieCnt++) {
		/*
		 * Send the write enable command to the Flash so that it can be
		 * written to, this needs to be sent as a separate transfer
		 * before the write
		 */
		FlashMsg[0].TxBfrPtr = &WriteEnableCmd;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		WriteBfrPtr[COMMAND_OFFSET]   = DIE_ERASE_CMD;
		Address = DieSize * DieCnt;
		RealAddr = GetRealAddr(QspiPsuPtr, Address);
		/*
		 * To be used only if 4B address sector erase cmd is
		 * supported by flash
		 */
		if (Flash_Config_Table[FCTIndex].FlashDeviceSize > SIXTEENMB) {
			WriteBfrPtr[ADDRESS_1_OFFSET] =
					(u8)((RealAddr & 0xFF000000) >> 24);
			WriteBfrPtr[ADDRESS_2_OFFSET] =
					(u8)((RealAddr & 0xFF0000) >> 16);
			WriteBfrPtr[ADDRESS_3_OFFSET] =
					(u8)((RealAddr & 0xFF00) >> 8);
			WriteBfrPtr[ADDRESS_4_OFFSET] =
					(u8)(RealAddr & 0xFF);
			FlashMsg[0].ByteCount = 5;
		} else {
			WriteBfrPtr[ADDRESS_1_OFFSET] =
					(u8)((RealAddr & 0xFF0000) >> 16);
			WriteBfrPtr[ADDRESS_2_OFFSET] =
					(u8)((RealAddr & 0xFF00) >> 8);
			WriteBfrPtr[ADDRESS_3_OFFSET] =
					(u8)(RealAddr & 0xFF);
			FlashMsg[0].ByteCount = 4;
		}

		FlashMsg[0].TxBfrPtr = WriteBfrPtr;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/*
		 * Wait for the write command to the Flash to be completed,
		 * it takes some time for the data to be written
		 */
		DelayCount = 0;
		while (1) {
			if (DelayCount == MAX_DELAY_CNT) {
				Status = XST_FAILURE;
				goto ERROR_PATH;
			} else {
				ReadStatusCmd = StatusCmd;
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
				if (QspiPsuPtr->Config.ConnectionMode ==
						XQSPIPSU_CONNECTION_MODE_PARALLEL) {
					FlashMsg[1].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
				}

				Status = XQspiPsu_PolledTransfer(QspiPsuPtr,
						FlashMsg, 2);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}
				if (QspiPsuPtr->Config.ConnectionMode ==
						XQSPIPSU_CONNECTION_MODE_PARALLEL) {
					if (FSRFlag) {
						FlashStatus[1] &= FlashStatus[0];
					} else {
						FlashStatus[1] |= FlashStatus[0];
					}
				}

				if (FSRFlag) {
					if ((FlashStatus[1] & 0x80) != 0) {
						break;
					}
				} else {
					if ((FlashStatus[1] & 0x01) == 0) {
						break;
					}
				}
				/* Wait for 1 usec */
				usleep(1);
				DelayCount++;
			}
		}
	}

ERROR_PATH:
	return Status;
}

/*****************************************************************************/
/**
 *
 * This functions translates the address based on the type of interconnection.
 * In case of stacked, this function asserts the corresponding slave select.
 *
 * @param	QspiPsuPtr is a pointer to the QSPIPSU driver component to use.
 * @param	Address which is to be accessed (for erase, write or read)
 *
 * @return	RealAddr is the translated address - for single it is unchanged;
 *		for stacked, the lower flash size is subtracted;
 *		for parallel the address is divided by 2.
 *
 * @note	In addition to get the actual address to work on flash this
 *		function also selects the CS and BUS based on the configuration
 *		detected.
 *
 ******************************************************************************/
u32 GetRealAddr(XQspiPsu *QspiPsuPtr, u32 Address)
{
	u32 RealAddr;

	switch (QspiPsuPtr->Config.ConnectionMode) {
	case XQSPIPSU_CONNECTION_MODE_SINGLE:
		XQspiPsu_SelectFlash(QspiPsuPtr,
				XQSPIPSU_SELECT_FLASH_CS_LOWER,
				XQSPIPSU_SELECT_FLASH_BUS_LOWER);
		RealAddr = Address;
		break;
	case XQSPIPSU_CONNECTION_MODE_STACKED:
		/* Select lower or upper Flash based on sector address */
		if (Address & Flash_Config_Table[FCTIndex].FlashDeviceSize) {

			XQspiPsu_SelectFlash(QspiPsuPtr,
					XQSPIPSU_SELECT_FLASH_CS_UPPER,
					XQSPIPSU_SELECT_FLASH_BUS_LOWER);
			/*
			 * Subtract first flash size when accessing second flash
			 */
			RealAddr = Address &
				(~Flash_Config_Table[FCTIndex].FlashDeviceSize);
		} else {
			/*
			 * Set selection to L_PAGE
			 */
			XQspiPsu_SelectFlash(QspiPsuPtr,
					XQSPIPSU_SELECT_FLASH_CS_LOWER,
					XQSPIPSU_SELECT_FLASH_BUS_LOWER);

			RealAddr = Address;

		}
		break;
	case XQSPIPSU_CONNECTION_MODE_PARALLEL:
		/*
		 * The effective address in each flash is the actual
		 * address / 2
		 */
		XQspiPsu_SelectFlash(QspiPsuPtr,
				XQSPIPSU_SELECT_FLASH_CS_BOTH,
				XQSPIPSU_SELECT_FLASH_BUS_BOTH);
		RealAddr = Address / 2;
		break;
	default:
		/* RealAddr wont be assigned in this case; */
	break;

	}

	return(RealAddr);
}

/*****************************************************************************/
/**
 * @brief
 * This API enters the flash device into 4 bytes addressing mode.
 * As per the Micron and ISSI spec, before issuing the command to enter
 * into 4 byte addr mode, a write enable command is issued.
 * For Macronix and Winbond flash parts write
 * enable is not required.
 *
 * @param	QspiPsuPtr is a pointer to the QSPIPSU driver component to use.
 * @param	Enable is a either 1 or 0 if 1 then enters 4 byte if 0 exits.
 *
 * @return
 *		- XST_SUCCESS if successful.
 *		- XST_FAILURE if it fails.
 *
 *
 ******************************************************************************/
int FlashEnterExit4BAddMode(XQspiPsu *QspiPsuPtr, unsigned int Enable)
{
	int Status;
	u8 WriteEnableCmd;
	u8 Cmd;
	u8 WriteDisableCmd;
	u8 ReadStatusCmd;
	u8 WriteBuffer[2] = {0};
	u8 FlashStatus[2] = {0};
	u32 DelayCount;

	if (Enable) {
		Cmd = ENTER_4B_ADDR_MODE;
	} else {
		if (FlashMake == ISSI_ID_BYTE0)
			Cmd = EXIT_4B_ADDR_MODE_ISSI;
		else
			Cmd = EXIT_4B_ADDR_MODE;
	}

	switch (FlashMake) {
	case ISSI_ID_BYTE0:
	case MICRON_ID_BYTE0:
		WriteEnableCmd = WRITE_ENABLE_CMD;
		GetRealAddr(QspiPsuPtr, TEST_ADDRESS);
		/*
		 * Send the write enable command to the
		 * Flash so that it can be written to, this
		 * needs to be sent as a separate transfer before
		 * the write
		 */
		FlashMsg[0].TxBfrPtr = &WriteEnableCmd;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		break;

	case SPANSION_ID_BYTE0:

		/* Read Extended Addres Register */
		WriteBuffer[0] = BANK_REG_RD;
		FlashMsg[0].TxBfrPtr = &WriteBuffer[0];
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		FlashMsg[1].TxBfrPtr = NULL;
		FlashMsg[1].RxBfrPtr = &WriteBuffer[1];
		FlashMsg[1].ByteCount = 1;
		FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[1].Flags = XQSPIPSU_MSG_FLAG_RX;

		Status = XQspiPsu_PolledTransfer(QspiPsuPtr,
				FlashMsg, 2);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;
		if (Enable) {
			WriteBuffer[0] = BANK_REG_WR;
			WriteBuffer[1] |= 1 << 7;
		} else {
			WriteBuffer[0] = BANK_REG_WR;
			WriteBuffer[1] &= ~(0x01 << 7);
		}

		FlashMsg[0].TxBfrPtr = &WriteBuffer[0];
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[1].TxBfrPtr = &WriteBuffer[1];
		FlashMsg[2].RxBfrPtr = NULL;
		FlashMsg[2].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[2].Flags = XQSPIPSU_MSG_FLAG_TX;
		FlashMsg[2].ByteCount = 1;

		Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 2);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		WriteBuffer[0] = BANK_REG_RD;
		FlashMsg[0].TxBfrPtr = &WriteBuffer[0];
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		FlashMsg[1].TxBfrPtr = NULL;
		FlashMsg[1].RxBfrPtr = &FlashStatus[0];
		FlashMsg[1].ByteCount = 1;
		FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[1].Flags = XQSPIPSU_MSG_FLAG_RX;

		Status = XQspiPsu_PolledTransfer(QspiPsuPtr,
				FlashMsg, 2);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;
		return Status;

	default:
		/*
		 * For Macronix and Winbond flash parts
		 * Write enable command is not required.
		 */
		break;
	}

	GetRealAddr(QspiPsuPtr, TEST_ADDRESS);

	FlashMsg[0].TxBfrPtr = &Cmd;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].ByteCount = 1;
	FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

	Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	DelayCount = 0;
	while (1) {
		if (DelayCount == MAX_DELAY_CNT) {
			Status = XST_FAILURE;
			goto ERROR_PATH;
		} else {
			ReadStatusCmd = StatusCmd;
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

			if (QspiPsuPtr->Config.ConnectionMode ==
					XQSPIPSU_CONNECTION_MODE_PARALLEL) {
				FlashMsg[1].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
			}

			Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 2);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}

			if (QspiPsuPtr->Config.ConnectionMode ==
					XQSPIPSU_CONNECTION_MODE_PARALLEL) {
				if (FSRFlag) {
					FlashStatus[1] &= FlashStatus[0];
				} else {
					FlashStatus[1] |= FlashStatus[0];
				}
			}

			if (FSRFlag) {
				if ((FlashStatus[1] & 0x80) != 0) {
					break;
				}
			} else {
				if ((FlashStatus[1] & 0x01) == 0) {
					break;
				}
			}
			/* Wait for 1 usec */
			usleep(1);
			DelayCount++;
		}
	}

	switch (FlashMake) {
	case ISSI_ID_BYTE0:
	case MICRON_ID_BYTE0:
		WriteDisableCmd = WRITE_DISABLE_CMD;
		GetRealAddr(QspiPsuPtr, TEST_ADDRESS);
		/*
		 * Send the write enable command to the
		 * Flash so that it can be written to,
		 * this needs to be sent as a separate
		 * transfer before
		 * the write
		 */
		FlashMsg[0].TxBfrPtr = &WriteDisableCmd;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		break;

	default:
		/*
		 * For Macronix and Winbond flash parts
		 * Write disable command is not required.
		 */
		break;
	}

ERROR_PATH:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This API enables Quad mode for the flash parts which require to enable quad
 * mode before using Quad commands.
 * For S25FL-L series flash parts this is required as the default configuration
 * is x1/x2 mode.
 *
 * @param	QspiPsuPtr is a pointer to the QSPIPSU driver component to use.
 *
 * @return
 *		- XST_SUCCESS if successful.
 *		- XST_FAILURE if it fails.
 *
 *
 ******************************************************************************/
int FlashEnableQuadMode(XQspiPsu *QspiPsuPtr)
{
	int Status;
	u8 WriteEnableCmd;
	u8 ReadStatusCmd;
	u8 FlashStatus[2];
	u8 StatusRegVal;
	u8 WriteBuffer[3] = {0};
	u32 DelayCount;

	switch (FlashMake) {
	case SPANSION_ID_BYTE0:
		TxBfrPtr = READ_CONFIG_CMD;
		FlashMsg[0].TxBfrPtr = &TxBfrPtr;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;
		FlashMsg[1].TxBfrPtr = NULL;
		FlashMsg[1].RxBfrPtr = &WriteBuffer[2];
		FlashMsg[1].ByteCount = 1;
		FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[1].Flags = XQSPIPSU_MSG_FLAG_RX;

		Status = XQspiPsu_PolledTransfer(QspiPsuPtr,
				FlashMsg, 2);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		WriteEnableCmd = WRITE_ENABLE_CMD;
		/*
		 * Send the write enable command to the
		 * Flash so that it can be written to, this
		 * needs to be sent as a separate transfer before
		 * the write
		 */
		FlashMsg[0].TxBfrPtr = &WriteEnableCmd;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		Status = XQspiPsu_PolledTransfer(QspiPsuPtr,
				FlashMsg, 1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		GetRealAddr(QspiPsuPtr, TEST_ADDRESS);

		WriteBuffer[0] = WRITE_CONFIG_CMD;
		WriteBuffer[1] |= 0x02;
		WriteBuffer[2] |= 0x01 << 1;

		FlashMsg[0].TxBfrPtr = &WriteBuffer[0];
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[1].TxBfrPtr = &WriteBuffer[1];
		FlashMsg[1].RxBfrPtr = NULL;
		FlashMsg[1].ByteCount = 2;
		FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[1].Flags = XQSPIPSU_MSG_FLAG_TX;

		Status = XQspiPsu_PolledTransfer(QspiPsuPtr,
				FlashMsg, 2);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		DelayCount = 0;
		while (1) {
			if (DelayCount == MAX_DELAY_CNT) {
				Status = XST_FAILURE;
				goto ERROR_PATH;
			} else {
				TxBfrPtr = READ_STATUS_CMD;
				FlashMsg[0].TxBfrPtr = &TxBfrPtr;
				FlashMsg[0].RxBfrPtr = NULL;
				FlashMsg[0].ByteCount = 1;
				FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
				FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

				FlashMsg[1].TxBfrPtr = NULL;
				FlashMsg[1].RxBfrPtr = FlashStatus;
				FlashMsg[1].ByteCount = 2;
				FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
				FlashMsg[1].Flags = XQSPIPSU_MSG_FLAG_RX;

				Status = XQspiPsu_PolledTransfer(QspiPsuPtr,
						FlashMsg, 2);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}
				if (QspiPsuPtr->Config.ConnectionMode ==
							XQSPIPSU_CONNECTION_MODE_PARALLEL) {
					if (FSRFlag) {
						FlashStatus[1] &= FlashStatus[0];
					}else {
						FlashStatus[1] |= FlashStatus[0];
					}
				}

				if ((FlashStatus[1] & 0x01) == 0x00)
					break;

				/* Wait for 1 usec */
				usleep(1);
				DelayCount++;
			}
		}
		TxBfrPtr = READ_CONFIG_CMD;
		FlashMsg[0].TxBfrPtr = &TxBfrPtr;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		FlashMsg[1].TxBfrPtr = NULL;
		FlashMsg[1].RxBfrPtr = ReadBfrPtr;
		FlashMsg[1].ByteCount = 1;
		FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[1].Flags = XQSPIPSU_MSG_FLAG_RX;

		Status = XQspiPsu_PolledTransfer(QspiPsuPtr,
				FlashMsg, 2);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		break;
	case ISSI_ID_BYTE0:
		/*
		 * Read Status register
		 */
		ReadStatusCmd = READ_STATUS_CMD;
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
		if (QspiPsuPtr->Config.ConnectionMode ==
						XQSPIPSU_CONNECTION_MODE_PARALLEL) {
				FlashMsg[1].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
		}
		Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 2);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		if (QspiPsuPtr->Config.ConnectionMode ==
					XQSPIPSU_CONNECTION_MODE_PARALLEL) {
			if (FSRFlag) {
				FlashStatus[1] &= FlashStatus[0];
			}else {
				FlashStatus[1] |= FlashStatus[0];
			}
		}
		/*
		 * Set Quad Enable Bit in the buffer
		 */
		StatusRegVal = FlashStatus[1];
		StatusRegVal |= 0x1 << QUAD_MODE_ENABLE_BIT;

		/*
		 * Write Enable
		 */
		WriteEnableCmd = WRITE_ENABLE_CMD;
		FlashMsg[0].TxBfrPtr = &WriteEnableCmd;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;
		Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		/*
		 * Write Status register
		 */
		WriteBuffer[COMMAND_OFFSET] = WRITE_STATUS_CMD;
		FlashMsg[0].TxBfrPtr = WriteBuffer;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		FlashMsg[1].TxBfrPtr = &StatusRegVal;
		FlashMsg[1].RxBfrPtr = NULL;
		FlashMsg[1].ByteCount = 1;
		FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[1].Flags = XQSPIPSU_MSG_FLAG_TX;
		if (QspiPsuPtr->Config.ConnectionMode ==
				XQSPIPSU_CONNECTION_MODE_PARALLEL) {
			FlashMsg[1].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
		}
		Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 2);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		/*
		 * Write Disable
		 */
		WriteEnableCmd = WRITE_DISABLE_CMD;
		FlashMsg[0].TxBfrPtr = &WriteEnableCmd;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;
		Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 2);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		break;

	case WINBOND_ID_BYTE0:
		ReadStatusCmd = READ_STATUS_REG_2_CMD;
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

		Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 2);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}


		if (QspiPsuPtr->Config.ConnectionMode ==
			XQSPIPSU_CONNECTION_MODE_PARALLEL) {
			if (FSRFlag) {
				FlashStatus[1] &= FlashStatus[0];
			} else {
				FlashStatus[1] |= FlashStatus[0];
			}
		}
		/*
		 * Set Quad Enable Bit in the buffer
		 */
		StatusRegVal = FlashStatus[1];

		StatusRegVal |= 0x1 << WB_QUAD_MODE_ENABLE_BIT;

		/*
		 * Write Enable
		 */
		WriteEnableCmd = WRITE_ENABLE_CMD;
		FlashMsg[0].TxBfrPtr = &WriteEnableCmd;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/*
		 * Write Status register
		 */
		WriteBuffer[COMMAND_OFFSET] = WRITE_STATUS_REG_2_CMD;
		FlashMsg[0].TxBfrPtr = WriteBuffer;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		FlashMsg[1].TxBfrPtr = &StatusRegVal;
		FlashMsg[1].RxBfrPtr = NULL;
		FlashMsg[1].ByteCount = 1;
		FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[1].Flags = XQSPIPSU_MSG_FLAG_TX;

		Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 2);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		DelayCount = 0;
		while(1) {
			if (DelayCount == MAX_DELAY_CNT) {
				Status = XST_FAILURE;
				goto ERROR_PATH;
			} else {
				ReadStatusCmd = READ_STATUS_CMD;
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

				Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 2);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}

				if (QspiPsuPtr->Config.ConnectionMode ==
					XQSPIPSU_CONNECTION_MODE_PARALLEL) {
					if (FSRFlag) {
						FlashStatus[1] &= FlashStatus[0];
					} else {
						FlashStatus[1] |= FlashStatus[0];
					}
				}
				if ((FlashStatus[1] & 0x01) == 0x00) {
					break;
				}

				/* Wait for 1 usec */
				usleep(1);
				DelayCount++;
			}
		}
		/*
		 * Write Disable
		 */
		WriteEnableCmd = WRITE_DISABLE_CMD;
		FlashMsg[0].TxBfrPtr = &WriteEnableCmd;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;
		Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 2);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		break;

	default:
		/*
		 * Currently only S25FL-L series requires the
		 * Quad enable bit to be set to 1.
		 */
		Status = XST_SUCCESS;
		break;
	}

ERROR_PATH:
	return Status;
}
