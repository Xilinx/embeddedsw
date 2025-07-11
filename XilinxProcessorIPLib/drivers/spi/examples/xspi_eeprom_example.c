/******************************************************************************
* Copyright (C) 2001 - 2023 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xspi_eeprom_example.c
*
*
* This file contains a design example using the SPI driver (XSpi) and
* hardware device with a serial EEPROM device.  The hardware which this
* example runs on must have a serial EEPROM (Microchip 25XX320 or 25XX160)
* for it to run.  This example has been tested with the SPI EEPROM on the ML410
* platform for PPC processor.
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00b jhl  02/27/01 First release
* 1.00c jhl  08/15/03 Fixed bugs (local instances and large buffers are now
*                                 moved to globals)
* 1.11a sv   9/10/07  Minor changes to comply to Doxygen and coding guidelines
* 3.00a ktn  10/28/09 Converted all register accesses to 32 bit access.
*		      Updated to use the HAL APIs/macros. Replaced call to
*		      XSpi_Initialize API with XSpi_LookupConfig and
*		      XSpi_CfgInitialize.
* 4.2   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
*       ms   04/05/17 Modified Comment lines to follow doxygen rules.
* 4.11  sb   07/11/23 Added support for system device-tree flow.
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"        /* EDK generated parameters */
#include "xspi.h"               /* SPI device driver */
#ifndef SDT
#include "xintc.h"              /* Interrupt controller device driver */
#else
#include "xinterrupt_wrap.h"
#endif
#include "xil_exception.h"
#include "xil_printf.h"


/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define SPI_DEVICE_ID		XPAR_SPI_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define SPI_INTR_ID		XPAR_INTC_0_SPI_0_VEC_ID
#endif


/*
 * The following constants define the commands which may be sent to the EEPROM
 * device.
 */
#define WRITE_STATUS_CMD	1
#define WRITE_CMD		2
#define READ_CMD		3
#define WRITE_DISABLE_CMD	4
#define READ_STATUS_CMD		5
#define WRITE_ENABLE_CMD	6

/*
 * The following constants define the offsets within a EepromBuffer data
 * type for each kind of data.  Note that the read data offset is not the
 * same as the write data because the SPI driver is designed to allow full
 * duplex transfers such that the number of bytes received is the number
 * sent and received.
 */
#define COMMAND_OFFSET		0 /* EEPROM instruction */
#define ADDRESS_MSB_OFFSET	1 /* MSB of address to read or write */
#define ADDRESS_LSB_OFFSET	2 /* LSB of address to read or write */
#define DATA_OFFSET		3
#define WRITE_DATA_OFFSET	3  /* Start of data to write to the EEPROM */
#define READ_DATA_OFFSET	6  /* Start of data read from the EEPROM */

/*
 * The following constants specify the extra bytes which are sent to the
 * EEPROM on the SPI interface, that are not data, but control information
 * which includes the command and address
 */
#define OVERHEAD_SIZE   3

/*
 * The following constants specify the page size and number of pages for the
 * EEPROM.  The page size specifies a max number of bytes that can be written
 * to the EEPROM with a single transfer using the SPI driver.
 */
#define PAGE_SIZE       16
#define PAGE_COUNT      128

/*
 * The following constants specify the max amount of data and the size of the
 * the buffer required to hold the data and overhead to transfer the data to
 * and from the EEPROM.
 */
#define MAX_DATA        PAGE_COUNT * PAGE_SIZE
#define BUFFER_SIZE     MAX_DATA + READ_DATA_OFFSET

/*
 * The following constant defines the slave select signal that is used to
 * to select the EEPROM device on the SPI bus, this signal is typically
 * connected to the chip select of the device
 */
#define SEEPROM_SPI_SELECT 0x01

/**************************** Type Definitions *******************************/

/*
 * The following data type is used to send and receive data to the serial
 * EEPROM device connected to the SPI interface.  It is an array of bytes
 * rather than a structure for portability avoiding packing issues.  The
 * application must setup the data to be written in this buffer and retrieve
 * the data read from it.
 */
typedef u8 EepromBuffer[BUFFER_SIZE];

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

#ifndef SDT
static int SetupInterruptSystem(XSpi *SpiPtr);
#endif

void SpiHandler(void *CallBackRef, u32 StatusEvent, unsigned int ByteCount);

void EepromRead(XSpi *SpiPtr, u16 Address, int ByteCount, EepromBuffer Buffer);

void EepromWrite(XSpi *SpiPtr, u16 Address, u8 ByteCount, EepromBuffer Buffer);

/************************** Variable Definitions *****************************/

/*
 * The instances to support the device drivers are global such that the
 * are initialized to zero each time the program runs.  They could be local
 * but should at least be static so they are zeroed.
 */
#ifndef SDT
XIntc InterruptController;
#endif
XSpi Spi;

/*
 * The following variables are shared between non-interrupt processing and
 * interrupt processing such that they must be global.
 */
volatile int TransferInProgress;

/*
 * The following variable tracks any errors that occur during interrupt
 * processing
 */
int Error;

/*
 * The following variable allows a test value to be added to the values that
 * are written to the EEPROM such that unique values can be generated to
 * guarantee the writes to the EEPROM were successful
 */
int Test;

/*
 * The following variables are used to read and write to the eeprom and they
 * are global to avoid having large buffers on the stack
 */
EepromBuffer ReadBuffer;
EepromBuffer WriteBuffer;

/******************************************************************************/
/**
*
* The purpose of this function is to illustrate how to use the XSpi
* device driver. This test writes and reads data from a Microchip serial EEPROM.
* This part must be present in the hardware to use this example.
*
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note
*
* This function calls functions which contain loops that may be infinite
* if interrupts are not working such that it may not return. If the device
* slave select is not correct and the device is not responding on bus it will
* read a status of 0xFF for the status register as the bus is pulled up.
*
*****************************************************************************/
int main(void)
{
	int Status;
	u8 *BufferPtr;
	u8 UniqueValue;
	int Count;
	int Page;
	XSpi_Config *ConfigPtr;	/* Pointer to Configuration data */

	/*
	 * Initialize the SPI driver so that it is  ready to use.
	 */
#ifndef SDT
	ConfigPtr = XSpi_LookupConfig(SPI_DEVICE_ID);
#else
	ConfigPtr = XSpi_LookupConfig(XPAR_XSPI_0_BASEADDR);
#endif
	if (ConfigPtr == NULL) {
		return XST_DEVICE_NOT_FOUND;
	}
	Status = XSpi_CfgInitialize(&Spi, ConfigPtr,
				    ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the SPI driver to the interrupt subsystem such that
	 * interrupts can occur.  This function is application specific.
	 */
#ifndef SDT
	Status = SetupInterruptSystem(&Spi);
#else
	Status = XSetupInterruptSystem(&Spi, &XSpi_InterruptHandler,
				       ConfigPtr->IntrId,
				       ConfigPtr->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the handler for the SPI that will be called from the interrupt
	 * context when an SPI status occurs, specify a pointer to the SPI
	 * driver instance as the callback reference so the handler is able to
	 * access the instance data
	 */
	XSpi_SetStatusHandler(&Spi, &Spi, (XSpi_StatusHandler)SpiHandler);

	/*
	 * Set the SPI device as a master and in manual slave select mode such
	 * that the slave select signal does not toggle for every byte of a
	 * transfer, this must be done before the slave select is set
	 */
	Status = XSpi_SetOptions(&Spi, XSP_MASTER_OPTION |
				 XSP_MANUAL_SSELECT_OPTION);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Select the slave on the SPI bus, the EEPROM device so that it can be
	 * read and written using the SPI bus
	 */
	Status = XSpi_SetSlaveSelect(&Spi, SEEPROM_SPI_SELECT);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the SPI driver so that interrupts and the device are enabled
	 */
	XSpi_Start(&Spi);

	/*
	 * Initialize the write buffer for a pattern to write to the EEPROM
	 * and the read buffer to zero so it can be verified after the read, the
	 * test value that is added to the unique value allows the value to be
	 * changed in a debug environment to guarantee
	 */
	for (UniqueValue = 10, Count = 0; Count < MAX_DATA;
	     Count++, UniqueValue++) {
		WriteBuffer[WRITE_DATA_OFFSET + Count] =
			(u8)(UniqueValue + Test);
		ReadBuffer[READ_DATA_OFFSET + Count] = 0;
	}

	/*
	 * Write the data in the write buffer to the serial EEPROM a page at a
	 * time
	 */
	for (Page = 0; Page < PAGE_COUNT; Page++) {
		EepromWrite(&Spi, Page * PAGE_SIZE, PAGE_SIZE,
			    &WriteBuffer[Page * PAGE_SIZE]);
	}

	/*
	 * Read the contents of the entire EEPROM from address 0, since this
	 * function reads the entire EEPROM it will take some amount of time to
	 * complete
	 */
	EepromRead(&Spi, 0, MAX_DATA, ReadBuffer);

	/*
	 * Setup a pointer to the start of the data that was read into the read
	 * buffer and verify the data read is the data that was written
	 */
	BufferPtr = &ReadBuffer[READ_DATA_OFFSET];

	for (UniqueValue = 10, Count = 0; Count < MAX_DATA;
	     Count++, UniqueValue++) {
		if (BufferPtr[Count] != (u8)(UniqueValue + Test)) {
			return XST_FAILURE;
		}
	}

	xil_printf("Successfully ran Spi eeprom Example\r\n");
	return XST_SUCCESS;
}

/******************************************************************************/
/**
*
* This function is the handler which performs processing for the SPI driver.
* It is called from an interrupt context such that the amount of processing
* performed should be minimized.  It is called when a transfer of SPI data
* completes or an error occurs.
*
* This handler provides an example of how to handle SPI interrupts
* but is application specific.
*
*
* @param 	CallBackRef is a reference passed to the handler.
* @param	StatusEvent is the status of the SPI .
* @param	ByteCount is the number of bytes transferred.
*
* @return	None
*
* @note		None.
*
******************************************************************************/
void SpiHandler(void *CallBackRef, u32 StatusEvent, unsigned int ByteCount)
{
	(void)CallBackRef;
	(void)ByteCount;
	/*
	 * Indicate the transfer on the SPI bus is no longer in progress
	 * regardless of the status event
	 */
	TransferInProgress = FALSE;

	/*
	 * If the event was not transfer done, then track it as an error
	 */
	if (StatusEvent != XST_SPI_TRANSFER_DONE) {
		Error++;
	}
}

/******************************************************************************/
/**
*
* This function reads from the Microchip serial EEPROM connected to the
* SPI interface.
*
* @param 	SpiPtr is a pointer to the SPI driver component to use.
* @param 	Address contains the address to read data from in the EEPROM.
* @param 	ByteCount contains the number of bytes to read.
* @param 	Buffer is a buffer to read the data into.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void EepromRead(XSpi *SpiPtr, u16 Address, int ByteCount, EepromBuffer Buffer)
{
	/*
	 * Setup the write command with the specified address and data for the
	 * EEPROM
	 */
	Buffer[COMMAND_OFFSET]     = READ_CMD;
	Buffer[ADDRESS_MSB_OFFSET] = (u8)((Address & 0xFF00) >> 8);
	Buffer[ADDRESS_LSB_OFFSET] = (u8)(Address & 0x00FF);

	/*
	 * Send the read command to the EEPROM to read the specified number
	 * of bytes from the EEPROM, send the read command and address and
	 * receive the specified number of bytes of data in the data buffer
	 */
	TransferInProgress = TRUE;

	XSpi_Transfer(SpiPtr, Buffer, &Buffer[DATA_OFFSET],
		      ByteCount + OVERHEAD_SIZE);

	/*
	 * Wait for the transfer on the SPI bus to be complete before proceeding
	 */
	while (TransferInProgress);
}

/******************************************************************************/
/**
*
* This function writes to the Microchip serial EEPROM connected to the
* SPI interface.  This function is not designed to be a driver to handle all
* the conditions of the EEPROM device.  The EEPROM contains a 32 byte write
* buffer which can be filled and then a write is automatically performed by
* the device.  All the data put into the buffer must be in the same page of
* the device with page boundaries being on 32 byte boundaries.
*
* @param 	SpiPtr is a pointer to the SPI driver component to use.
* @param 	Address contains the address to write data to in the EEPROM.
* @param 	ByteCount contains the number of bytes to write.
* @param 	Buffer is a buffer of data to write from.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void EepromWrite(XSpi *SpiPtr, u16 Address, u8 ByteCount, EepromBuffer Buffer)
{
	u8 WriteEnableCmd = { WRITE_ENABLE_CMD };
	u8 ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* must send 2 bytes */
	u8 EepromStatus[2];
	int DelayCount = 0;

	/*
	 * Send the write enable command to the SEEPOM so that it can be
	 * written to, this needs to be sent as a separate transfer before
	 * the write
	 */
	TransferInProgress = TRUE;

	XSpi_Transfer(SpiPtr, &WriteEnableCmd, NULL, sizeof(WriteEnableCmd));

	/*
	 * Wait for the transfer on the SPI bus to be complete before proceeding
	 */
	while (TransferInProgress);

	/*
	 * Setup the write command with the specified address and data for the
	 * EEPROM
	 */
	Buffer[COMMAND_OFFSET]     = WRITE_CMD;
	Buffer[ADDRESS_MSB_OFFSET] = (u8)((Address & 0xFF00) >> 8);
	Buffer[ADDRESS_LSB_OFFSET] = (u8)(Address & 0x00FF);

	/*
	 * Send the write command, address, and data to the EEPROM to be
	 * written, no receive buffer is specified since there is nothing to
	 * receive
	 */
	TransferInProgress = TRUE;

	XSpi_Transfer(SpiPtr, Buffer, NULL, ByteCount + OVERHEAD_SIZE);

	while (TransferInProgress);

	/*
	 * Wait for a bit of time to allow the programming to occur as reading
	 * the status while programming causes it to fail because of noisy power
	 * on the board containing the EEPROM, this loop does not need to be
	 * very long but is longer to hopefully work for a faster processor
	 */
	while (DelayCount++ < 10000) {
	}

	/*
	 * Wait for the write command to the EEPROM to be completed, it takes
	 * some time for the data to be written
	 */
	while (1) {
		/*
		 * Poll the status register of the device to determine when it
		 * completes by sending a read status command and receiving the
		 * status byte
		 */
		TransferInProgress = TRUE;

		XSpi_Transfer(SpiPtr, ReadStatusCmd, EepromStatus,
			      sizeof(ReadStatusCmd));

		/*
		 * Wait for the transfer on the SPI bus to be complete before
		 * proceeding
		 */
		while (TransferInProgress);

		/*
		 * If the status indicates the write is done, the stop waiting,
		 * if a value of 0xFF in the status byte is read from the
		 * device and this loop never exits, the device slave select is
		 * possibly incorrect such that the device status is not being
		 * read
		 */
		if ((EepromStatus[1] & 0x03) == 0) {
			break;
		}
	}
}

/****************************************************************************
*
*
* This function setups the interrupt system such that interrupts can occur
* for the SPI driver.  This function is application specific since the actual
* system may or may not have an interrupt controller. The SPI device could
* be directly connected to a processor without an interrupt controller.  The
* user should modify this function to fit the application.
*
* @param 	SpiPtr contains a pointer to the instance of the XSpi component
* 		which is going to be connected to the interrupt controller.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
#ifndef SDT
static int SetupInterruptSystem(XSpi *SpiPtr)
{

	int Status;


	/*
	 * Initialize the interrupt controller driver so that
	 * it's ready to use, specify the device ID that is generated in
	 * xparameters.h
	 */
	Status = XIntc_Initialize(&InterruptController, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the device occurs, the device driver handler performs the
	 * specific interrupt processing for the device
	 */
	Status = XIntc_Connect(&InterruptController,
			       SPI_INTR_ID,
			       (XInterruptHandler)XSpi_InterruptHandler,
			       (void *)SpiPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts, specific real mode so that
	 * the SPI can cause interrupts through the interrupt controller.
	 */
	Status = XIntc_Start(&InterruptController, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupt for the Spi device
	 */
	XIntc_Enable(&InterruptController, SPI_INTR_ID);


	/*
	 * Initialize the exception table
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler)XIntc_InterruptHandler,
				     &InterruptController);

	/*
	 * Enable non-critical exceptions
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
#endif

