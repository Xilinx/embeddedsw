/******************************************************************************
* Copyright (c) 2012 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************
*
* @file xilisf_stm_read_write_example.c
*
* This file contains a design example using the In-system and Serial Flash
* Library (XilIsf) for Reading/Writing to a STM (Numonyx) M25P16 Serial Flash
* Device. This example shows the Erase, Read and Write features.
*
* This example
* - Erases a Sector
* - Writes to a Page within the Erased Sector
* - Reads back the Page that is written and compares the data
*
* This example  has been tested with a STM (Numonyx) M25P16 device on Xilinx
* Spartan-3A Starter Kit board.
*
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
* 1.00a sdm  03/17/08 First release
* 2.00a ktn  11/22/09 Updated to use HAL processor APIs.
* 5.0   sb   08/05/14 Registering to Xilisf Interrupt handler
*		      instead of driver handler.
* 5.2   asa  05/12/15 Added support for Micron N25Q256A flash.
* 5.5   sk  01/14/16 Added support for Spansion flash in extended address
*                    mode.
* 5.6   sk   05/12/16 Corrected the missing WE before erase. CR# 951694.
* 5.14  akm  08/01/19 Initialized Status variable to XST_FAILURE.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"	/* EDK generated parameters */
#include "xintc.h"		/* Interrupt controller device driver */
#include <xilisf.h>		/* Serial Flash Library header file */
#include "xil_exception.h"

/************************** Constant Definitions *****************************/


/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define SPI_DEVICE_ID			XPAR_SPI_0_DEVICE_ID
#define INTC_DEVICE_ID			XPAR_INTC_0_DEVICE_ID
#define SPI_INTR_ID			XPAR_INTC_0_SPI_0_VEC_ID

/*
 * The following constant defines the slave select signal that is used to
 * to select the Flash device on the SPI bus, this signal is typically
 * connected to the chip select of the device.
 */
#define ISF_SPI_SELECT		0x01

/*
 * Number of bytes per page in the flash device.
 */
#define ISF_PAGE_SIZE		256

/*
 * Address of the page to perform Erase, Write and Read operations.
 */
#define ISF_TEST_ADDRESS	0x010000;
#define ISF_TEST_BYTE		0x05 /* Test Byte offset value written */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static int IsfWaitForFlashNotBusy(void);
void SpiHandler(void *CallBackRef, u32 StatusEvent, u16 ByteCount);
static int SetupInterruptSystem(XSpi *SpiPtr);
static int IsfStmFlashExample();

/************************** Variable Definitions *****************************/

/*
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs. They could be local
 * but should at least be static so they are zeroed.
 */
static XIsf Isf;
static XIntc InterruptController;
static XSpi Spi;

/*
 * The following variables are shared between non-interrupt processing and
 * interrupt processing such that they must be global.
 */
volatile static int TransferInProgress; /* State of Spi Transfer */
static int ErrorCount;		/* Errors occurred during Spi transfers */

/*
 * The user needs to allocate a buffer to be used by the In-system and Serial
 * Flash Library to perform any read/write operations on the Serial Flash
 * device.
 * User applications must pass the address of this memory to the Library in
 * Serial Flash Initialization function, for the Library to work.
 * For Write operations:
 * - The size of this buffer should be equal to the Number of bytes to be
 * written to the Serial Flash + XISF_CMD_MAX_EXTRA_BYTES.
 * - The size of this buffer should be large enough for usage across all the
 * applications that use a common instance of the Serial Flash.
 * - A minimum of one byte and a maximum of ISF_PAGE_SIZE bytes can be written
 * to the Serial Flash, through a single Write operation.
 * The size of this buffer should be equal to XISF_CMD_MAX_EXTRA_BYTES, if the
 * application only reads from the Serial Flash (no write operations).
 */
u8 IsfWriteBuffer[ISF_PAGE_SIZE + XISF_CMD_SEND_EXTRA_BYTES + 1];

/*
 * Buffers used during Read/Write transactions.
 */
u8 ReadBuffer[ISF_PAGE_SIZE + XISF_CMD_SEND_EXTRA_BYTES + 1]; /* Read Buffer */
u8 WriteBuffer[ISF_PAGE_SIZE];				  /* Write buffer */

/************************** Function Definitions ******************************/

/*****************************************************************************/
/**
*
* Main function to call the STM Serial Flash Read/Write example.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;

	xil_printf("STM Serial Flash Read/Write example\r\n");

	/*
	 * Run the STM flash example.
	 */
	Status = IsfStmFlashExample();
	if (Status != XST_SUCCESS) {
		xil_printf("STM Serial Flash Read/Write example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran STM Serial Flash Read/Write example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Function to execute the STM Serial Flash Read/Write example.
*
* @param	None
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None
*
******************************************************************************/
static int IsfStmFlashExample()
{
	int Status = XST_FAILURE;
	u32 Index;
	u32 Address;
	XIsf_WriteParam WriteParam;
	XIsf_ReadParam ReadParam;

	/*
	 * Initialize the SPI driver so that it's ready to use,
	 * specify the device ID that is generated in xparameters.h.
	 */
	Status = XSpi_Initialize(&Spi, SPI_DEVICE_ID);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the SPI driver to the interrupt subsystem such that
	 * interrupts can occur. This function is application specific.
	 */
	Status = SetupInterruptSystem(&Spi);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the handler for the SPI that will be called from the interrupt
	 * context when an SPI status occurs, specify a pointer to the SPI
	 * driver instance as the callback reference so the handler is able to
	 * access the instance data.
	 */
	XIsf_SetStatusHandler(&Isf, &Spi, (XSpi_StatusHandler)SpiHandler);

	/*
	 * Start the SPI driver so that interrupts and the device are enabled.
	 */
	XSpi_Start(&Spi);

	/*
	 * Initialize the Serial Flash Library.
	 */
	Status = XIsf_Initialize(&Isf, &Spi, ISF_SPI_SELECT, IsfWriteBuffer);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set The transfer Mode to Interrupt
	 */
	XIsf_SetTransferMode(&Isf,XISF_INTERRUPT_MODE);

	/*
	 * Specify the address in the Serial Flash for the Erase/Write/Read
	 * operations.
	 */
	Address = ISF_TEST_ADDRESS;

	/*
	 * Check if the flash part is micron or spansion in which case,
	 * switch to 4 byte addressing mode if FlashSize is >128Mb.
	 */
	if (((Isf.ManufacturerID == XISF_MANUFACTURER_ID_MICRON) ||
				(Isf.ManufacturerID == XISF_MANUFACTURER_ID_SPANSION)) &&
				(((u8)Isf.DeviceCode) > XISF_SPANSION_ID_BYTE2_128)) {
		TransferInProgress = TRUE;
		Status = XIsf_WriteEnable(&Isf, XISF_WRITE_ENABLE);
		if(Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		while(TransferInProgress);
		if(ErrorCount != 0) {
			return XST_FAILURE;
		}

		Status = IsfWaitForFlashNotBusy();
		if(Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		TransferInProgress = TRUE;
		XIsf_MicronFlashEnter4BAddMode(&Isf);

		while(TransferInProgress);
		if(ErrorCount != 0) {
			return XST_FAILURE;
		}

		Status = IsfWaitForFlashNotBusy();
		if(Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	TransferInProgress = TRUE;
	Status = XIsf_WriteEnable(&Isf, XISF_WRITE_ENABLE);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	while(TransferInProgress);
	if(ErrorCount != 0) {
		return XST_FAILURE;
	}

	Status = IsfWaitForFlashNotBusy();
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	TransferInProgress = TRUE;
	Status = XIsf_Erase(&Isf, XISF_SECTOR_ERASE, Address);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	while(TransferInProgress);
	if(ErrorCount != 0) {
		return XST_FAILURE;
	}

	Status = IsfWaitForFlashNotBusy();
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	TransferInProgress = TRUE;
	Status = XIsf_WriteEnable(&Isf, XISF_WRITE_ENABLE);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	while(TransferInProgress);
	if(ErrorCount != 0) {
		return XST_FAILURE;
	}

	Status = IsfWaitForFlashNotBusy();
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the
	 * - Address within the Serial Flash where the data is to be written.
	 * - Number of bytes to be written to the Serial Flash.
	 * - Write Buffer which contains the data to be written to the Serial
	 *   Flash.
	 */
	WriteParam.Address = Address;
	WriteParam.NumBytes = ISF_PAGE_SIZE;
	WriteParam.WritePtr = WriteBuffer;

	/*
	 * Prepare the write buffer. Fill in the data need to be written into
	 * Serial Flash.
	 */
	for(Index = 0; Index < ISF_PAGE_SIZE; Index++) {
		WriteParam.WritePtr[Index] = Index + ISF_TEST_BYTE;
	}

	TransferInProgress = TRUE;
	Status = XIsf_Write(&Isf, XISF_WRITE, (void*) &WriteParam);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	while(TransferInProgress);
	if(ErrorCount != 0) {
		return XST_FAILURE;
	}

	Status = IsfWaitForFlashNotBusy();
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the
	 * - Address in the Serial Flash where the data is to be read from.
	 * - Number of bytes to be read from the Serial Flash.
	 * - Read Buffer to which the data is to be read.
	 */
	ReadParam.Address = Address;
	ReadParam.NumBytes = ISF_PAGE_SIZE;
	ReadParam.ReadPtr = ReadBuffer;

	TransferInProgress = TRUE;
	Status = XIsf_Read(&Isf, XISF_READ, (void*) &ReadParam);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	while(TransferInProgress);
	if(ErrorCount != 0) {
		return XST_FAILURE;
	}

	/*
	 * Compare the data read against the data Written.
	 */
	if (Isf.ManufacturerID == XISF_MANUFACTURER_ID_MICRON) {
		for(Index = 0; Index < ISF_PAGE_SIZE; Index++) {
			if(ReadBuffer[Index + XISF_CMD_SEND_EXTRA_BYTES_4BYTE_MODE] !=
						(u8)(Index + ISF_TEST_BYTE)) {
				return XST_FAILURE;
			}
		}
	} else {
		for(Index = 0; Index < ISF_PAGE_SIZE; Index++) {
			if(ReadBuffer[Index + XISF_CMD_SEND_EXTRA_BYTES] !=
						(u8)(Index + ISF_TEST_BYTE)) {
				return XST_FAILURE;
			}
		}
	}

	if (((Isf.ManufacturerID == XISF_MANUFACTURER_ID_MICRON) ||
				(Isf.ManufacturerID == XISF_MANUFACTURER_ID_SPANSION)) &&
				(((u8)Isf.DeviceCode) > XISF_SPANSION_ID_BYTE2_128)) {
		TransferInProgress = TRUE;
		Status = XIsf_WriteEnable(&Isf, XISF_WRITE_ENABLE);
		if(Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		while(TransferInProgress);
		if(ErrorCount != 0) {
			return XST_FAILURE;
		}

		Status = IsfWaitForFlashNotBusy();
		if(Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		TransferInProgress = TRUE;
		XIsf_MicronFlashExit4BAddMode(&Isf);

		while(TransferInProgress);
		if(ErrorCount != 0) {
			return XST_FAILURE;
		}

		Status = IsfWaitForFlashNotBusy();
		if(Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function waits till the STM serial Flash is ready to accept next command.
*
* @param	None
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		This function reads the status register of the Serial Flash and
*		waits till the Serial Flash is ready to accept next command.
*
******************************************************************************/
int IsfWaitForFlashNotBusy(void)
{
	int Status = XST_FAILURE;
	u8 StatusReg;

	while(1) {

		/*
		 * Get the Status Register.
		 */
		TransferInProgress = TRUE;
		Status = XIsf_GetStatus(&Isf, ReadBuffer);
		if(Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/*
		 * Wait till the Transfer is complete and check for errors.
		 */
		while(TransferInProgress);
		if(ErrorCount != 0) {
			return XST_FAILURE;
		}

		/*
		 * Check if the Serial Flash is ready to accept the next
		 * command. If so break.
		 */
		StatusReg = ReadBuffer[BYTE2];
		if((StatusReg & XISF_SR_IS_READY_MASK) == 0) {
			break;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is the handler which performs processing for the SPI driver.
* It is called from an interrupt context such that the amount of processing
* performed should be minimized. It is called when a transfer of SPI data
* completes or an error occurs.
*
* This handler provides an example of how to handle SPI interrupts and
* is application specific.
*
* @param	CallBackRef is the upper layer callback reference passed back
*		when the callback function is invoked.
* @param	StatusEvent is the event that just occurred.
* @param	ByteCount is the number of bytes transferred up until the event
*		occurred.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void SpiHandler(void *CallBackRef, u32 StatusEvent, u16 ByteCount)
{
	/*
	 * Indicate the transfer on the SPI bus is no longer in progress
	 * regardless of the status event.
	 */
	TransferInProgress = FALSE;

	/*
	 * If the event was not transfer done, then track it as an error.
	 */
	if (StatusEvent != XST_SPI_TRANSFER_DONE) {
		ErrorCount++;
	}
}

/*****************************************************************************/
/**
*
* This function setups the interrupt system such that interrupts can occur
* for the Spi device. This function is application specific since the actual
* system may or may not have an interrupt controller. The Spi device could be
* directly connected to a processor without an interrupt controller.  The
* user should modify this function to fit the application.
*
* @param	SpiPtr is a pointer to the instance of the Spi device.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
******************************************************************************/
static int SetupInterruptSystem(XSpi *SpiPtr)
{

	int Status = XST_FAILURE;

	/*
	 * Initialize the interrupt controller driver so that
	 * it's ready to use, specify the device ID that is generated in
	 * xparameters.h
	 */
	Status = XIntc_Initialize(&InterruptController, INTC_DEVICE_ID);
	if(Status != XST_SUCCESS) {
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
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts, specify real mode so that
	 * the SPI can cause interrupts through the interrupt controller.
	 */
	Status = XIntc_Start(&InterruptController, XIN_REAL_MODE);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupt for the SPI.
	 */
	XIntc_Enable(&InterruptController, SPI_INTR_ID);

	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			 (Xil_ExceptionHandler)XIntc_InterruptHandler,
			 &InterruptController);

	/*
	 * Enable non-critical exceptions.
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
