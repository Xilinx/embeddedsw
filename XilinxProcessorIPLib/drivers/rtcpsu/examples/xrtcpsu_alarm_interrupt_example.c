/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/****************************************************************************/
/**
*
* @file xrtcpsu_alarm_interrupt_example.c
*
* This file contains an alarm example using the XRtcPsu driver in interrupt
* mode. It sets alarm for a specified time from the current time.
*
*
* @note
* The example contains an infinite loop such that if interrupts are not
* working it may hang.
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
#define RTC_ALARM_INT_IRQ_ID	XPAR_XRTCPSU_ALARM_INTR

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define ALARM_PERIOD 10U

/************************** Function Prototypes *****************************/

int RtcPsuAlarmIntrExample(XScuGic *IntcInstPtr, XRtcPsu *RtcInstPtr,
			u16 DeviceId, u16 RtcIntrId);


static int SetupInterruptSystem(XScuGic *IntcInstancePtr,
				XRtcPsu *RtcInstancePtr,
				u16 RtcIntrId);

void Handler(void *CallBackRef, u32 Event);


/************************** Variable Definitions ***************************/

XRtcPsu RtcPsu;		/* Instance of the RTC Device */
XScuGic InterruptController;	/* Instance of the Interrupt Controller */
u32 IsAlarmGen;


/**************************************************************************/
/**
*
* Main function to call the RTC Alarm interrupt example.
*
* @param	None
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
	Status = RtcPsuAlarmIntrExample(&InterruptController, &RtcPsu,
				RTC_DEVICE_ID, RTC_ALARM_INT_IRQ_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("RTC Alarm Interrupt Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran RTC Alarm Interrupt Example Test\r\n");
	return XST_SUCCESS;
}
#endif

/**************************************************************************/
/**
*
* This function does a minimal test on the Rtc device and driver as a
* design example. The purpose of this function is to illustrate
* how to use alarm feature in the XRtcPsu driver.
*
* This function sets alarm for a specified time from the current time.
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
* This function contains an infinite loop such that if interrupts are not
* working it may never return.
*
**************************************************************************/
int RtcPsuAlarmIntrExample(XScuGic *IntcInstPtr, XRtcPsu *RtcInstPtr,
			u16 DeviceId, u16 RtcIntrId)
{
	int Status;
	XRtcPsu_Config *Config;
	u32 CurrentTime, Alarm;
	XRtcPsu_DT dt0;

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

	xil_printf("\n\rDay Convention : 0-Fri, 1-Sat, 2-Sun, 3-Mon, 4-Tue, 5-Wed, 6-Thur\n\r");
	xil_printf("Current RTC time is..\n\r");
	CurrentTime = XRtcPsu_GetCurrentTime(RtcInstPtr);
	XRtcPsu_SecToDateTime(CurrentTime,&dt0);
	xil_printf("YEAR:MM:DD HR:MM:SS \t %04d:%02d:%02d %02d:%02d:%02d\t Day = %d\n\r",
			dt0.Year,dt0.Month,dt0.Day,dt0.Hour,dt0.Min,dt0.Sec,dt0.WeekDay);

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
	XRtcPsu_SetInterruptMask(RtcInstPtr, XRTC_INT_EN_ALRM_MASK );

	CurrentTime = XRtcPsu_GetCurrentTime(RtcInstPtr);
	Alarm = CurrentTime + ALARM_PERIOD;
	XRtcPsu_SetAlarm(RtcInstPtr,Alarm,0);

	while( IsAlarmGen != 1);

	/*
	 * Disable the interrupt of the RTC device so interrupts will not occur.
	 */
	XRtcPsu_ClearInterruptMask(RtcInstPtr,XRTC_INT_DIS_ALRM_MASK);

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
	/* Alarm event */
	if (Event == XRTCPSU_EVENT_ALARM_GEN) {
		IsAlarmGen = 1;
		xil_printf("Alarm generated.\n\r");
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
*		XPAR_<XRTCPSU_instance>_INTR value from xparameters_ps.h.
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
