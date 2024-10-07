/**************************************************************************************************
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_init.c
 *
 * This file contains the initialization code for ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   10/11/23 Initial release
 * 1.1   ma   01/02/24 Enable IPI interrupts as IO Module level
 * 1.2   ma   02/02/24 Update TaskTimeNow with PIT3 timer tick
 *       ma   02/08/24 Added performance related APIs
 *       ma   03/16/24 Added error codes at required places
 *       ma   07/23/24 Added RTCA initialization related code
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_init.h"
#include "xparameters.h"
#include "xasufw_debug.h"
#include "xtask.h"
#include "xiomodule.h"
#include "xil_exception.h"
#include "xasufw_ipi.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xasufw_memory.h"

/************************************ Constant Definitions ***************************************/
#define MB_IOMODULE_GPO1_PIT1_PRESCALE_SRC_MASK	(0x2U) /**< IO Module PIT1 prescaler source mask */
#define XASUFW_ASU_IRO_FREQ_IN_HZ				(500000000U) /**< ASU IRO frequency 500Mhz */
#define XASUFW_IOMODULE_IPI_INTRNUM				(28U) /**< IPI interrupt number in IO Module */
#define XASUFW_PIT3_TIMER_TICK					(10U) /**< PIT3 timer tick in milli-seconds */

#define XASUFW_PIT1_RESET_VALUE		(0xFFFFFFFDU) /**< PIT1 reset value */
#define XASUFW_PIT2_RESET_VALUE		(0xFFFFFFFEU) /**< PIT2 reset value */
#define XASUFW_PIT1_CYCLE_VALUE		((u64)XASUFW_PIT1_RESET_VALUE + 1U) /**< PIT1 cycle value */
#define XASUFW_PIT2_CYCLE_VALUE		(XASUFW_PIT2_RESET_VALUE + 1U) /**< PIT2 cycle value */
#define XASUFW_PIT1					(0U) /**< ASUFW PIT1 */
#define XASUFW_PIT2					(1U) /**< ASUFW PIT2 */
#define XASUFW_PIT3					(2U) /**< ASUFW PIT3 */
#define XASUFW_PIT_FREQ_DIVISOR		(100U) /**< ASUFW PIT frequency divisor */
#define XASUFW_MEGA			        (1000000U) /**< Value for mega */
#define XASUFW_KILO			        (1000UL) /**< Value for kilo */
#define XASUFW_WORD_SIZE_IN_BITS	(32U) /**< Define for word size in bits */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static void XAsufw_ExceptionEnable(void);
static void XAsufw_ExceptionHandler(void *Data);
static void XAsufw_InitPitTimer(u8 Timer, u32 ResetValue);
static void XAsufw_Pit3TimerHandler(const void *Data);

/************************************ Variable Definitions ***************************************/
static XIOModule IOModule; /* Instance of the IO Module */

/*************************************************************************************************/
/**
 * @brief	This function initializes interrupts, registers exception handler for all the
 * 		exceptions and enables processor interrupts and exceptions.
 *
 *************************************************************************************************/
static void XAsufw_ExceptionEnable(void)
{
	s32 Status = XASUFW_FAILURE;
	u16 Index;

	XAsufw_Printf(DEBUG_GENERAL, "Exception Init Start\r\n");

	/** Initialize processor registers related to interrupts and exceptions. */
	Xil_ExceptionInit();

	/** Register exception handlers. */
	for (Index = XIL_EXCEPTION_ID_FIRST; Index <= XIL_EXCEPTION_ID_LAST; Index++) {
		Xil_ExceptionRegisterHandler(Index, XAsufw_ExceptionHandler, (void *)Status);
	}

	/* TODO: Register separate handler for illegal instruction trap to execute secure lockdown */

	/** Register the IO module interrupt handler with the exception table. */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler)XIOModule_DeviceInterruptHandler,
				     (void *) XASUFW_IOMODULE_DEVICE_ID);

	/** Enable processor interrupts and exceptions. */
	Xil_ExceptionEnable();

	XAsufw_Printf(DEBUG_GENERAL, "Exception Init Done\r\n");
}

/*************************************************************************************************/
/**
 * @brief	This is a function handler for all processor exceptions which will be called
 * 		whenever any exception occurs.
 *
 * @param	Data	Pointer to the error status.
 *
 *************************************************************************************************/
static void XAsufw_ExceptionHandler(void *Data)
{
	/** Print processor registers when any exception occurs. */
	XAsufw_Printf(DEBUG_PRINT_ALWAYS, "Received Exception \n\r");
	XAsufw_Printf(DEBUG_PRINT_ALWAYS, "CSR(mstatus): 0x%x, CSR(mcause): 0x%x, "
			"CSR(mtval): 0x%x\r\n", csrr(mstatus), csrr(mcause), csrr(mtval));

	/*
	 * TODO: Need to add an illegal instruction trap here so that its respective handler will be
	 * called which will execute secure lockdown
	 */

	/** Enters infinite loop just in case if control reaches here. */
	while (XASU_TRUE) {
		;
	}
}

/*************************************************************************************************/
/**
 * @brief	This function initializes the Programmable Interval Timer. It sets timer options,
 * 		PIT reset values and starts the timer.
 *
 * @param	Timer		PIT timer number to be initialized.
 * @param	ResetValue	Reset value of timer to be written before starting the timer.
 *
 *************************************************************************************************/
static void XAsufw_InitPitTimer(u8 Timer, u32 ResetValue)
{
	/**
	 * When used in PIT1 prescalar to PIT2, PIT2 has least 32bits.
	 * So, PIT2 is reloaded to get 64bit timer value.
	 */
	if ((XASUFW_PIT2 == Timer) || (XASUFW_PIT3 == Timer)) {
		XIOModule_Timer_SetOptions(&IOModule, Timer, XTC_AUTO_RELOAD_OPTION);
	}

	/**
	 * Set a reset value for the Programmable Interval Timers such that they will expire earlier
	 * than letting them roll over from 0, the reset value is loaded into the Programmable Interval
	 * Timers when they are started.
	 */
	XIOModule_SetResetValue(&IOModule, Timer, ResetValue);

	/** Start the Programmable Interval Timers and they are decrementing by default. */
	XIOModule_Timer_Start(&IOModule, Timer);
}

/*************************************************************************************************/
/**
 * @brief	This function is the handler for IO Module PIT3 interrupt which will be called
 * 		whenever PIT3 is expired. ASUFW loads the PIT1, 2 and 3 timers so that PIT3
 * 		expires for every 10ms.
 *
 * @param	Data	Interrupt number that is received.
 *
 *************************************************************************************************/
static void XAsufw_Pit3TimerHandler(const void *Data)
{
	/* Update TaskTimeNow every time the scheduler handler is called for every 10ms */
	TaskTimeNow += XASUFW_PIT3_TIMER_TICK;
}

/*************************************************************************************************/
/**
 * @brief	This function is called during boot up of ASUFW to initialize IO module and
 * 		PIT timer, and also to start IO module.
 *
 * @return
 * 	- Returns XASUFW_SUCCESS on successful initialization of IOModule.
 * 	- XASUFW_IOMODULE_INIT_FAILED, if IO module initialization fails.
 * 	- XASUFW_IOMODULE_SELF_TEST_FAILED, if IO module self test fails.
 * 	- XASUFW_IOMODULE_START_FAILED, if IO module start fails.
 *
 *************************************************************************************************/
s32 XAsufw_StartTimer(void)
{
	s32 Status = XASUFW_FAILURE;
	u32 Pit1ResetValue = XASUFW_PIT1_RESET_VALUE;
	u32 Pit2ResetValue = XASUFW_PIT2_RESET_VALUE;
	u32 Pit3ResetValue = XASUFW_ASU_IRO_FREQ_IN_HZ / XASUFW_PIT_FREQ_DIVISOR;

	/**
	 * Initialize the IO Module so that it's ready to use, specify the device ID that is
	 * generated in xparameters.h.
	 */
	Status = XIOModule_Initialize(&IOModule, XASUFW_IOMODULE_DEVICE_ID);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(XASUFW_IOMODULE_INIT_FAILED, Status);
		goto END;
	}

	/** Perform a self-test to ensure that the hardware was built correctly. */
	Status = XIOModule_SelfTest(&IOModule);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(XASUFW_IOMODULE_SELF_TEST_FAILED, Status);
		goto END;
	}

	/** Start the IO Module to receive interrupts. */
	Status = XIOModule_Start(&IOModule);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(XASUFW_IOMODULE_START_FAILED, Status);
		goto END;
	}

	/**
	 * Initialize and start the timer
	 *   - Use PIT1 and PIT2 in prescaler mode.
	 *   - Set the Prescaler mode for PIT1, PIT2 and PIT3.
	 */
	XAsufw_WriteReg(IOModule.BaseAddress + (u32)XGO_OUT_OFFSET,
			MB_IOMODULE_GPO1_PIT1_PRESCALE_SRC_MASK);
	XAsufw_InitPitTimer((u8)XASUFW_PIT2, Pit2ResetValue);
	XAsufw_InitPitTimer((u8)XASUFW_PIT1, Pit1ResetValue);
	XAsufw_InitPitTimer((u8)XASUFW_PIT3, Pit3ResetValue);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is called during boot up of ASUFW which will connect the IO module
 * 		interrupt handlers to the processor interrupts. This function also calls
 * 		XAsufw_ExceptionEnable function to initialize and enable processor interrpts
 * 		and exceptions.
 *
 * @return
 * 	- Returns XASUFW_SUCCESS on successful interrupt setup of IOModule interrupts.
 * 	- XASUFW_IOMODULE_CONNECT_FAILED, if IO module connection fails.
 *
 *************************************************************************************************/
s32 XAsufw_SetUpInterruptSystem(void)
{
	s32 Status = XASUFW_FAILURE;
	u8 IntrNum;

	IntrNum = XIN_IOMODULE_PIT_3_INTERRUPT_INTR;

	/** Connect PIT3 interrupt to its handler. */
	Status = XIOModule_Connect(&IOModule, IntrNum, (XInterruptHandler)XAsufw_Pit3TimerHandler,
				   (void *)(u32)IntrNum);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(XASUFW_IOMODULE_CONNECT_FAILED, Status);
		goto END;
	}

	/** Enable the IO Module PIT3 interrupt. */
	XIOModule_Enable(&IOModule, IntrNum);

	IntrNum = XASUFW_IOMODULE_IPI_INTRNUM;

	/** Connect IPI interrupt to its handler. */
	Status = XIOModule_Connect(&IOModule, IntrNum, (XInterruptHandler)XAsufw_IpiHandler,
				   (void *)(u32)IntrNum);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(XASUFW_IOMODULE_CONNECT_FAILED, Status);
		goto END;
	}

	XIOModule_Enable(&IOModule, IntrNum);

	/** Enable interrupts and exceptions. */
	XAsufw_ExceptionEnable();

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is used to read the 64 bit timer value. It reads from PIT1 and PIT2
 * 		and makes it 64 bit.
 *
 * @return
 * 	- Returns 64 bit timer value.
 *
 *************************************************************************************************/
u64 XAsufw_GetTimerValue(void)
{
	u64 TimerValue;
	u64 TPit1;
	u32 TPit2;

	TPit1 = XIOModule_GetValue(&IOModule, (u8)XASUFW_PIT1);
	TPit2 = XIOModule_GetValue(&IOModule, (u8)XASUFW_PIT2);

	/**
	 * Pit1 starts at 0 and preload the full value after pit2 expires. So, recasting TPit1 0 value
	 * to highest so that u64 comparison works for Tpit1 0 and TPit1 0xfffffffe
	 */
	if (TPit1 == 0U) {
		TPit1 = XASUFW_PIT1_CYCLE_VALUE;
	}
	TimerValue = (TPit1 << XASUFW_WORD_SIZE_IN_BITS) | (u64)TPit2;

	return TimerValue;
}

/*************************************************************************************************/
/**
 * @brief	This function measures the total time taken between two points for performance
 * 		measurement.
 *
 * @param	TCur		Current time
 * @param	PerfTime	Variable to hold the time elapsed
 *
 *************************************************************************************************/
void XAsufw_MeasurePerfTime(u64 TCur, XAsufw_PerfTime *PerfTime)
{
	u64 TEnd;
	u64 PerfUs;
	u64 TDiff;
	u32 AsuIroFreqMHz = XASUFW_ASU_IRO_FREQ_IN_HZ / XASUFW_MEGA;

	/** Get the difference between two points. */
	TEnd = XAsufw_GetTimerValue();
	TDiff = TCur - TEnd;

	/** Convert TPerf into microseconds. */
	PerfUs = TDiff / AsuIroFreqMHz;
	PerfTime->TPerfMsFrac = PerfUs % XASUFW_KILO;
	PerfTime->TPerfMs = PerfUs / XASUFW_KILO;
}

/*************************************************************************************************/
/**
 * @brief	This function prints time stamp for ASUFW in milli seconds.
 *
 *************************************************************************************************/
void XAsufw_PrintAsuTimeStamp(void)
{
	XAsufw_PerfTime PerfTime;

	/* Print time stamp of ASUFW */
	XAsufw_MeasurePerfTime((XASUFW_PIT1_CYCLE_VALUE << XASUFW_WORD_SIZE_IN_BITS) |
			       XASUFW_PIT2_CYCLE_VALUE, &PerfTime);
	XAsufw_Printf(DEBUG_PRINT_ALWAYS, "[%u.%03u]", (u32)PerfTime.TPerfMs,
		      (u32)PerfTime.TPerfMsFrac);
}

/*************************************************************************************************/
/**
 * @brief	This function initializes the ASUFW RTC area with default values.
 *
 *************************************************************************************************/
void XAsufw_RtcaInit(void)
{
	XAsufw_WriteReg(XASUFW_RTCA_IDENTIFICATION_ADDR, XASUFW_RTCA_IDENTIFICATION_STRING);
	XAsufw_WriteReg(XASUFW_RTCA_VERSION_ADDR, XASUFW_RTCA_VERSION);
	XAsufw_WriteReg(XASUFW_RTCA_SIZE_ADDR, XASUFW_RTCA_SIZE);
}
/** @} */
