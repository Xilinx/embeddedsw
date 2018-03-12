/******************************************************************************
 *
 * Copyright (C) 2017 Xilinx, Inc. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"),to deal
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
 * XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice,the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
 ****************************************************************************/
/*****************************************************************************/
/**
 * @file xi2srx_intr_example.c
 *
 * This file contains a example for using the I2S receiver hardware device
 * and I2S receiver driver using interrupt mode.This example assumes
 * that the interrupt controller is also present as a part of the system.
 *
 *
 * <pre> MODIFICATION HISTORY:
 *
 * Ver	Who	Date		Changes
 * ----	---	--------	----------------------------------------------
 * 1.0	kar	01/25/18	First release
 *
 * </pre>
 *
 ****************************************************************************/

/***************************** Include Files **********************************/
#include "xparameters.h"
#include "xi2srx.h"
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
#define I2S_RX_DEVICE_ID	XPAR_XI2SRX_0_DEVICE_ID
#define I2S_RX_INTERRUPT_ID	XPAR_FABRIC_I2SRX_0_VEC_ID
#define XPAR_I2S_TRANSMITTER_0_BA XPAR_XI2STX_0_BASEADDR
#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#else
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#endif /* XPAR_INTC_0_DEVICE_ID */

#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC		XIntc
#define INTC_HANDLER	XIntc_InterruptHandler
#else
#define INTC			XScuGic
#define INTC_HANDLER	XScuGic_InterruptHandler
#endif /* XPAR_INTC_0_DEVICE_ID */

#define I2S_RX_FS		48 //kHz
#define I2S_RX_MCLK		(384 * I2S_RX_FS)
#define I2S_RX_TIME_OUT 500000
/**************************** Type Definitions ********************************/

/************************** Function Prototypes *******************************/

int I2sRxIntrExample(INTC *IntcInstancePtr, XI2s_Rx *I2sRxInstancePtr,
		u16 DeviceId, u16 IntrId);

static int I2sRxSetupIntrSystem(INTC *IntcInstancePtr,
		XI2s_Rx *I2sRxInstancePtr,
		u16 DeviceId, u16 IntrId);

void I2sRxDisableIntr(INTC *IntcInstancePtr, u16 IntrId);

void I2sRxAesBlockCmplIntrHandler(void *CallBackRef);

void I2sRxOvrflwIntrHandler(void *CallBackRef);

/************************** Variable Definitions ******************************/
INTC InterruptController;	/* The instance of the Interrupt Controller */
XI2s_Rx I2sRxInstance;		/* Instance of the I2S receiver device */
u32 IntrReceived;
/*****************************************************************************/
/**
 * This function is the main function of the I2S receiver example
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
	 * Run the I2S receiver - Interrupt example.
	 */
	Status = I2sRxIntrExample(&InterruptController, &I2sRxInstance,
			I2S_RX_DEVICE_ID, I2S_RX_INTERRUPT_ID);

	if (Status != XST_SUCCESS) {
		xil_printf("I2S receiver interrupt Example Failed\r\n");
		return XST_FAILURE;
	}
	xil_printf("Successfully ran I2S receiver interrupt Example\r\n");
	return XST_SUCCESS;
}
/*****************************************************************************/
/**
 * This function does a minimal test on the I2S receiver device and driver
 * as a  design example.  The purpose of this function is to illustrate how
 * to use the XI2s_Rx component.  It initializes a I2S receiver and then
 * sets it up such that a periodic interrupt is generated.
 *
 * This function uses interrupt driven mode of the I2S receiver.
 *
 * @param	IntcInstancePtr is a pointer to the Interrupt Controller
 *		driver Instance
 * @param	I2sRxInstancePtr is a pointer to the XI2s_Rx driver Instance
 * @param	DeviceId is the XPAR_<i2s_rx_instance>_DEVICE_ID value from
 *		xparameters.h
 * @param	IntrId is XPAR_<INTC_instance>_<i2s_rx_instance>_INTERRUPT_INTR
 *		value from xparameters.h
 *
 * @return	XST_SUCCESS if the Test is successful, otherwise XST_FAILURE
 *
 *****************************************************************************/
int I2sRxIntrExample(INTC *IntcInstancePtr, XI2s_Rx *I2sRxInstancePtr,
		u16 DeviceId, u16 IntrId)
{
	xil_printf("I2s Receiver\r\n");
	int Status;
	XI2srx_Config *Config;
	u32 IntrCount =0;
	/*
	 * Lookup and Initialize the I2S receiver so that it's ready to use.
	 */
	Config = XI2s_Rx_LookupConfig(DeviceId);
	if (Config == NULL)
		return XST_FAILURE;

	Status = XI2s_Rx_CfgInitialize(I2sRxInstancePtr, Config,
			Config->BaseAddress);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;
	/*
	 * Connect the I2S receiver counter to the interrupt
	 * subsystem such that interrupts can occur.
	 * This function is application specific.
	 */
	Status = I2sRxSetupIntrSystem(IntcInstancePtr, I2sRxInstancePtr,
			DeviceId, IntrId);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	/*
	 * Setup the handler for the I2S receiver that will be called from the
	 * interrupt context when the I2S receiver receives a block complete
	 * interrupt or Underflow interrupt.
	 */

	XI2s_Rx_SetHandler(I2sRxInstancePtr, XI2S_RX_HANDLER_AES_BLKCMPLT,
			&I2sRxAesBlockCmplIntrHandler,
			(void *)I2sRxInstancePtr);
	XI2s_Rx_SetHandler(I2sRxInstancePtr, XI2S_RX_HANDLER_AUD_OVRFLW,
			&I2sRxOvrflwIntrHandler, (void *)I2sRxInstancePtr);
	XI2s_Rx_SetSclkOutDiv(I2sRxInstancePtr, I2S_RX_MCLK,I2S_RX_FS);
	XI2s_Rx_SetChMux(I2sRxInstancePtr, 0x0, XI2S_RX_CHMUX_XI2S_01);
	XI2s_Rx_IntrEnable(I2sRxInstancePtr, XI2S_RX_GINTR_EN_MASK);
	XI2s_Rx_IntrEnable(I2sRxInstancePtr, XI2S_RX_INTR_AES_BLKCMPLT_MASK);
	XI2s_Rx_IntrEnable(I2sRxInstancePtr, XI2S_RX_INTR_AUDOVRFLW_MASK);
	XI2s_Rx_Enable(I2sRxInstancePtr, TRUE);
	/*
	 * Enable non-critical exceptions.
	 */
	Xil_ExceptionEnable();
	while (IntrCount < I2S_RX_TIME_OUT) {
		//	while(1) {
		/* Wait until an interrupts has been received. */
		if (IntrReceived == 1) {
			break;
		}
		IntrCount++;

		Status =  XST_SUCCESS;
	}
	return Status;
	}
/*****************************************************************************/
/**
* This function is the handler which performs processing for the I2S
* receiver.
* It is called from an interrupt context when the I2S receiver receives a
* AES Block Complete Interrupt.
*
* This handler provides an example of how to handle I2S receiver interrupts
* but is application specific.
*
* @param	CallBackRef is a pointer to the callback function
*
* @return	None.
*
* @note	None.
*
******************************************************************************/
void I2sRxAesBlockCmplIntrHandler(void *CallBackRef)
{
		XI2s_Rx *InstancePtr = (XI2s_Rx *)CallBackRef;
		/* Set the interrupt received flag. */
		IntrReceived = 1;
}
/*****************************************************************************/
/**
* This function is the handler which performs processing for the I2S
* receiver.
* It is called from an interrupt context when the I2S receiver receives a
* Underflow Interrupt.
*
* This handler provides an example of how to handle I2S receiver interrupts
* but is application specific.
*
* @param	CallBackRef is a pointer to the callback function
*
* @return	None.
*
* @note	None.
*
******************************************************************************/
void I2sRxOvrflwIntrHandler(void *CallBackRef)
{
		XI2s_Rx *InstancePtr = (XI2s_Rx *)CallBackRef;
		/* Set the interrupt received flag. */
		IntrReceived = 1;
}
/*****************************************************************************/
/**
* This function setups the interrupt system such that interrupts can occur
* for the I2S receiver. This function is application specific since the actual
* system may or may not have an interrupt controller.  The I2S receiver could
* be directly connected to a processor without an interrupt controller.  The
* user should modify this function to fit the application.
*
* @param	IntcInstancePtr is a pointer to the Interrupt Controller
*		driver Instance.
* @param	I2sRxInstancePtr is a pointer to the XI2S_Rx driver Instance.
* @param	DeviceId is the XPAR_<i2s_rx_instance>_DEVICE_ID value from
*		xparameters.h.
* @param	IntrId is XPAR_<INTC_instance>_<i2s_rx_instance>_VEC_ID
*		value from xparameters.h.
*
* @return	XST_SUCCESS if the Test is successful, otherwise XST_FAILURE.
*
* @note	This function contains an infinite loop such that if interrupts
*		are not working it may never return.
*
******************************************************************************/
static int I2sRxSetupIntrSystem(INTC *IntcInstancePtr,
			XI2s_Rx *I2sRxInstancePtr,
			u16 DeviceId, u16 IntrId)
{
		int Status;
#ifdef XPAR_INTC_0_DEVICE_ID
		/*
		 * Initialize the interrupt controller driver so that
		 * it's ready to use, specify the device ID that is generated
		 * in xparameters.h
		 */
		Status = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;
		/*
		 * Connect a device driver handler that will be called when an
		 * interrupt for the device occurs, the device driver handler
		 * performs the specific interrupt processing for the device
		 */
		Status = XIntc_Connect(IntcInstancePtr, IntrId,
				(XInterruptHandler)XI2S_Rx_IntrHandler,
				(void *)I2sRxInstancePtr);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;

		/*
		 * Start the interrupt controller such that interrupts are
		 * enabled for all devices that cause interrupts, specific
		 * real mode so that the I2S receiver can cause interrupts
		 * through the interrupt controller.
		 */
		Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;

		/*
		 * Enable the interrupt for the I2S receiver.
		 */
		XIntc_Enable(IntcInstancePtr, IntrId);

#else
		XScuGic_Config *IntcConfig;
		/*
		 * Initialize the interrupt controller driver so that it is
		 * ready to use.
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
				(Xil_ExceptionHandler)XI2s_Rx_IntrHandler,
				I2sRxInstancePtr);
		if (Status != XST_SUCCESS)
			return Status;
		/*
		 * Enable the interrupt for the I2S receiver device.
		 */
		XScuGic_Enable(IntcInstancePtr, IntrId);

#endif /* XPAR_INTC_0_DEVICE_ID */

#ifndef TESTAPP_GEN
		/*
		 * Initialize the exception table.
		 */
		Xil_ExceptionInit();

		/*
		 * Register the interrupt controller handler with the exception
		 * table.
		 */
		Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler) INTC_HANDLER,
				IntcInstancePtr);
#endif
		return XST_SUCCESS;
}
/*****************************************************************************/
/**
*
* This function disables the interrupts for the I2S receiver.
*
* @param	IntcInstancePtr is a reference to the Interrupt Controller
*		driver Instance.
* @param	IntrId is XPAR_<INTC_instance>_<i2s_rx_instance>_VEC_ID
*		value from xparameters.h.
*
* @return	None.
*
* @note	None.
*
******************************************************************************/
void I2sRxDisableIntr(INTC *IntcInstancePtr, u16 IntrId)
{
		/*
		 * Disable the interrupt for the I2S receiver
		 */
#ifdef XPAR_INTC_0_DEVICE_ID
		XIntc_Disable(IntcInstancePtr, IntrId);
#else
		/* Disconnect the interrupt */
		XScuGic_Disable(IntcInstancePtr, IntrId);
		XScuGic_Disconnect(IntcInstancePtr, IntrId);
#endif
}
