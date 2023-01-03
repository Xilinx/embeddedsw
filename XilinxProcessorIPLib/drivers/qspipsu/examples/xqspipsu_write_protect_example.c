/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xqspipsu_write_protect_example.c
*
*
* This file contains a design example using the QSPIPSU driver (XQspiPsu)
* with a serial Flash device greater than or equal to 128Mb.
* The example writes to flash and reads it back in DMA mode.
* This examples runs with GENFIFO Manual start. It runs in interrupt mode.
* This example works only with single mode and x1 or x2 data mode.
* This example will not work with x4 data mode and dual parallel or stacked
* configuration. The hardware which this example runs on, must have a serial
* Flash (Micron N25Q or MT25Q) for it to run. In order to test in single flash
* configurations the necessary HW must be present
* and QSPI_MODE (also reflected in ConnectionMode in the instance) has
* to be in sync with HW flash configuration being tested.
*
* This example neither set the required MIO to control the Write Protect
* pin nor the driver is setting the MIO. The MIO must be configured to be
* used by QSPI.This example has been tested with the Micron Serial
* Flash (N25Q512) and ISSI Serial Flash parts of IS25WP and IS25LP series
* flashes in single mode using A53 and R5 processors.
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
* 1.7	tjs 01/25/18 Added support to toggle the WP pin of flash. (PR#2448)
* 1.7	tjs 26/03/18 In dual parallel mode enable both CS when issuing Write
*		     		 enable command. CR-998478
* 1.8	tjs 05/02/18 Added support for IS25LP064 and IS25WP064.
* 1.8	tjs 16/07/18 Added support for the low density ISSI flash parts.
* 1.9	akm 02/27/19 Added support for IS25LP128, IS25WP128, IS25LP256,
*  		     IS25WP256, IS25LP512, IS25WP512 Flash Devices
* 1.9   akm 04/03/19 Fixed data alignment warnings on IAR compiler.
* 1.13  akm 11/30/20 Removed unwanted header files.
* 1.15  akm 11/19/21 Fix read/write failures on Spansion flash parts.
* 1.15  akm 12/22/21 Initialize variables before use.
* 1.16  akm 08/16/22 Fix logical error in NumSect calculation.
* 1.17  akm 12/16/22 Add timeout in QSPIPSU driver examples.
*
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xqspipsu_flash_config.h"
#include "xscugic.h"		/* Interrupt controller device driver */
#include "xil_printf.h"
#include "xil_util.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define QSPIPSU_DEVICE_ID	XPAR_XQSPIPSU_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define QSPIPSU_INTR_ID		XPAR_XQSPIPS_0_INTR

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


#define UNIQUE_VALUE		0x07

#define ENTER_4B	1
#define EXIT_4B		0

/*
 * Values of GQSPI GPIO to toggle the WP Pin
 */
#define ENABLE		0
#define DISABLE		1

/**************************** Type Definitions *******************************/

u8 ReadCmd;
u8 WriteCmd;
u8 StatusCmd;
u8 SectorEraseCmd;
u8 FSRFlag;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int QspiPsuWriteProtectFlashExample(XScuGic *IntcInstancePtr, XQspiPsu *QspiPsuInstancePtr,
				u16 QspiPsuDeviceId, u16 QspiPsuIntrId);
int FlashReadID(XQspiPsu *QspiPsuPtr);
int FlashErase(XQspiPsu *QspiPsuPtr, u32 Address, u32 ByteCount, u8 *WriteBfrPtr);
int FlashWrite(XQspiPsu *QspiPsuPtr, u32 Address, u32 ByteCount, u8 Command,
				u8 *WriteBfrPtr);
int FlashRead(XQspiPsu *QspiPsuPtr, u32 Address, u32 ByteCount, u8 Command,
				u8 *WriteBfrPtr, u8 *ReadBfrPtr);
u32 GetRealAddr(XQspiPsu *QspiPsuPtr, u32 Address);
int BulkErase(XQspiPsu *QspiPsuPtr, u8 *WriteBfrPtr);
int DieErase(XQspiPsu *QspiPsuPtr, u8 *WriteBfrPtr);
static int QspiPsuSetupIntrSystem(XScuGic *IntcInstancePtr,
			       XQspiPsu *QspiPsuInstancePtr, u16 QspiPsuIntrId);
static void QspiPsuDisableIntrSystem(XScuGic *IntcInstancePtr, u16 QspiPsuIntrId);
void QspiPsuHandler(void *CallBackRef, u32 StatusEvent, unsigned int ByteCount);
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
static XScuGic IntcInstance;
static XQspiPsu QspiPsuInstance;

static XQspiPsu_Msg FlashMsg[5];

/*
 * The following variables are shared between non-interrupt processing and
 * interrupt processing such that they must be global.
 */
volatile int TransferDone;

/*
 * The following variable tracks any errors that occur during interrupt
 * processing
 */
int Error;

/*
 * The following variable allows a test value to be added to the values that
 * are written to the Flash such that unique values can be generated to
 * guarantee the writes to the Flash were successful
 */
int Test = 1;

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
 * Main function to call the QSPIPSU Flash example.
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

	xil_printf("QSPIPSU Write Protect Example Test \r\n");

	/*
	 * Run the QspiPsu Write Protect example.
	 */
	Status = QspiPsuWriteProtectFlashExample(&IntcInstance,
			&QspiPsuInstance,
			QSPIPSU_DEVICE_ID, QSPIPSU_INTR_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("QSPIPSU Write Protect Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran QSPIPSU Write Protect Example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * The purpose of this function is to illustrate how to use the XQspiPsu
 * device driver in single mode with x1 or x2 data mode using
 * flash devices greater than or equal to 128Mb and drive the Write
 * Protect pin of the flash device using GQSPI GPIO.
 *
 * @param	IntcInstancePtr is a pointer to the instance of the Intc device.
 * @param	QspiPsuInstancePtr is a pointer to the instance of the QspiPsu
 *		device.
 * @param	QspiPsuDeviceId is the Device ID of the Qspi Device and is the
 *		XPAR_<QSPI_instance>_DEVICE_ID value from xparameters.h.
 * @param	QspiPsuIntrId is the interrupt Id for an QSPIPSU device.
 *
 * @return	XST_SUCCESS if successful, else XST_FAILURE.
 *
 * @note	None.
 *
 *****************************************************************************/
int QspiPsuWriteProtectFlashExample(XScuGic *IntcInstancePtr,
		XQspiPsu *QspiPsuInstancePtr,
		u16 QspiPsuDeviceId, u16 QspiPsuIntrId)
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

	Status = XQspiPsu_CfgInitialize(QspiPsuInstancePtr, QspiPsuConfig,
			QspiPsuConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	xil_printf("Cfg Init done, Baseaddress: 0x%x\n\r",
			QspiPsuInstancePtr->Config.BaseAddress);

	/*
	 * Connect the QspiPsu device to the interrupt subsystem such that
	 * interrupts can occur. This function is application specific
	 */
	Status = QspiPsuSetupIntrSystem(IntcInstancePtr, QspiPsuInstancePtr,
				     QspiPsuIntrId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the handler for the QSPIPSU that will be called from the
	 * interrupt context when an QSPIPSU status occurs, specify a pointer to
	 * the QSPIPSU driver instance as the callback reference so the handler is
	 * able to access the instance data
	 */
	XQspiPsu_SetStatusHandler(QspiPsuInstancePtr, QspiPsuInstancePtr,
				 (XQspiPsu_StatusHandler) QspiPsuHandler);

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
	 * Initial setup for Write Protect if user wishes to use.
	 */
	XQspiPsu_SetWP(QspiPsuInstancePtr, XQSPIPSU_SET_WP);
	XQspiPsu_WriteProtectToggle(QspiPsuInstancePtr,ENABLE);

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
	 * Address size and read command selection
	 * Micron flash on REMUS doesn't support these 4B write/erase commands
	 */
	ReadCmd = FAST_READ_CMD;
	WriteCmd = WRITE_CMD;
	SectorEraseCmd = SEC_ERASE_CMD;

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
		if(Status != XST_SUCCESS) {
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

	XQspiPsu_WriteProtectToggle(QspiPsuInstancePtr,ENABLE);

	Status = FlashErase(QspiPsuInstancePtr, TEST_ADDRESS, MaxData, CmdBfr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	for (Page = 0; Page < PAGE_COUNT; Page++) {
		Status = FlashWrite(QspiPsuInstancePtr,
				(Page * PageSize) +	TEST_ADDRESS,
				PageSize, WriteCmd, WriteBuffer);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	XQspiPsu_WriteProtectToggle(QspiPsuInstancePtr,DISABLE);

	Status = FlashRead(QspiPsuInstancePtr, TEST_ADDRESS, MaxData, ReadCmd,
				CmdBfr, ReadBuffer);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
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
		if(Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	QspiPsuDisableIntrSystem(IntcInstancePtr, QspiPsuIntrId);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * Callback handler.
 *
 * @param	CallBackRef is the upper layer callback reference passed back
 *		when the callback function is invoked.
 * @param	StatusEvent is the event that just occurred.
 * @param	ByteCount is the number of bytes transferred up until the event
 *		occurred.
 *
 * @return	None
 *
 * @note	None.
 *
 *****************************************************************************/
void QspiPsuHandler(void *CallBackRef, u32 StatusEvent, unsigned int ByteCount)
{
	/*
	 * Indicate the transfer on the QSPIPSU bus is no longer in progress
	 * regardless of the status event
	 */
	TransferDone = TRUE;

	/*
	 * If the event was not transfer done, then track it as an error
	 */
	if (StatusEvent != XST_SPI_TRANSFER_DONE) {
		Error++;
	}
}

/*****************************************************************************/
/**
 *
 * Reads the flash ID and identifies the flash in FCT table.
 *
 * @param	QspiPsuPtr is a pointer to the QSPIPSU driver component to use.
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

	TransferDone = FALSE;
	Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 2);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	if (Xil_WaitForEventSet(MAX_DELAY_CNT, 1, (u32 *)&TransferDone) != XST_SUCCESS)
		return XST_FAILURE;

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
 * @param	WriteBfrPtr is pointer to the write buffer (which is to be transmitted)
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

	TransferDone = FALSE;

	Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	if (Xil_WaitForEventSet(MAX_DELAY_CNT, 1, (u32 *)&TransferDone) != XST_SUCCESS)
		return XST_FAILURE;

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

	TransferDone = FALSE;

	Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 2);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	if (Xil_WaitForEventSet(MAX_DELAY_CNT, 1, (u32 *)&TransferDone) != XST_SUCCESS)
		return XST_FAILURE;

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
			if(QspiPsuPtr->Config.ConnectionMode == XQSPIPSU_CONNECTION_MODE_PARALLEL){
				FlashMsg[1].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
			}

			TransferDone = FALSE;
			Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 2);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
			if (Xil_WaitForEventSet(MAX_DELAY_CNT, 1, (u32 *)&TransferDone) != XST_SUCCESS)
				return XST_FAILURE;

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
		 * written to, this needs to be sent as a separate transfer before
		 * the write
		 */
		FlashMsg[0].TxBfrPtr = &WriteEnableCmd;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		TransferDone = FALSE;
		Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		if (Xil_WaitForEventSet(MAX_DELAY_CNT, 1, (u32 *)&TransferDone) != XST_SUCCESS)
			return XST_FAILURE;

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

		TransferDone = FALSE;
		Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		if (Xil_WaitForEventSet(MAX_DELAY_CNT, 1, (u32 *)&TransferDone) != XST_SUCCESS)
			return XST_FAILURE;

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

				TransferDone = FALSE;
				Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 2);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}
				if (Xil_WaitForEventSet(MAX_DELAY_CNT, 1, (u32 *)&TransferDone) != XST_SUCCESS)
					return XST_FAILURE;

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
 * This function performs a read. Default setting is in DMA mode.
 *
 * @param	QspiPsuPtr is a pointer to the QSPIPSU driver component to use.
 * @param	Address contains the address of the first sector which needs to
 *			be erased.
 * @param	ByteCount contains the total size to be erased.
 * @param	Command is the command used to read data from the flash. Supports
 * 			normal, fast, dual and quad read commands.
 * @param	WriteBfrPtr is pointer to the write buffer which contains data to be transmitted
 * @param	ReadBfrPtr is pointer to the read buffer to which valid received data should be
 * 			written
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
	if ((Command == FAST_READ_CMD) || (Command == DUAL_READ_CMD) ||
	    (Command == QUAD_READ_CMD) || (Command == FAST_READ_CMD_4B) ||
	    (Command == DUAL_READ_CMD_4B) || (Command == QUAD_READ_CMD_4B)) {
		/* Update Dummy cycles as per flash specs for QUAD IO */

		/*
		 * It is recommended that Bus width value during dummy
		 * phase should be same as data phase
		 */
		if ((Command == FAST_READ_CMD) ||
				(Command == FAST_READ_CMD_4B)) {
			FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		}

		if ((Command == DUAL_READ_CMD) ||
				(Command == DUAL_READ_CMD_4B)) {
			FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_DUALSPI;
		}

		if ((Command == QUAD_READ_CMD) ||
				(Command == QUAD_READ_CMD_4B)) {
			FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_QUADSPI;
		}

		FlashMsg[1].TxBfrPtr = NULL;
		FlashMsg[1].RxBfrPtr = NULL;
		FlashMsg[1].ByteCount = DUMMY_CLOCKS;
		FlashMsg[1].Flags = 0;

		FlashMsgCnt++;
	}

	/* Dummy cycles need to be changed as per flash specs for QUAD IO */
	if ((Command == FAST_READ_CMD) || (Command == FAST_READ_CMD_4B)) {
		FlashMsg[FlashMsgCnt].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	}

	if ((Command == DUAL_READ_CMD) || (Command == DUAL_READ_CMD_4B)) {
		FlashMsg[FlashMsgCnt].BusWidth = XQSPIPSU_SELECT_MODE_DUALSPI;
	}

	if ((Command == QUAD_READ_CMD) || (Command == QUAD_READ_CMD_4B)) {
		FlashMsg[FlashMsgCnt].BusWidth = XQSPIPSU_SELECT_MODE_QUADSPI;
	}

	FlashMsg[FlashMsgCnt].TxBfrPtr = NULL;
	FlashMsg[FlashMsgCnt].RxBfrPtr = ReadBfrPtr;
	FlashMsg[FlashMsgCnt].ByteCount = ByteCount;
	FlashMsg[FlashMsgCnt].Flags = XQSPIPSU_MSG_FLAG_RX;

	if (QspiPsuPtr->Config.ConnectionMode ==
			XQSPIPSU_CONNECTION_MODE_PARALLEL) {
		FlashMsg[FlashMsgCnt].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
	}

	TransferDone = FALSE;
	Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, FlashMsgCnt+1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	if (Xil_WaitForEventSet(MAX_DELAY_CNT, 1, (u32 *)&TransferDone) != XST_SUCCESS)
		return XST_FAILURE;

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

	TransferDone = FALSE;
	Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	if (Xil_WaitForEventSet(MAX_DELAY_CNT, 1, (u32 *)&TransferDone) != XST_SUCCESS)
		return XST_FAILURE;

	WriteBfrPtr[COMMAND_OFFSET]   = BULK_ERASE_CMD;
	FlashMsg[0].TxBfrPtr = WriteBfrPtr;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].ByteCount = 1;
	FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

	TransferDone = FALSE;
	Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	if (Xil_WaitForEventSet(MAX_DELAY_CNT, 1, (u32 *)&TransferDone) != XST_SUCCESS)
		return XST_FAILURE;

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

			TransferDone = FALSE;
			Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 2);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
			if (Xil_WaitForEventSet(MAX_DELAY_CNT, 1, (u32 *)&TransferDone) != XST_SUCCESS)
				return XST_FAILURE;

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
	u32 DelayCount;

	WriteEnableCmd = WRITE_ENABLE_CMD;
	for (DieCnt = 0;
		DieCnt < Flash_Config_Table[FCTIndex].NumDie;
		DieCnt++) {
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

		TransferDone = FALSE;
		Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		if (Xil_WaitForEventSet(MAX_DELAY_CNT, 1, (u32 *)&TransferDone) != XST_SUCCESS)
			return XST_FAILURE;

		WriteBfrPtr[COMMAND_OFFSET]   = DIE_ERASE_CMD;
		/* Check these number of address bytes as per flash device */
		WriteBfrPtr[ADDRESS_1_OFFSET] = 0;
		WriteBfrPtr[ADDRESS_2_OFFSET] = 0;
		WriteBfrPtr[ADDRESS_3_OFFSET] = 0;

		FlashMsg[0].TxBfrPtr = WriteBfrPtr;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 4;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		TransferDone = FALSE;
		Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		if (Xil_WaitForEventSet(MAX_DELAY_CNT, 1, (u32 *)&TransferDone) != XST_SUCCESS)
			return XST_FAILURE;

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

				TransferDone = FALSE;
				Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 2);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}
				if (Xil_WaitForEventSet(MAX_DELAY_CNT, 1, (u32 *)&TransferDone) != XST_SUCCESS)
					return XST_FAILURE;

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
 *
 * This function setups the interrupt system for a QspiPsu device.
 *
 * @param	IntcInstancePtr is a pointer to the instance of the Intc device.
 * @param	QspiPsuInstancePtr is a pointer to the instance of the
 *		QspiPsu device.
 * @param	QspiPsuIntrId is the interrupt Id for an QSPIPSU device.
 *
 * @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
 *
 * @note	None.
 *
 ******************************************************************************/
static int QspiPsuSetupIntrSystem(XScuGic *IntcInstancePtr,
			       XQspiPsu *QspiPsuInstancePtr, u16 QspiPsuIntrId)
{
	int Status;

	XScuGic_Config *IntcConfig; /* Instance of the interrupt controller */

	Xil_ExceptionInit();

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (IntcConfig == NULL) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
					IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler)XScuGic_InterruptHandler,
				IntcInstancePtr);

	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, QspiPsuIntrId,
				(Xil_ExceptionHandler)XQspiPsu_InterruptHandler,
				(void *)QspiPsuInstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the interrupt for the QspiPsu device.
	 */
	XScuGic_Enable(IntcInstancePtr, QspiPsuIntrId);

	/*
	 * Enable interrupts in the Processor.
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This function disables the interrupts that occur for the QspiPsu device.
 *
 * @param	IntcInstancePtr is a pointer to the instance of the Intc device.
 * @param	QspiPsuInstancePtr is a pointer to the instance of the
 *		QspiPsu device.
 * @param	QspiPsuIntrId is the interrupt Id for an QSPIPSU device.
 *
 * @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
 *
 * @note	None.
 *
 ******************************************************************************/
static void QspiPsuDisableIntrSystem(XScuGic *IntcInstancePtr,
		u16 QspiPsuIntrId)
{
	/*
	 * Disable the interrupt for the QSPIPSU device.
	 */
	XScuGic_Disable(IntcInstancePtr, QspiPsuIntrId);

	/*
	 * Disconnect and disable the interrupt for the QspiPsu device.
	 */
	XScuGic_Disconnect(IntcInstancePtr, QspiPsuIntrId);
}

/*****************************************************************************/
/**
 * @brief
 * This API enters the flash device into 4 bytes addressing mode.
 * As per the Micron and ISSI spec, before issuing the command to enter into
 * 4 byte addr mode, a write enable command is issued. For Macronix and
 * Winbond flash parts write enable is not required.
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
int FlashEnterExit4BAddMode(XQspiPsu *QspiPsuPtr,unsigned int Enable)
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
		if(FlashMake == ISSI_ID_BYTE0)
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
		 * Send the write enable command to the Flash so that it can be
		 * written to, this needs to be sent as a separate
		 * transfer before the write
		 */
		FlashMsg[0].TxBfrPtr = &WriteEnableCmd;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		TransferDone = FALSE;
		Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		if (Xil_WaitForEventSet(MAX_DELAY_CNT, 1, (u32 *)&TransferDone) != XST_SUCCESS)
			return XST_FAILURE;

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
		TransferDone = FALSE;
		Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 2);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		if (Xil_WaitForEventSet(MAX_DELAY_CNT, 1, (u32 *)&TransferDone) != XST_SUCCESS)
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

		TransferDone = FALSE;
		Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 2);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		if (Xil_WaitForEventSet(MAX_DELAY_CNT, 1, (u32 *)&TransferDone) != XST_SUCCESS)
			return XST_FAILURE;
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

		TransferDone = FALSE;
		Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 2);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		if (Xil_WaitForEventSet(MAX_DELAY_CNT, 1, (u32 *)&TransferDone) != XST_SUCCESS)
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

	TransferDone = FALSE;
	Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	if (Xil_WaitForEventSet(MAX_DELAY_CNT, 1, (u32 *)&TransferDone) != XST_SUCCESS)
		return XST_FAILURE;

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

			TransferDone = FALSE;
			Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 2);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
			if (Xil_WaitForEventSet(MAX_DELAY_CNT, 1, (u32 *)&TransferDone) != XST_SUCCESS)
				return XST_FAILURE;

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
		 * Send the write enable command to the Flash so that it can be
		 * written to, this needs to be sent as a separate
		 * transfer before the write
		 */
		FlashMsg[0].TxBfrPtr = &WriteDisableCmd;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		TransferDone = FALSE;
		Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		if (Xil_WaitForEventSet(MAX_DELAY_CNT, 1, (u32 *)&TransferDone) != XST_SUCCESS)
			return XST_FAILURE;

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

		TransferDone = FALSE;
		Status = XQspiPsu_InterruptTransfer(QspiPsuPtr,
				FlashMsg, 2);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		if (Xil_WaitForEventSet(MAX_DELAY_CNT, 1, (u32 *)&TransferDone) != XST_SUCCESS)
			return XST_FAILURE;

		WriteEnableCmd = WRITE_ENABLE_CMD;
		/*
		 * Send the write enable command to the Flash
		 * so that it can be written to, this needs
		 * to be sent as a separate transfer before
		 * the write
		 */
		FlashMsg[0].TxBfrPtr = &WriteEnableCmd;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		TransferDone = FALSE;
		Status = XQspiPsu_InterruptTransfer(QspiPsuPtr,
				FlashMsg, 1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		if (Xil_WaitForEventSet(MAX_DELAY_CNT, 1, (u32 *)&TransferDone) != XST_SUCCESS)
			return XST_FAILURE;

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

		TransferDone = FALSE;
		Status = XQspiPsu_InterruptTransfer(QspiPsuPtr,
				FlashMsg, 2);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		if (Xil_WaitForEventSet(MAX_DELAY_CNT, 1, (u32 *)&TransferDone) != XST_SUCCESS)
			return XST_FAILURE;

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

				TransferDone = FALSE;
				Status = XQspiPsu_InterruptTransfer(QspiPsuPtr,
						FlashMsg, 2);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}
				if (Xil_WaitForEventSet(MAX_DELAY_CNT, 1, (u32 *)&TransferDone) != XST_SUCCESS)
					return XST_FAILURE;
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

		TransferDone = FALSE;
		Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 2);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		if (Xil_WaitForEventSet(MAX_DELAY_CNT, 1, (u32 *)&TransferDone) != XST_SUCCESS)
			return XST_FAILURE;
		break;
	case ISSI_ID_BYTE0:
		/*
		 * Read Status Register to a buffer
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
		TransferDone = FALSE;
		Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 2);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		if (Xil_WaitForEventSet(MAX_DELAY_CNT, 1, (u32 *)&TransferDone) != XST_SUCCESS)
			return XST_FAILURE;
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
		StatusRegVal |= 0x1 << QUAD_MODE_ENABLE_BIT;

		/*
		 * Write enable
		 */
		WriteEnableCmd = WRITE_ENABLE_CMD;
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
		TransferDone = FALSE;
		Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		if (Xil_WaitForEventSet(MAX_DELAY_CNT, 1, (u32 *)&TransferDone) != XST_SUCCESS)
			return XST_FAILURE;

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
		TransferDone = FALSE;
		Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 2);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		if (Xil_WaitForEventSet(MAX_DELAY_CNT, 1, (u32 *)&TransferDone) != XST_SUCCESS)
			return XST_FAILURE;

		/*
		 * Write Disable
		 */
		WriteEnableCmd = WRITE_DISABLE_CMD;
		FlashMsg[0].TxBfrPtr = &WriteEnableCmd;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;
		TransferDone = FALSE;
		Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		if (Xil_WaitForEventSet(MAX_DELAY_CNT, 1, (u32 *)&TransferDone) != XST_SUCCESS)
			return XST_FAILURE;

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
