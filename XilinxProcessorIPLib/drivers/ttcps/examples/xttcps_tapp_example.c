/******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file  xttcps_tapp_example.c
*
* This file contains an example uses ttc to generate interrupt and
* update a flag which is checked in interrupt example to confirm whether the
* interrupt is generated or not.
*
*
* @note
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who    Date     Changes
* ---- ------ -------- ---------------------------------------------
* 3.00 pkp    01/12/15 First release
* 3.01 pkp	  01/30/16 Modified SetupTimer to remove XTtcps_Stop before TTC
*					   configuration as it is added in xttcps.c in
*					   XTtcPs_CfgInitialize
* 3.01 pkp	  03/04/16 Added status check after SetupTicker is called by
*					   TmrInterruptExample
* 3.2  mus    10/28/16 Updated TmrCntrSetup as per prototype of
*                      XTtcPs_CalcIntervalFromFreq
*      ms     01/23/17 Modified xil_printf statement in main function to
*                      ensure that "Successfully ran" and "Failed" strings
*                      are available in all examples. This is a fix for
*                      CR-965028.
* 3.10 aru    05/30/19 Updated the example to use XTtcPs_InterruptHandler().
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include <stdio.h>
#include <stdlib.h>
#include "xparameters.h"
#include "xstatus.h"
#include "xil_io.h"
#include "xil_exception.h"
#include "xttcps.h"
#include "xscugic.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are only defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef TESTAPP_GEN
#define TTC_TICK_DEVICE_ID	XPAR_XTTCPS_0_DEVICE_ID
#define TTC_TICK_INTR_ID	XPAR_XTTCPS_0_INTR
#endif

#define INTC_DEVICE_ID		XPAR_SCUGIC_0_DEVICE_ID
#define	TICK_TIMER_FREQ_HZ	100  /* Tick timer counter's output frequency */

/**************************** Type Definitions *******************************/
typedef struct {
	u32 OutputHz;	/* Output frequency */
	XInterval Interval;	/* Interval value */
	u8 Prescaler;	/* Prescaler value */
	u16 Options;	/* Option settings */
} TmrCntrSetup;

/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int TmrInterruptExample(XTtcPs *TtcPsInst,u16 DeviceID, u16 TtcTickIntrID,
						XScuGic *InterruptController);  /* Main test */

/* Set up routines for timer counters */
static int SetupTicker(XTtcPs *TtcPsInst,u16 DeviceID, u16 TtcTickIntrID,
						XScuGic *InterruptController);
static int SetupTimer(u16 DeviceID, XTtcPs *TtcPsInst);

static int SetupInterruptSystem(u16 IntcDeviceID, XScuGic *IntcInstancePtr);

static void TickHandler(void *CallBackRef, u32 StatusEvent);

/************************** Variable Definitions *****************************/
static TmrCntrSetup SettingsTable=
	{TICK_TIMER_FREQ_HZ, 0, 0, 0};	/* Ticker timer counter initial setup,
									only output freq */

static volatile u8 UpdateFlag;	/* Flag to update the seconds counter */

#ifndef TESTAPP_GEN
XScuGic InterruptController;  	/* Interrupt controller instance */
static XTtcPs TtcPsInst;  /* Timer counter instance */
#endif


/*****************************************************************************/
/**
*
* This is the main function that calls the TTC interrupt example.
*
* @param	None
*
* @return
*		- XST_SUCCESS to indicate Success
*		- XST_FAILURE to indicate a Failure.
*
* @note		None.
*
*****************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

	xil_printf("Starting Timer interrupt Example \r\n");
	Status = TmrInterruptExample(&TtcPsInst, TTC_TICK_DEVICE_ID,
								TTC_TICK_INTR_ID, &InterruptController);
	if (Status != XST_SUCCESS) {
		xil_printf("ttcps tapp Example Failed\r\n!");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran ttcps tapp Example\r\n");
	return XST_SUCCESS;
}
#endif
/*****************************************************************************/
/**
*
* This is the main function of the interrupt example.
*
*
* @param	TtcPsInst is a pointer to the ttc instance.
* @param	DeviceID is the unique ID for the device.
* @param	TtcTickIntrID is the unique interrupt ID for the timer.
* @param	InterruptController is a pointer to the interrupt controller
*			instance..
*
* @return	XST_SUCCESS to indicate success, else XST_FAILURE to indicate
*		a Failure.
*
****************************************************************************/
int TmrInterruptExample(XTtcPs *TtcPsInst,u16 DeviceID,u16 TtcTickIntrID,
						XScuGic *InterruptController)
{
	int Status,Index;

	/*
	 * Connect the Intc to the interrupt subsystem such that interrupts can
	 * occur.  This function is application specific.
	 */
	Status = SetupInterruptSystem(INTC_DEVICE_ID, InterruptController);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/*
	 * TTC interrupt example is setup and run for all the three timer/counter
	 * one after another.
	 */
	for(Index = 0; Index < 3; Index++){

		UpdateFlag = 0;

		/*
		 * Set up  the Ticker timer
		 */

		Status = SetupTicker(TtcPsInst,DeviceID,TtcTickIntrID,
						InterruptController);
		if (Status != XST_SUCCESS) {
				return XST_FAILURE;
		}
		/*
		 * The Ticker interrupt sets a flag for update.
		 * Wait until the flag is updated by interrupt routine
		 */

		while (!UpdateFlag);
		DeviceID++;
		TtcTickIntrID++;

		/*
		 * Stop the counter
		 */
		XTtcPs_Stop(TtcPsInst);
	}


	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function sets up the Ticker timer.
*
* @param	TtcPsInst is a pointer to the ttc instance.
* @param	DeviceID is the unique ID for the device.
* @param	TtcTickIntrID is the unique interrupt ID for the timer.
* @param	InterruptController is a pointer to the interrupt controller
*			instance..
*
* @return	XST_SUCCESS if everything sets up well, XST_FAILURE otherwise.
*
*****************************************************************************/
int SetupTicker(XTtcPs *TtcPsInst,u16 DeviceID,u16 TtcTickIntrID,
				XScuGic *InterruptController)
{
	int Status;
	TmrCntrSetup *TimerSetup;
	XTtcPs *TtcPsTick;

	TimerSetup = &SettingsTable;

	/*
	 * Set up appropriate options for Ticker: interval mode without
	 * waveform output.
	 */
	TimerSetup->Options |= (XTTCPS_OPTION_INTERVAL_MODE |
					      XTTCPS_OPTION_WAVE_DISABLE);

	/*
	 * Calling the timer setup routine
	 *  . initialize device
	 *  . set options
	 */
	Status = SetupTimer(DeviceID,TtcPsInst);
	if(Status != XST_SUCCESS) {
		return Status;
	}

	TtcPsTick = TtcPsInst;

	/*
	 * Connect to the interrupt controller
	 */
	Status = XScuGic_Connect(InterruptController, TtcTickIntrID,
		(Xil_InterruptHandler)XTtcPs_InterruptHandler, (void *)TtcPsTick);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	XTtcPs_SetStatusHandler(TtcPsInst, TtcPsInst,
					 (XTtcPs_StatusHandler) TickHandler);

	/*
	 * Enable the interrupt for the Timer counter
	 */
	XScuGic_Enable(InterruptController, TtcTickIntrID);

	/*
	 * Enable the interrupts for the tick timer/counter
	 */
	XTtcPs_EnableInterrupts(TtcPsTick, XTTCPS_IXR_INTERVAL_MASK);

	/*
	 * Start the tick timer/counter
	 */
	XTtcPs_Start(TtcPsTick);

	return Status;
}
/****************************************************************************/
/**
*
* This function sets up a timer counter device, using the information in its
* setup structure.
*  . initialize device
*  . set options
*  . set interval and prescaler value for given output frequency.
*
* @param	DeviceID is the unique ID for the device.
* @param	TtcPsInst is a pointer to the ttc instance.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
*****************************************************************************/
int SetupTimer(u16 DeviceID,XTtcPs *TtcPsInst)
{
	int Status;
	XTtcPs_Config *Config;
	XTtcPs *Timer;
	TmrCntrSetup *TimerSetup;

	TimerSetup = &SettingsTable;

	Timer = TtcPsInst;

	/*
	 * Look up the configuration based on the device identifier
	 */
	Config = XTtcPs_LookupConfig(DeviceID);
	if (NULL == Config) {
		return XST_FAILURE;
	}
	/*
	 * Initialize the device
	 */
	Status = XTtcPs_CfgInitialize(Timer, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the options
	 */
	XTtcPs_SetOptions(Timer, TimerSetup->Options);

	/*
	 * Timer frequency is preset in the TimerSetup structure,
	 * however, the value is not reflected in its other fields, such as
	 * IntervalValue and PrescalerValue. The following call will map the
	 * frequency to the interval and prescaler values.
	 */
	XTtcPs_CalcIntervalFromFreq(Timer, TimerSetup->OutputHz,
		&(TimerSetup->Interval), &(TimerSetup->Prescaler));

	/*
	 * Set the interval and prescale
	 */
	XTtcPs_SetInterval(Timer, TimerSetup->Interval);
	XTtcPs_SetPrescaler(Timer, TimerSetup->Prescaler);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function setups the interrupt system such that interrupts can occur.
* This function is application specific since the actual system may or may not
* have an interrupt controller.  The TTC could be directly connected to a
* processor without an interrupt controller.  The user should modify this
* function to fit the application.
*
* @param	IntcDeviceID is the unique ID of the interrupt controller
* @param	IntcInstacePtr is a pointer to the interrupt controller
*		instance.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
*****************************************************************************/
static int SetupInterruptSystem(u16 IntcDeviceID,
				    XScuGic *IntcInstancePtr)
{
	int Status;
	XScuGic_Config *IntcConfig; /* The configuration parameters of the
					   interrupt controller */

	/*
	 * Initialize the interrupt controller driver
	 */
	IntcConfig = XScuGic_LookupConfig(IntcDeviceID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
					IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the ARM processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
				(Xil_ExceptionHandler) XScuGic_InterruptHandler,
				IntcInstancePtr);


	/*
	 * Enable interrupts in the ARM
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

/***************************************************************************/
/**
*
* This function is the handler which updates the flag when TTC interrupt is
* occurred
*
* @param	CallBackRef contains a callback reference from the driver, in
*		this case it is the instance pointer for the TTC driver.
*
* @return	None.
*
*****************************************************************************/
static void TickHandler(void *CallBackRef, u32 StatusEvent)
{
	/*update the flag if interrupt has been occurred*/
	UpdateFlag = TRUE;

}
