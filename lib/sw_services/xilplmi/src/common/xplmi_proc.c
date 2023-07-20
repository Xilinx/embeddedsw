/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
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
* 1.01  kc   04/09/2019 Added code to register/enable/disable interrupts
*       sn   07/04/2019 Added support for enabling GIC proxy for sysmon
*       kc   07/16/2019 Added PERF macro to print task times
*       kc   07/16/2019 Added logic to determine the IRO frequency
*       kc   08/01/2019 Added PLM and ROM boot times
* 1.02  kc   02/10/2020 Updated scheduler to add/remove tasks
*       ma   02/28/2020 Added support for new error actions
*       kc   03/20/2020 Scheduler frequency is increased to 100ms for QEMU
*       bsv  04/04/2020 Code clean up
*       kc   04/23/2020 Added interrupt support for SEU event
* 1.03  bm   10/14/2020 Code clean up
*       td   10/19/2020 MISRA C Fixes
* 1.04  td   11/23/2020 MISRA C Rule 10.4 Fixes
*       ma   03/24/2021 Reduced minimum digits of time stamp decimals to 3
*       skd  03/25/2021 Compilation warning fix
*       bm   04/03/2021 Move task creation out of interrupt context
* 1.05  td   07/08/2021 Fix doxygen warnings
*       bm   07/12/2021 Updated IRO frequency to 400MHz for MP and HP parts
*       bsv  07/16/2021 Fix doxygen warnings
*       bsv  08/02/2021 Code clean up to reduce size
*       bsv  08/15/2021 Removed unwanted goto statements
* 1.06  ma   01/17/2022 Move EFUSE defines to xplmi_hw.h file
*       bm   01/27/2022 Fix setup interrupt system logic
*       rama 01/31/2022 Added STL error interrupt register functionality
*       bm   03/16/2022 Fix ROM time calculation
* 1.07  skd  04/21/2022 Misra-C violation Rule 18.1 fixed
* 1.08  bm   07/06/2022 Refactor versal and versal_net code
* 1.09  ng   11/11/2022 Updated doxygen comments
*       bm   01/03/2023 Remove usage of double data type
*       bm   03/11/2023 Set PmcIroFreq as 320MHz by default
*       ng   03/30/2023 Updated algorithm and return values in doxygen comments
* 1.10  bm   04/28/2023 Use XPlmi_GetRomIroFreq API to get IRO frequency used
*                       during ROM
*       ng   06/21/2023 Added support for system device-tree flow
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_proc.h"
#include "xplmi_hw.h"
#include "xplmi_scheduler.h"
#include "xplmi_debug.h"
#include "xplmi_err_common.h"
#include "xplmi_plat.h"
#include "xplmi_config.h"

/**@cond xplmi_internal
 * @{
 */

/************************** Constant Definitions *****************************/
#define XPLMI_MB_MSR_BIP_MASK		(0x8U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/**
 * @}
 * @endcond
 */

/************************** Function Prototypes ******************************/
static void XPlmi_InitPitTimer(u8 Timer, u32 ResetValue);
static void XPlmi_GetPerfTime(u64 TCur, u64 TStart, u32 IroFreq,
		XPlmi_PerfTime *PerfTime);

/************************** Variable Definitions *****************************/
static u32 PmcIroFreq = XPLMI_PMC_IRO_FREQ_320_MHZ; /* Frequency of the PMC IRO */
static XIOModule IOModule; /* Instance of the IO Module */

/*****************************************************************************/
/**
* @brief	This function provides the pointer to PmcIroFreq variable
*
* @return	Pointer to PmcIroFreq variable
*
*****************************************************************************/
u32 *XPlmi_GetPmcIroFreq(void)
{
	return &PmcIroFreq;
}

/*****************************************************************************/
/**
* @brief	This function provides the pointer to IOModule variable
*
* @return	Pointer to IOModule variable
*
*****************************************************************************/
XIOModule *XPlmi_GetIOModuleInst(void)
{
	return &IOModule;
}

/*****************************************************************************/
/**
* @brief	It initializes the Programmable Interval Timer.
*
* @param	Timer is PIT timer number to be initialized
* @param	ResetValue is the reset value of timer when started.
*
* @return
* 			- None
*
*****************************************************************************/
static void XPlmi_InitPitTimer(u8 Timer, u32 ResetValue)
{
	/*
	 * When used in PIT1 prescalar to PIT2, PIT2 has least 32bits
	 * So, PIT2 is reloaded to get 64bit timer value.
	 */
	if ((XPLMI_PIT2 == Timer) || (XPLMI_PIT3 == Timer)) {
		XIOModule_Timer_SetOptions(&IOModule, Timer,
				XTC_AUTO_RELOAD_OPTION);
	}

	/**
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
 * @brief	This function is used to read the 64 bit timer value.
 * It reads from PIT1 and PIT2 and makes it 64 bit.
 *
 * @return	Returns 64 bit timer value
 *
 ******************************************************************************/
u64 XPlmi_GetTimerValue(void)
{
	u64 TimerValue;
	u64 TPit1;
	u32 TPit2;

	TPit1 = XIOModule_GetValue(&IOModule, (u8)XPLMI_PIT1);
	TPit2 = XIOModule_GetValue(&IOModule, (u8)XPLMI_PIT2);

	/**
	 * Pit1 starts at 0 and preload the full value
	 * after pit2 expires. So, recasting TPit1 0 value
	 * to highest so that u64 comparison works for
	 * Tpit1 0 and TPit1 0xfffffffe
	 */
	if (TPit1 == 0U) {
		TPit1 = XPLMI_PIT1_CYCLE_VALUE;
	}
	TimerValue = (TPit1 << 32U) | (u64)TPit2;

	return TimerValue;
}

/*****************************************************************************/
/**
 * @brief	This function prints the total time taken between two points for
 * 			performance measurement.
 *
 * @param	TCur is the current time
 * @param	TStart is the start time
 * @param	IroFreq is the frequency at which PMC IRO is running
 * @param	PerfTime is the pointer to variable holding the performance time
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
static void XPlmi_GetPerfTime(u64 TCur, u64 TStart, u32 IroFreq,
		XPlmi_PerfTime *PerfTime)
{
	u64 PerfUs;
	u64 TDiff = TCur - TStart;
	u32 PmcIroFreqMHz = IroFreq / XPLMI_MEGA;

	/* Convert TPerf into microseconds */
	PerfUs = TDiff / (u64)PmcIroFreqMHz;
	PerfTime->TPerfMsFrac = PerfUs % (u64)XPLMI_KILO;
	PerfTime->TPerfMs = PerfUs / (u64)XPLMI_KILO;
}

/*****************************************************************************/
/**
 * @brief	This function measures the total time taken between two points for
 * performance measurement.
 *
 * @param	TCur is current time
 * @param	PerfTime is the variable to hold the time elapsed
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
void XPlmi_MeasurePerfTime(u64 TCur, XPlmi_PerfTime *PerfTime)
{
	u64 TEnd = XPlmi_GetTimerValue();
	XPlmi_GetPerfTime(TCur, TEnd, PmcIroFreq, PerfTime);
}

/*****************************************************************************/
/**
 * @brief	This function prints the ROM time.
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
void XPlmi_PrintRomTime(void)
{
	u64 PmcRomTime;
	XPlmi_PerfTime PerfTime;

	/* Get PMC ROM time */
	PmcRomTime = (u64)XPlmi_In32(PMC_GLOBAL_GLOBAL_GEN_STORAGE0);
	PmcRomTime |= (u64)XPlmi_In32(PMC_GLOBAL_GLOBAL_GEN_STORAGE1) << 32U;

	/* Print time stamp of PLM */
	XPlmi_GetPerfTime((XPLMI_PIT1_CYCLE_VALUE << 32U) |
		XPLMI_PIT2_CYCLE_VALUE, PmcRomTime,
		XPlmi_GetRomIroFreq(), &PerfTime);
	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "%u.%03u ms: ROM Time\r\n",
		(u32)PerfTime.TPerfMs, (u32)PerfTime.TPerfMsFrac);
}

/*****************************************************************************/
/**
 * @brief	This function prints the PLM time stamp.
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
void XPlmi_PrintPlmTimeStamp(void)
{
	XPlmi_PerfTime PerfTime;

	/* Print time stamp of PLM */
	XPlmi_MeasurePerfTime((XPLMI_PIT1_CYCLE_VALUE << 32U) |
		XPLMI_PIT2_CYCLE_VALUE, &PerfTime);
	xil_printf("[%u.%03u]", (u32)PerfTime.TPerfMs, (u32)PerfTime.TPerfMsFrac);
}

/*****************************************************************************/
/**
* @brief	It initializes the IO module structures and PIT timers.
*
* @return
* 			- XST_SUCCESS on success.
* 			- XPLMI_ERR_IOMOD_INIT if IOModdule drive lookup fails.
* 			- XPLMI_ERR_IOMOD_START if IOModdule drive startup fails.
* 			- XPLMI_ERR_SET_PMC_IRO_FREQ if failed to set PMC IRO frequency.
*
*****************************************************************************/
int XPlmi_StartTimer(void)
{
	int Status =  XST_FAILURE;
	u32 Pit1ResetValue;
	u32 Pit2ResetValue;
	u32 Pit3ResetValue;

	/**
	 * - Get Pit1 and Pit2 reset values
	 */
	Status = XPlmi_GetPitResetValues(&Pit1ResetValue, &Pit2ResetValue);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * - Initialize the IO Module so that it's ready to use,
	 * specify the device ID that is generated in xparameters.h
	 */
	Status = XIOModule_Initialize(&IOModule, IOMODULE_DEVICE);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_IOMOD_INIT, Status);
		goto END;
	}

	Status = XIOModule_Start(&IOModule);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_IOMOD_START, Status);
		goto END;
	}

	Status = XPlmi_SetPmcIroFreq();
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_SET_PMC_IRO_FREQ, Status);
		goto END;
	}

	/*
	 * PLM scheduler is running too fast for QEMU, so increasing the
	 * scheduler's poling time to 100ms for QEMU instead of 10ms
	 */
	if (XPLMI_PLATFORM == PMC_TAP_VERSION_QEMU) {
		Pit3ResetValue = PmcIroFreq / XPLMI_PIT_FREQ_DIVISOR_QEMU;
	} else {
		Pit3ResetValue = PmcIroFreq / XPLMI_PIT_FREQ_DIVISOR;
	}

	XPlmi_SchedulerInit();

	/**
	 * - Initialize and start the timer
	 *   - Use PIT1 and PIT2 in prescaler mode
	 *   - Set the Prescaler mode
	 */
	XPlmi_Out32(IOModule.BaseAddress + (u32)XGO_OUT_OFFSET,
		MB_IOMODULE_GPO1_PIT1_PRESCALE_SRC_MASK);
	XPlmi_InitPitTimer((u8)XPLMI_PIT2, Pit2ResetValue);
	XPlmi_InitPitTimer((u8)XPLMI_PIT1, Pit1ResetValue);
	XPlmi_InitPitTimer((u8)XPLMI_PIT3, Pit3ResetValue);

END:
	return Status;
}

/******************************************************************************/
/**
*
* @brief	This function connects the interrupt handler of the IO Module to the
* 			processor.
*
* @return
* 			- XST_SUCCESS if handlers are registered properly.
* 			- XPLMI_ERR_IOMOD_CONNECT if IOModule driver fails to establish
* 			connection.
*
****************************************************************************/
int XPlmi_SetUpInterruptSystem(void)
{
	int Status =  XST_FAILURE;
	u8 IntrNum = XIN_IOMODULE_EXTERNAL_INTERRUPT_INTR;
	u8 Index;
	XInterruptHandler *g_TopLevelInterruptTable = XPlmi_GetTopLevelIntrTbl();
	u8 Size = XPlmi_GetTopLevelIntrTblSize();

	microblaze_disable_interrupts();
	/**
	 * - Connect a device driver handler that will be called when an interrupt
	 * for the device occurs, the device driver handler performs the specific
	 * interrupt processing for the device
	 */
	for (Index = 0U; Index < Size; ++Index) {
		Status = XIOModule_Connect(&IOModule, IntrNum,
			(XInterruptHandler)g_TopLevelInterruptTable[Index],
			(void *)(u32)IntrNum);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XPLMI_ERR_IOMOD_CONNECT, Status);
			goto END;
		}
		++IntrNum;
	}
	++IntrNum;
	while (IntrNum < XILPLMI_IOMODULE_INTC_MAX_INTR_SIZE) {
		Status = XIOModule_Connect(&IOModule, IntrNum,
			(XInterruptHandler)XPlmi_IntrHandler,
			(void *)(u32)IntrNum);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XPLMI_ERR_IOMOD_CONNECT, Status);
			goto END;
		}
		IntrNum++;
	}

	Status = XIOModule_Connect(&IOModule, XIN_IOMODULE_PIT_3_INTERRUPT_INTR,
		(XInterruptHandler)XPlmi_SchedulerHandler, (void *)(u32)IntrNum);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_IOMOD_CONNECT, Status);
		goto END;
	}

	/**
	 * - Register the IO module interrupt handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
		(Xil_ExceptionHandler)XIOModule_DeviceInterruptHandler,
		(void*) IOMODULE_DEVICE);
	/**
	 * - Enable interrupts
	 */
	XPlmi_EnableIomoduleIntr();
	microblaze_enable_interrupts();

	/**
	 * - Clear Break In Progress to get interrupts
	 */
	mtmsr(mfmsr() & (~XPLMI_MB_MSR_BIP_MASK));
END:
	return Status;
}

/****************************************************************************/
/**
* @brief    This function is default interrupt handler for the device.
*
* @param    CallbackRef is presently the interrupt number that is received.
*
* @return
* 			- None
*
****************************************************************************/
void XPlmi_IntrHandler(void *CallbackRef)
{
	/*
	 * Indicate Interrupt received
	 */
	XPlmi_Printf(DEBUG_GENERAL, "Received Interrupt: 0x%0x\n\r",
		(u32) CallbackRef);
}

/****************************************************************************/
/**
* @brief    This function will enable the Iomodule interrupt.
*
* @param    IntrId Interrupt ID as specified in the xplmi_proc.h
*
* @return
* 			- None
*
****************************************************************************/
void XPlmi_PlmIntrEnable(u32 IntrId)
{
	u32 IntrNum = IntrId;

	/* For Backward Compatibility of Xilsem */
	if (IntrId == 0U) {
		IntrNum = XPLMI_IOMODULE_CFRAME_SEU;
	}

	XIOModule_Enable(&IOModule, (u8)IntrNum);
}

/****************************************************************************/
/**
* @brief    This function will disable the Iomodule interrupt.
*
* @param    IntrId Interrupt ID as specified in the xplmi_proc.h
*
* @return
* 			- XST_SUCCESS on success and error code on failure
*
****************************************************************************/
int XPlmi_PlmIntrDisable(u32 IntrId)
{
	u32 IntrNum = IntrId;

	/* For Backward Compatibility of Xilsem */
	if (IntrId == 0U) {
		IntrNum = XPLMI_IOMODULE_CFRAME_SEU;
	}

	XIOModule_Disable(&IOModule, (u8)IntrNum);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief    This function will clear the Iomodule interrupt.
*
* @param    IntrId Interrupt ID as specified in the xplmi_proc.h
*
* @return
* 			- XST_SUCCESS on success and error code on failure
*
****************************************************************************/
int XPlmi_PlmIntrClear(u32 IntrId)
{
	u32 IntrNum = IntrId;

	/* For Backward Compatibility of Xilsem */
	if (IntrId == 0U) {
		IntrNum = XPLMI_IOMODULE_CFRAME_SEU;
	}

	XIOModule_Acknowledge(&IOModule, (u8)IntrNum);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief    This function will register the handler and enable the Iomodule
*           interrupt.
*
* @param    IntrId Interrupt ID as specified in the xplmi_hw.h
* @param    Handler to be registered for the interrupt
* @param    Data to be passed to handler
*
* @return
* 			- XST_SUCCESS on success.
* 			- XPLMI_ERR_REGISTER_IOMOD_HANDLER if failed to register the handler.
*
****************************************************************************/
int XPlmi_RegisterHandler(u32 IntrId, GicIntHandler_t Handler, void *Data)
{
	int Status = XST_FAILURE;
	u32 IntrNum = IntrId;

	/* For Backward Compatibility of Xilsem */
	if (IntrId == 0U) {
		IntrNum = XPLMI_IOMODULE_CFRAME_SEU;
	}

	Status = XIOModule_Connect(&IOModule, (u8)IntrNum,
			(XInterruptHandler)(void *)Handler, (void *)Data);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_GENERAL, "IoModule Connect Failed:0x%0x\n\r",
			     Status);
		Status = XPlmi_UpdateStatus(XPLMI_ERR_REGISTER_IOMOD_HANDLER,
					    Status);
	}

	return Status;
}
