/******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file	xrtcpsu_seconds_interrupt_example.c
*
* This file contains an seconds example using the XRtcPsu driver in interrupt
* mode.
*
*
* @note
* In the example,if interrupts are not working it may hang.
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- ----------------------------------------------
* 1.00  kvn 05/12/15 First Release
*       ms  04/10/17 Modified filename tag to include the file in doxygen
*                    examples.
* </pre>
****************************************************************************/

/***************************** Include Files *******************************/

#include "xparameters.h"	/* SDK generated parameters */
#include "xrtcpsu.h"		/* RTCPSU device driver */
#include "xscugic.h"		/* Interrupt controller device driver */
#include "xil_exception.h"
#include "xil_printf.h"

/************************** Constant Definitions **************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */

#define RTC_DEVICE_ID         XPAR_XRTCPSU_0_DEVICE_ID
#define INTC_DEVICE_ID			XPAR_SCUGIC_SINGLE_DEVICE_ID
#define RTC_SEC_INT_IRQ_ID	XPAR_XRTCPSU_SECONDS_INTR


/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define REPETATIONS 10

/************************** Function Prototypes *****************************/

int RtcPsuSecondsIntrExample(XScuGic *IntcInstPtr, XRtcPsu *RtcInstPtr,
			u16 DeviceId, u16 RtcIntrId);


static int SetupInterruptSystem(XScuGic *IntcInstancePtr,
				XRtcPsu *RtcInstancePtr,
				u16 RtcIntrId);

void Handler(void *CallBackRef, u32 Event);


/************************** Variable Definitions ***************************/

XRtcPsu RtcPsu;		/* Instance of the RTC Device */
XScuGic InterruptController;	/* Instance of the Interrupt Controller */
u32 Seconds = 0;


/**************************************************************************/
/**
*
* Main function to call the RTC Seconds interrupt example.
*
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful
*
* @note		None
*
**************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

	/* Run the RtcPsu Interrupt example, specify the the Device ID */
	Status = RtcPsuSecondsIntrExample(&InterruptController, &RtcPsu,
				RTC_DEVICE_ID, RTC_SEC_INT_IRQ_ID);
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
* This function does a minimal test on the Rtc device and driver as a
* design example. The purpose of this function is to illustrate
* how to seconds feature is available in the XRtcPsu driver.
*
* This function shows the seconds interrupt feature..
*
* @param	IntcInstPtr is a pointer to the instance of the ScuGic driver.
* @param	RtcInstPtr is a pointer to the instance of the RTC driver
*		which is going to be connected to the interrupt controller.
* @param	DeviceId is the device Id of the RTC device and is typically
*		XPAR_<RTCPSU_instance>_DEVICE_ID value from xparameters.h.
* @param	RtcIntrId is the interrupt Id and is typically
*		XPAR_<RTCPSU_instance>_INTR value from xparameters.h.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note
*
* In this function ,if interrupts are not working it may never return.
*
**************************************************************************/
int RtcPsuSecondsIntrExample(XScuGic *IntcInstPtr, XRtcPsu *RtcInstPtr,
			u16 DeviceId, u16 RtcIntrId)
{
	int Status;
	XRtcPsu_Config *Config;

	/*
	 * Initialize the RTC driver so that it's ready to use
	 * Look up the configuration in the config table, then initialize it.
	 */
	Config = XRtcPsu_LookupConfig(DeviceId);
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
	Status = SetupInterruptSystem(IntcInstPtr, RtcInstPtr, RtcIntrId);
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
	XRtcPsu_SetInterruptMask(RtcInstPtr,XRTC_INT_EN_SECS_MASK);

	while( Seconds != REPETATIONS);

	/*
	 * Disable the interrupt of the RTC device so interrupts will not occur.
	 */
	XRtcPsu_ClearInterruptMask(RtcInstPtr,XRTC_INT_DIS_SECS_MASK);

	return XST_SUCCESS;
}

/**************************************************************************/
/**
*
* This function is the handler which performs processing to handle interrupt
* events from the device.  It is called from an interrupt context. so the
* amount of processing should be minimal.
*
* This handler provides an example of how to handle interrupt data for the
* device and is application specific.
*
* @param	CallBackRef contains a callback reference from the driver,
*		in this case it is the instance pointer for the XRtcPsu driver.
* @param	Event contains the specific kind of event that has occurred.
*
* @return	None.
*
* @note		None.
*
***************************************************************************/
void Handler(void *CallBackRef, u32 Event)
{
	/* A new second event */
	if (Event == XRTCPSU_EVENT_SECS_GEN) {
		Seconds++;
		xil_printf("A new second is generated.\n\r");
	}
}


/*****************************************************************************/
/**
*
* This function sets up the interrupt system so interrupts can occur for the
* RTC. This function is application-specific. The user should modify this
* function to fit the application.
*
* @param	IntcInstancePtr is a pointer to the instance of the INTC.
* @param	RtcInstancePtr contains a pointer to the instance of the RTC
*		driver which is going to be connected to the interrupt
*		controller.
* @param	RtcIntrId is the interrupt Id and is typically
*		XPAR_<XRTCPSU_instance>_INTR value from xparameters.h.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
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
