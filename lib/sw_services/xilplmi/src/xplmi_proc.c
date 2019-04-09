/******************************************************************************
* Copyright (C) 2019 Xilinx, Inc. All rights reserved.
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
* @file xplmi_proc.c
*
* This file contains the processor related code.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/07/2019 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_proc.h"

/************************** Constant Definitions *****************************/
#define XPLMI_MB_MSR_BIP_MASK		(0x8U)
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XPLMI_MAP_PLMID(Lvl0, Lvl1, Lvl2)	\
	(Lvl0<<0U | Lvl1<<8U | Lvl2<<16U)

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
static XIOModule IOModule; /* Instance of the IO Module */
static u32 PlmIntrMap [] = {
	[XPLMI_IPI_IRQ] = XPLMI_MAP_PLMID(XPLMI_IOMODULE_PMC_GIC_IRQ,
					  XPLMI_PMC_GIC_IRQ_GICP0,
					  XPLMI_GICP0_SRC27),
	[XPLMI_SBI_DATA_RDY] = XPLMI_MAP_PLMID(XPLMI_IOMODULE_PMC_GIC_IRQ,
					  XPLMI_PMC_GIC_IRQ_GICP4,
					  XPLMI_GICP4_SRC8),
};

/*****************************************************************************/
/**
* It initializes the Programmable Interval Timer
*
* @param	TimerNo PIT Timer to be initialized
* @param	ResetValue is the reset value of timer when started
* @return	None
*
*****************************************************************************/
void XPlmi_InitPitTimer(u8 Timer, u32 ResetValue)
{
	/*
	 * When used in PIT1 prescalar to PIT2, PIT2 has least 32bits
	 * So, PIT2 is reloaded to get 64bit timer value.
	 */
	if (XPLMI_PIT2 == Timer) {
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
u64 XPlmi_GetTimerValue(void )
{
	u64 TimerValue;
	u32 TPit1, TPit2;

	TPit1 = XIOModule_GetValue(&IOModule, (u8)XPLMI_PIT1);
	TPit2 = XIOModule_GetValue(&IOModule, (u8)XPLMI_PIT2);
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
void XPlmi_MeasurePerfTime(u64 tCur)
{
	u64 tEnd = 0;
	u64 tDiff = 0;
	u64 tPerfNs;
	u64 tPerfMs = 0;
	u64 tPerfMsFrac = 0;

	tEnd = XPlmi_GetTimerValue();
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
int XPlmi_StartTimer()
{
	int Status;

	/*
	 * Initialize the IO Module so that it's ready to use,
	 * specify the device ID that is generated in xparameters.h
	 */
	Status = XIOModule_Initialize(&IOModule, IOMODULE_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XPLMI_ERR_IOMOD_INIT,
					     Status);
		goto END;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	//Status = XIOModule_SelfTest(&IOModule);
	Status = XIOModule_Start(&IOModule);
	if (Status != XST_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XPLMI_ERR_IOMOD_START,
					     Status);
		goto END;
	}

	/** Initialize and start the timer
	 *  Use PIT1 and PIT2 in prescalor mode
	 */
	/* Setting for Prescaler mode */
	Xil_Out32(IOModule.BaseAddress + XGO_OUT_OFFSET,
			   MB_IOMODULE_GPO1_PIT1_PRESCALE_SRC_MASK);
	XPlmi_InitPitTimer((u8)XPLMI_PIT2,
			    XPLMI_PIT2_RESET_VALUE);
	XPlmi_InitPitTimer((u8)XPLMI_PIT1,
			    XPLMI_PIT1_RESET_VALUE);
END:
	return Status;
}

/* Structure for Top level interrupt table */
static struct HandlerTable g_TopLevelInterruptTable[] = {
	{XPlmi_IntrHandler},
	{XPlmi_IntrHandler},
	{XPlmi_IntrHandler},
	{XPlmi_IntrHandler},
	{XPlmi_IntrHandler},
	{XPlmi_IntrHandler},
	{XPlmi_IntrHandler},
	{XPlmi_IntrHandler},
	{XPlmi_IntrHandler},
	{XPlmi_IntrHandler},
	{XPlmi_IntrHandler},
	{XPlmi_IntrHandler},
	{XPlmi_IntrHandler},
	{XPlmi_IntrHandler},
	{XPlmi_IntrHandler},
	{XPlmi_IntrHandler},
	{XPlmi_GicIntrHandler},
	{XPlmi_IntrHandler},
	{XPlmi_IntrHandler},
	{XPlmi_IntrHandler},
	{XPlmi_IntrHandler},
	{XPlmi_IntrHandler},
	{XPlmi_IntrHandler},
	{XPlmi_IntrHandler},
	{XPlmi_IntrHandler},
	{XPlmi_IntrHandler},
	{XPlmi_IntrHandler},
	{XPlmi_IntrHandler},
	{XPlmi_IntrHandler},
	{XPlmi_IntrHandler},
	{XPlmi_IntrHandler},
	{XPlmi_IntrHandler}
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
int XPlmi_SetUpInterruptSystem()
{
	int Status;
	u32 IntrNum;

	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the device occurs, the device driver handler performs the specific
	 * interrupt processing for the device
	 */
	for (IntrNum = XIN_IOMODULE_EXTERNAL_INTERRUPT_INTR;
	     IntrNum < XPAR_IOMODULE_INTC_MAX_INTR_SIZE; IntrNum++)
	{
		Status = XIOModule_Connect(&IOModule, IntrNum,
		   (XInterruptHandler) g_TopLevelInterruptTable[IntrNum].Handler,
				   (void *)IntrNum);
		if (Status != XST_SUCCESS)
		{
			Status = XPLMI_UPDATE_STATUS(XPLMI_ERR_IOMOD_CONNECT,
						     Status);
			goto END;
		}
	}

	/**
	 * Enable interrupts for the device and then cause interrupts so the
	 * handlers will be called.
	 */
	for (IntrNum = XIN_IOMODULE_EXTERNAL_INTERRUPT_INTR;
		IntrNum< XPAR_IOMODULE_INTC_MAX_INTR_SIZE; IntrNum++)
	{
		XIOModule_Enable(&IOModule, IntrNum);
	}

	/**
	 * Register the IO module interrupt handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
	     (Xil_ExceptionHandler)XIOModule_DeviceInterruptHandler,
		     (void*) IOMODULE_DEVICE_ID);

	/**
	 * Enable interrupts
	 */
	microblaze_enable_interrupts();

	/**
	 * Clear Break In Progress to get interrupts
	 */
	mtmsr(mfmsr() & (~XPLMI_MB_MSR_BIP_MASK));
END:
	return Status;
}

/****************************************************************************/
/**
* @brief    This function is default interrupt handler for the device.
* @param    CallbackRef is presently the interrupt number that is received
* @return   None.
****************************************************************************/
void XPlmi_IntrHandler(void *CallbackRef)
{
	/**
	 * Indicate Interrupt received
	 */
	XPlmi_Printf(DEBUG_GENERAL,
	      "Received Interrupt: 0x%0x\n\r", (u32) CallbackRef);
}

/****************************************************************************/
/**
* @brief    This function will enable the interrupt.
* @param    IntrId Interrupt ID as specified in the xplmi_proc.h
* @return   None.
****************************************************************************/
void XPlmi_PlmIntrEnable(u32 IntrId)
{
	u32 PlmIntrId;
	u32 IoModIntrNum;

	PlmIntrId = PlmIntrMap[IntrId];
	IoModIntrNum = PlmIntrId & XPLMI_IOMODULE_MASK;

	switch (IoModIntrNum)
	{
		case XPLMI_IOMODULE_PMC_GIC_IRQ:
			XPlmi_GicIntrEnable(PlmIntrId);
			break;

		default:
			break;
	}
}

/****************************************************************************/
/**
* @brief    This function will disable the interrupt.
* @param    IntrId Interrupt ID as specified in the xplmi_proc.h
* @return   None.
****************************************************************************/
void XPlmi_PlmIntrDisable(u32 IntrId)
{
	u32 PlmIntrId;
	u32 IoModIntrNum;

	PlmIntrId = PlmIntrMap[IntrId];
	IoModIntrNum = PlmIntrId & XPLMI_IOMODULE_MASK;

	switch (IoModIntrNum)
	{
		case XPLMI_IOMODULE_PMC_GIC_IRQ:
			XPlmi_GicIntrDisable(PlmIntrId);
			break;

		default:
			break;
	}
}

/****************************************************************************/
/**
* @brief    This function will register the handler and enable the interrupt.
* @param    IntrId Interrupt ID as specified in the xplmi_proc.h
* @param    Handler Handler to the registered for the interrupt
* @param    Data Data to be passed to handler
* @return   None.
****************************************************************************/
void XPlmi_RegisterHandler(u32 IntrId, Function_t Handler, void * Data)
{
	u32 PlmIntrId;
	u32 IoModIntrNum;

	PlmIntrId = PlmIntrMap[IntrId];
	IoModIntrNum = PlmIntrId & XPLMI_IOMODULE_MASK;

	switch (IoModIntrNum)
	{
		case XPLMI_IOMODULE_PMC_GIC_IRQ:
			XPlmi_GicRegisterHandler(PlmIntrId, Handler, Data);
			break;

		default:
			break;
	}
}
