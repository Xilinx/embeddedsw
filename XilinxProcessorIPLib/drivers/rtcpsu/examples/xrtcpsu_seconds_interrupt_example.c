/******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file	xrtcpsu_seconds_interrupt_example.c
* @brief Demonstrates the XRtcPsu seconds feature in interrupt mode.
*
* This file contains a seconds example using the XRtcPsu driver in interrupt
* mode.
*
* @note
* In the example, if interrupts are not working, it may hang.
*
* <pre>
* MODIFICATION HISTORY:
* Ver   Who    Date     Changes
* ----- ------ -------- ----------------------------------------------
* 1.00  kvn 05/12/15 First Release
*       ms  04/10/17 Modified filename tag to include the file in doxygen
*                    examples.
* 1.12	sne 04/25/22 Added volatile keyword for Seconds variable.
* 1.13	ht  06/21/23 Added support for system device-tree flow.
* 1.15  ht  12/13/24 Fix C++ compilation warnings and errors in SDT flow.
* 1.17  vlt 12/18/25 Update Doxygen comments to include SDT flow details.
* 1.18  vn  07/30/26 Fix Doxygen warnings
* 1.18  vn  09/30/26 Align Doxygen comments with AMD standards
* </pre>
****************************************************************************/

/***************************** Include Files *******************************/

#include "xparameters.h"	/* SDK generated parameters */
#include "xrtcpsu.h"		/* RTCPSU device driver */
#include "xscugic.h"		/* Interrupt controller device driver */
#include "xil_exception.h"
#include "xil_printf.h"
#ifdef SDT
#include "xinterrupt_wrap.h"
#endif

/************************** Constant Definitions **************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */

#ifndef SDT
/** @name Device and interrupt IDs
 * @{
 */
#define RTC_DEVICE_ID         XPAR_XRTCPSU_0_DEVICE_ID /**< RTC device ID. */
/** Interrupt controller device ID. */
#define INTC_DEVICE_ID			XPAR_SCUGIC_SINGLE_DEVICE_ID
/** RTC seconds interrupt ID. */
#define RTC_SEC_INT_IRQ_ID	XPAR_XRTCPSU_SECONDS_INTR
/** @} */
#endif

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define REPETATIONS 2 /**< Number of seconds events to wait for. */

/************************** Function Prototypes *****************************/

#ifndef SDT
int RtcPsuSecondsIntrExample(XScuGic *IntcInstPtr, XRtcPsu *RtcInstPtr,
			     u16 DeviceId, u16 RtcIntrId);


static int SetupInterruptSystem(XScuGic *IntcInstancePtr,
				XRtcPsu *RtcInstancePtr,
				u16 RtcIntrId);
#else
int RtcPsuSecondsIntrExample(XRtcPsu *RtcInstPtr, UINTPTR BaseAddress);
#endif
void Handler(void *CallBackRef, u32 Event);


/************************** Variable Definitions ***************************/

XRtcPsu RtcPsu;		/**< Instance of the RTC Device. */
#ifndef SDT
XScuGic InterruptController;	/**< Instance of the Interrupt Controller. */
#endif
volatile u32 Seconds = 0;	/**< Count of seconds events generated. */


/**************************************************************************/
/**
*
* @brief Calls the RTC Seconds interrupt example.
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
*
**************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

	/* Run the RtcPsu Interrupt example, specify the the Device ID */
#ifndef SDT
	Status = RtcPsuSecondsIntrExample(&InterruptController, &RtcPsu,
					  RTC_DEVICE_ID, RTC_SEC_INT_IRQ_ID);
#else
	Status = RtcPsuSecondsIntrExample(&RtcPsu, XPAR_XRTCPSU_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("RTC Seconds Interrupt Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran RTC Seconds Interrupt Example Test\r\n");
	return XST_SUCCESS;
}
#endif

/**************************************************************************/
/**
*
* @brief Runs a minimal test on the RTC device and driver as a design
* example.
*
* The purpose of this function is to illustrate how to use the seconds
* feature available in the XRtcPsu driver.
*
* This function shows the seconds interrupt feature.
*
* @param[out]	RtcInstPtr Pointer to the instance of the RTC driver
*		that is connected to the interrupt controller.
* @if SDT
* @param[in]	BaseAddress Base address of the device.
* @else
* @param[out]	IntcInstPtr Pointer to the instance of the ScuGic driver.
* @param[in]	DeviceId Device ID of the RTC device, typically the
*		XPAR_<RTCPSU_instance>_DEVICE_ID value from xparameters.h.
* @param[in]	RtcIntrId Interrupt ID, typically the
*		XPAR_<RTCPSU_instance>_INTR value from xparameters.h.
* @endif
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note         - In XSCT/classic flow, the driver uses DeviceId to look up the
*		  device configuration.
*		- This function may never return if interrupts are not
*		  working.
*
**************************************************************************/
#ifndef SDT
int RtcPsuSecondsIntrExample(XScuGic *IntcInstPtr, XRtcPsu *RtcInstPtr,
			     u16 DeviceId, u16 RtcIntrId)
#else
int RtcPsuSecondsIntrExample(XRtcPsu *RtcInstPtr, UINTPTR BaseAddress)
#endif
{
	int Status;
	XRtcPsu_Config *Config;

	/*
	 * Initialize the RTC driver so that it's ready to use
	 * Look up the configuration in the config table, then initialize it.
	 */
#ifndef SDT
	Config = XRtcPsu_LookupConfig(DeviceId);
#else
	Config = XRtcPsu_LookupConfig(BaseAddress);
#endif
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XRtcPsu_CfgInitialize(RtcInstPtr, Config, Config->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Check hardware build */
	Status = XRtcPsu_SelfTest(RtcInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the RTC to the interrupt subsystem such that interrupts
	 * can occur. This function is application specific.
	 */
#ifndef SDT
	Status = SetupInterruptSystem(IntcInstPtr, RtcInstPtr, RtcIntrId);
#else
	Status = XSetupInterruptSystem(RtcInstPtr, (void *)&XRtcPsu_InterruptHandler,
				       Config->IntrId[1],
				       Config->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the handlers for the RTC that will be called from the
	 * interrupt context when alarm and seconds interrupts are raised,
	 * specify a pointer to the RTC driver instance as the callback reference
	 * so the handlers are able to access the instance data
	 */
	XRtcPsu_SetHandler(RtcInstPtr, (XRtcPsu_Handler)Handler, RtcInstPtr);

	/*
	 * Enable the interrupt of the RTC device so interrupts will occur.
	 */
	XRtcPsu_SetInterruptMask(RtcInstPtr, XRTC_INT_EN_SECS_MASK);

	while ( Seconds != REPETATIONS);

	/*
	 * Disable the interrupt of the RTC device so interrupts will not occur.
	 */
	XRtcPsu_ClearInterruptMask(RtcInstPtr, XRTC_INT_DIS_SECS_MASK);

	return XST_SUCCESS;
}

/**************************************************************************/
/**
*
* @brief Handles interrupt events from the device.
*
* It is called from an interrupt context, so the amount of processing must
* be minimal.
*
* This handler provides an example of how to handle interrupt data for the
* device and is application specific.
*
* @param[in]	CallBackRef Callback reference from the driver. In this
*		case it is the instance pointer for the XRtcPsu driver.
* @param[in]	Event Specific kind of event that has occurred.
*
***************************************************************************/
void Handler(void *CallBackRef, u32 Event)
{
	Xil_AssertVoid(CallBackRef != NULL);
	/* A new second event */
	if (Event == XRTCPSU_EVENT_SECS_GEN) {
		Seconds++;
	}
}

#ifndef SDT
/*****************************************************************************/
/**
*
* @brief Sets up the interrupt system so interrupts can occur for the RTC.
*
* This function is application-specific. Modify this function to fit the
* application.
*
* @param[in,out] IntcInstancePtr Pointer to the instance of the INTC.
* @param[in]	 RtcInstancePtr Pointer to the instance of the RTC
*		 driver that is connected to the interrupt controller.
* @param[in]	 RtcIntrId Interrupt ID, typically the
*		 XPAR_<XRTCPSU_instance>_INTR value defined in xparameters.h.
*
* @return	 XST_SUCCESS if successful, or XST_FAILURE otherwise.
*
****************************************************************************/
static int SetupInterruptSystem(XScuGic *IntcInstancePtr,
				XRtcPsu *RtcInstancePtr,
				u16 RtcIntrId)
{
	int Status;

#ifndef TESTAPP_GEN
	XScuGic_Config *IntcConfig; /* Config for interrupt controller */

	/* Initialize the interrupt controller driver */
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
	 * Connect the interrupt controller interrupt handler to the
	 * hardware interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler) XScuGic_InterruptHandler,
				     IntcInstancePtr);
#endif

	/*
	 * Connect a device driver handler that will be called when an
	 * interrupt for the device occurs, the device driver handler
	 * performs the specific interrupt processing for the device
	 */
	Status = XScuGic_Connect(IntcInstancePtr, RtcIntrId,
				 (Xil_ExceptionHandler) XRtcPsu_InterruptHandler,
				 (void *) RtcInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Enable the interrupt for the device */
	XScuGic_Enable(IntcInstancePtr, RtcIntrId);


#ifndef TESTAPP_GEN
	/* Enable interrupts */
	Xil_ExceptionEnable();
#endif

	return XST_SUCCESS;
}
#endif
