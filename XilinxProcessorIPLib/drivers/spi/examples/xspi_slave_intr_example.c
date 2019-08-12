/******************************************************************************
*
* Copyright (C) 2008 - 2019 Xilinx, Inc.  All rights reserved.
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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
* @file xspi_slave_intr_example.c
*
*
* This file contains a design example using the Spi driver (XSpi) and the Spi
* device as a Slave, in interrupt mode.
*
* This example fills the Spi Tx buffer with the number of data bytes it expects
* to receive from the master and then Spi device waits for an external master to
* initiate the transfer. When the master initiates the transfer, the Spi device
* receives data from the master and simultaneously sends the data in Tx buffer
* to the master. Once the transfer is complete, a transfer complete interrupt is
* generated and this example prints the data received from the master. The
* number of bytes to be received by the Spi slave is defined by the constant
* BUFFER_SIZE in this file.
*
* The external SPI devices that are present on the Xilinx boards don't support
* the Master functionality. This example has been tested with Aardvark I2C/SPI
* Host Adapter, an off board external SPI Master device and the Xilinx SPI
* device configured as a Slave. This example has been tested for byte-wide SPI
* transfers.
*
* @note
*
* This example assumes that there is a STDIO device in the system.
*
*<pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00a psk  09/05/08 First Release
* 3.00a ktn  10/22/09 Converted all register accesses to 32 bit access.
*		      Updated to use the HAL APIs/macros. Replaced call to
*		      XSpi_Initialize API with XSpi_LookupConfig and
*		      XSpi_CfgInitialize.
* 3.01a sdm  04/23/10 Enabled DTR Half_empty interrupt so that Tx FIFO is
*		      not empty during a transfer in slave mode.
* 4.2   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
*
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"	/* XPAR parameters */
#include "xspi.h"		/* SPI device driver */
#include "xintc.h"		/* Interrupt controller device driver */
#include "stdio.h"
#include "xil_exception.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define SPI_DEVICE_ID		XPAR_SPI_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define SPI_IRPT_INTR		XPAR_INTC_0_SPI_0_VEC_ID

/*
 * This is the size of the buffer to be transmitted/received in this example.
 */
#define BUFFER_SIZE		32

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

static int SpiSlaveIntrExample(XSpi *SpiInstancePtr, u16 SpiDeviceId);

static int SetupInterruptSystem(XSpi *SpiInstance);

static void SpiHandler(void *CallBackRef, u32 StatusEvent,
		       unsigned int ByteCount);

/************************** Variable Definitions *****************************/
/*
 * The instances to support the device drivers are global such that they are
 * initialized to zero each time the program runs. They could be local but
 * should at least be static so that they are zeroed.
 */
static XSpi  SpiInstance;   /* Instance of the SPI device */
static XIntc IntcInstance;  /* Instance of the Interrupt controller device */

/*
 * The following variables are used to read/write from the  Spi device, these
 * are global to avoid having large buffers on the stack.
 */
u8 ReadBuffer[BUFFER_SIZE];
u8 WriteBuffer[BUFFER_SIZE];

/*
 * The following variable allows a test value to be added to the values that
 * are sent in reflection to the Master transmission such that unique values can
 * be generated to guarantee the transfer from Slave to Master is successful.
 */
int Test;

/*
 * The following variables are shared between non-interrupt processing and
 * interrupt processing such that they must be global.
 */
static volatile int TransferInProgress;

/*****************************************************************************/
/**
*
* Main function to call the Spi Slave example in interrupt mode.
*
* @param	None
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int main(void)
{
	int Status;

	/*
	 * Run the Spi Slave interrupt example.
	 */
	Status = SpiSlaveIntrExample(&SpiInstance, SPI_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Spi slave interrupt Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Spi slave interrupt Example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function does a minimal test on the Spi device and driver as a design
* example. The purpose of this function is to illustrate the device slave
* functionality in interrupt mode. This function receives data from a master and
* prints the received data.
*
* @param	SpiInstancePtr is a pointer to the instance of Spi component.
* @param	SpiDeviceId is the Device ID of the Spi Device and is the
*		XPAR_<SPI_instance>_DEVICE_ID value from xparameters.h.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		This function contains an infinite loop such that if the Spi
*		device doesn't receive any data or if the interrupts are not
*		working, it may never return.
*
******************************************************************************/
static int SpiSlaveIntrExample(XSpi *SpiInstancePtr, u16 SpiDeviceId)
{
	XSpi_Config *ConfigPtr;
	int Status;
	u32 Count;

	xil_printf("\r\nEntering the Spi Slave Interrupt Example.\r\n");
	xil_printf("Waiting for data from SPI master\r\n");

	/*
	 * Initialize the SPI driver so that it's ready to use, specify the
	 * device ID that is generated in xparameters.h.
	 */
	ConfigPtr = XSpi_LookupConfig(SpiDeviceId);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}

	Status = XSpi_CfgInitialize(SpiInstancePtr, ConfigPtr,
			ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the SPI driver to the interrupt subsystem such that
	 * interrupts can occur. This function is application specific.
	 */
	Status = SetupInterruptSystem(SpiInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the handler for the SPI that will be called from the interrupt
	 * context when an SPI status occurs, specify a pointer to the SPI
	 * driver instance as the callback reference so the handler is able to
	 * access the instance data.
	 */
	XSpi_SetStatusHandler(SpiInstancePtr,SpiInstancePtr,(XSpi_StatusHandler)
			      SpiHandler);

	/*
	 * The SPI device is a slave by default and the clock phase and polarity
	 * have to be set according to its master. In this example, CPOL is set
	 * to active low and CPHA is set to 1.
	 */
	Status = XSpi_SetOptions(SpiInstancePtr, XSP_CLK_PHASE_1_OPTION |
				 XSP_CLK_ACTIVE_LOW_OPTION);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the SPI driver so that the device is enabled.
	 */
	XSpi_Start(SpiInstancePtr);

	/*
	 * Enable the DTR half-empty interrupt while transferring more than
	 * FIFO_DEPTH number of bytes in slave mode, so that the Tx FIFO
	 * is never empty during a transfer. If the Tx FIFO is empty during
	 * a transfer, it results in master receiving invalid data.
	 */
	XSpi_IntrEnable(SpiInstancePtr, XSP_INTR_TX_HALF_EMPTY_MASK);

	/*
	 * Initialize the write buffer with pattern to write, initialize the
	 * read buffer to zero so it can be verified after the read.
	 */
	Test = 0x50;
	for (Count = 0; Count < BUFFER_SIZE; Count++) {
		WriteBuffer[Count] = (u8)(Count + Test);
		ReadBuffer[Count] = 0;
	}

	/*
	 * Transmit data as a slave, when the master starts sending data.
	 */
	TransferInProgress = TRUE;
	Status = XSpi_Transfer(SpiInstancePtr, WriteBuffer, ReadBuffer,
				BUFFER_SIZE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till the transfer is complete.
	 */
	while (TransferInProgress == TRUE);

	/*
	 * Print all the data received from the master.
	 */
	xil_printf("\r\nReceived data is:\r\n");
	for (Count = 0; Count < BUFFER_SIZE; Count++) {
		xil_printf("0x%x \r\n", ReadBuffer[Count]);
	}

	xil_printf("\r\nExiting the Spi Slave Interrupt Example.\r\n");

	return XST_SUCCESS;
}
/****************************************************************************/
/**
*
* This function setups the interrupt system such that interrupts can occur
* for the SPI driver. This function is application specific since the actual
* system may or may not have an interrupt controller. The SPI device could
* be directly connected to a processor without an interrupt controller.  The
* user should modify this function to fit the application.
*
* @param	SpiInstance contains a pointer to the instance of the XSpi
* 		component which is going to be connected to the interrupt
*		controller.
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
static int SetupInterruptSystem(XSpi *SpiInstance)
{
	int Status;

	/*
	 * Initialize the interrupt controller driver so that it's ready to use,
	 * specify the device ID that is generated in "xparameters.h".
	 */
	Status = XIntc_Initialize(&IntcInstance, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the device occurs, the device driver handler performs the
	 * specific interrupt processing for the device.
	 */
	Status = XIntc_Connect(&IntcInstance,
				SPI_IRPT_INTR,
				(XInterruptHandler)XSpi_InterruptHandler,
				(void *)SpiInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts, specific real mode so that the SPI
	 * can cause interrupts through the interrupt controller.
	 */
	Status = XIntc_Start(&IntcInstance, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupt for the Spi device.
	 */
	XIntc_Enable(&IntcInstance, SPI_IRPT_INTR);


	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler)XIntc_InterruptHandler,
				&IntcInstance);

	/*
	 * Enable non-critical exceptions.
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

/******************************************************************************/
/**
*
* This function is the handler which performs processing for the SPI driver. It
* is called from an interrupt context such that the amount of processing to be
* performed should be minimized. This handler provides an example of how to
* handle SPI interrupts but is application specific.
*
* @param 	CallBackRef is a reference passed to the handler.
* @param	StatusEvent is the status of the SPI.
* @param	ByteCount is the number of bytes transferred.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void SpiHandler(void *CallBackRef, u32 StatusEvent,
		       unsigned int ByteCount)
{
	if (StatusEvent == XST_SPI_TRANSFER_DONE) {
		/*
		 * Indicate the transfer on the SPI bus is no longer in
		 * progress.
		 */
		TransferInProgress = FALSE;
	}
}
