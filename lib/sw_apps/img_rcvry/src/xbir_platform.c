/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xbir_platform.c
*
* This file contains platform specific API's for System Board.
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xparameters_ps.h"	/* Defines XPAR values */
#include "xscugic.h"
#include "lwip/tcp.h"
#include "xbir_config.h"
#include "netif/xadapter.h"
#include "xbir_platform.h"

/************************** Constant Definitions *****************************/
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define TIMER_DEVICE_ID		XPAR_XTTCPS_0_DEVICE_ID
#define TIMER_IRPT_INTR		XPAR_XTTCPS_0_INTR
#define INTC_BASE_ADDR		XPAR_SCUGIC_0_CPU_BASEADDR
#define INTC_DIST_BASE_ADDR	XPAR_SCUGIC_0_DIST_BASEADDR
#define PLATFORM_TIMER_INTR_RATE_HZ	(4U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
static XTtcPs TimerInstance = {0U};
volatile u8 TcpFastTmrFlag = FALSE;
volatile u8 TcpSlowTmrFlag = FALSE;

/*****************************************************************************/
/**
 * @brief
 * This function is callback function for handling timer interrupts.
 *
 * @param	TimerInstance	Pointer to instance of XTtcPs
 *
 * @return	None
 *
 *****************************************************************************/
void Xbir_Platform_TimerCallback (void)
{
	/* We need to call tcp_fasttmr & tcp_slowtmr at intervals specified
	 * by lwIP. It is not important that the timing is absoluetly accurate.
	 */
	static u8 Odd = 1U;

	TcpFastTmrFlag = TRUE;
	Odd = ~Odd;
	if (Odd > 0U) {
		TcpSlowTmrFlag = 1U;
	}

	Xbir_Platform_ClearInterrupt ();
}

/*****************************************************************************/
/**
 * @brief
 * This function sets up the platform timer required for lwip.
 *
 * @param	None
 *
 * @return	XST_SUCCESS on success
 *		Error code on failure
 *
 *****************************************************************************/
int Xbir_Platform_SetupTimer (void)
{
	int Status = XST_FAILURE;
	XTtcPs *Timer = &TimerInstance;
	XTtcPs_Config *Config;
	XInterval  Interval;
	u8 Prescaler;

	Config = XTtcPs_LookupConfig(TIMER_DEVICE_ID);
	if (Config == NULL) {
		Xbir_Printf(DEBUG_INFO, " In %s: Look up config failed\r\n", __func__);
		goto END;
	}
	Status = XTtcPs_CfgInitialize(Timer, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		Xbir_Printf(DEBUG_INFO, " In %s: Timer Cfg initialization failed...\r\n",
			__func__);
		goto END;
	}

	XTtcPs_SetOptions(Timer,
		XTTCPS_OPTION_INTERVAL_MODE | XTTCPS_OPTION_WAVE_DISABLE);
	XTtcPs_CalcIntervalFromFreq(Timer, PLATFORM_TIMER_INTR_RATE_HZ,
		&Interval, &Prescaler);
	XTtcPs_SetInterval(Timer, Interval);
	XTtcPs_SetPrescaler(Timer, Prescaler);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function clears the timer interrupt.
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
void Xbir_Platform_ClearInterrupt (void)
{
	u32 StatusEvent;

	StatusEvent = XTtcPs_GetInterruptStatus(&TimerInstance);
	XTtcPs_ClearInterruptStatus(&TimerInstance, StatusEvent);
	(void) StatusEvent;
}

/*****************************************************************************/
/**
 * @brief
 * This function registers interrupt handler for platform interrupts.
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
void Xbir_Platform_SetupInterrupts (void)
{
	Xil_ExceptionInit();

	XScuGic_DeviceInitialize(INTC_DEVICE_ID);

	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
		(Xil_ExceptionHandler)XScuGic_DeviceInterruptHandler,
		(void *)INTC_DEVICE_ID);

	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	XScuGic_RegisterHandler(INTC_BASE_ADDR, TIMER_IRPT_INTR,
		(Xil_ExceptionHandler) Xbir_Platform_TimerCallback,
		(void *) &TimerInstance);

	/* Enable the interrupt for SCU timer */
	XScuGic_EnableIntr(INTC_DIST_BASE_ADDR, TIMER_IRPT_INTR);

	return;
}

/*****************************************************************************/
/**
 * @brief
 * This function enables required platform interrupts.
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
void Xbir_Platform_EnableInterrupts (void)
{
	/* Enable timer interrupts */
	Xil_ExceptionEnableMask(XIL_EXCEPTION_IRQ);
	XTtcPs_EnableInterrupts(&TimerInstance, XTTCPS_IXR_INTERVAL_MASK);
	XTtcPs_Start(&TimerInstance);

	return;
}

/*****************************************************************************/
/**
 * @brief
 * This function initialize platform to run this application.
 *
 * @param	None
 *
 * @return	XST_SUCCESS on success
 *		Error code on failure
 *
 *****************************************************************************/
int Xbir_Platform_Init (void)
{
	int Status = XST_FAILURE;

	Status = Xbir_Platform_SetupTimer();
	if (Status == XST_SUCCESS) {
		Xbir_Platform_SetupInterrupts();
	}

	return Status;
}
