/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file  xttcps_rtc_example.c
*
* This example uses one timer/counter to make a real time clock. The number of
* minutes and seconds is displayed on the console.
*
*
* @note
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who    Date     Changes
* ---- ------ -------- ---------------------------------------------
* 1.00 drg/jz 01/23/10 First release
* 3.01 pkp	  01/30/16 Modified SetupTimer to remove XTtcps_Stop before TTC
*					   configuration as it is added in xttcps.c in
*					   XTtcPs_CfgInitialize
* 3.2  mus    10/28/16 Updated TmrCntrSetup as per prototype of
*                      XTtcPs_CalcIntervalFromFreq
*      ms     01/23/17 Modified xil_printf statement in main function to
*                      ensure that "Successfully ran" and "Failed" strings
*                      are available in all examples. This is a fix for
*                      CR-965028.
* 3.10 aru    05/30/19 Updated the example to use XTtcPs_InterruptHandler().
* 3.16 adk    04/19/22 Fix infinite loop in the example by adding polled
* 		       timeout loop.
* 3.18 gm     07/17/23 Invoke XTtcPs_Release to release ttc node.
* 3.18 gm     03/06/24 Added support for system device-tree flow.
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
#include "sleep.h"
#ifdef SDT
#include "xinterrupt_wrap.h"
#endif

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are only defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define TTC_TICK_DEVICE_ID	XPAR_XTTCPS_0_DEVICE_ID
#define TTC_TICK_INTR_ID	XPAR_XTTCPS_0_INTR
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#else
#define TTC_TICK_BASEADDR   XPAR_XTTCPS_0_BASEADDR
#endif

/*
 * Constants to set the basic operating parameters.
 * PWM_DELTA_DUTY is critical to the running time of the test. Smaller values
 * make the test run longer.
 */
#define	TICK_TIMER_FREQ_HZ	100  /* Tick timer counter's output frequency */

#define TICKS_PER_CHANGE_PERIOD	TICK_TIMER_FREQ_HZ /* Tick signals per update */
#define XTTCPS_SW_TIMEOUT_VAL	180 /* wait for 3 minutes */

/**************************** Type Definitions *******************************/
typedef struct {
	u32 OutputHz;	/* Output frequency */
	XInterval Interval;	/* Interval value */
	u8 Prescaler;	/* Prescaler value */
	u16 Options;	/* Option settings */
} TmrCntrSetup;

/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

static int TmrRtcInterruptExample(void);  /* Main test */

/* Set up routines for timer counters */
#ifndef SDT
static int SetupTimer(int DeviceID);
#else
static int SetupTimer(u32 BaseAddr);
#endif
static int SetupTicker(void);
/* Interleaved interrupt test for both timer counters */
static int WaitForDutyCycleFull(void);

#ifndef SDT
static int SetupInterruptSystem(u16 IntcDeviceID, XScuGic *IntcInstancePtr);
#endif
static void TickHandler(void *CallBackRef, u32 StatusEvent);

/************************** Variable Definitions *****************************/

static XTtcPs TtcPsInst;  /* Timer counter instance */

static TmrCntrSetup SettingsTable=
	{TICK_TIMER_FREQ_HZ, 0, 0, 0};	/* Ticker timer counter initial setup,
						only output freq */

XScuGic InterruptController;  	/* Interrupt controller instance */

static u8 ErrorCount;		/* Errors seen at interrupt time */
static volatile u8 UpdateFlag;	/* Flag to update the seconds counter */
static u32 TickCount;		/* Ticker interrupts between seconds change */


/*****************************************************************************/
/**
*
* This is the main function that calls the TTC RTC interrupt example.
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
int main(void)
{
	int Status;

	xil_printf("Starting Timer RTC Example");
	Status = TmrRtcInterruptExample();
	if (Status != XST_SUCCESS) {
		xil_printf("ttcps rtc Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran ttcps rtc Example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This is the main function of the interrupt example.
*
*
* @param	None.
*
* @return	XST_SUCCESS to indicate success, else XST_FAILURE to indicate
*		a Failure.
*
****************************************************************************/
static int TmrRtcInterruptExample(void)
{
	int Status;

	/*
	 * Make sure the interrupts are disabled, in case this is being run
	 * again after a failure.
	 */
#ifndef SDT
	/*
	 * Connect the Intc to the interrupt subsystem such that interrupts can
	 * occur.  This function is application specific.
	 */
	Status = SetupInterruptSystem(INTC_DEVICE_ID, &InterruptController);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif

	/*
	 * Set up  the Ticker timer
	 */
	Status = SetupTicker();
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Status = WaitForDutyCycleFull();
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Stop the counters
	 */
	XTtcPs_Stop(&TtcPsInst);

	XTtcPs_Release(&TtcPsInst);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function sets up the Ticker timer.
*
* @param	None
*
* @return	XST_SUCCESS if everything sets up well, XST_FAILURE otherwise.
*
*****************************************************************************/
int SetupTicker(void)
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
#ifndef SDT
	Status = SetupTimer(TTC_TICK_DEVICE_ID);
#else
	Status = SetupTimer(TTC_TICK_BASEADDR);
#endif
	if(Status != XST_SUCCESS) {
		return Status;
	}

	TtcPsTick = &TtcPsInst;

	/*
	 * Connect to the interrupt controller
	 */
#ifndef SDT
	Status = XScuGic_Connect(&InterruptController, TTC_TICK_INTR_ID,
		(Xil_InterruptHandler)XTtcPs_InterruptHandler, (void *)TtcPsTick);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XTtcPs_SetStatusHandler(TtcPsTick, TtcPsTick,
				 (XTtcPs_StatusHandler) TickHandler);
	/*
	 * Enable the interrupt for the Timer counter
	 */
	XScuGic_Enable(&InterruptController, TTC_TICK_INTR_ID);
#else

	Status = XSetupInterruptSystem(TtcPsTick, XTtcPs_InterruptHandler,
				       TtcPsTick->Config.IntrId[0],
				       TtcPsTick->Config.IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XTtcPs_SetStatusHandler(TtcPsTick, TtcPsTick,
				 (XTtcPs_StatusHandler) TickHandler);

#endif
	/*
	 * Enable the interrupts for the tick timer/counter
	 * We only care about the interval timeout.
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
* This function uses the interrupt inter-locking between the ticker timer
* counter and the waveform output timer counter. When certain amount of
* interrupts have happened to the ticker timer counter, a flag, UpdateFlag,
* is set to true.
*
*
* @param	None
*
* @return	XST_SUCCESS if duty cycle successfully reaches beyond 100,
* 		otherwise XST_FAILURE.
*
*****************************************************************************/
int WaitForDutyCycleFull(void)
{
	u32 seconds;
	u32 PollCount = XTTCPS_SW_TIMEOUT_VAL;

	/*
	 * Initialize some variables used by the interrupts and in loops.
	 */

	ErrorCount = 0;
	seconds = 0;

	/*
	 * Loop until two minutes passes.
	 */
	while (seconds <= 121) {

		/*
		 * If error occurs, disable interrupts, and exit.
		 */
		if ((0 != ErrorCount) || (PollCount == 0)) {
			return XST_FAILURE;
		}

		/*
		 * The Ticker interrupt sets a flag for update.
		 */
		if (UpdateFlag) {
			/*
			 * Calculate the time setting here, not at the time
			 * critical interrupt level.
			 */
			seconds++;
			UpdateFlag = FALSE;

			xil_printf("Time: %d\n\r", seconds);

		} else {
			PollCount--;
			sleep(1U);
		}
	}

	return XST_SUCCESS;
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
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
*****************************************************************************/
#ifndef SDT
int SetupTimer(int DeviceID)
#else
int SetupTimer(u32 BaseAddr)
#endif
{
	int Status;
	XTtcPs_Config *Config;
	XTtcPs *Timer;
	TmrCntrSetup *TimerSetup;

	TimerSetup = &SettingsTable;

	Timer = &TtcPsInst;

	/*
	 * Look up the configuration based on the device identifier
	 */
#ifndef SDT
	Config = XTtcPs_LookupConfig(DeviceID);
#else
	Config = XTtcPs_LookupConfig(BaseAddr);
#endif
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
#ifndef SDT
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
#endif
/***************************************************************************/
/**
*
* This function is the handler which handles the periodic tick interrupt.
* It updates its count, and set a flag to signal PWM timer counter to
* update its duty cycle.
*
* This handler provides an example of how to handle data for the TTC and
* is application specific.
*
* @param	CallBackRef contains a callback reference from the driver, in
*		this case it is the instance pointer for the TTC driver.
*
* @return	None.
*
*****************************************************************************/
static void TickHandler(void *CallBackRef, u32 StatusEvent)
{

	if (0 != (XTTCPS_IXR_INTERVAL_MASK & StatusEvent)) {
		TickCount++;

		if (TICKS_PER_CHANGE_PERIOD == TickCount) {
			TickCount = 0;
			UpdateFlag = TRUE;
		}

	}
	else {
		/*
		 * The Interval event should be the only one enabled. If it is
		 * not it is an error
		 */
		ErrorCount++;
	}
}
