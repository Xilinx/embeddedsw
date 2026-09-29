/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xgpiops_intr_example.c
* @brief Demonstrates the GPIO PS driver (XGpioPs) in interrupt-driven mode.
*
* This file contains a design example that uses the GPIO driver (XGpioPs) in
* an interrupt driven mode of operation.
*
* The example uses the interrupt capability of the GPIO to detect push button
* events and set the output LEDs based on the input.
*
* @note
* This example assumes that the hardware design contains a UART device.
* This example supports only the zcu102 board on the ZynqMP platform and only
* the zc702 board on the Zynq platform.
* For the ZynqMP platform, the input pin is 22 (sw19 on the zcu102 board) and
* the output pin is 23 (DS50 on the zcu102 board).
* For the Zynq platform, the input pins are 12 (sw14 on the zc702 board) and
* 14 (sw13 on the zc702 board), and the output pin is 10 (DS23 on the zc702
* board). SW15 on the zc702 board is a combination of sw13 and sw14. To
* operate either of the input pins, keep SW15 low (both should be 00).
* This example supports the VCK190 and VMK180 for Versal, but requires a PL
* shim. See Answer Record AR# 75677 and Figure 61 in AM011 Versal TRM
* for more details.
* The driver supports both PS GPIO and PMC GPIO.
* To access PMC GPIOs, set "GPIO.PmcGpio = 1" in the application. Otherwise,
* the application accesses PS GPIO.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a sv   01/18/10 First Release
* 3.3   ms   04/17/17 Added notes about gpio input and output pin description
*                     for zcu102 and zc702 boards, configured Interrupt pin
*                     to input pin for proper working of interrupt example.
* 3.7	sne  12/04/19 Reverted versal example support.
* 3.8	sne  09/17/20 Added description for Versal PS and PMC GPIO pins.
* 3.9	sne  11/19/20 Added versal PmcGpio example support.
* 3.12  gm   07/11/23 Added SDT support.
* 3.15  vlt  01/27/26 Fixed codespell issues
* 3.16  vn   07/30/26 Fix Doxygen warnings
* 3.16  vn   09/10/26 Align Doxygen comments with AMD standards
*
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xgpiops.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "xplatform_info.h"
#include <xil_printf.h>
#ifdef SDT
#include "xinterrupt_wrap.h"
#endif

/************************** Constant Definitions *****************************/

/** @name Device parameters
 *
 * The following constants map to the names of the hardware instances that
 * were created in the EDK XPS system. They are defined here so that you can
 * easily change all the needed device IDs in one place.
 * @{
 */
#ifndef SDT
/** GPIO device ID. */
#define GPIO_DEVICE_ID		XPAR_XGPIOPS_0_DEVICE_ID
/** Interrupt controller device ID. */
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#ifdef versal
/** GPIO interrupt ID (Versal). */
#define GPIO_INTERRUPT_ID	XPMC_GPIO_INT_ID
#else
#define GPIO_INTERRUPT_ID	XPAR_XGPIOPS_0_INTR /**< GPIO interrupt ID. */
#endif
#else
/** GPIO base address. */
#define	XGPIOPS_BASEADDR	XPAR_XGPIOPS_0_BASEADDR
#endif
/** @} */

/** @name GPIO bank constants
 *
 * The following constants define the GPIO banks that are used.
 * @{
 */
#ifdef versal
#define GPIO_BANK	XGPIOPS_BANK3  /**< Bank 3 of the GPIO Device. */
#else
#define GPIO_BANK	XGPIOPS_BANK0  /**< Bank 0 of the GPIO Device. */
#endif
/** @} */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

#ifndef SDT
static int GpioIntrExample(XScuGic *Intc, XGpioPs *Gpio, u16 DeviceId,
			   u16 GpioIntrId);
static int SetupInterruptSystem(XScuGic *Intc, XGpioPs *Gpio, u16 GpioIntrId);
#else
static int GpioIntrExample(XGpioPs *Gpio, UINTPTR BaseAddress);
#endif
static void IntrHandler(void *CallBackRef, u32 Bank, u32 Status);
/************************** Variable Definitions *****************************/

/**
 * @brief Holds the global GPIO driver instance.
 *
 * The instance is declared globally so it is zeroed and easily accessible
 * from a debugger.
 */
static XGpioPs Gpio;
#ifndef SDT
static XScuGic Intc; /**< Instance of the interrupt controller driver. */
#endif

static u32 AllButtonsPressed; /**< Interrupt status of the bank. */
static u32 Input_Bank_Pin; /**< Pin number within the bank. */
static u32 Input_Pin; /**< Switch button. */
static u32 Output_Pin; /**< LED button. */

/****************************************************************************/
/**
* @brief Calls the GPIO interrupt example.
*
* @return
*              - XST_SUCCESS if the example has completed successfully.
*              - XST_FAILURE if the example has failed.
*
*****************************************************************************/
int main(void)
{
	int Status;

	xil_printf("GPIO Interrupt Example Test \r\n");

	/*
	 * Run the GPIO interrupt example, specify the parameters that
	 * are generated in xparameters.h.
	 */
#ifndef SDT
	Status = GpioIntrExample(&Intc, &Gpio, GPIO_DEVICE_ID,
				 GPIO_INTERRUPT_ID);
#else
	Status = GpioIntrExample(&Gpio, XGPIOPS_BASEADDR);
#endif

	if (Status != XST_SUCCESS) {
		xil_printf("GPIO Interrupt Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran GPIO Interrupt Example Test\r\n");
	return XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief Demonstrates interrupt-driven GPIO usage.
*
* Initializes the GPIO device, sets up interrupts, and provides a foreground
* loop so that interrupts can occur in the background.
*
* @if SDT
* @param[out]  Gpio Pointer to the XGpioPs driver instance.
* @param[in]   BaseAddress Base address of the GPIO device.
* @else
* @param[out]  Intc Pointer to the XScuGic driver instance.
* @param[out]  Gpio Pointer to the XGpioPs driver instance.
* @param[in]   DeviceId XPAR_<Gpio_Instance>_PS_DEVICE_ID value from
*              xparameters.h.
* @param[in]   GpioIntrId XPAR_<GIC>_<GPIO_Instance>_VEC_ID value from
*              xparameters.h.
* @endif
*
* @return
*              - XST_SUCCESS if the example has completed successfully.
*              - XST_FAILURE if the example has failed.
*
*****************************************************************************/
#ifndef SDT
int GpioIntrExample(XScuGic *Intc, XGpioPs *Gpio, u16 DeviceId, u16 GpioIntrId)
#else
int GpioIntrExample(XGpioPs *Gpio, UINTPTR BaseAddress)
#endif
{
	XGpioPs_Config *ConfigPtr;
	int Status;
	int Type_of_board;

	/* Initialize the Gpio driver. */
#ifndef SDT
	ConfigPtr = XGpioPs_LookupConfig(DeviceId);
#else
	ConfigPtr = XGpioPs_LookupConfig(BaseAddress);
#endif
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}
	Type_of_board = XGetPlatform_Info();
	switch (Type_of_board) {
		case XPLAT_ZYNQ_ULTRA_MP:
			Input_Bank_Pin = 22;
			Input_Pin = 22;
			Output_Pin = 23;
			break;

		case XPLAT_ZYNQ:
			Input_Bank_Pin = 14;
			Input_Pin = 14;
			Output_Pin = 10;
			break;
#ifdef versal
		case XPLAT_VERSAL:
			/* Accessing PMC GPIO by setting field to 1 */
			Gpio->PmcGpio  =  1;
			Input_Bank_Pin =  4;
			Input_Pin      = 56;
			Output_Pin     = 52;
			break;
#endif
	}
	XGpioPs_CfgInitialize(Gpio, ConfigPtr, ConfigPtr->BaseAddr);

	/* Run a self-test on the GPIO device. */
	Status = XGpioPs_SelfTest(Gpio);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Set the direction for the specified pin to be input */
	XGpioPs_SetDirectionPin(Gpio, Input_Pin, 0x0);

	/* Set the direction for the specified pin to be output. */
	XGpioPs_SetDirectionPin(Gpio, Output_Pin, 1);
	XGpioPs_SetOutputEnablePin(Gpio, Output_Pin, 1);
	XGpioPs_WritePin(Gpio, Output_Pin, 0x0);

	/*
	 * Setup the interrupts such that interrupt processing can occur. If
	 * an error occurs then exit.
	 */
#ifndef SDT
	Status = SetupInterruptSystem(Intc, Gpio, GPIO_INTERRUPT_ID);
#else

	/* Enable falling edge interrupts for all the pins in GPIO bank. */
	XGpioPs_SetIntrType(Gpio, GPIO_BANK, 0x00, 0xFFFFFFFF, 0x00);

	/* Set the handler for gpio interrupts. */
	XGpioPs_SetCallbackHandler(Gpio, (void *)Gpio, IntrHandler);


	/* Enable the GPIO interrupts of GPIO Bank. */
	XGpioPs_IntrEnable(Gpio, GPIO_BANK, (1 << Input_Bank_Pin));

	Status = XSetupInterruptSystem(Gpio, (void *)&XGpioPs_IntrHandler,
				       ConfigPtr->IntrId,
				       ConfigPtr->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#ifdef SDT
	XGpioPs_SetCallbackHandler(Gpio, (void *)Gpio, IntrHandler);
#endif

	xil_printf("\n\rPush Switch button to exit\n\r");
	AllButtonsPressed = FALSE;

	/*
	 * Loop forever while the button changes are handled by the interrupt
	 * level processing.
	 */
	while (AllButtonsPressed == FALSE);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Handles the bank 0 interrupts of the GPIO device.
*
* Acts as the user layer callback function for the bank 0 interrupts of the
* GPIO device. Checks if all the switches have been pressed to stop the
* interrupt processing and exit from the example.
*
* @param[in]   CallBackRef Pointer to the upper layer callback reference.
* @param[in]   Bank GPIO bank that caused the interrupt.
* @param[in]   Status Interrupt status of the GPIO bank.
*
******************************************************************************/
static void IntrHandler(void *CallBackRef, u32 Bank, u32 Status)
{
	XGpioPs *Gpio = (XGpioPs *)CallBackRef;
	u32 DataRead;

	(void)Bank;
	(void)Status;

	/* Push the switch button */
	DataRead = XGpioPs_ReadPin(Gpio, Input_Pin);
	if (DataRead != 0) {
		XGpioPs_SetDirectionPin(Gpio, Output_Pin, 1);
		XGpioPs_SetOutputEnablePin(Gpio, Output_Pin, 1);
		XGpioPs_WritePin(Gpio, Output_Pin, DataRead);
		AllButtonsPressed = TRUE;
	}
}

#ifndef SDT
/*****************************************************************************/
/**
* @brief Sets up the interrupt system for the GPIO example.
*
* Enables falling edge interrupts for all the pins of bank 0 in the GPIO
* device.
*
* @param[out]    GicInstancePtr Pointer to the XScuGic driver instance.
* @param[in,out] Gpio Pointer to the instance of the GPIO component to
*                connect to the interrupt controller.
* @param[in]     GpioIntrId Interrupt ID, typically the
*                XPAR_<GICPS>_<GPIOPS_instance>_VEC_ID value from
*                xparameters.h.
*
* @return        XST_SUCCESS if successful, otherwise XST_FAILURE.
*
******************************************************************************/
static int SetupInterruptSystem(XScuGic *GicInstancePtr, XGpioPs *Gpio,
				u16 GpioIntrId)
{
	int Status;

	XScuGic_Config *IntcConfig; /* Instance of the interrupt controller */

	Xil_ExceptionInit();

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(GicInstancePtr, IntcConfig,
				       IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler)XScuGic_InterruptHandler,
				     GicInstancePtr);

	/*
	 * Connect the device driver handler that is called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	Status = XScuGic_Connect(GicInstancePtr, GpioIntrId,
				 (Xil_ExceptionHandler)XGpioPs_IntrHandler,
				 (void *)Gpio);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* Enable falling edge interrupts for all the pins in GPIO bank. */
	XGpioPs_SetIntrType(Gpio, GPIO_BANK, 0x00, 0xFFFFFFFF, 0x00);

	/* Set the handler for gpio interrupts. */
	XGpioPs_SetCallbackHandler(Gpio, (void *)Gpio, IntrHandler);


	/* Enable the GPIO interrupts of GPIO Bank. */
	XGpioPs_IntrEnable(Gpio, GPIO_BANK, (1 << Input_Bank_Pin));


	/* Enable the interrupt for the GPIO device. */
	XScuGic_Enable(GicInstancePtr, GpioIntrId);


	/* Enable interrupts in the Processor. */
	Xil_ExceptionEnableMask(XIL_EXCEPTION_IRQ);

	return XST_SUCCESS;
}
#endif
