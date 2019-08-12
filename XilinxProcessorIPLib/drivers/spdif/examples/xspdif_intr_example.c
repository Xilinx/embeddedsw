/******************************************************************************
 *
 * Copyright (C) 2017 - 2018 Xilinx, Inc. All rights reserved.
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
 *****************************************************************************/
/*****************************************************************************/
/**
 * @file xspsdif_intr_example.c
 *
 * This file contains a example for using the XSpdif hardware device
 * and XSpdif driver using interrupt mode.This example assumes
 * that the interrupt controller is also present as a part of the system.
 *
 *
 * <pre> MODIFICATION HISTORY:
 *
 * Ver	Who	Date		Changes
 *----	---	--------	----------------------------------------------
 * 1.0	kar	01/25/18	First release
 * </pre>
 *
 ****************************************************************************/

/***************************** Include Files **********************************/
#include "xparameters.h"
#include "xspdif.h"
#include "xspdif_hw.h"
#include "xil_printf.h"

#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
#include <stdio.h>
#else
#include "xscugic.h"
#endif

/************************** Constant Definitions ******************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are only defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define SPDIF_0_DEVICE_ID	XPAR_XSPDIF_0_DEVICE_ID
#define SPDIF_1_DEVICE_ID	XPAR_XSPDIF_1_DEVICE_ID

#define SPDIF_0_INTERRUPT_ID	XPAR_INTC_0_SPDIF_0_VEC_ID
#define XPAR_SPDIF_0_BA		XPAR_XSPDIF_0_BASEADDR

#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#else
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#endif

#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC		XIntc
#define INTC_HANDLER	XIntc_InterruptHandler
#else
#define INTC			XScuGic
#define INTC_HANDLER	XScuGic_InterruptHandler
#endif /* XPAR_INTC_0_DEVICE_ID */

#define SPDIF_TIME_OUT 500000

/**************************** Type Definitions ********************************/

/************************** Function Prototypes *******************************/

int SpdifSelfTestExample(u16 DeviceId);

int SpdifIntrExample(INTC *IntcInstancePtr, XSpdif *SpdifInstancePtr,
		u16 DeviceId, u16 IntrId);

static int SpdifSetupIntrSystem(INTC *IntcInstancePtr,
		XSpdif *SpdifInstancePtr,
		u16 DeviceId, u16 IntrId);

void SpdifDisableIntr(INTC *IntcInstancePtr, u16 IntrId);

void SpdifStartOfBlockHandler(void *CallBackRef);

void SpdifPreambleErrHandler(void *CallBackRef);

void SpdifBmcErrHandler(void *CallBackRef);

void SpdifTxOrRxFifoEmptyHandler(void *CallBackRef);

void SpdifTxOrRxFifoFullHandler(void *CallBackRef);

/************************** Variable Definitions ******************************/

INTC InterruptController;	/* The instance of the Interrupt Controller */
XSpdif SpdifInstance;		/* Instance of the Spdif device */
unsigned int IntrReceived;

/*****************************************************************************/
/**
 * This function is the main function of the Spdif example
 * using Interrupts.
 *
 * @param	None.
 *
 * @return	XST_SUCCESS to indicate success, else XST_FAILURE to indicate a
 *		Failure.
 *
 * @note	None.
 *
 ******************************************************************************/
int main(void)
{
	int Status;
	/*
	 * Run the Spdif - Interrupt example.
	 */
	Status = SpdifIntrExample(&InterruptController, &SpdifInstance,
			SPDIF_0_DEVICE_ID, SPDIF_0_INTERRUPT_ID);

	if (Status != XST_SUCCESS) {
		xil_printf("Spdif interrupt Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Spdif interrupt Example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function does a minimal test on the Spdif device and driver
 * as a  design example.  The purpose of this function is to illustrate how
 * to use the Xspdif component.  It initializes Spdif and then
 * sets it up such that a interrupt is generated.
 *
 * This function uses interrupt driven mode of the Spdif.
 *
 * @param	IntcInstancePtr is a pointer to the Interrupt Controller
 *		driver Instance
 * @param	Spdif InstancePtr is a pointer to the Xspdif driver Instance
 * @param	DeviceId is the XPAR_<spdif_instance>_DEVICE_ID value from
 *		xparameters.h
 * @param	IntrId is XPAR_<INTC_instance>_<spdif_instance>_INTERRUPT_INTR
 *		value from xparameters.h
 *
 * @return	XST_SUCCESS if the Test is successful, otherwise XST_FAILURE
 *
 * @note	This function contains an infinite loop such that if interrupts
 *		are not working it may never return.
 *
 *****************************************************************************/
int SpdifIntrExample(INTC *IntcInstancePtr, XSpdif *SpdifInstancePtr,
		u16 DeviceId, u16 IntrId)
{
	int Status;
	XSpdif_Config *Config;
	unsigned int IntrCount = 0;
	/*
	 * Lookup and Initialize the Spdif so that it's ready to use.
	 */
	Config = XSpdif_LookupConfig(DeviceId);
	if (Config == NULL)
		return XST_FAILURE;

	Status = XSpdif_CfgInitialize(SpdifInstancePtr, Config,
			Config->BaseAddress);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;
	/*
	 * Connect Spdif to the interrupt
	 * subsystem such that interrupts can occur.
	 * This function is application specific.
	 */
	Status = SpdifSetupIntrSystem(IntcInstancePtr, SpdifInstancePtr,
			DeviceId, IntrId);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;
	/*
	 * Setup the handler for the Spdif that will be called from the
	 * interrupt context when the Spdif receives a interrupt.
	 */
	XSpdif_SoftReset(SpdifInstancePtr);

	XSpdif_SetHandler(SpdifInstancePtr, XSPDIF_HANDLER_START_OF_BLOCK,
			SpdifStartOfBlockHandler,
			(void *)SpdifInstancePtr);

	XSpdif_SetHandler(SpdifInstancePtr, XSPDIF_HANDLER_PREAMBLE_ERROR,
			SpdifPreambleErrHandler,
			(void *)SpdifInstancePtr);

	XSpdif_SetHandler(SpdifInstancePtr, XSPDIF_HANDLER_BMC_ERROR,
			SpdifBmcErrHandler,
			(void *)SpdifInstancePtr);

	XSpdif_SetHandler(SpdifInstancePtr, XSPDIF_HANDLER_TX_OR_RX_FIFO_EMPTY,
			SpdifTxOrRxFifoEmptyHandler,
			(void *)SpdifInstancePtr);

	XSpdif_SetHandler(SpdifInstancePtr, XSPDIF_HANDLER_TX_OR_RX_FIFO_FULL,
			SpdifTxOrRxFifoFullHandler,
			(void *)SpdifInstancePtr);

	XSpdif_IntrEnable(SpdifInstancePtr, XSPDIF_START_OF_BLOCK_MASK);

	XSpdif_IntrEnable(SpdifInstancePtr, XSPDIF_BMC_ERROR_MASK);

	XSpdif_IntrEnable(SpdifInstancePtr, XSPDIF_PREAMBLE_ERROR_MASK);

	XSpdif_IntrEnable(SpdifInstancePtr, XSPDIF_TX_OR_RX_FIFO_EMPTY_MASK);

	XSpdif_IntrEnable(SpdifInstancePtr, XSPDIF_TX_OR_RX_FIFO_FULL_MASK);

	XSpdif_Global_IntEnable(SpdifInstancePtr);

	XSpdif_ResetFifo(SpdifInstancePtr);

	XSpdif_SetClkConfig(SpdifInstancePtr, XSPDIF_CLK_8);

	XSpdif_Enable(SpdifInstancePtr, TRUE);

	Xil_ExceptionEnable();
	while (IntrCount < SPDIF_TIME_OUT) {
		/* Wait until an interrupts has been received. */
		if (IntrReceived == 1) {
			/* Disable SPDIF */
			XSpdif_Enable(SpdifInstancePtr, 0x0);
			Status =  XST_SUCCESS;
			break;
		}
		IntrCount++;
	}

	return Status;
}

/*****************************************************************************/
/**
 * This function is the handler which performs processing for the Spdif
 * It is called from an interrupt context when the Spdif receives a
 * start of block Interrupt.
 *
 * This handler provides an example of how to handle Spdif interrupts
 * but is application specific.
 *
 * @param	CallBackRef is a pointer to the callback function
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void SpdifStartOfBlockHandler(void *CallBackRef)
{
	XSpdif *InstancePtr = (XSpdif *)CallBackRef;
	/* Set the interrupt received flag. */
	IntrReceived = 1;
}

/*****************************************************************************/
/**
 * This function is the handler which performs processing for the Spdif
 * It is called from an interrupt context when the Spdif receives a
 * Preamble Error Interrupt.
 *
 * This handler provides an example of how to handle Spdif interrupts
 * but is application specific.
 *
 * @param	CallBackRef is a pointer to the callback function
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void SpdifPreambleErrHandler(void *CallBackRef)
{
	XSpdif *InstancePtr = (XSpdif *)CallBackRef;
	/* Set the interrupt received flag. */
	IntrReceived = 1;
}

/*****************************************************************************/
/**
 * This function is the handler which performs processing for the Spdif
 * It is called from an interrupt context when the Spdif receives a
 * BMC error Interrupt.
 *
 * This handler provides an example of how to handle Spdif interrupts
 * but is application specific.
 *
 * @param	CallBackRef is a pointer to the callback function
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void SpdifBmcErrHandler(void *CallBackRef)
{
	XSpdif *InstancePtr = (XSpdif *)CallBackRef;
	/* Set the interrupt received flag. */
	IntrReceived = 1;
}

/*****************************************************************************/
/**
 * This function is the handler which performs processing for the Spdif
 * It is called from an interrupt context when the Spdif receives a
 * Transmitter or Receiver Fifo Empty Interrupt.
 *
 * This handler provides an example of how to handle Spdif interrupts
 * but is application specific.
 *
 * @param	CallBackRef is a pointer to the callback function
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void SpdifTxOrRxFifoEmptyHandler(void *CallBackRef)
{
	XSpdif *InstancePtr = (XSpdif *)CallBackRef;
	/* Set the interrupt received flag. */
	IntrReceived = 1;
}

/*****************************************************************************/
/**
 * This function is the handler which performs processing for the Spdif
 * It is called from an interrupt context when the Spdif receives a
 * Transmitter or Receiver Fifo Full Interrupt.
 *
 * This handler provides an example of how to handle Spdif interrupts
 * but is application specific.
 *
 * @param	CallBackRef is a pointer to the callback function
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void SpdifTxOrRxFifoFullHandler(void *CallBackRef)
{
	XSpdif *InstancePtr = (XSpdif *)CallBackRef;
	/* Set the interrupt received flag. */
	IntrReceived = 1;
}

/*****************************************************************************/
/**
 * This function setups the interrupt system such that interrupts can occur
 * for the Spdif. This function is application specific since the actual
 * system may or may not have an interrupt controller.  The Spdif could
 * be directly connected to a processor without an interrupt controller.  The
 * user should modify this function to fit the application.
 *
 * @param	IntcInstancePtr is a pointer to the Interrupt Controller
 *		driver Instance.
 * @param	SpdifInstancePtr is a pointer to the XSpdif driver Instance.
 * @param	DeviceId is the XPAR_<spdif_instance>_DEVICE_ID value from
 *		xparameters.h.
 * @param	IntrId is XPAR_<INTC_instance>_<spdif_instance>_VEC_ID
 *		value from xparameters.h.
 *
 * @return	XST_SUCCESS if the Test is successful, otherwise XST_FAILURE.
 *
 * @note	This function contains an infinite loop such that if interrupts
 *		are not working it may never return.
 *
 ******************************************************************************/
static int SpdifSetupIntrSystem(INTC *IntcInstancePtr,
		XSpdif *SpdifInstancePtr,
		u16 DeviceId, u16 IntrId)
{
	int Status;

#ifdef XPAR_INTC_0_DEVICE_ID
	/*
	 * Initialize the interrupt controller driver so that
	 * it's ready to use, specify the device ID that is generated in
	 * xparameters.h
	 */
	Status = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;
	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the device occurs, the device driver handler performs the
	 * specific interrupt processing for the device
	 */
	Status = XIntc_Connect(IntcInstancePtr, IntrId,
			(XInterruptHandler)XSpdif_IntrHandler,
			(void *)SpdifInstancePtr);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts, specific real mode so that
	 * the SPDIF can cause interrupts through the interrupt controller.
	 */
	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	/*
	 * Enable the interrupt for the Spdif.
	 */
	XIntc_Enable(IntcInstancePtr, IntrId);

#else

	XScuGic_Config *IntcConfig;

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (IntcConfig == NULL)
		return XST_FAILURE;

	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
			IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	/*
	 * Connect the interrupt handler that will be called when an
	 * interrupt occurs for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, IntrId,
			(Xil_ExceptionHandler)XSpdif_IntrHandler,
			SpdifInstancePtr);
	if (Status != XST_SUCCESS)
		return Status;

	/*
	 * Enable the interrupt for the SPDIF device.
	 */
	XScuGic_Enable(IntcInstancePtr, IntrId);
#endif /* XPAR_INTC_0_DEVICE_ID */

#ifndef TESTAPP_GEN
	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			(Xil_ExceptionHandler) INTC_HANDLER,
			IntcInstancePtr);
	/*
	 * Enable non-critical exceptions.
	 */
#endif
	return XST_SUCCESS;
}
/******************************************************************************/
/**
 *
 * This function disables the interrupts for the Spdif.
 *
 * @param	IntcInstancePtr is a reference to the Interrupt Controller
 *		driver Instance.
 * @param	IntrId is XPAR_<INTC_instance>_<spdif_instance>_VEC_ID
 *		value from xparameters.h.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void SpdifDisableIntr(INTC *IntcInstancePtr, u16 IntrId)
{
	/*
	 * Disable the interrupt for the Spdif
	 */
#ifdef XPAR_INTC_0_DEVICE_ID
	XIntc_Disable(IntcInstancePtr, IntrId);
#else
	/* Disconnect the interrupt */
	XScuGic_Disable(IntcInstancePtr, IntrId);
	XScuGic_Disconnect(IntcInstancePtr, IntrId);
#endif
}

