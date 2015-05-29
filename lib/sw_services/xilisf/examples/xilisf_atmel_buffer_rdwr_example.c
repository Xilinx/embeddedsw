/******************************************************************************
*
* Copyright (C) 2012 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
* @file xilisf_atmel_buffer_rdwr_example.c
*
*
* This file contains a design example using the Xilinx In-system and Serial
* Flash Library (XilIsf). This example shows the Buffer Write, Buffer to Page
* Program with Built-in Erase, Page to Buffer Transfer and Buffer Read features.
*
* This example
* - Writes to a SRAM page buffer using Buffer Write feature
* - Erase a Page and data is transferred to this page from the SRAM page buffer
*   using the Buffer to Page Program with Built-in Erase feature.
* - Transfers the data to a SRAM Page using Page to Buffer Transfer feature.
* - Reads back the Buffer that is written using the Buffer Read feature and
*   compares the data.
*
* The example works for AT45DB011D/AT45DB021D/AT45DB041D/AT45DB081D Serial Flash
* devices. The bytes per page (ISF_PAGE_SIZE) in these devices is 264 for
* Default addressing mode and 256 in Power-of-2 addressing mode.
*
* For AT45DB161D/AT45DB321D the Bytes Per Page (ISF_PAGE_SIZE) is 528 for
* Default addressing mode and 512 in Power-Of-2 addressing mode.
*
* For AT45DB642D the Bytes Per Page (ISF_PAGE_SIZE) is 1056 for Default
* addressing mode and 1024 in Power-Of-2 addressing mode.
*
* The ISF_PAGE_SIZE should be defined by the user according to the Device used.
*
* For further details of each device refer to the Spartan-3AN Serial Flash User
* Guide and data sheets of Atmel AT45XXXD .
*
* This example has been tested with the In-system Flash Memory available on the
* Spartan-3AN on a Xilinx Spartan-3AN Starter Kit board.
*
* @note
*
* None.
*
*<pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- -------  -------- ---------------------------------------------------
* 1.00a ksu/sdm  03/22/08 First release
* 2.00a ktn  	 11/22/09 Updated to use HAL processor APIs.
* 5.0   sb   	 08/05/14 Registering to Xilisf Interrupt handler
*		      	  instead of driver handler.
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_types.h"
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
#define SPI_DEVICE_ID		XPAR_SPI_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define SPI_INTR_ID		XPAR_INTC_0_SPI_0_VEC_ID

/*
 * The following constant defines the slave select signal that is used to
 * to select the Serial Flash device on the SPI bus, this signal is typically
 * connected to the chip select of the device.
 */
#define ISF_SPI_SELECT 		0x01

/*
 * Page size of the Serial Flash.
 */
#define ISF_PAGE_SIZE		264

/*
 * Address of the page to perform Erase, Write and Read operations.
 */
#define TEST_ADDRESS		0xA3600

#define ISF_TEST_BYTE		0x28 /* Test Byte offset value written to Serial
					Flash */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static int IsfWaitForFlashNotBusy();

static int SetupInterruptSystem(XSpi *SpiPtr);

void SpiHandler(void *CallBackRef, u32 StatusEvent, u16 ByteCount);

/************************** Variable Definitions *****************************/

/*
 * The instances to support the device drivers are global such that the
 * are initialized to zero each time the program runs.
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
u8 IsfWriteBuffer[ISF_PAGE_SIZE + XISF_CMD_SEND_EXTRA_BYTES];

/*
 * Buffers used during read and write transactions.
 */
u8 ReadBuffer[ISF_PAGE_SIZE + XISF_CMD_SEND_EXTRA_BYTES] ; /* Read Buffer */
u8 WriteBuffer[ISF_PAGE_SIZE]; 				   /* Write buffer */

/*****************************************************************************/
/**
*
* Main function to execute the Atmel Serial Flash Buffer Read/Write example.
*
* @param	None
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int main(void)
{
	int Status;
	u16 Index;
	XIsf_BufferWriteParam BufferWriteParam;
	XIsf_BufferToFlashWriteParam BufferToFlashWriteParam;
	XIsf_FlashToBufTransferParam FlashToBufTransferParam;
	XIsf_BufferReadParam BufferReadParam;

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
	Status = XSpi_Start(&Spi);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Initialize the In-system and Serial Flash Library.
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
	 * Set the
	 * - SRAM page buffer number where the data is to be written.
	 * - Byte offset within SRAM page buffer where the data is to be written
	 * - The number of bytes to be written.
	 * - The Write Buffer with the data to be written.
	 */
	BufferWriteParam.BufferNum = XISF_PAGE_BUFFER1;
	BufferWriteParam.ByteOffset = 0;
	BufferWriteParam.NumBytes = ISF_PAGE_SIZE;
	BufferWriteParam.WritePtr = WriteBuffer;

	/*
	 * Prepare the write buffer. Fill in the data need to be written into
	 * Serial Flash.
	 */
	for(Index = 0; Index < ISF_PAGE_SIZE; Index++) {
		BufferWriteParam.WritePtr[Index] = Index + ISF_TEST_BYTE;
	}

	/*
	 * Write to SRAM Page buffer.
	 */
	TransferInProgress = TRUE;
	Status = XIsf_Write(&Isf, XISF_BUFFER_WRITE, (void*) &BufferWriteParam);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till the Transfer is complete and check for any errors.
	 */
	while(TransferInProgress);
	if(ErrorCount != 0) {
		return XST_FAILURE;
	}


	/*
	 * Set the
	 * - SRAM page buffer number from where the data is to be written.
	 * - Page Address in the Serial Flash where the data is to be written.
	 */
	BufferToFlashWriteParam.BufferNum = XISF_PAGE_BUFFER1;
	BufferToFlashWriteParam.Address = TEST_ADDRESS;

	/*
	 * Perform Erase then Write to Serial Flash from the SRAM page buffer.
	 */
	TransferInProgress = TRUE;
	Status = XIsf_Write(&Isf, XISF_BUF_TO_PAGE_WRITE_WITH_ERASE,
					(void*) &BufferToFlashWriteParam);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till the Transfer is complete and check for any errors.
	 */
	while(TransferInProgress);
	if(ErrorCount != 0) {
		return XST_FAILURE;
	}

	/*
	 * Wait till the Serial Flash is ready to accept next command.
	 */
	Status = IsfWaitForFlashNotBusy();
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the
	 * - SRAM page buffer number to which the data is to be read.
	 * - Set the Page Address from where the data is to be transferred.
	 */
	FlashToBufTransferParam.BufferNum = XISF_PAGE_BUFFER1;
	FlashToBufTransferParam.Address = TEST_ADDRESS;

	/*
	 *  Transfer data from the Serial Flash to the SRAM page buffer.
	 */
	TransferInProgress = TRUE;
	Status = XIsf_Read(&Isf, XISF_PAGE_TO_BUF_TRANS,
					(void*) &FlashToBufTransferParam);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till the Transfer is complete and check for any errors.
	 */
	while(TransferInProgress);
	if(ErrorCount != 0) {
		return XST_FAILURE;
	}

	/*
	 * Wait till the Serial Flash is ready to accept next command.
	 */
	Status = IsfWaitForFlashNotBusy();
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the
	 * - SRAM page buffer number where the data is to be read.
	 * - Byte offset within SRAM page buffer where the data is to be read.
	 * - The number of bytes to be read.
	 * - The Read Buffer to which with the data is to be read.
	 */
	BufferReadParam.BufferNum = XISF_PAGE_BUFFER1;
	BufferReadParam.ByteOffset = 0;
	BufferReadParam.NumBytes = ISF_PAGE_SIZE;
	BufferReadParam.ReadPtr = ReadBuffer;

	/*
	 * Read data from SRAM page buffer.
	 */
	TransferInProgress = TRUE;
	Status = XIsf_Read(&Isf, XISF_BUFFER_READ, (void*) &BufferReadParam);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till the Transfer is complete and check for any errors.
	 */
	while(TransferInProgress);
	if(ErrorCount != 0) {
		return XST_FAILURE;
	}

	/*
	 * Compare the data read against the data written.
	 */
	for(Index = 0; Index < ISF_PAGE_SIZE; Index++) {
		if(ReadBuffer[Index + XISF_CMD_SEND_EXTRA_BYTES] !=
				(u8)(Index + ISF_TEST_BYTE)) {
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function waits till the Atmel Serial Flash is ready to accept next
* command.
*
* @param	None
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static int IsfWaitForFlashNotBusy()
{
	int Status;
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
		 * Wait till the Transfer is complete and check for any errors.
		 */
		while(TransferInProgress);
		if(ErrorCount != 0) {
			return XST_FAILURE;
		}

		/*
		 * Check if the Serial Flash is ready to accept the next
		 * command. If so break.
		 */
		StatusReg = ReadBuffer[1];
		if(StatusReg & XISF_SR_IS_READY_MASK) {
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

	int Status;

	/*
	 * Initialize the interrupt controller driver so that it's ready to use,
	 * specify the device ID that is generated in xparameters.h
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
	 * all devices that cause interrupts, specify real mode so that the SPI
	 * can cause interrupts through the interrupt controller.
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
