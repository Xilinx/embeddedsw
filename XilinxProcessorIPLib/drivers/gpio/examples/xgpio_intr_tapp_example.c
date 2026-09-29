/******************************************************************************
* Copyright (C) 2002 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xgpio_intr_tapp_example.c
* @brief Demonstrates usage of the GPIO driver (XGpio) in interrupt-driven mode.
*
* This file contains a design example using the GPIO driver (XGpio) in an
* interrupt-driven mode of operation. This example assumes that there is
* an interrupt controller in the hardware system and the GPIO device is
* connected to the interrupt controller.
*
* This file is used in the Peripheral Tests Application in SDK to include a
* simplified test for GPIO interrupts.
*
* The buttons and LEDs are on two separate channels of the GPIO. This
* separation prevents interrupts when the LEDs turn on and off.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -------------------------------------------------------
* 2.01a sn   05/09/06 Modified to be used by TestAppGen to include test for
*		      interrupts.
* 3.00a ktn  11/21/09 Updated to use HAL Processor APIs and minor changes
*		      as per coding guidelines.
* 3.00a sdm  02/16/11 Updated to support ARM Generic Interrupt Controller
* 4.1   lks  11/18/15 Updated to use canonical xparameters and
*		      clean up of the comments and code for CR 900381
* 4.3   ms   01/23/17 Modified xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* 4.10  gm   07/11/23 Added SDT support.
* 4.11  gm   12/06/23 Added IER and GIER calls in SDT flow.
* 4.13 vlt   12/02/25 Added GlobalIntrMask initialization for SDT flow.
* 4.13  vlt  12/12/25 Update Doxygen comments to include SDT flow details.
* 4.14  vn   07/30/26 Fix Doxygen warnings
* 4.14  vn   09/10/26 Align Doxygen comments with AMD standards
*
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xgpio.h"
#include "xil_exception.h"

#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
#include <stdio.h>
#else
#include "xscugic.h"
#include "xil_printf.h"
#endif
#else
#include "xinterrupt_wrap.h"
#endif

/************************** Constant Definitions *****************************/
#ifndef TESTAPP_GEN
/** @name Device parameters
 *
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here so that all the needed
 * parameters can be easily changed in one place.
 * @{
 */
#ifndef SDT
#define GPIO_DEVICE_ID		XPAR_GPIO_0_DEVICE_ID /**< Device ID. */
#else
#define	XGPIO_AXI_BASEADDRESS	XPAR_XGPIO_0_BASEADDR /**< Base address. */
#endif
#define GPIO_CHANNEL1		1 /**< GPIO channel 1. */

#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID
/** GPIO interrupt ID. */
#define INTC_GPIO_INTERRUPT_ID	XPAR_INTC_0_GPIO_0_VEC_ID
#define INTC_DEVICE_ID	XPAR_INTC_0_DEVICE_ID /**< INTC device ID. */
#else
/** GPIO interrupt ID. */
#define INTC_GPIO_INTERRUPT_ID	XPAR_FABRIC_AXI_GPIO_0_IP2INTC_IRPT_INTR
/** INTC device ID. */
#define INTC_DEVICE_ID	XPAR_SCUGIC_SINGLE_DEVICE_ID
#endif /* XPAR_INTC_0_DEVICE_ID */
#endif
/** @} */

/** @name Button and LED masks
 *
 * The following constants define the positions of the buttons and LEDs in
 * each channel of the GPIO.
 * @{
 */
#define GPIO_ALL_LEDS		0xFFFF /**< All LED bits mask. */
#define GPIO_ALL_BUTTONS	0xFFFF /**< All button bits mask. */
/** @} */

/** @name Channel definitions
 *
 * The following constants define the GPIO channel that is used for the buttons
 * and the LEDs. They allow the channels to be reversed easily.
 * @{
 */
#define BUTTON_CHANNEL	 1	/**< Channel 1 of the GPIO device. */
#define LED_CHANNEL	 2	/**< Channel 2 of the GPIO device. */
#define BUTTON_INTERRUPT XGPIO_IR_CH1_MASK  /**< Channel 1 interrupt mask. */
/** @} */

/**
 * @brief Interrupt control value.
 *
 * The following constant determines which buttons must be pressed at the same
 * time to cause interrupt processing to stop and start.
 */
#define INTERRUPT_CONTROL_VALUE 0x7

/**
 * @brief LED visibility delay count.
 *
 * The following constant is used to wait after an LED is turned on to make
 * sure that it is visible to the human eye. This constant might need to be
 * tuned for faster or slower processor speeds.
 */
#define LED_DELAY	1000000

#endif /* TESTAPP_GEN */

#define INTR_DELAY	0x00FFFFFF /**< Interrupt delay count. */

#ifndef SDT
/** @name Interrupt controller abstraction
 *
 * The following constants abstract the interrupt controller type.
 * @{
 */
#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC_DEVICE_ID	XPAR_INTC_0_DEVICE_ID /**< INTC device ID. */
#define INTC		XIntc /**< Interrupt controller type. */
#define INTC_HANDLER	XIntc_InterruptHandler /**< Interrupt handler. */
#else
/** INTC device ID. */
#define INTC_DEVICE_ID	XPAR_SCUGIC_SINGLE_DEVICE_ID
#define INTC		XScuGic /**< Interrupt controller type. */
#define INTC_HANDLER	XScuGic_InterruptHandler /**< Interrupt handler. */
#endif /* XPAR_INTC_0_DEVICE_ID */
/** @} */
#endif

/************************** Function Prototypes ******************************/
void GpioHandler(void *CallBackRef);

#ifndef SDT
int GpioIntrExample(INTC *IntcInstancePtr, XGpio *InstancePtr,
		    u16 DeviceId, u16 IntrId,
		    u16 IntrMask, u32 *DataRead);

int GpioSetupIntrSystem(INTC *IntcInstancePtr, XGpio *InstancePtr,
			u16 DeviceId, u16 IntrId, u16 IntrMask);

void GpioDisableIntr(INTC *IntcInstancePtr, XGpio *InstancePtr,
		     u16 IntrId, u16 IntrMask);
#else

int GpioIntrExample(XGpio *InstancePtr,
		    UINTPTR BaseAddress,
		    u16 IntrMask, u32 *DataRead);
#endif
/************************** Variable Definitions *****************************/

/**
 * @brief Holds the global GPIO driver instance.
 *
 * The instance is declared globally so it is zeroed and easily accessible
 * from a debugger.
 */
XGpio Gpio;

#ifndef SDT
INTC Intc; /**< Instance of the interrupt controller driver. */
#endif

/** GPIO channel mask that is needed by the interrupt handler. */
static u16 GlobalIntrMask;

static volatile u32 IntrFlag; /**< Interrupt handler flag. */

/*****************************************************************************/
/**
* @brief Runs the GPIO interrupt example.
*
* This is the main function of the GPIO example. It is responsible for
* initializing the GPIO device, setting up interrupts, and providing a
* foreground loop so that interrupts can occur in the background.
*
* @return
*               - XST_SUCCESS to indicate success.
*               - XST_FAILURE to indicate failure.
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;
	u32 DataRead;

	print(" Press button to Generate Interrupt\r\n");

#ifndef SDT
	Status = GpioIntrExample(&Intc, &Gpio,
				 GPIO_DEVICE_ID,
				 INTC_GPIO_INTERRUPT_ID,
				 GPIO_CHANNEL1, &DataRead);
#else
	Status = GpioIntrExample(&Gpio,
				 XGPIO_AXI_BASEADDRESS,
				 GPIO_CHANNEL1, &DataRead);
#endif

	if (Status == 0 ) {
		if (DataRead == 0) {
			print("No button pressed. \r\n");
		} else {
			print("Successfully ran Gpio Interrupt Tapp Example\r\n");
		}
	} else {
		print("Gpio Interrupt Tapp Example Failed.\r\n");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
* @brief Tests GPIO interrupts when enabled.
*
* This is the entry function from the TestAppGen tool generated application,
* which tests the interrupts when enabled in the GPIO.
*
* @if SDT
* @param[out]    InstancePtr Pointer to the GPIO driver instance.
* @param[in]     BaseAddress Base address of the device.
* @else
* @param[in,out] IntcInstancePtr Pointer to the Interrupt Controller
*                driver instance.
* @param[out]    InstancePtr Pointer to the GPIO driver instance.
* @param[in]     DeviceId XPAR_<GPIO_instance>_DEVICE_ID value from
*                xparameters.h.
* @param[in]     IntrId XPAR_<INTC_instance>_<GPIO_instance>_IP2INTC_IRPT_INTR
*                value from xparameters.h.
* @endif
* @param[in]     IntrMask GPIO channel mask.
* @param[out]    DataRead Pointer where the data read from the GPIO input
*                is returned.
*
* @return
*               - XST_SUCCESS if the test is successful.
*               - XST_FAILURE if the test is not successful.
*
* @note          In XSCT/classic flow, DeviceId is used to look up the device
*                configuration.
*
******************************************************************************/
#ifndef SDT
int GpioIntrExample(INTC *IntcInstancePtr, XGpio *InstancePtr, u16 DeviceId,
		    u16 IntrId, u16 IntrMask, u32 *DataRead)
#else
int GpioIntrExample(XGpio *InstancePtr, UINTPTR BaseAddress,
		    u16 IntrMask, u32 *DataRead)
#endif
{
	int Status;
	u32 delay;
#ifdef SDT
	XGpio_Config *ConfigPtr;

	ConfigPtr = XGpio_LookupConfig(BaseAddress);
#endif
	/* Initialize the GPIO driver. If an error occurs then exit */
#ifndef SDT
	Status = XGpio_Initialize(InstancePtr, DeviceId);
#else
	Status = XGpio_Initialize(InstancePtr, BaseAddress);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#ifndef SDT
	Status = GpioSetupIntrSystem(IntcInstancePtr, InstancePtr, DeviceId,
				     IntrId, IntrMask);
#else
	GlobalIntrMask = IntrMask;
	Status = XSetupInterruptSystem(InstancePtr, &GpioHandler,
				       ConfigPtr->IntrId,
				       ConfigPtr->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
	XGpio_InterruptEnable(InstancePtr, IntrMask);
	XGpio_InterruptGlobalEnable(InstancePtr);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	IntrFlag = 0;
	delay = 0;

	while (!IntrFlag && (delay < INTR_DELAY)) {
		delay++;
	}

#ifndef SDT
	GpioDisableIntr(IntcInstancePtr, InstancePtr, IntrId, IntrMask);
#else
	XDisconnectInterruptCntrl(ConfigPtr->IntrId, ConfigPtr->IntrParent);
#endif

	*DataRead = IntrFlag;

	return Status;
}

#ifndef SDT
/*****************************************************************************/
/**
* @brief Sets up the GPIO interrupt system.
*
* @param[in,out] IntcInstancePtr Pointer to the Interrupt Controller
*                driver instance.
* @param[in]     InstancePtr Pointer to the GPIO driver instance.
* @param[in]     DeviceId XPAR_<GPIO_instance>_DEVICE_ID value from
*                xparameters.h.
* @param[in]     IntrId XPAR_<INTC_instance>_<GPIO_instance>_IP2INTC_IRPT_INTR
*                value from xparameters.h.
* @param[in]     IntrMask GPIO channel mask.
*
* @return        XST_SUCCESS if the test is successful, otherwise
*                XST_FAILURE.
*
******************************************************************************/
int GpioSetupIntrSystem(INTC *IntcInstancePtr, XGpio *InstancePtr,
			u16 DeviceId, u16 IntrId, u16 IntrMask)
{
	int Result;

	GlobalIntrMask = IntrMask;

#ifdef XPAR_INTC_0_DEVICE_ID

#ifndef TESTAPP_GEN
	/*
	 * Initialize the interrupt controller driver so that it's ready to use.
	 * specify the device ID that was generated in xparameters.h
	 */
	Result = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
	if (Result != XST_SUCCESS) {
		return Result;
	}
#endif /* TESTAPP_GEN */

	/* Hook up interrupt service routine */
	XIntc_Connect(IntcInstancePtr, IntrId,
		      (Xil_ExceptionHandler)GpioHandler, InstancePtr);

	/* Enable the interrupt vector at the interrupt controller */
	XIntc_Enable(IntcInstancePtr, IntrId);

#ifndef TESTAPP_GEN
	/*
	 * Start the interrupt controller such that interrupts are recognized
	 * and handled by the processor
	 */
	Result = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Result != XST_SUCCESS) {
		return Result;
	}
#endif /* TESTAPP_GEN */

#else /* !XPAR_INTC_0_DEVICE_ID */

#ifndef TESTAPP_GEN
	XScuGic_Config *IntcConfig;

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Result = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
				       IntcConfig->CpuBaseAddress);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif /* TESTAPP_GEN */

	XScuGic_SetPriorityTriggerType(IntcInstancePtr, IntrId,
				       0xA0, 0x3);

	/*
	 * Connect the interrupt handler that is called when an
	 * interrupt occurs for the device.
	 */
	Result = XScuGic_Connect(IntcInstancePtr, IntrId,
				 (Xil_ExceptionHandler)GpioHandler, InstancePtr);
	if (Result != XST_SUCCESS) {
		return Result;
	}

	/* Enable the interrupt for the GPIO device.*/
	XScuGic_Enable(IntcInstancePtr, IntrId);
#endif /* XPAR_INTC_0_DEVICE_ID */

	/*
	 * Enable the GPIO channel interrupts so that push button can be
	 * detected and enable interrupts for the GPIO device
	 */
	XGpio_InterruptEnable(InstancePtr, IntrMask);
	XGpio_InterruptGlobalEnable(InstancePtr);

	/*
	 * Initialize the exception table and register the interrupt
	 * controller handler with the exception table
	 */
	Xil_ExceptionInit();

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler)INTC_HANDLER, IntcInstancePtr);

	/* Enable non-critical exceptions */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
* @brief Handles GPIO interrupts.
*
* @param[in]   CallbackRef Callback reference for the handler.
*
******************************************************************************/
void GpioHandler(void *CallbackRef)
{
	XGpio *GpioPtr = (XGpio *)CallbackRef;

	IntrFlag = 1;

	/* Clear the Interrupt */
	XGpio_InterruptClear(GpioPtr, GlobalIntrMask);

}

#ifndef SDT
/*****************************************************************************/
/**
* @brief Disables GPIO interrupts.
*
* @param[in,out] IntcInstancePtr Pointer to the Interrupt Controller
*                driver instance.
* @param[in]     InstancePtr Pointer to the GPIO driver instance.
* @param[in]     IntrId XPAR_<INTC_instance>_<GPIO_instance>_VEC value from
*                xparameters.h.
* @param[in]     IntrMask GPIO channel mask.
*
******************************************************************************/
void GpioDisableIntr(INTC *IntcInstancePtr, XGpio *InstancePtr,
		     u16 IntrId, u16 IntrMask)
{
	XGpio_InterruptDisable(InstancePtr, IntrMask);
#ifdef XPAR_INTC_0_DEVICE_ID
	XIntc_Disable(IntcInstancePtr, IntrId);
#else
	/* Disconnect the interrupt */
	XScuGic_Disable(IntcInstancePtr, IntrId);
	XScuGic_Disconnect(IntcInstancePtr, IntrId);
#endif
	return;
}
#endif
