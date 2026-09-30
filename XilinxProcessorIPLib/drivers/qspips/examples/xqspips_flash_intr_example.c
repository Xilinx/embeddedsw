/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xqspips_flash_intr_example.c
* @brief Implements a design example using the QSPI driver (XQspiPs) in
* interrupt mode with a serial FLASH device.
*
* The example performs some transfers in Manual Chip Select and Start mode.
* It is recommended to use Manual CS + Auto start for best performance.
* The hardware which this example runs on, must have a serial FLASH (Numonyx
* N25Q, Winbond W25Q, Spansion S25FL, ISSI IS25WP) for it to run. The example
* has been tested with the Numonyx Serial Flash (N25Q128) and IS25WP series
* flash parts.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.00  sdm 11/25/10 First release
* 1.01  srt 06/12/12 Changed to meet frequency requirements of READ command
*		     for CR 663787
* 2.00a	kka 22/08/12 Updated the example as XQspiPs_Transfer API has
*		     changed. Changed the prescalar to use divide by 8.
*		     The user can change the prescalar to a maximum of
*		     divide by 2 based on the reference clock in the
*		     system.
* 	 	     Set the Holdb_dr bit in the configuration register using
*		     XQSPIPS_HOLD_B_DRIVE_OPTION. Setting this bit
*		     drives the HOLD bit of the QSPI controller.
*		     This is required for QSPI to be used in Non QSPI boot
*		     mode else there needs to be an external pullup on this
*		     line. See http://www.xilinx.com/support/answers/47596.htm
* 2.01a sg  02/03/13 Created a function FlashReadID. Removed multiple
*		     initialization using SetOptions.
*       ms  04/05/17 Modified Comment lines in functions to
*                    recognize it as documentation block for doxygen
*                    generation.
* 3.5	tjs 07/16/18 Added support for low density ISSI flash parts.
*		     Added FlashQuadEnable API to enable quad mode in flash.
* 3.6   akm 04/15/19 Modified FlashQuadEnable, FlashWrie and FlashErase APIs,
*                    to wait for the on going operation to complete before
*                    performing the next operation.
* 3.10  akm 08/17/22 Fix logical error in NumSect calculation.
* 3.11  akm 07/10/23 Add support for system device-tree flow for example.
* 3.14  sb  03/25/25 Fixed gcc and g++ warnings.
* 3.15  vlt 12/16/25 Update Doxygen comments to include SDT flow details.
* 3.16  vn  08/26/26 Fix Doxygen warnings
* 3.16  tb  09/30/26 Align Doxygen comments with AMD documentation standards
* </pre>
*
******************************************************************************/

/** @addtogroup qspips_flash_intr_example
 * @{
 */

/***************************** Include Files *********************************/

#ifndef SDT
#include "xparameters.h"	/* SDK generated parameters */
#endif
#include "xqspips.h"		/* QSPI device driver */
#ifndef SDT
#include "xscugic.h"		/* Interrupt controller device driver */
#endif
#include "xil_exception.h"
#include "xil_printf.h"
#ifdef SDT
#include "xinterrupt_wrap.h"
#endif

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define QSPI_DEVICE_ID		XPAR_XQSPIPS_0_DEVICE_ID /**< QSPI device ID. */
/** Interrupt controller device ID. */
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define QSPI_INTR_ID		XPAR_XQSPIPS_0_INTR /**< QSPI interrupt ID. */
#endif

/*
 * The following constants define the commands which may be sent to the FLASH
 * device.
 */
#define WRITE_STATUS_CMD	0x01	/**< Write status register command. */
#define WRITE_CMD		0x02	/**< Write (page program) command. */
#define READ_CMD		0x03	/**< Normal read command. */
#define WRITE_DISABLE_CMD	0x04	/**< Write disable command. */
#define READ_STATUS_CMD		0x05	/**< Read status register command. */
#define WRITE_ENABLE_CMD	0x06	/**< Write enable command. */
#define FAST_READ_CMD		0x0B	/**< Fast read command. */
#define DUAL_READ_CMD		0x3B	/**< Dual read command. */
#define QUAD_READ_CMD		0x6B	/**< Quad read command. */
#define BULK_ERASE_CMD		0xC7	/**< Bulk erase command. */
#define	SEC_ERASE_CMD		0xD8	/**< Sector erase command. */
#define READ_ID			0x9F	/**< Read ID command. */

/*
 * The following constants define the offsets within a FlashBuffer data
 * type for each kind of data.  Note that the read data offset is not the
 * same as the write data because the QSPI driver is designed to allow full
 * duplex transfers such that the number of bytes received is the number
 * sent and received.
 */
#define COMMAND_OFFSET		0 /**< FLASH instruction. */
#define ADDRESS_1_OFFSET	1 /**< MSB byte of address to read or write. */
/** Middle byte of address to read or write. */
#define ADDRESS_2_OFFSET	2
#define ADDRESS_3_OFFSET	3 /**< LSB byte of address to read or write. */
#define DATA_OFFSET		4 /**< Start of Data for Read/Write. */
#define DUMMY_OFFSET		4 /**< Dummy byte offset for fast, dual and quad
				     reads. */
#define DUMMY_SIZE		1 /**< Number of dummy bytes for fast, dual and
				     quad reads. */
#define RD_ID_SIZE		4 /**< Read ID command + 3 bytes ID response. */
#define BULK_ERASE_SIZE		1 /**< Bulk Erase command size. */
#define SEC_ERASE_SIZE		4 /**< Sector Erase command + Sector address. */

/*
 * The following constants specify the extra bytes which are sent to the
 * FLASH on the QSPI interface, that are not data, but control information
 * which includes the command and address
 */
#define OVERHEAD_SIZE		4	/**< Command and address byte count. */

/*
 * The following constants specify the page size, sector size, and number of
 * pages and sectors for the FLASH.  The page size specifies a max number of
 * bytes that can be written to the FLASH with a single transfer.
 */
#define SECTOR_SIZE		0x10000	/**< Sector size in bytes. */
#define NUM_SECTORS		0x100	/**< Number of sectors. */
#define NUM_PAGES		0x10000	/**< Number of pages. */
#define PAGE_SIZE		256	/**< Page size in bytes. */

/** Number of flash pages to be written. */
#define PAGE_COUNT		16

/** Flash address to which data is to be written. */
#define TEST_ADDRESS		0x00090000
#define UNIQUE_VALUE		0x05	/**< Start value of the data pattern. */
/*
 * The following constants specify the max amount of data and the size of the
 * the buffer required to hold the data and overhead to transfer the data to
 * and from the FLASH.
 */
/** Maximum amount of data transferred to and from the FLASH. */
#define MAX_DATA		(PAGE_COUNT * PAGE_SIZE)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

#ifndef SDT
static int QspiSetupIntrSystem(XScuGic *IntcInstancePtr,
			       XQspiPs *QspiInstancePtr, u16 QspiIntrId);

static void QspiDisableIntrSystem(XScuGic *IntcInstancePtr, u16 QspiIntrId);
#endif

void QspiHandler(void *CallBackRef, u32 StatusEvent, unsigned int ByteCount);

void FlashErase(XQspiPs *QspiPtr, u32 Address, u32 ByteCount);

void FlashWrite(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command);

void FlashRead(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command);

int FlashReadID(void);

void FlashQuadEnable(XQspiPs *QspiPtr);

#ifndef SDT
int QspiFlashIntrExample(XScuGic *IntcInstancePtr, XQspiPs *QspiInstancePtr,
			 u16 QspiDeviceId, u16 QspiIntrId);
#else
int QspiFlashIntrExample(XQspiPs *QspiInstancePtr, UINTPTR BaseAddress);
#endif

/************************** Variable Definitions *****************************/

/*
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs. They could be local
 * but should at least be static so they are zeroed.
 */
#ifndef SDT
static XScuGic IntcInstance;	/**< Interrupt controller instance. */
#endif
static XQspiPs QspiInstance;	/**< QSPI driver instance. */

/**
 * The following variables are shared between non-interrupt processing and
 * interrupt processing such that they must be global.
 */
volatile int TransferInProgress;

/**
 * The following variable tracks any errors that occur during interrupt
 * processing
 */
int Error;

/**
 * The following variable allows a test value to be added to the values that
 * are written to the FLASH such that unique values can be generated to
 * guarantee the writes to the FLASH were successful
 */
int Test = 0xF;

/*
 * The following variables are used to read and write to the flash and they
 * are global to avoid having large buffers on the stack
 */
u8 ReadBuffer[MAX_DATA + DATA_OFFSET + DUMMY_SIZE];	/**< Read buffer. */
u8 WriteBuffer[PAGE_SIZE + DATA_OFFSET];	/**< Write buffer. */

/*****************************************************************************/
/**
*
* @brief Runs the QSPI Flash interrupt example.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
******************************************************************************/
int main(void)
{
	int Status;

	xil_printf("QSPI FLASH Interrupt Example Test \r\n");

	/* Run the Qspi Interrupt example.*/
#ifndef SDT
	Status = QspiFlashIntrExample(&IntcInstance, &QspiInstance,
				      QSPI_DEVICE_ID, QSPI_INTR_ID);
#else
	Status = QspiFlashIntrExample(&QspiInstance,XPAR_XQSPIPS_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("QSPI FLASH Interrupt Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran QSPI FLASH Interrupt Example Test\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Illustrates how to use the XQspiPs device driver in interrupt mode.
*
* The function writes and reads data from a serial FLASH.
*
* @param[in]	QspiInstancePtr Pointer to the QSPIPS driver to use.
* @if SDT
* @param[in]	BaseAddress The base address of the device
* @else
* @param[in]        IntcInstancePtr Pointer to the instance of the INTC
*		component.
* @param[in]	QspiDeviceId The XPAR_<QSPIPS_instance>_DEVICE_ID value
*		from xparameters.h.
* @param[in]	QspiIntrId The QSPIPS Interrupt Id.
* @endif
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note	        In XSCT/classic flow, the device configuration is looked up
*               based on DeviceId.
*               The function calls other functions which contain loops that
*               may be infinite if interrupts are not working such that it may
*               not return. If the device slave select is not correct and the
*               device is not responding on bus it reads a status of 0xFF for
*               the status register as the bus is pulled up.
*
*****************************************************************************/
#ifndef SDT
int QspiFlashIntrExample(XScuGic *IntcInstancePtr, XQspiPs *QspiInstancePtr,
			 u16 QspiDeviceId, u16 QspiIntrId)
#else
int QspiFlashIntrExample(XQspiPs *QspiInstancePtr, UINTPTR BaseAddress)
#endif
{
	int Status;
	u8 *BufferPtr;
	u8 UniqueValue;
	int Count;
	int Page;
	XQspiPs_Config *QspiConfig;

	/* Initialize the QSPI driver so that it's ready to use*/
#ifndef SDT
	QspiConfig = XQspiPs_LookupConfig(QspiDeviceId);
#else
	QspiConfig = XQspiPs_LookupConfig(BaseAddress);
#endif
	if (QspiConfig == NULL) {
		return XST_FAILURE;
	}

	Status = XQspiPs_CfgInitialize(QspiInstancePtr, QspiConfig,
					QspiConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Perform a self-test to check hardware build*/
	Status = XQspiPs_SelfTest(QspiInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the Qspi device to the interrupt subsystem such that
	 * interrupts can occur. This function is application specific
	 */
#ifndef SDT
	Status = QspiSetupIntrSystem(IntcInstancePtr, QspiInstancePtr,
				     QspiIntrId);
#else
	Status = XSetupInterruptSystem(QspiInstancePtr, &XQspiPs_InterruptHandler,
				       QspiConfig->IntrId,
				       QspiConfig->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the handler for the QSPI that will be called from the
	 * interrupt context when an QSPI status occurs, specify a pointer to
	 * the QSPI driver instance as the callback reference so the handler is
	 * able to access the instance data
	 */
	XQspiPs_SetStatusHandler(QspiInstancePtr, QspiInstancePtr,
				 (XQspiPs_StatusHandler) QspiHandler);

	/*
	 * Set Manual Start and Manual Chip select options and drive the
	 * HOLD_B high.
	 */
	XQspiPs_SetOptions(QspiInstancePtr, XQSPIPS_FORCE_SSELECT_OPTION |
				XQSPIPS_MANUAL_START_OPTION |
				XQSPIPS_HOLD_B_DRIVE_OPTION);

	/* Set the operating clock frequency using the clock divider*/
	XQspiPs_SetClkPrescaler(QspiInstancePtr, XQSPIPS_CLK_PRESCALE_8);

	/* Assert the FLASH chip select*/
	XQspiPs_SetSlaveSelect(QspiInstancePtr);

	/*
	 * Initialize the write buffer for a pattern to write to the FLASH
	 * and the read buffer to zero so it can be verified after the read, the
	 * test value that is added to the unique value allows the value to be
	 * changed in a debug environment to guarantee
	 */
	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < PAGE_SIZE;
	     Count++, UniqueValue++) {
		WriteBuffer[DATA_OFFSET + Count] = (u8)(UniqueValue + Test);
	}

	memset(ReadBuffer, 0x00, sizeof(ReadBuffer));

	FlashReadID();

	FlashQuadEnable(QspiInstancePtr);

	/* Erase the flash.*/
	FlashErase(QspiInstancePtr, TEST_ADDRESS, MAX_DATA);

	/*
	 * Write the data in the write buffer to the serial FLASH a page at a
	 * time, starting from TEST_ADDRESS
	 */
	for (Page = 0; Page < PAGE_COUNT; Page++) {
		FlashWrite(QspiInstancePtr, (Page * PAGE_SIZE) + TEST_ADDRESS,
			   PAGE_SIZE, WRITE_CMD);
	}

	/*
	 * Read the contents of the FLASH from TEST_ADDRESS, using Normal Read
	 * command.
	 */
	FlashRead(QspiInstancePtr, TEST_ADDRESS, MAX_DATA, READ_CMD);

	/*
	 * Setup a pointer to the start of the data that was read into the read
	 * buffer and verify the data read is the data that was written
	 */
	BufferPtr = &ReadBuffer[DATA_OFFSET];

	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < MAX_DATA;
	     Count++, UniqueValue++) {
		if (BufferPtr[Count] != (u8)(UniqueValue + Test)) {
			return XST_FAILURE;
		}
	}

	/*
	 * Read the contents of the FLASH from TEST_ADDRESS, using Fast Read
	 * command
	 */
	memset(ReadBuffer, 0x00, sizeof(ReadBuffer));
	FlashRead(QspiInstancePtr, TEST_ADDRESS, MAX_DATA, FAST_READ_CMD);

	/*
	 * Setup a pointer to the start of the data that was read into the read
	 * buffer and verify the data read is the data that was written
	 */
	BufferPtr = &ReadBuffer[DATA_OFFSET + DUMMY_SIZE];
	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < MAX_DATA;
	     Count++, UniqueValue++) {
		if (BufferPtr[Count] != (u8)(UniqueValue + Test)) {
			return XST_FAILURE;
		}
	}

	/*
	 * Read the contents of the FLASH from TEST_ADDRESS, using Dual Read
	 * command
	 */
	memset(ReadBuffer, 0x00, sizeof(ReadBuffer));
	FlashRead(QspiInstancePtr, TEST_ADDRESS, MAX_DATA, DUAL_READ_CMD);

	/*
	 * Setup a pointer to the start of the data that was read into the read
	 * buffer and verify the data read is the data that was written
	 */
	BufferPtr = &ReadBuffer[DATA_OFFSET + DUMMY_SIZE];

	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < MAX_DATA;
	     Count++, UniqueValue++) {
		if (BufferPtr[Count] != (u8)(UniqueValue + Test)) {
			return XST_FAILURE;
		}
	}

	/*
	 * Read the contents of the FLASH from TEST_ADDRESS, using Quad Read
	 * command
	 */
	memset(ReadBuffer, 0x00, sizeof(ReadBuffer));
	FlashRead(QspiInstancePtr, TEST_ADDRESS, MAX_DATA, QUAD_READ_CMD);

	/*
	 * Setup a pointer to the start of the data that was read into the read
	 * buffer and verify the data read is the data that was written
	 */
	BufferPtr = &ReadBuffer[DATA_OFFSET + DUMMY_SIZE];

	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < MAX_DATA;
	     Count++, UniqueValue++) {
		if (BufferPtr[Count] != (u8)(UniqueValue + Test)) {
			return XST_FAILURE;
		}
	}

	/*
	 * Set Auto Start and Manual Chip select options and drive the
	 * HOLD_B high.
	 */
	XQspiPs_SetOptions(QspiInstancePtr, XQSPIPS_FORCE_SSELECT_OPTION |
				XQSPIPS_HOLD_B_DRIVE_OPTION);

	/*
	 * Initialize the write buffer for a pattern to write to the FLASH
	 * and the read buffer to zero so it can be verified after the read, the
	 * test value that is added to the unique value allows the value to be
	 * changed in a debug environment to guarantee
	 */
	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < PAGE_SIZE;
	     Count++, UniqueValue++) {
		WriteBuffer[DATA_OFFSET + Count] = (u8)(UniqueValue + Test);
	}

	memset(ReadBuffer, 0x00, sizeof(ReadBuffer));

	/* Erase the flash.*/
	FlashErase(QspiInstancePtr, TEST_ADDRESS, MAX_DATA);

	/*
	 * Write the data in the write buffer to the serial FLASH a page at a
	 * time, starting from TEST_ADDRESS
	 */
	for (Page = 0; Page < PAGE_COUNT; Page++) {
		FlashWrite(QspiInstancePtr, (Page * PAGE_SIZE) + TEST_ADDRESS,
			   PAGE_SIZE, WRITE_CMD);
	}

	/*
	 * Read the contents of the FLASH from TEST_ADDRESS, using Normal Read
	 * command.
	 */
	FlashRead(QspiInstancePtr, TEST_ADDRESS, MAX_DATA, READ_CMD);

	/*
	 * Setup a pointer to the start of the data that was read into the read
	 * buffer and verify the data read is the data that was written
	 */
	BufferPtr = &ReadBuffer[DATA_OFFSET];

	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < MAX_DATA;
	     Count++, UniqueValue++) {
		if (BufferPtr[Count] != (u8)(UniqueValue + Test)) {
			return XST_FAILURE;
		}
	}

	/*
	 * Read the contents of the FLASH from TEST_ADDRESS, using Fast Read
	 * command
	 */
	memset(ReadBuffer, 0x00, sizeof(ReadBuffer));
	FlashRead(QspiInstancePtr, TEST_ADDRESS, MAX_DATA, FAST_READ_CMD);

	/*
	 * Setup a pointer to the start of the data that was read into the read
	 * buffer and verify the data read is the data that was written
	 */
	BufferPtr = &ReadBuffer[DATA_OFFSET + DUMMY_SIZE];
	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < MAX_DATA;
	     Count++, UniqueValue++) {
		if (BufferPtr[Count] != (u8)(UniqueValue + Test)) {
			return XST_FAILURE;
		}
	}

	/*
	 * Read the contents of the FLASH from TEST_ADDRESS, using Dual Read
	 * command
	 */
	memset(ReadBuffer, 0x00, sizeof(ReadBuffer));
	FlashRead(QspiInstancePtr, TEST_ADDRESS, MAX_DATA, DUAL_READ_CMD);

	/*
	 * Setup a pointer to the start of the data that was read into the read
	 * buffer and verify the data read is the data that was written
	 */
	BufferPtr = &ReadBuffer[DATA_OFFSET + DUMMY_SIZE];

	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < MAX_DATA;
	     Count++, UniqueValue++) {
		if (BufferPtr[Count] != (u8)(UniqueValue + Test)) {
			return XST_FAILURE;
		}
	}

	/*
	 * Read the contents of the FLASH from TEST_ADDRESS, using Quad Read
	 * command
	 */
	memset(ReadBuffer, 0x00, sizeof(ReadBuffer));
	FlashRead(QspiInstancePtr, TEST_ADDRESS, MAX_DATA, QUAD_READ_CMD);

	/*
	 * Setup a pointer to the start of the data that was read into the read
	 * buffer and verify the data read is the data that was written
	 */
	BufferPtr = &ReadBuffer[DATA_OFFSET + DUMMY_SIZE];

	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < MAX_DATA;
	     Count++, UniqueValue++) {
		if (BufferPtr[Count] != (u8)(UniqueValue + Test)) {
			return XST_FAILURE;
		}
	}

#ifndef SDT
	QspiDisableIntrSystem(IntcInstancePtr, QspiIntrId);
#else
	XDisconnectInterruptCntrl(QspiConfig->IntrId, QspiConfig->IntrParent);
#endif
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Handles the processing for the QSPI driver.
*
* It is called from an interrupt context such that the amount of processing
* performed should be minimized.  It is called when a transfer of QSPI data
* completes or an error occurs.
*
* This handler provides an example of how to handle QSPI interrupts but is
* application specific.
*
* @param[in]	CallBackRef A reference passed to the handler.
* @param[in]	StatusEvent The status of the QSPI.
* @param[in]	ByteCount The number of bytes transferred.
*
******************************************************************************/
void QspiHandler(void *CallBackRef, u32 StatusEvent, unsigned int ByteCount)
{
	(void)CallBackRef;
	(void)ByteCount;

	/*
	 * Indicate the transfer on the QSPI bus is no longer in progress
	 * regardless of the status event
	 */
	TransferInProgress = FALSE;

	/* If the event was not transfer done, then track it as an error*/
	if (StatusEvent != XST_SPI_TRANSFER_DONE) {
		Error++;
	}
}

/*****************************************************************************/
/**
*
* @brief Writes to the serial FLASH connected to the QSPI interface.
*
* All the data put into the buffer must be in the same page of the device with
* page boundaries being on 256 byte boundaries.
*
* @param[in,out]	QspiPtr Pointer to the QSPI driver component to use.
* @param[in]	Address The address to write data to in the FLASH.
* @param[in]	ByteCount The number of bytes to write.
* @param[in]	Command The command used to write data to the flash. QSPI
*		device supports only Page Program command to write data to the
*		flash.
*
******************************************************************************/
void FlashWrite(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command)
{
	u8 WriteEnableCmd = { WRITE_ENABLE_CMD };
	u8 ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* must send 2 bytes */
	u8 FlashStatus[2];

	/*
	 * Send the write enable command to the FLASH so that it can be
	 * written to, this needs to be sent as a separate transfer before
	 * the write
	 */
	TransferInProgress = TRUE;

	XQspiPs_Transfer(QspiPtr, &WriteEnableCmd, NULL,
			  sizeof(WriteEnableCmd));

	/*
	 * Wait for the transfer on the QSPI bus to be complete before
	 * proceeding
	 */
	while (TransferInProgress);

	/*
	 * Setup the write command with the specified address and data for the
	 * FLASH
	 */
	WriteBuffer[COMMAND_OFFSET]   = Command;
	WriteBuffer[ADDRESS_1_OFFSET] = (u8)((Address & 0xFF0000) >> 16);
	WriteBuffer[ADDRESS_2_OFFSET] = (u8)((Address & 0xFF00) >> 8);
	WriteBuffer[ADDRESS_3_OFFSET] = (u8)(Address & 0xFF);

	/*
	 * Send the write command, address, and data to the FLASH to be
	 * written, no receive buffer is specified since there is nothing to
	 * receive
	 */
	TransferInProgress = TRUE;
	XQspiPs_Transfer(QspiPtr, WriteBuffer, NULL,
			  ByteCount + OVERHEAD_SIZE);

	while (TransferInProgress);

	/*
	 * Wait for the write command to the FLASH to be completed, it takes
	 * some time for the data to be written
	 */
	while (1) {
		/*
		 * Poll the status register of the FLASH to determine when it
		 * completes, by sending a read status command and receiving the
		 * status byte
		 */
		TransferInProgress = TRUE;

		XQspiPs_Transfer(QspiPtr, ReadStatusCmd, FlashStatus,
				  sizeof(ReadStatusCmd));

		/*
		 * Wait for the transfer on the QSPI bus to be complete before
		 * proceeding
		 */
		while (TransferInProgress);

		/*
		 * If the status indicates the write is done, then stop waiting,
		 * if a value of 0xFF in the status byte is read from the
		 * device and this loop never exits, the device slave select is
		 * possibly incorrect such that the device status is not being
		 * read
		 */
		FlashStatus[1] |= FlashStatus[0];
		if ((FlashStatus[1] & 0x01) == 0) {
			break;
		}
	}
}

/*****************************************************************************/
/**
*
* @brief Reads from the serial FLASH connected to the QSPI interface.
*
* @param[in,out]	QspiPtr Pointer to the QSPI driver component to use.
* @param[in]	Address The address to read data from in the FLASH.
* @param[in]	ByteCount The number of bytes to read.
* @param[in]	Command The command used to read data from the flash. QSPI
*		device supports one of the Read, Fast Read, Dual Read and Fast
*		Read commands to read data from the flash.
*
******************************************************************************/
void FlashRead(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command)
{
	/*
	 * Setup the write command with the specified address and data for the
	 * FLASH
	 */
	WriteBuffer[COMMAND_OFFSET]   = Command;
	WriteBuffer[ADDRESS_1_OFFSET] = (u8)((Address & 0xFF0000) >> 16);
	WriteBuffer[ADDRESS_2_OFFSET] = (u8)((Address & 0xFF00) >> 8);
	WriteBuffer[ADDRESS_3_OFFSET] = (u8)(Address & 0xFF);

	if ((Command == FAST_READ_CMD) || (Command == DUAL_READ_CMD) ||
	    (Command == QUAD_READ_CMD)) {
		ByteCount += DUMMY_SIZE;
	}
	/*
	 * Send the read command to the FLASH to read the specified number
	 * of bytes from the FLASH, send the read command and address and
	 * receive the specified number of bytes of data in the data buffer
	 */
	TransferInProgress = TRUE;

	XQspiPs_Transfer(QspiPtr, WriteBuffer, ReadBuffer,
			  ByteCount + OVERHEAD_SIZE);

	/*
	 * Wait for the transfer on the QSPI bus to be complete before
	 * proceeding
	 */
	while (TransferInProgress);
}

/*****************************************************************************/
/**
*
* @brief Erases the sectors in the serial FLASH connected to the QSPI interface.
*
* @param[in,out]	QspiPtr Pointer to the QSPI driver component to use.
* @param[in]	Address The address of the first sector which needs to
*		be erased.
* @param[in]	ByteCount The total size to be erased.
*
******************************************************************************/
void FlashErase(XQspiPs *QspiPtr, u32 Address, u32 ByteCount)
{
	u8 WriteEnableCmd = { WRITE_ENABLE_CMD };
	u8 ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* must send 2 bytes */
	u8 FlashStatus[2];
	u32 Sector;
	u32 NumSect;

	/*
	 * If erase size is same as the total size of the flash, use bulk erase
	 * command
	 */
	if (ByteCount == (NUM_SECTORS * SECTOR_SIZE)) {
		/*
		 * Send the write enable command to the FLASH so that it can be
		 * written to, this needs to be sent as a separate transfer
		 * before the erase
		 */
		TransferInProgress = TRUE;

		XQspiPs_Transfer(QspiPtr, &WriteEnableCmd, NULL,
				  sizeof(WriteEnableCmd));

		/*
		 * Wait for the transfer on the QSPI bus to be complete before
		 * proceeding
		 */
		while (TransferInProgress);

		/* Setup the bulk erase command*/
		WriteBuffer[COMMAND_OFFSET] = BULK_ERASE_CMD;

		/*
		 * Send the bulk erase command; no receive buffer is specified
		 * since there is nothing to receive
		 */
		TransferInProgress = TRUE;
		XQspiPs_Transfer(QspiPtr, WriteBuffer, NULL,
				  BULK_ERASE_SIZE);

		while (TransferInProgress);

		/* Wait for the erase command to the FLASH to be completed*/
		while (1) {
			/*
			 * Poll the status register of the device to determine
			 * when it completes, by sending a read status command
			 * and receiving the status byte
			 */
			TransferInProgress = TRUE;

			XQspiPs_Transfer(QspiPtr, ReadStatusCmd, FlashStatus,
					  sizeof(ReadStatusCmd));

			/*
			 * Wait for the transfer on the QSPI bus to be complete
			 * before proceeding
			 */
			while (TransferInProgress);

			/*
			 * If the status indicates the write is done, then stop
			 * waiting; if a value of 0xFF in the status byte is
			 * read from the device and this loop never exits, the
			 * device slave select is possibly incorrect such that
			 * the device status is not being read
			 */
			FlashStatus[1] |= FlashStatus[0];
			if ((FlashStatus[1] & 0x01) == 0) {
				break;
			}
		}

		return;
	}

	/*
	 * Calculate no. of sectors to erase based on byte count
	 */
	if (ByteCount % SECTOR_SIZE)
		NumSect = ByteCount/SECTOR_SIZE + 1;
	else
		NumSect = ByteCount/SECTOR_SIZE;

	/*
	 * If the erase size is less than the total size of the flash, use
	 * sector erase command
	 */
	for (Sector = 0; Sector < NumSect; Sector++) {
		/*
		 * Send the write enable command to the SEEPOM so that it can be
		 * written to, this needs to be sent as a separate transfer
		 * before the write
		 */
		TransferInProgress = TRUE;

		XQspiPs_Transfer(QspiPtr, &WriteEnableCmd, NULL,
				  sizeof(WriteEnableCmd));

		/*
		 * Wait for the transfer on the QSPI bus to be complete before
		 * proceeding
		 */
		while (TransferInProgress);

		/*
		 * Setup the write command with the specified address and data
		 * for the FLASH
		 */
		WriteBuffer[COMMAND_OFFSET]   = SEC_ERASE_CMD;
		WriteBuffer[ADDRESS_1_OFFSET] = (u8)(Address >> 16);
		WriteBuffer[ADDRESS_2_OFFSET] = (u8)(Address >> 8);
		WriteBuffer[ADDRESS_3_OFFSET] = (u8)(Address & 0xFF);

		/*
		 * Send the sector erase command and address; no receive buffer
		 * is specified since there is nothing to receive
		 */
		TransferInProgress = TRUE;
		XQspiPs_Transfer(QspiPtr, WriteBuffer, NULL,
				  SEC_ERASE_SIZE);

		while (TransferInProgress);

		/*
		 * Wait for the sector erse command to the
		 * FLASH to be completed
		 */
		while (1) {
			/*
			 * Poll the status register of the device to determine
			 * when it completes, by sending a read status command
			 * and receiving the status byte
			 */
			TransferInProgress = TRUE;

			XQspiPs_Transfer(QspiPtr, ReadStatusCmd, FlashStatus,
					  sizeof(ReadStatusCmd));

			/*
			 * Wait for the transfer on the QSPI bus to be complete
			 * before proceeding
			 */
			while (TransferInProgress);

			/*
			 * If the status indicates the write is done, then stop
			 * waiting, if a value of 0xFF in the status byte is
			 * read from the device and this loop never exits, the
			 * device slave select is possibly incorrect such that
			 * the device status is not being read
			 */
			FlashStatus[1] |= FlashStatus[0];
			if ((FlashStatus[1] & 0x01) == 0) {
				break;
			}
		}

		Address += SECTOR_SIZE;
	}
}

/*****************************************************************************/
/**
*
* @brief Reads serial FLASH ID connected to the SPI interface.
*
* @return	XST_SUCCESS if read id, otherwise XST_FAILURE.
*
******************************************************************************/
int FlashReadID(void)
{
	/* Read ID*/
	WriteBuffer[COMMAND_OFFSET]   = READ_ID;
	WriteBuffer[ADDRESS_1_OFFSET] = 0x23;		/* 3 dummy bytes */
	WriteBuffer[ADDRESS_2_OFFSET] = 0x08;
	WriteBuffer[ADDRESS_3_OFFSET] = 0x09;

	TransferInProgress = TRUE;

	XQspiPs_Transfer(&QspiInstance, WriteBuffer, ReadBuffer,
			  RD_ID_SIZE);

	while (TransferInProgress);

	xil_printf("FlashID=0x%x 0x%x 0x%x\n\r", ReadBuffer[1], ReadBuffer[2],
		   ReadBuffer[3]);

	return XST_SUCCESS;
}

#ifndef SDT
/*****************************************************************************/
/**
*
* @brief Sets up the interrupt system for an Qspi device.
*
* @param[in]	IntcInstancePtr Pointer to the instance of the Intc device.
* @param[in]	QspiInstancePtr Pointer to the instance of the Qspi device.
* @param[in]	QspiIntrId The interrupt Id for an QSPI device.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
******************************************************************************/
static int QspiSetupIntrSystem(XScuGic *IntcInstancePtr,
			       XQspiPs *QspiInstancePtr, u16 QspiIntrId)
{
	int Status;

	XScuGic_Config *IntcConfig; /* Instance of the interrupt controller */

	Xil_ExceptionInit();

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
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
	Status = XScuGic_Connect(IntcInstancePtr, QspiIntrId,
				(Xil_ExceptionHandler)XQspiPs_InterruptHandler,
				(void *)QspiInstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* Enable the interrupt for the Qspi device.*/
	XScuGic_Enable(IntcInstancePtr, QspiIntrId);

	/* Enable interrupts in the Processor.*/
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Disables the interrupts that occur for the Qspi device.
*
* @param[in]	IntcInstancePtr Pointer to an INTC instance.
* @param[in]	QspiIntrId The interrupt Id for an QSPI device.
*
******************************************************************************/
static void QspiDisableIntrSystem(XScuGic *IntcInstancePtr, u16 QspiIntrId)
{
	/* Disable the interrupt for the QSPI device.*/
	XScuGic_Disable(IntcInstancePtr, QspiIntrId);

	/* Disconnect and disable the interrupt for the Qspi device.*/
	XScuGic_Disconnect(IntcInstancePtr, QspiIntrId);
}
#endif

/**
 *
 * @brief Enables quad mode in the serial flash connected to the SPI interface.
 *
 * @param[in,out]	QspiPtr Pointer to the QSPI driver component to use.
 *
 ******************************************************************************/
void FlashQuadEnable(XQspiPs *QspiPtr)
{
	u8 WriteEnableCmd = {WRITE_ENABLE_CMD};
	u8 ReadStatusCmd[] = {READ_STATUS_CMD, 0};
	u8 QuadEnableCmd[] = {WRITE_STATUS_CMD, 0};
	u8 FlashStatus[2];

	if (ReadBuffer[1] == 0x9D) {

		TransferInProgress = TRUE;
		XQspiPs_Transfer(QspiPtr, ReadStatusCmd,
					FlashStatus,
					sizeof(ReadStatusCmd));
		while (TransferInProgress);

		QuadEnableCmd[1] = FlashStatus[1] | 1 << 6;

		TransferInProgress = TRUE;
		XQspiPs_Transfer(QspiPtr, &WriteEnableCmd, NULL,
				  sizeof(WriteEnableCmd));
		while (TransferInProgress);

		TransferInProgress = TRUE;
		XQspiPs_Transfer(QspiPtr, QuadEnableCmd, NULL,
					sizeof(QuadEnableCmd));
		while (TransferInProgress);

		while (1) {

			TransferInProgress = TRUE;
			/*
			 * Poll the status register of the FLASH to determine when
			 * Quad Mode is enabled and device is ready, by sending a
			 * read status command and receiving the status byte
			 */
			XQspiPs_Transfer(QspiPtr, ReadStatusCmd, FlashStatus,
					sizeof(ReadStatusCmd));

			while (TransferInProgress);
			/*
			 * If 6th bit is set & 0th bit is reset, then Quad is Enabled
			 * and device is ready.
			 */
			if ((FlashStatus[0] == 0x40) && (FlashStatus[1] == 0x40)) {
				break;
			}
		}

	}
}
/** @} */
