/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
*
* @file xil_nested_interrupts_example.c
*
* Implements example that demonstrates usage of macros available for nested
* interrupt handling in xil_exception.h. This example can be used on
* Cortex-A9, Cortex-A53 (64 bit mode), Cortex-A72 (only at EL1 NS) and Cortex-R5
* based platforms. This example would work only if TTC is present and connected
* to interrupt controller in targeted HW design.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 7.3   mus  04/16/20  First release of example which demonstrates nested
*                      interrupt handling.
* 9.1   ml   02/29/24  Fixed compilation error.
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#if !defined (__microblaze__) && !defined(ARMA53_32)
#include <stdio.h>
#include <stdlib.h>
#include "xparameters.h"
#include "xstatus.h"
#include "xil_io.h"
#include "xil_exception.h"
#include "xttcps.h"
#include "xscugic.h"
#include "xil_printf.h"
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
#define TTC_CNT0_TICK_DEVICE_ID	XPAR_XTTCPS_0_DEVICE_ID
#define TTC_CNT1_TICK_DEVICE_ID	XPAR_XTTCPS_1_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_SCUGIC_0_DEVICE_ID
#else
#define TTC_CNT0_BASEADDR XPAR_XTTCPS_0_BASEADDR
#define TTC_CNT1_BASEADDR XPAR_XTTCPS_1_BASEADDR
#define INTC_DIST_BASEADDR XPAR_XSCUGIC_0_BASEADDR
#endif

#define TTC_CNT0_TICK_INTR_ID	XPAR_XTTCPS_0_INTR
#define TTC_CNT1_TICK_INTR_ID	XPAR_XTTCPS_1_INTR

#define TTC_CNT0_INTR_PRIORITY	0x30
#define TTC_CNT1_INTR_PRIORITY	0x20


/* Tick timer counters' output frequency */
#define	TICK_TIMER0_FREQ_HZ	100000
#define	TICK_TIMER1_FREQ_HZ	100

/**************************** Type Definitions *******************************/
typedef struct {
	u32 OutputHz;	/* Output frequency */
	XInterval Interval;	/* Interval value */
	u8 Prescaler;	/* Prescaler value */
	u16 Options;	/* Option settings */
} TmrCntrSetup;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int TmrNestedInterruptExample();  /* Main test */

/* Set up routines for timer counters */
#ifndef SDT
int SetupTicker(XTtcPs *TtcPsInst, u16 DeviceID, u16 TtcTickIntrID,
		void *TickHandler, XScuGic *InterruptController,
		TmrCntrSetup SettingsTable);
int SetupTimer(u16 DeviceID, XTtcPs *TtcPsInst, TmrCntrSetup SettingsTable);
static int SetupInterruptSystem(u16 IntcDeviceID, XScuGic *IntcInstancePtr);
#else
int SetupTicker(XTtcPs *TtcPsInst, u32 BaseAddr, u32 priority,
		void *TickHandler, TmrCntrSetup SettingsTable);
int SetupTimer(u32 BaseAddr, XTtcPs *TtcPsInst, TmrCntrSetup SettingsTable);
#endif

static void TtcInst0TickHandler(void *CallBackRef1);
static void TtcInst1TickHandler(void *CallBackRef1);
/************************** Variable Definitions *****************************/
/* Ticker timer counter initial setup, only output freq */
static TmrCntrSetup SettingsTable0 =
{TICK_TIMER0_FREQ_HZ, 0, 0, 0};

static TmrCntrSetup SettingsTable1 =
{TICK_TIMER1_FREQ_HZ, 0, 0, 0};

static volatile u8 LowPriorityIntrFlag;	/* Flag to update low priority interrupt counter */
static volatile u8 HighPriorityIntrFlag;	/* Flag to update high priority interrupt counter */

XScuGic InterruptController;  	/* Interrupt controller instance */

/* Timer counter instances */
static XTtcPs TtcPsInst0;
static XTtcPs TtcPsInst1;

/*****************************************************************************/
/**
*
* This is the main function that calls the Nested Timer interrupt example.
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

	xil_printf("Starting nested timer interrupt example \r\n");
	Status = TmrNestedInterruptExample();
	if (Status != XST_SUCCESS) {
		xil_printf("Nested timer interrupt example failed \n");
		return XST_FAILURE;
	}

	xil_printf("Nested timer interrupt example passed \n");
	return XST_SUCCESS;
}
/*****************************************************************************/
/**
*
* This is the main function of the nested interrupt example.
*
* @param	None
*
* @return	XST_SUCCESS to indicate success, else XST_FAILURE to indicate
*		a Failure.
*
****************************************************************************/
int TmrNestedInterruptExample()
{
	u32 Status;
	u32 IntrPriority;
	u32 TtcTickIntrID;
#ifndef SDT
	u32 DeviceID;
#else
	u32 BaseAddr;
#endif
	/*
	 * Connect the Intc to the interrupt subsystem such that interrupts can
	 * occur.  This function is application specific.
	 */
#ifndef SDT
	Status = SetupInterruptSystem(INTC_DEVICE_ID, &InterruptController);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif
	/*
	 * TTC timers are configured to start at the same time with
	 * different frequency
	 */

	/* Configure counter 0 */
#ifndef SDT
	TtcTickIntrID = TTC_CNT0_TICK_INTR_ID;
	DeviceID = TTC_CNT0_TICK_DEVICE_ID;

	/* configure the priority for counter 0 */
	IntrPriority = XScuGic_DistReadReg(&InterruptController,
					   XSCUGIC_PRIORITY_OFFSET_CALC(TtcTickIntrID));
	IntrPriority = (IntrPriority & ~( XSCUGIC_PRIORITY_MASK << ((TtcTickIntrID % 4) * 8)))
		       | (TTC_CNT0_INTR_PRIORITY << ((TtcTickIntrID % 4) * 8));
	XScuGic_DistWriteReg(&InterruptController,
			     XSCUGIC_PRIORITY_OFFSET_CALC(TtcTickIntrID), IntrPriority);
#else
	BaseAddr = TTC_CNT0_BASEADDR;
#endif

#ifndef SDT
	Status = SetupTicker(&TtcPsInst0, DeviceID, TtcTickIntrID, (void *) TtcInst0TickHandler,
			     &InterruptController, SettingsTable0);
#else
	Status = SetupTicker(&TtcPsInst0, BaseAddr, TTC_CNT0_INTR_PRIORITY, (void *) TtcInst0TickHandler,
			     SettingsTable0);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Configure counter 1 */
#ifndef SDT
	TtcTickIntrID = TTC_CNT1_TICK_INTR_ID;
	DeviceID = TTC_CNT1_TICK_DEVICE_ID;

	/* configure the priority for counter 1 */
	IntrPriority = XScuGic_DistReadReg(&InterruptController,
					   XSCUGIC_PRIORITY_OFFSET_CALC(TtcTickIntrID));
	IntrPriority = (IntrPriority & ~( XSCUGIC_PRIORITY_MASK << ((TtcTickIntrID % 4) * 8)))
		       | (TTC_CNT1_INTR_PRIORITY << ((TtcTickIntrID % 4) * 8));
	XScuGic_DistWriteReg(&InterruptController,
			     XSCUGIC_PRIORITY_OFFSET_CALC(TtcTickIntrID), IntrPriority);
#else
	BaseAddr = TTC_CNT1_BASEADDR;
#endif

#ifndef SDT
	Status = SetupTicker(&TtcPsInst1, DeviceID, TtcTickIntrID, (void *) TtcInst1TickHandler,
			     &InterruptController, SettingsTable1);
#else
	Status = SetupTicker(&TtcPsInst1, BaseAddr, TTC_CNT1_INTR_PRIORITY, (void *) TtcInst1TickHandler,
			     SettingsTable1);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* start all the timer at once */
	XTtcPs_Start(&TtcPsInst0);

	XTtcPs_Start(&TtcPsInst1);


	while (LowPriorityIntrFlag == 0 || HighPriorityIntrFlag == 0);

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
* @param	TickHandler is a interrupt handler to be registered with
			the ttc instance
* @param	SettingsTable is ticker timer counter initial setup.
*
* @return	XST_SUCCESS if everything sets up well, XST_FAILURE otherwise.
*
*****************************************************************************/
#ifndef SDT
int SetupTicker(XTtcPs *TtcPsInst, u16 DeviceID, u16 TtcTickIntrID,
		void *TickHandler, XScuGic *InterruptController,
		TmrCntrSetup SettingsTable)
#else
int SetupTicker(XTtcPs *TtcPsInst, u32 BaseAddr, u32 priority,
		void *TickHandler,
		TmrCntrSetup SettingsTable)
#endif
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
	Status = SetupTimer(DeviceID, TtcPsInst, SettingsTable);
#else
	Status = SetupTimer(BaseAddr, TtcPsInst, SettingsTable);
#endif
	if (Status != XST_SUCCESS) {
		return Status;
	}

	TtcPsTick = TtcPsInst;

#ifndef  SDT
	/*
	 * Connect to the interrupt controller
	 */
	Status = XScuGic_Connect(InterruptController, TtcTickIntrID,
				 (Xil_InterruptHandler)TickHandler, (void *)TtcPsTick);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/*
	 * Enable the interrupt for the Timer counter
	 */
	XScuGic_Enable(InterruptController, TtcTickIntrID);
#else

	Status = XSetupInterruptSystem(TtcPsTick, TickHandler,
				       TtcPsTick->Config.IntrId[0],
				       TtcPsTick->Config.IntrParent,
				       priority);
	if (Status != XST_SUCCESS) {
		return Status;
	}
#endif

	/*
	 * Enable the interrupts for the tick timer/counter
	 */
	XTtcPs_EnableInterrupts(TtcPsTick, XTTCPS_IXR_INTERVAL_MASK);

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
#ifndef SDT
int SetupTimer(u16 DeviceID, XTtcPs *TtcPsInst, TmrCntrSetup SettingsTable)
#else
int SetupTimer(u32 BaseAddr, XTtcPs *TtcPsInst, TmrCntrSetup SettingsTable)
#endif
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
* This function is the handler, which updates the flag when
* interrupts for TTC counter configured with lower interrupt priority
* are triggered. It enables the nested interrupts and does not exit till
* other counter with higher priority interrupt is not triggered.
*
* @param	CallBackRef contains a callback reference from the driver, in
*		this case it is the instance pointer for the TTC driver.
*
* @return	None.
*
*****************************************************************************/

static void TtcInst0TickHandler(void *CallBackRef1)
{
	u32 StatusEvent;

	/* stop the counter first to avoid any further interrupt generation */
	XTtcPs_Stop((XTtcPs *)CallBackRef1);

	/* update the flag to indicate the interrupt */
	LowPriorityIntrFlag++;

	/*
	 * Read the interrupt status, then write it back to clear the interrupt.
	 */
	StatusEvent = XTtcPs_GetInterruptStatus((XTtcPs *)CallBackRef1);
	XTtcPs_ClearInterruptStatus((XTtcPs *)CallBackRef1, StatusEvent);

	/* Enable the nested interrupts to allow preemption */
	Xil_EnableNestedInterrupts();

	/* wait till interrupts from counter configured with high priority interrupt */
	while (HighPriorityIntrFlag == 0);

	/* Disable the nested interrupt before exiting IRQ mode */
	Xil_DisableNestedInterrupts();
}

/***************************************************************************/
/**
*
* This function is the interrupt handler for TTC counter configured with high
* priority interrupt. It preempts the interrupt handler of TTC counter
* configured with low priority (TtcInst0TickHandler), and increments
* HighPriorityIntrFlag.
*
* @param        CallBackRef contains a callback reference from the driver, in
*               this case it is the instance pointer for the TTC driver.
*
* @return       None.
*
*****************************************************************************/

static void TtcInst1TickHandler(void *CallBackRef1)
{
	u32 StatusEvent;

	/* stop the counter first to avoid any further interrupt generation */
	XTtcPs_Stop((XTtcPs *)CallBackRef1);

	/*
	 * Read the interrupt status, then write it back to clear the interrupt.
	 */
	StatusEvent = XTtcPs_GetInterruptStatus((XTtcPs *)CallBackRef1);
	XTtcPs_ClearInterruptStatus((XTtcPs *)CallBackRef1, StatusEvent);

	/* update the flag to indicate the interrupt */
	HighPriorityIntrFlag++;

}

#endif
