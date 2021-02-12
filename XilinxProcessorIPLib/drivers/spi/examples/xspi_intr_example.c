/******************************************************************************
* Copyright (C) 2006 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xspi_intr_example.c
*
*
* This file contains a design example using the Spi driver (XSpi) and the Spi
* device using the interrupt mode.
*
* This example works with a PPC/MicroBlaze processor.
*
* @note
*
* None.
*
*<pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.01a sv   05/29/06 First release for Test App Integration
*                     for Interrupt examples
* 1.11a sdm  03/03/08 Minor changes to comply to Doxygen and coding guidelines
* 3.00a ktn  10/22/09 Converted all register accesses to 32 bit access.
*		      Updated to use the HAL APIs/macros. Replaced call to
*		      XSpi_Initialize API with XSpi_LookupConfig and
*		      XSpi_CfgInitialize.
* 3.02a sdm  05/04/11 Updated to run the loopback test only in standard spi
*		      mode.
* 4.2   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* 4.5	akm  07/12/19 Fixed compilation error by passing the correct interrupt
*		      controller instance to SpiIntrExample() function.
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"	/* XPAR parameters */
#include "xspi.h"		/* SPI device driver */
#include "xil_exception.h"
#include "xil_printf.h"

#ifdef XPAR_INTC_0_DEVICE_ID
 #include "xintc.h"
#else
 #include "xscugic.h"
#endif

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef TESTAPP_GEN
#define SPI_DEVICE_ID		XPAR_SPI_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define SPI_IRPT_INTR		XPAR_INTC_0_SPI_0_VEC_ID
#endif

#ifdef XPAR_INTC_0_DEVICE_ID
 #define INTC           XIntc
 #define INTC_HANDLER   XIntc_InterruptHandler
#else
 #define INTC           XScuGic
 #define INTC_HANDLER   XScuGic_InterruptHandler
#endif

/*
 *  This is the size of the buffer to be transmitted/received in this example.
 */
#define BUFFER_SIZE			 12


/**************************** Type Definitions *******************************/

/*
 * The following data type is used to send and receive data on the SPI
 * interface.
 */
typedef u8 DataBuffer[BUFFER_SIZE];


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int SpiIntrExample(INTC *IntcInstancePtr, XSpi *SpiInstancePtr,
					u16 SpiDeviceId, u16
					SpiIntrId);

void SpiIntrHandler(void *CallBackRef, u32 StatusEvent, u32 ByteCount);

static int SpiSetupIntrSystem(INTC *IntcInstancePtr, XSpi *SpiInstancePtr,
					u16 SpiIntrId);

static void SpiDisableIntrSystem(INTC *IntcInstancePtr, u16 SpiIntrId);

/************************** Variable Definitions *****************************/

/*
 * The instances to support the device drivers are global such that the
 * are initialized to zero each time the program runs.
 */
#ifndef TESTAPP_GEN
static INTC Intc;	 /* The instance of the Interrupt Controller */
static XSpi  SpiInstance;	 /* The instance of the SPI device */
#endif

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
 * The following variables are used to read and write to the  Spi device, they
 * are global to avoid having large buffers on the stack.
 */
u8 ReadBuffer[BUFFER_SIZE];
u8 WriteBuffer[BUFFER_SIZE];


/*****************************************************************************/
/**
*
* Main function to call the Spi interrupt example.
*
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

	/*
	 * Run the Spi Interrupt example.
	 */
	Status = SpiIntrExample(&Intc,
				&SpiInstance,
				SPI_DEVICE_ID,
				SPI_IRPT_INTR);
	if (Status != XST_SUCCESS) {
		xil_printf("Spi interrupt Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Spi interrupt Example\r\n");
	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function does a minimal test on the Spi device and driver as a
* design example. The purpose of this function is to illustrate how to use
* the XSpi component using the interrupt mode.
*
* This function sends data and expects to receive the same data.
*
*
* @param	IntcInstancePtr is a pointer to the instance of the INTC
* 		component.
* @param	SpiInstancePtr is a pointer to the instance of Spi component.
* @param	SpiDeviceId is the Device ID of the Spi Device and is the
*		XPAR_<SPI_instance>_DEVICE_ID value from xparameters.h.
* @param	SpiIntrId is the interrupt Id and is typically
*		XPAR_<INTC_instance>_<SPI_instance>_VEC_ID value from
*		xparameters.h .
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note
*
* This function contains an infinite loop such that if interrupts are not
* working it may never return.
*
******************************************************************************/
int SpiIntrExample(INTC *IntcInstancePtr, XSpi *SpiInstancePtr,
			u16 SpiDeviceId, u16 SpiIntrId)
{
	int Status;
	u32 Count;
	u8 Test;
	XSpi_Config *ConfigPtr;	/* Pointer to Configuration data */

	/*
	 * Initialize the SPI driver so that it is  ready to use.
	 */
	ConfigPtr = XSpi_LookupConfig(SpiDeviceId);
	if (ConfigPtr == NULL) {
		return XST_DEVICE_NOT_FOUND;
	}

	Status = XSpi_CfgInitialize(SpiInstancePtr, ConfigPtr,
				  ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	Status = XSpi_SelfTest(SpiInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Run loopback test only in case of standard SPI mode.
	 */
	if (SpiInstancePtr->SpiMode != XSP_STANDARD_MODE) {
		return XST_SUCCESS;
	}

	/*
	 * Connect the Spi device to the interrupt subsystem such that
	 * interrupts can occur. This function is application specific.
	 */
	Status = SpiSetupIntrSystem(IntcInstancePtr,
					SpiInstancePtr,
				 	SpiIntrId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the handler for the SPI that will be called from the interrupt
	 * context when an SPI status occurs, specify a pointer to the SPI
	 * driver instance as the callback reference so the handler is able to
	 * access the instance data.
	 */
	XSpi_SetStatusHandler(SpiInstancePtr, SpiInstancePtr,
		 		(XSpi_StatusHandler) SpiIntrHandler);

	/*
	 * Set the Spi device as a master and in loopback mode.
	 */
	Status = XSpi_SetOptions(SpiInstancePtr, XSP_MASTER_OPTION |
 					XSP_LOOPBACK_OPTION);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Start the SPI driver so that interrupts and the device are enabled.
	 */
	XSpi_Start(SpiInstancePtr);

	/*
	 * Initialize the write buffer with pattern to write, initialize the
	 * read buffer to zero so it can be verified after the read, the
	 * Test value that is added to the unique value allows the value to be
	 * changed in a debug environment.
	 */
	Test = 0x10;
	for (Count = 0; Count < BUFFER_SIZE; Count++) {
		WriteBuffer[Count] = (u8)(Count + Test);
		ReadBuffer[Count] = 0;
	}


	/*
	 * Transmit the data.
	 */
	TransferInProgress = TRUE;
	XSpi_Transfer(SpiInstancePtr, WriteBuffer, ReadBuffer, BUFFER_SIZE);

	/*
	 * Wait for the transmission to be complete.
	 */
	while (TransferInProgress);


	/*
	 * Disable the Spi interrupt.
	 */
	SpiDisableIntrSystem(IntcInstancePtr, SpiIntrId);

	/*
	 * Compare the data received with the data that was transmitted.
	 */
	for (Count = 0; Count < BUFFER_SIZE; Count++) {
		if (WriteBuffer[Count] != ReadBuffer[Count]) {
			return XST_FAILURE;
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
void SpiIntrHandler(void *CallBackRef, u32 StatusEvent, u32 ByteCount)
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
		Error++;
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
* @param	IntcInstancePtr is a pointer to the instance of the Intc device.
* @param	SpiInstancePtr is a pointer to the instance of the Spi device.
* @param	SpiIntrId is the interrupt Id and is typically
*		XPAR_<INTC_instance>_<SPI_instance>_VEC_ID value from
*		xparameters.h
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
******************************************************************************/
static int SpiSetupIntrSystem(INTC *IntcInstancePtr, XSpi *SpiInstancePtr,
					 u16 SpiIntrId)
{
	int Status;

#ifdef XPAR_INTC_0_DEVICE_ID
#ifndef TESTAPP_GEN
	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	Status = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif

	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the device occurs, the device driver handler performs the
	 * specific interrupt processing for the device.
	 */
	Status = XIntc_Connect(IntcInstancePtr, SpiIntrId,
	 			(XInterruptHandler) XSpi_InterruptHandler,
				(void *)SpiInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#ifndef TESTAPP_GEN
	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts, specific real mode so that
	 * the SPI can cause interrupts through the interrupt controller.
	 */
	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
 	   return XST_FAILURE;
	}
#endif

	/*
	 * Enable the interrupt for the SPI device.
	 */
	XIntc_Enable(IntcInstancePtr, SpiIntrId);
#else

	XScuGic_Config *IntcConfig;

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
#ifndef TESTAPP_GEN
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
				IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif
	
	XScuGic_SetPriorityTriggerType(IntcInstancePtr, SpiIntrId, 0xA0, 0x3);

	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, SpiIntrId,
				(Xil_InterruptHandler)XSpi_InterruptHandler,
				SpiInstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	XScuGic_Enable(IntcInstancePtr, SpiIntrId);
#endif

#ifndef TESTAPP_GEN


	
	/* Enable interrupts from the hardware */
	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
		(Xil_ExceptionHandler)INTC_HANDLER,
		(void *)IntcInstancePtr);

	Xil_ExceptionEnable();



#endif /* TESTAPP_GEN */

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function disables the interrupts that occur for the Spi device.
*
* @param	IntcInstancePtr is the pointer to the instance of the INTC
*		component.
* @param	SpiIntrId is the interrupt Id and is typically
*		XPAR_<INTC_instance>_<SPI_instance>_VEC_ID value from
*		xparameters.h
*
* @return	None
*
* @note		None
*
******************************************************************************/
static void SpiDisableIntrSystem(INTC *IntcInstancePtr, u16 SpiIntrId)
{

	/*
	 * Disconnect and disable the interrupt for the Spi device.
	 */
#ifdef XPAR_INTC_0_DEVICE_ID
	/* Disconnect the interrupts for the Master complete and error */
	XIntc_Disconnect(IntcInstancePtr, SpiIntrId);
#else
	XScuGic_Disconnect(IntcInstancePtr, SpiIntrId);
#endif

	
}
