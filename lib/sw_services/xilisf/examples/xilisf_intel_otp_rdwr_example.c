/******************************************************************************
* Copyright (c) 2012 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************
*
* @file xilisf_intel_otp_rdwr_example.c
*
* This file contains a design example using the Xilinx In-system and Serial
* Flash Library (XilIsf). This example shows the One-Time Programmable (OTP)
* Read/Write features.
*
* This example
* - Writes a Byte to OTP region
* - Reads back the Byte written to the OTP region and compares the data
*
* This example has been tested with Intel (Numonyx) Serial Flash Memory (S33)
* on a S3A-DSP starter kit. For further details about the S33 Flash device refer
* to the Intel (Numonyx) Serial Flash Memory (S33) Data sheets.
*
*
* @note
*
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a sdm  03/24/08 First release
* 2.00a ktn  11/22/09 Updated to use HAL processor APIs.
* 5.0   sb   08/05/14 Registering to Xilisf Interrupt handler
*		      instead of driver handler.
* 5.14  akm  08/01/19 Initialized Status variable to XST_FAILURE.
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
 * Number of bytes to be written to the OTP area of the Serial Flash.
 */
#define ISF_OTP_WRITE_SIZE	256

/*
 * Address of the page to perform Erase, Write and Read operations.
 */
#define ISF_OTP_TEST_ADDRESS	0x0225;
#define ISF_TEST_BYTE		0x30 /* Test Byte offset value written */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static int IsfWaitForFlashNotBusy(void);
void SpiHandler(void *CallBackRef, u32 StatusEvent, u16 ByteCount);
static int SetupInterruptSystem(XSpi *SpiPtr);
static int IsfIntelFlashExample();

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
u8 IsfWriteBuffer[ISF_OTP_WRITE_SIZE + XISF_OTP_RDWR_EXTRA_BYTES];

/*
 * Buffers used during read and write transactions.
 */
u8 ReadBuffer[ISF_OTP_WRITE_SIZE + XISF_OTP_RDWR_EXTRA_BYTES]; /* Read Buffer */
u8 WriteBuffer[ISF_OTP_WRITE_SIZE];			/* Write Buffer */

/************************** Function Definitions ******************************/

/*****************************************************************************/
/**
*
* Main function to call the Intel OTP Read/Write Flash.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;

	xil_printf("Intel OTP Read/Write Flash Interrupt example\r\n");

	/*
	 * Run the Intel flash example.
	 */
	Status = IsfIntelFlashExample();
	if (Status != XST_SUCCESS) {
		xil_printf("Intel OTP Read/Write Flash example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Intel OTP Read/Write Flash example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Function to execute the Intel OTP Read/Write Flash example.
*
* @param	None
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None
*
******************************************************************************/
static int IsfIntelFlashExample()
{
	int Status = XST_FAILURE;
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
	 * Specify the address in the OTP region of the Serial Flash for the
	 * Write/Read operations.
	 */
	Address = ISF_OTP_TEST_ADDRESS;

	/*
	 * Perform the Write Enable operation.
	 */
	TransferInProgress = TRUE;
	Status = XIsf_WriteEnable(&Isf, XISF_WRITE_ENABLE);
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
	 * Wait till the Flash is not Busy.
	 */
	Status = IsfWaitForFlashNotBusy();
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the
	 * - Address within the OTP region of the Serial Flash where the data is
	 *   to be written.
	 * - The number of bytes to be written to the Serial Flash, this is
	 *   always 1 as only 1 byte can be written.
	 * - Write Buffer which contains the data to be written to the OTP
	 *   region of the Serial Flash.
	 */
	WriteParam.Address = Address;
	WriteParam.NumBytes = ISF_OTP_WRITE_SIZE;
	WriteParam.WritePtr = WriteBuffer;

	/*
	 * Prepare the write buffer. Fill in the data need to be written into
	 * the OTP region of the Serial Flash.
	 */
	WriteParam.WritePtr[BYTE1] = ISF_TEST_BYTE;

	/*
	 * Perform the OTP Write operation.
	 */
	TransferInProgress = TRUE;
	Status = XIsf_Write(&Isf, XISF_OTP_WRITE, (void*) &WriteParam);
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
	 * Wait till the Flash is not Busy.
	 */
	Status = IsfWaitForFlashNotBusy();
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the
	 * - Address in the OTP region of the Serial Flash where the data is to
	 *   be read from.
	 * - Number of bytes to be read from the Serial Flash.
	 * - Read Buffer to which the data is to be read.
	 */
	ReadParam.Address = Address;
	ReadParam.NumBytes = ISF_OTP_WRITE_SIZE;
	ReadParam.ReadPtr = ReadBuffer;

	/*
	 * Perform the OTP read operation.
	 */
	TransferInProgress = TRUE;
	Status = XIsf_Read(&Isf, XISF_OTP_READ, (void*) &ReadParam);
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
	 * Compare the data read against the data Written.
	 */
	if(ReadBuffer[BYTE1 + XISF_OTP_RDWR_EXTRA_BYTES] !=
					(u8)(ISF_TEST_BYTE)) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function waits till the Intel serial Flash is ready to accept the next
* command.
*
* @param	None
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		This function reads the Status Register of the Serial Flash and
*		waits till the WIP bit of the Status Register becomes 0.
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
		 * Wait till the Transfer is complete and check for any errors.
		 */
		while(TransferInProgress);
		if(ErrorCount != 0) {
			return XST_FAILURE;
		}

		/*
		 * Check if the flash is ready to accept the next command.
		 * If so break.
		 */
		StatusReg = ReadBuffer[BYTE2];
		if((StatusReg & XISF_SR_IS_READY_MASK) == 0) {

			/*
			 * If there were errors return XST_FAILURE.
			 */
			if (StatusReg & (XISF_SR_PROG_FAIL_MASK |
						XISF_SR_ERASE_FAIL_MASK)) {
				return XST_FAILURE;
			}
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
