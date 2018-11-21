/******************************************************************************
* Copyright (C) 2018 Xilinx, Inc. All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplm_proc.c
*
* This file contains the processor related code. It uses libmetal for
* exceptions, interrupts and timers.
* It may also have iomodule related code when libmetal is not enabled with
* the required functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   03/27/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_proc.h"
#include "xplm_main.h"

/************************** Constant Definitions *****************************/
#define XPLM_MB_MSR_BIP_MASK		(0x8U)
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
static XIOModule IOModule; /* Instance of the IO Module */
/*****************************************************************************/
/**
* It initializes the Programmable Interval Timer
*
* @param	TimerNo PIT Timer to be initialized
* @param	ResetValue is the reset value of timer when started
* @return	None
*
*****************************************************************************/
void XPlm_InitPitTimer(u8 Timer, u32 ResetValue)
{
	/*
	 * When used in PIT1 prescalar to PIT2, PIT2 has least 32bits
	 * So, PIT2 is reloaded to get 64bit timer value.
	 */
	if (XPLM_PIT2 == Timer) {
		XIOModule_Timer_SetOptions(&IOModule, Timer,
				   XTC_AUTO_RELOAD_OPTION);
	}

	/*
	 * Set a reset value for the Programmable Interval Timers such that
	 * they will expire earlier than letting them roll over from 0, the
	 * reset value is loaded into the Programmable Interval Timers when
	 * they are started.
	 */
	XIOModule_SetResetValue(&IOModule, Timer, ResetValue);

	/*
	 * Start the Programmable Interval Timers and they are
	 * decrementing by default
	 */
	XIOModule_Timer_Start(&IOModule, Timer);
}

/*****************************************************************************/
/**
 *
 * This function is used to read the 64 bit timer value.
 * It reads from PIT1 and PIT2 and makes it 64bit
 *
 * @param       None
 *
 * @return      Returns 64 bit timer value
 *
 ******************************************************************************/
u64 XPlm_GetTimerValue(void )
{
	u64 TimerValue;
	u32 TPit1, TPit2;

	TPit1 = XIOModule_GetValue(&IOModule, (u8)XPLM_PIT1);
	TPit2 = XIOModule_GetValue(&IOModule, (u8)XPLM_PIT2);
	/* XPlmi_Printf(DEBUG_INFO, "pit1 %08x pit2 %08x\r\n", TPit1, TPit2); */

	/**
	* Pit1 starts at 0 and preload the full value
	* after pit2 expires. So, recasting TPit1 0 value
	* to highest so that u64 comparison works fo
	* Tpit1 0 and TPit1 0xfffffffe
	*/
	if (TPit1 == 0U)
	{
		TPit1 = 0xfffffffeU;
	}

	TimerValue = (((u64)TPit1) << 32) | (u64)TPit2;
	return TimerValue;
}

/*****************************************************************************/
/**
 * This function measures the total time taken between two points for
 * performance measurement.
 *
 * @param Start time
 *
 * @return none
 *****************************************************************************/
void XPlm_MeasurePerfTime(u64 tCur)
{
	u64 tEnd = 0;
	u64 tDiff = 0;
	u64 tPerfNs;
	u64 tPerfMs = 0;
	u64 tPerfMsFrac = 0;

	tEnd = XPlm_GetTimerValue();
	tDiff = tCur - tEnd;

	/* Convert tPerf into nanoseconds */
	tPerfNs = ((double)tDiff / (double)XPAR_CPU_CORE_CLOCK_FREQ_HZ) * 1e9;

	tPerfMs = tPerfNs / 1e6;
	tPerfMsFrac = tPerfNs % (u64)1e6;

	/* Print the whole (in ms.) and fractional part */
	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "%d.%06d ms.",
			(u32)tPerfMs, (u32)tPerfMsFrac);
}

/*****************************************************************************/
/**
* It initializes the IO module strutures and PIT timers
* @return	XST_SUCCESS if the initialization is successful
*
*****************************************************************************/
int XPlm_StartTimer()
{
	int Status;

	/*
	 * Initialize the IO Module so that it's ready to use,
	 * specify the device ID that is generated in xparameters.h
	 */
	Status = XIOModule_Initialize(&IOModule, IOMODULE_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		Status = XPLM_UPDATE_ERR(XPLM_ERR_IOMOD_INIT, Status);
		goto END;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	//Status = XIOModule_SelfTest(&IOModule);
	Status = XIOModule_Start(&IOModule);
	if (Status != XST_SUCCESS) {
		Status = XPLM_UPDATE_ERR(XPLM_ERR_IOMOD_START, Status);
		goto END;
	}

	/** Initialize and start the timer
	 *  Use PIT1 and PIT2 in prescalor mode
	 */
	/* Setting for Prescaler mode */
	Xil_Out32(IOModule.BaseAddress + XGO_OUT_OFFSET,
			   MB_IOMODULE_GPO1_PIT1_PRESCALE_SRC_MASK);
	XPlm_InitPitTimer((u8)XPLM_PIT2,
			    XPLM_PIT2_RESET_VALUE);
	XPlm_InitPitTimer((u8)XPLM_PIT1,
			    XPLM_PIT1_RESET_VALUE);
END:
	return Status;
}

/* Structure for Top level interrupt table */
static struct HandlerTable g_TopLevelInterruptTable[] = {
	{XPlm_IntrHandler},
	{XPlm_IntrHandler},
	{XPlm_IntrHandler},
	{XPlm_IntrHandler},
	{XPlm_IntrHandler},
	{XPlm_IntrHandler},
	{XPlm_IntrHandler},
	{XPlm_IntrHandler},
	{XPlm_IntrHandler},
	{XPlm_IntrHandler},
	{XPlm_IntrHandler},
	{XPlm_IntrHandler},
	{XPlm_IntrHandler},
	{XPlm_IntrHandler},
	{XPlm_IntrHandler},
	{XPlm_IntrHandler},
	{XPlm_GicIntrHandler},
	{XPlm_IntrHandler},
	{XPlm_IntrHandler},
	{XPlm_IntrHandler},
	{XPlm_IntrHandler},
	{XPlm_IntrHandler},
	{XPlm_IntrHandler},
	{XPlm_IntrHandler},
	{XPlm_IntrHandler},
	{XPlm_IntrHandler},
	{XPlm_IntrHandler},
	{XPlm_IntrHandler},
	{XPlm_IntrHandler},
	{XPlm_IntrHandler},
	{XPlm_IntrHandler},
	{XPlm_IntrHandler}
};

/******************************************************************************/
/**
*
* This function connects the interrupt handler of the IO Module to the
* processor.
* @param    None.
*
* @return	XST_SUCCESS if handlers are registered properly
****************************************************************************/
int XPlm_SetUpInterruptSystem()
{
	int Status;
	u32 IntrNum;

	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the device occurs, the device driver handler performs the specific
	 * interrupt processing for the device
	 */
	for (IntrNum = 0U; IntrNum < XPAR_IOMODULE_INTC_MAX_INTR_SIZE; IntrNum++)
	{
		Status = XIOModule_Connect(&IOModule, IntrNum,
				   (XInterruptHandler) g_TopLevelInterruptTable[IntrNum].Handler,
				   (void *)IntrNum);
		if (Status != XST_SUCCESS)
		{
			Status = XPLM_UPDATE_ERR(
					XPLM_ERR_IOMOD_CONNECT, Status);
			goto END;
		}
	}

	/*
	 * Enable interrupts for the device and then cause interrupts so the
	 * handlers will be called.
	 */
	for (IntrNum = XIN_IOMODULE_EXTERNAL_INTERRUPT_INTR; 
		IntrNum< XPAR_IOMODULE_INTC_MAX_INTR_SIZE; IntrNum++)
	{
		XIOModule_Enable(&IOModule, IntrNum);
	}

	/*
	 * Register the IO module interrupt handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
	     (Xil_ExceptionHandler)XIOModule_DeviceInterruptHandler,
		     (void*) IOMODULE_DEVICE_ID);

	/*
	 * Enable interrupts
	 */
	microblaze_enable_interrupts();

	/*
	 * Clear Break In Progress to get interrupts
	 */
	mtmsr(mfmsr() & (~XPLM_MB_MSR_BIP_MASK));
END:
	return Status;
}

/******************************************************************************/
/**
* This function is an interrupt handler for the device.
* @param    CallbackRef is presently the interrupt number that is received
* @return   None.
****************************************************************************/
void XPlm_IntrHandler(void *CallbackRef)
{
	/*
	 * Indicate Interrupt received
	 */
	XPlmi_Printf(DEBUG_GENERAL,
	      "Received Interrupt: 0x%0x\n\r", (u32) CallbackRef);
}

/*****************************************************************************/
/**
 * This function enables the exceptions and interrupts
 * Enable interrupts from the hardware
 *
 * @param
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XPlm_ExceptionInit(void )
{
	u32 Index;
	Xil_ExceptionInit();

	/* Register exception handlers */
	for (Index = XIL_EXCEPTION_ID_FIRST;
	     Index <= XIL_EXCEPTION_ID_LAST; Index++)
	{
		Xil_ExceptionRegisterHandler(Index,
			     (Xil_ExceptionHandler)XPlm_ExceptionHandler,
			     (void *)XPLM_ERR_EXCEPTION);
	}

	microblaze_enable_exceptions();
}

/*****************************************************************************/
/**
 * This is a function handler for exceptions
 *
 * @param
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XPlm_ExceptionHandler(u32 Status)
{
	XPlmi_Printf(DEBUG_GENERAL, "Received Exception \n\r"
		      "MSR: 0x%08x, EAR: 0x%08x, EDR: 0x%08x, ESR: 0x%08x, \n\r"
		      "R14: 0x%08x, R15: 0x%08x, R16: 0x%08x, R17: 0%08x \n\r",
		      mfmsr(), mfear(), mfedr(), mfesr(),
		      mfgpr(r14), mfgpr(r15), mfgpr(r16), mfgpr(r17));

	XPlm_ErrMgr(Status);

	/* Just in case if it returns */
	while(1);
}

/*****************************************************************************/
/**
 * This function initializes the processor, enables exceptions and start
 * timer
 *
 * @param none
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
int XPlm_InitProc(void )
{
	int Status;

	XPlm_ExceptionInit();
	Status = XPlm_StartTimer();

	return Status;
}
