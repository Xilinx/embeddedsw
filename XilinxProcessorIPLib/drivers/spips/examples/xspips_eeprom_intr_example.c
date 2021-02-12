/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xspips_eeprom_intr_example.c
*
*
* This file contains a design example using the SPI driver (XSpiPs) in
* interrupt mode and hardware device with a serial EEPROM device.  The
* hardware which this example runs on must have a serial EEPROM (Microchip
* 25XX320 or 25XX160) for it to run.
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
* 1.00  sdm  02/27/10 First release
* 1.00  sdm  10/25/11 Updated the chip select to be used to second chip select
*       ms   04/05/17 Modified Comment lines in functions to
*                     recognize it as documentation block for doxygen
*                     generation and also modified filename tag to include
*                     the file in doxygen examples.
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"	/* EDK generated parameters */
#include "xspips.h"		/* SPI device driver */
#include "xscugic.h"		/* Interrupt controller device driver */
#include "xil_exception.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define SPI_DEVICE_ID		XPAR_XSPIPS_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define SPI_INTR_ID		XPAR_XSPIPS_0_INTR

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
#define OVERHEAD_SIZE		3

/*
 * The following constants specify the page size and number of pages for the
 * EEPROM.  The page size specifies a max number of bytes that can be written
 * to the EEPROM with a single transfer using the SPI driver.
 */
#define PAGE_SIZE		16
#define PAGE_COUNT		128

/*
 * The following constants specify the max amount of data and the size of the
 * the buffer required to hold the data and overhead to transfer the data to
 * and from the EEPROM.
 */
#define MAX_DATA		PAGE_COUNT * PAGE_SIZE
#define BUFFER_SIZE		MAX_DATA + READ_DATA_OFFSET

/*
 * The following constant defines the slave select signal that is used to
 * to select the EEPROM device on the SPI bus, this signal is typically
 * connected to the chip select of the device
 */
#define EEPROM_SPI_SELECT	0x01

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

static int SpiSetupIntrSystem(XScuGic *IntcInstancePtr,
			      XSpiPs *SpiInstancePtr, u16 SpiIntrId);

static void SpiDisableIntrSystem(XScuGic *IntcInstancePtr, u16 SpiIntrId);

void SpiHandler(void *CallBackRef, u32 StatusEvent, unsigned int ByteCount);

void EepromRead(XSpiPs *SpiPtr, u16 Address, int ByteCount,
		EepromBuffer Buffer);

void EepromWrite(XSpiPs *SpiPtr, u16 Address, u8 ByteCount,
		 EepromBuffer Buffer);

int SpiPsEepromIntrExample(XScuGic *IntcInstancePtr, XSpiPs *SpiInstancePtr,
			 u16 SpiDeviceId, u16 SpiIntrId);

/************************** Variable Definitions *****************************/

/*
 * The instances to support the device drivers are global such that the
 * are initialized to zero each time the program runs.  They could be local
 * but should at least be static so they are zeroed.
 */
static XScuGic IntcInstance;
static XSpiPs SpiInstance;

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

/*****************************************************************************/
/**
*
* Main function to call the Spi Eeprom example.
*
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int main(void)
{
	int Status;

	xil_printf("SPI EEPROM Interrupt Example Test \r\n");

	/*
	 * Run the Spi Interrupt example.
	 */
	Status = SpiPsEepromIntrExample(&IntcInstance, &SpiInstance,
				      SPI_DEVICE_ID, SPI_INTR_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("SPI EEPROM Interrupt Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran SPI EEPROM Interrupt Example Test\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* The purpose of this function is to illustrate how to use the XSpiPs
* device driver in interrupt mode . This test writes and reads data from a
* serial EEPROM.
* This part must be present in the hardware to use this example.
*
* @param	IntcInstancePtr is a pointer to the GIC driver to use.
* @param	SpiInstancePtr is a pointer to the SPI driver to use.
* @param	SpiDeviceId is the DeviceId of the Spi device.
* @param	SpiIntrId is the Spi Interrupt Id.
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
int SpiPsEepromIntrExample(XScuGic *IntcInstancePtr, XSpiPs *SpiInstancePtr,
			 u16 SpiDeviceId, u16 SpiIntrId)
{
	int Status;
	u8 *BufferPtr;
	u8 UniqueValue;
	int Count;
	int Page;
	XSpiPs_Config *SpiConfig;

	/*
	 * Initialize the SPI driver so that it's ready to use
	 */
	SpiConfig = XSpiPs_LookupConfig(SpiDeviceId);
	if (NULL == SpiConfig) {
		return XST_FAILURE;
	}

	Status = XSpiPs_CfgInitialize(SpiInstancePtr, SpiConfig,
				       SpiConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to check hardware build
	 */
	Status = XSpiPs_SelfTest(SpiInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the Spi device to the interrupt subsystem such that
	 * interrupts can occur. This function is application specific
	 */
	Status = SpiSetupIntrSystem(IntcInstancePtr, SpiInstancePtr, SpiIntrId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the handler for the SPI that will be called from the
	 * interrupt context when an SPI status occurs, specify a pointer to
	 * the SPI driver instance as the callback reference so the handler is
	 * able to access the instance data
	 */
	XSpiPs_SetStatusHandler(SpiInstancePtr, SpiInstancePtr,
				 (XSpiPs_StatusHandler) SpiHandler);

	/*
	 * Set the Spi device as a master. External loopback is required.
	 */
	XSpiPs_SetOptions(SpiInstancePtr, XSPIPS_MASTER_OPTION |
			   XSPIPS_FORCE_SSELECT_OPTION);

	XSpiPs_SetClkPrescaler(SpiInstancePtr, XSPIPS_CLK_PRESCALE_64);

	/*
	 * Initialize the write buffer for a pattern to write to the EEPROM
	 * and the read buffer to zero so it can be verified after the read, the
	 * test value that is added to the unique value allows the value to be
	 * changed in a debug environment to guarantee
	 */
	for (UniqueValue = 13, Count = 0; Count < MAX_DATA;
					Count++, UniqueValue++) {
		WriteBuffer[WRITE_DATA_OFFSET + Count] =
					(u8)(UniqueValue + Test);
		ReadBuffer[READ_DATA_OFFSET + Count] = 0xA5;
	}

	/*
	 * Assert the EEPROM chip select
	 */
	XSpiPs_SetSlaveSelect(SpiInstancePtr, EEPROM_SPI_SELECT);

	/*
	 * Write the data in the write buffer to the serial EEPROM a page at a
	 * time
	 */
	for (Page = 0; Page < PAGE_COUNT; Page++) {
		EepromWrite(SpiInstancePtr, Page * PAGE_SIZE, PAGE_SIZE,
				&WriteBuffer[Page * PAGE_SIZE]);
	}

	/*
	 * Read the contents of the entire EEPROM from address 0, since this
	 * function reads the entire EEPROM it will take some amount of time to
	 * complete
	 */
	EepromRead(SpiInstancePtr, 0, MAX_DATA, ReadBuffer);

	/*
	 * Setup a pointer to the start of the data that was read into the read
	 * buffer and verify the data read is the data that was written
	 */
	BufferPtr = &ReadBuffer[READ_DATA_OFFSET];

	for (UniqueValue = 13, Count = 0; Count < MAX_DATA;
					Count++, UniqueValue++) {
		if (BufferPtr[Count] != (u8)(UniqueValue + Test)) {
			return XST_FAILURE;
		}
	}

	SpiDisableIntrSystem(IntcInstancePtr, SpiIntrId);
	return XST_SUCCESS;
}

/*****************************************************************************/
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
* @param	CallBackRef is a reference passed to the handler.
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

/*****************************************************************************/
/**
*
* This function reads from the serial EEPROM connected to the SPI interface.
*
* @param	SpiPtr is a pointer to the SPI driver instance to use.
* @param	Address contains the address to read data from in the EEPROM.
* @param	ByteCount contains the number of bytes to read.
* @param	Buffer is a buffer to read the data into.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void EepromRead(XSpiPs *SpiPtr, u16 Address, int ByteCount,
		EepromBuffer Buffer)
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

	XSpiPs_Transfer(SpiPtr, Buffer, &Buffer[DATA_OFFSET],
			 ByteCount + OVERHEAD_SIZE);

	/*
	 * Wait for the transfer on the SPI bus to be complete before proceeding
	 */
	while (TransferInProgress);
}

/*****************************************************************************/
/**
*
* This function writes to the serial EEPROM connected to the SPI interface.
* This function is not designed to be a driver to handle all
* the conditions of the EEPROM device.  The EEPROM contains a 32 byte write
* buffer which can be filled and then a write is automatically performed by
* the device.  All the data put into the buffer must be in the same page of
* the device with page boundaries being on 32 byte boundaries.
*
* @param	SpiPtr is a pointer to the SPI driver instance to use.
* @param	Address contains the address to write data to in the EEPROM.
* @param	ByteCount contains the number of bytes to write.
* @param	Buffer is a buffer of data to write from.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void EepromWrite(XSpiPs *SpiPtr, u16 Address, u8 ByteCount,
		 EepromBuffer Buffer)
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

	XSpiPs_Transfer(SpiPtr, &WriteEnableCmd, NULL, sizeof(WriteEnableCmd));

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
	XSpiPs_Transfer(SpiPtr, Buffer, NULL, ByteCount + OVERHEAD_SIZE);

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

		XSpiPs_Transfer(SpiPtr, ReadStatusCmd, EepromStatus,
				 sizeof(ReadStatusCmd));

		/*
		 * Wait for the transfer on the SPI bus to be complete before
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
		if ((EepromStatus[1] & 0x03) == 0) {
			break;
		}
	}
}

/*****************************************************************************/
/**
*
* This function setups the interrupt system for an Spi device.
* This function is application specific since the actual system may or may not
* have an interrupt controller. The Spi device could be directly connected to
* a processor without an interrupt controller.  The user should modify this
* function to fit the application.
*
* @param	IntcInstancePtr is a pointer to the instance of the Intc device.
* @param	SpiInstancePtr is a pointer to the instance of the Spi device.
* @param	SpiIntrId is the interrupt Id for an SPI device.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static int SpiSetupIntrSystem(XScuGic *IntcInstancePtr,
			      XSpiPs *SpiInstancePtr, u16 SpiIntrId)
{
	int Status;

#ifndef TESTAPP_GEN
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
#endif

	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, SpiIntrId,
				(Xil_ExceptionHandler)XSpiPs_InterruptHandler,
				(void *)SpiInstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the interrupt for the Spi device.
	 */
	XScuGic_Enable(IntcInstancePtr, SpiIntrId);

#ifndef TESTAPP_GEN
	/*
	 * Enable interrupts in the Processor.
	 */
	Xil_ExceptionEnable();
#endif

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function disables the interrupts that occur for the Spi device.
*
* @param	IntcInstancePtr is the pointer to a ScuGic driver instance.
* @param	SpiIntrId is the interrupt Id for an SPI device.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void SpiDisableIntrSystem(XScuGic *IntcInstancePtr, u16 SpiIntrId)
{
	/*
	 * Disable the interrupt for the SPI device.
	 */
	XScuGic_Disable(IntcInstancePtr, SpiIntrId);

	/*
	 * Disconnect and disable the interrupt for the Spi device.
	 */
	XScuGic_Disconnect(IntcInstancePtr, SpiIntrId);
}
