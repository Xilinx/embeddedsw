/******************************************************************************
 *
 * Copyright (C) 2017 Xilinx, Inc. All rights reserved.
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
 * XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
 * @file xi2stx_intr_example.c
 *
 * This file contains a example for using the I2s Transmitter hardware device
 * and I2s Transmitter driver using interrupt mode.This example assumes
 * that the interrupt controller is also present as a part of the system.
 *
 *
 * <pre> MODIFICATION HISTORY:
 *
 * Ver	 Who  Date    	Changes
 * ----- --- -------- 	-----------------------------------------------
 * 1.0   kar  01/25/18 	First release
 * </pre>
 *
 ****************************************************************************/

/***************************** Include Files **********************************/
#include "xparameters.h"
#include "xi2stx.h"
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
#define I2S_TX_DEVICE_ID	XPAR_XI2STX_0_DEVICE_ID
#define I2S_TX_INTERRUPT_ID	XPAR_FABRIC_I2STX_0_VEC_ID

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

#define I2S_TX_FS		48 //kHz
#define I2S_TX_MCLK		(384*I2S_TX_FS)
#define I2S_TX_TIME_OUT 500000

/**************************** Type Definitions ********************************/

/************************** Function Prototypes *******************************/
int I2sSelfTestExample(u16 DeviceId);

int I2sTxIntrExample(INTC* IntcInstancePtr,
		XI2s_Tx* I2sTxInstancePtr,
		u16 DeviceId,
		u16 IntrId);

static int I2sTxSetupIntrSystem(INTC* IntcInstancePtr,
		XI2s_Tx* I2sTxInstancePtr,
		u16 DeviceId,
		u16 IntrId);
void I2sTxDisableIntr(INTC* IntcInstancePtr, u16 IntrId);

void I2sTxAesBlockCmplIntrHandler(void *CallBackRef);
void I2sTxAesBlockErrIntrHandler(void *CallBackRef);
void I2sTxAesGetChStsHandler(void *CallBackRef);
void I2sTxUnderflowIntrHandler(void *CallBackRef);

/************************** Variable Definitions ******************************/

INTC InterruptController;  /* The instance of the Interrupt Controller */
XI2s_Tx I2sTxInstance;		/* Instance of the I2s Transmitter device */
u32 IntrReceived = 0;

/*****************************************************************************/
/**
 * This function is the main function of the I2S Transmitter example
 * using Interrupts.
 *
 * @param	None.
 *
 * @return	XST_SUCCESS to indicate success,
 *           else XST_FAILURE to indicate a
 *		    Failure.
 *
 * @note		None.
 *
 ******************************************************************************/
int main(void)
{
	xil_printf("I2S Transmitter\r\n");
	int Status;
	/*
	 * Run the I2S Transmitter - Interrupt example.
	 */
	Status = I2sTxIntrExample(&InterruptController,
			&I2sTxInstance,
			I2S_TX_DEVICE_ID,
			I2S_TX_INTERRUPT_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("I2S Transmitter interrupt Example Failed\r\n");
		return XST_FAILURE;
	}
	xil_printf("Successfully ran I2S Transmitter interrupt Example\r\n");
	return XST_SUCCESS;
}
/*****************************************************************************/
/**
 * This function does a minimal test on the I2s Transmitter device and driver
 * as a  design example.  The purpose of this function is to illustrate how
 * to use the XI2S_Tx component.  It initializes a I2s Transmitter and then
 * sets it up such that a periodic interrupt is generated.
 *
 * This function uses interrupt driven mode of the I2s Transmitter.
 *
 * @param	IntcInstancePtr is a pointer to the Interrupt Controller
 *		driver Instance
 * @param	I2sTxInstancePtr is a pointer to the XI2s_Tx driver Instance
 * @param	DeviceId is the XPAR_<i2s_tx_instance>_DEVICE_ID value from
 *		xparameters.h
 * @param	IntrId is XPAR_<INTC_instance>_<i2s_tx_instance>_INTERRUPT_INTR
 *		value from xparameters.h
 *
 * @return	XST_SUCCESS if the Test is successful, otherwise XST_FAILURE
 *
 * @note	This function contains an infinite loop such that if
 *              interrupts are not working it may never return.
 *
 *****************************************************************************/
int I2sTxIntrExample(INTC* IntcInstancePtr,
		XI2s_Tx* I2sTxInstancePtr,
		u16 DeviceId,
		u16 IntrId)
{
	int Status;
	XI2stx_Config *Config;
	u32 IntrCount = 0;
	/*
	 * Lookup and Initialize the I2s Transmitter so that it's ready to use.
	 */
	Config = XI2s_Tx_LookupConfig(DeviceId);
	if (Config == NULL) {
		return XST_FAILURE;
	}

	Status = XI2s_Tx_CfgInitialize(I2sTxInstancePtr, Config,
		       	Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	Status = XI2s_Tx_SelfTest(I2sTxInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/*
	 * Connect the I2s Transmitter counter to the interrupt
	 * subsystem such that interrupts can occur.
	 * This function is application specific.
	 */
	Status = I2sTxSetupIntrSystem(IntcInstancePtr,
			I2sTxInstancePtr,
			DeviceId,
			IntrId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/*
	 * Setup the handler for the I2s Transmitter that will be called from
	 * the interrupt context when the I2s Transmitter receives a block
	 * complete interrupt, AES Block Sync Error interrupt, AES Channel
	 * Status Updated interrupt, Underflow interrupt or Global interrupt.
	 */
	XI2s_Tx_SetHandler(I2sTxInstancePtr, XI2S_TX_HANDLER_AES_BLKCMPLT,
			&I2sTxAesBlockCmplIntrHandler,
			(void *)I2sTxInstancePtr);
	XI2s_Tx_SetHandler(I2sTxInstancePtr, XI2S_TX_HANDLER_AES_BLKSYNCERR,
			&I2sTxAesBlockErrIntrHandler,
			(void *)I2sTxInstancePtr);
	XI2s_Tx_SetHandler(I2sTxInstancePtr,XI2S_TX_HANDLER_AES_CHSTSUPD,
			&I2sTxAesGetChStsHandler,
			(void *)I2sTxInstancePtr);
	XI2s_Tx_SetHandler(I2sTxInstancePtr, XI2S_TX_HANDLER_AUD_UNDRFLW,
			&I2sTxUnderflowIntrHandler,
			(void *)I2sTxInstancePtr);
	XI2s_Tx_SetSclkOutDiv(I2sTxInstancePtr,I2S_TX_MCLK,I2S_TX_FS);
	XI2s_Tx_SetChMux(I2sTxInstancePtr, 0, XI2S_TX_CHMUX_AXIS_01);
	XI2s_Tx_IntrEnable(I2sTxInstancePtr, XI2S_TX_GINTR_EN_MASK);
	XI2s_Tx_IntrEnable(I2sTxInstancePtr, XI2S_TX_INTR_AES_BLKCMPLT_MASK);
	XI2s_Tx_IntrEnable(I2sTxInstancePtr, XI2S_TX_INTR_AUDUNDRFLW_MASK);
	XI2s_Tx_Enable(I2sTxInstancePtr, 0x1);
	Xil_ExceptionEnable();
	while (IntrCount < I2S_TX_TIME_OUT) {
		/* Wait until an interrupt has been received. */
		if (IntrReceived == 1) {
			break;
		}

		IntrCount++;
	}
	return XST_SUCCESS;
}
/*****************************************************************************/
/**
 * This function is the handler which performs processing for the I2s
 * Transmitter.
 * It is called from an interrupt context when the I2s Transmitter receives a
 * AES Block Complete Interrupt.
 *
 * This handler provides an example of how to handle I2s Transmitter interrupts
 * but is application specific.
 *
 * @param	CallBackRef is a pointer to the callback function
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/
void I2sTxAesBlockCmplIntrHandler(void *CallBackRef)
{
	XI2s_Tx *InstancePtr = (XI2s_Tx *)CallBackRef;
	/* Set the interrupt received flag. */
	IntrReceived = 1;
}
/*****************************************************************************/
/**
 * This function is the handler which performs processing for the I2s
 * Transmitter.
 * It is called from an interrupt context when the I2s Transmitter receives a
 * AES Block Error Interrupt.
 *
 * This handler provides an example of how to handle I2s Transmitter interrupts
 * but is application specific.
 *
 * @param	CallBackRef is a pointer to the callback function
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/
void I2sTxAesBlockErrIntrHandler(void *CallBackRef)
{
	XI2s_Tx *InstancePtr = (XI2s_Tx *)CallBackRef;
	/* Set the interrupt received flag. */
	IntrReceived = 1;
}

/*****************************************************************************/
/**
 * This function is the handler which performs processing for the I2s
 * Transmitter.
 * It is called from an interrupt context when the I2s Transmitter receives a
 * AES Channel Status Updated Interrupt.
 *
 * This handler provides an example of how to handle I2s Transmitter interrupts
 * but is application specific.
 *
 * @param	CallBackRef is a pointer to the callback function
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/
void I2sTxAesGetChStsHandler(void *CallBackRef)
{
	XI2s_Tx *InstancePtr = (XI2s_Tx *)CallBackRef;
	/* Set the interrupt received flag. */
	IntrReceived = 1;
}

/*****************************************************************************/
/**
 * This function is the handler which performs processing for the I2s
 * Transmitter.
 * It is called from an interrupt context when the I2s Transmitter receives a
 * Underflow Interrupt.
 *
 * This handler provides an example of how to handle I2s Transmitter interrupts
 * but is application specific.
 *
 * @param	CallBackRef is a pointer to the callback function
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/
void I2sTxUnderflowIntrHandler(void *CallBackRef)
{
	XI2s_Tx *InstancePtr = (XI2s_Tx *)CallBackRef;
	/* Set the interrupt received flag. */
	IntrReceived = 1;
}

/*****************************************************************************/
/**
 * This function setups the interrupt system such that interrupts can occur
 * for the I2s Transmitter. This function is application specific since the
 * actual system may or may not have an interrupt controller.  The I2s
 * Transmitter could be directly connected to a processor without an interrupt
 * controller. The user should modify this function to fit the application.
 *
 * @param	IntcInstancePtr is a pointer to the Interrupt Controller
 *		driver Instance.
 * @param	I2sTxInstancePtr is a pointer to the XI2s_Tx driver Instance.
 * @param	DeviceId is the XPAR_<i2s_tx_instance>_DEVICE_ID value from
 *		xparameters.h.
 * @param	IntrId is XPAR_<INTC_instance>_<i2s_tx_instance>_VEC_ID
 *		value from xparameters.h.
 *
 * @return	XST_SUCCESS if the Test is successful, otherwise XST_FAILURE.
 *
 ******************************************************************************/
static int I2sTxSetupIntrSystem(INTC* IntcInstancePtr,
		XI2s_Tx* I2sTxInstancePtr,
		u16 DeviceId,
		u16 IntrId)
{
	int Status;
#ifdef XPAR_INTC_0_DEVICE_ID
	/*
	 * Initialize the interrupt controller driver so that
	 * it's ready to use, specify the device ID that is generated in
	 * xparameters.h
	 */
	Status = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the device occurs, the device driver handler performs the
	 * specific interrupt processing for the device
	 */
	Status = XIntc_Connect(IntcInstancePtr, IntrId,
			(XInterruptHandler)XI2S_Tx_IntrHandler,
			(void *)I2sTxInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts, specific real mode so that
	 * the I2s Transmitter can cause interrupts thru the interrupt
	 * controller.
	 */
	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupt for the I2s Transmitter.
	 */
	XIntc_Enable(IntcInstancePtr, IntrId);

#else
	XScuGic_Config *IntcConfig;
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
	 * Connect the interrupt handler that will be called when an
	 * interrupt occurs for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, IntrId,
			(Xil_ExceptionHandler)XI2s_Tx_IntrHandler,
			I2sTxInstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}
	/*
	 * Enable the interrupt for the I2s Transmitter device.
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
			(Xil_ExceptionHandler)
			INTC_HANDLER,
			IntcInstancePtr);
#endif
	return XST_SUCCESS;
}
/******************************************************************************/
/**
 *
 * This function disables the interrupts for the I2s Transmitter.
 *
 * @param	IntcInstancePtr is a reference to the Interrupt Controller
 *		driver Instance.
 * @param	IntrId is XPAR_<INTC_instance>_<i2s_tx_instance>_VEC_ID
 *		value from xparameters.h.
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/
void I2sTxDisableIntr(INTC* IntcInstancePtr, u16 IntrId)
{
	/*
	 * Disable the interrupt for the I2s Transmitter
	 */
#ifdef XPAR_INTC_0_DEVICE_ID
	XIntc_Disable(IntcInstancePtr, IntrId);
#else
	/* Disconnect the interrupt */
	XScuGic_Disable(IntcInstancePtr, IntrId);
	XScuGic_Disconnect(IntcInstancePtr, IntrId);
#endif

	return;
}
