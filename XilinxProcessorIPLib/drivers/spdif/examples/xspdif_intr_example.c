/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
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

#include "xinterrupt_wrap.h"

/************************** Constant Definitions ******************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are only defined here such that a user can easily
 * change all the needed parameters in one place.
 */

#define SPDIF_TIME_OUT 500000

/**************************** Type Definitions ********************************/

/************************** Function Prototypes *******************************/
int SpdifSelfTestExample(UINTPTR BaseAddress);
static int SpdifIntrExample(XSpdif *SpdifInstancePtr, UINTPTR BaseAddress);
static int SpdifSetupIntrSystem(XSpdif *SpdifInstancePtr,
				UINTPTR BaseAddress);
void SpdifStartOfBlockHandler(void *CallBackRef);

void SpdifPreambleErrHandler(void *CallBackRef);

void SpdifBmcErrHandler(void *CallBackRef);

void SpdifTxOrRxFifoEmptyHandler(void *CallBackRef);

void SpdifTxOrRxFifoFullHandler(void *CallBackRef);

/************************** Variable Definitions ******************************/
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
	Status = SpdifIntrExample(&SpdifInstance, XPAR_XSPDIF_0_BASEADDR);
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
int SpdifIntrExample(XSpdif *SpdifInstancePtr, UINTPTR BaseAddress)
{
	int Status;
	XSpdif_Config *Config;
	unsigned int IntrCount = 0;
	/*
	 * Lookup and Initialize the Spdif so that it's ready to use.
	 */
	Config = XSpdif_LookupConfig(BaseAddress);
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
	Status = SpdifSetupIntrSystem(SpdifInstancePtr, BaseAddress);
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
static int SpdifSetupIntrSystem(XSpdif *SpdifInstancePtr,
				UINTPTR BaseAddress)
{
	int Status;

	Status = XSetupInterruptSystem(SpdifInstancePtr, &XSpdif_IntrHandler,
				       SpdifInstancePtr->Config.IntrId,
				       SpdifInstancePtr->Config.IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
	if (Status == XST_FAILURE) {
		xil_printf("IRQ init failed.\n\r\r");
		return XST_FAILURE;
	}
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

