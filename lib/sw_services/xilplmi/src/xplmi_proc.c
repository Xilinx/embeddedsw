/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
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
#include "xplmi_err.h"
#include "microblaze_sleep.h"

/**@cond xplmi_internal
 * @{
 */

/************************** Constant Definitions *****************************/
#define XPLMI_MB_MSR_BIP_MASK		(0x8U)
#define XPLMI_EFUSE_CTRL_UNLOCK_VAL	(0xDF0DU)
#define XPLMI_EFUSE_CTRL_LOCK_VAL	(1U)
#define XPLMI_EFUSE_IRO_TRIM_320MHZ	(0U)
#define XPLMI_EFUSE_IRO_TRIM_400MHZ	(1U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XPLMI_MAP_PLMID(Lvl0, Lvl1, Lvl2)	\
	(((Lvl0) << 0U) | ((Lvl1) << 8U) | ((Lvl2) << 16U))
#define XPLMI_PMC_VOLTAGE_MULTIPLIER	(32768.0f)
#define XPLMI_PMC_VERSION_1_0		(0x10U)

/**
 * @}
 * @endcond
 */

/*****************************************************************************/
/**
 * @brief        This function converts voltage to raw voltage value
 *
 * @param        Voltage is the floating point voltage value
 *
 * @return       32-bit voltage value
 *
 ******************************************************************************/
static inline u32 XPlmi_GetRawVoltage(float Voltage)
{
	float RawVoltage = Voltage * XPLMI_PMC_VOLTAGE_MULTIPLIER;

	return (u32)RawVoltage;
}

/************************** Function Prototypes ******************************/
static int XPlmi_IoModuleRegisterHandler(u32 IoModIntrNum,
			XInterruptHandler Handler, void *Data);
static void XPlmi_InitPitTimer(u8 Timer, u32 ResetValue);
static void XPlmi_IntrHandler(void *CallbackRef);
static void XPlmi_GetPerfTime(u64 TCur, u64 TStart, XPlmi_PerfTime *PerfTime);

/************************** Variable Definitions *****************************/
static u32 PmcIroFreq; /* Frequency of the PMC IRO */
static XIOModule IOModule; /* Instance of the IO Module */
static u32 PlmIntrMap[XPLMI_MAX_EXT_INTR] = {
	[XPLMI_CFRAME_SEU] = XPLMI_MAP_PLMID(XPLMI_IOMODULE_CFRAME_SEU,
						0x0U, 0x0U),
	[XPLMI_IPI_IRQ] = XPLMI_MAP_PLMID(XPLMI_IOMODULE_PMC_GIC_IRQ,
						XPLMI_PMC_GIC_IRQ_GICP0,
						XPLMI_GICP0_SRC27),
	[XPLMI_SBI_DATA_RDY] = XPLMI_MAP_PLMID(XPLMI_IOMODULE_PMC_GIC_IRQ,
						XPLMI_PMC_GIC_IRQ_GICP4,
						XPLMI_GICP4_SRC8),
};

/*****************************************************************************/
/**
* @brief	It initializes the Programmable Interval Timer.
*
* @param	Timer is PIT timer number to be initialized
* @param	ResetValue is the reset value of timer when started.
*
* @return	None
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

	/*
	 * Pit1 starts at 0 and preload the full value
	 * after pit2 expires. So, recasting TPit1 0 value
	 * to highest so that u64 comparison works fo
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
 * performance measurement.
 *
 * @param	TCur is the current time
 * @param	TStart is the start time
 * @param	PerfTime is the pointer to variable holding the performance time
 *
 * @return	None
 *
 *****************************************************************************/
static void XPlmi_GetPerfTime(u64 TCur, u64 TStart, XPlmi_PerfTime *PerfTime)
{
	u64 PerfNs;
	u64 TDiff = TCur - TStart;
	double PerfTemp;

	/* Convert TPerf into nanoseconds */
	PerfTemp = ((double)TDiff * XPLMI_GIGA) / (double)PmcIroFreq;
	PerfNs = (u64)PerfTemp;
	PerfTemp /= XPLMI_MEGA;
	PerfTime->TPerfMs = (u64)PerfTemp;
	PerfTime->TPerfMsFrac = PerfNs % (u64)XPLMI_MEGA;
	PerfTime->TPerfMsFrac /= (u64)XPLMI_MILLI;
}

/*****************************************************************************/
/**
 * @brief	This function measures the total time taken between two points for
 * performance measurement.
 *
 * @param	TCur is current time
 * @param	PerfTime is the variable to hold the time elapsed
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_MeasurePerfTime(u64 TCur, XPlmi_PerfTime *PerfTime)
{
	u64 TEnd = 0U;

	TEnd = XPlmi_GetTimerValue();
	XPlmi_GetPerfTime(TCur, TEnd, PerfTime);
}

/*****************************************************************************/
/**
 * @brief	This function prints the ROM time.
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_PrintRomTime(void)
{
	u64 PmcRomTime;
	XPlmi_PerfTime PerfTime = {0U};

	/* Get PMC ROM time */
	PmcRomTime = (u64)XPlmi_In32(PMC_GLOBAL_GLOBAL_GEN_STORAGE0);
	PmcRomTime |= (u64)XPlmi_In32(PMC_GLOBAL_GLOBAL_GEN_STORAGE1) << 32U;

	/* Print time stamp of PLM */
	XPlmi_GetPerfTime((XPLMI_PIT1_CYCLE_VALUE << 32U) |
		XPLMI_PIT2_CYCLE_VALUE, PmcRomTime, &PerfTime);
	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "%u.%03u ms: ROM Time\r\n",
		(u32)PerfTime.TPerfMs, (u32)PerfTime.TPerfMsFrac);
}

/*****************************************************************************/
/**
 * @brief	This function prints the PLM time stamp.
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_PrintPlmTimeStamp(void)
{
	XPlmi_PerfTime PerfTime = {0U};

	/* Print time stamp of PLM */
	XPlmi_MeasurePerfTime((XPLMI_PIT1_CYCLE_VALUE << 32U) |
		XPLMI_PIT2_CYCLE_VALUE, &PerfTime);
	xil_printf("[%u.%03u]", (u32)PerfTime.TPerfMs, (u32)PerfTime.TPerfMsFrac);
}

/*****************************************************************************/
/**
* @brief	It sets the PMC IRO frequency.
*
* @return	XST_SUCCESS on success and error code failure
*
*****************************************************************************/
static int XPlmi_SetPmcIroFreq(void)
{
	int Status = XST_FAILURE;
	u32 RawVoltage = 0U;
	u32 PmcVersion = XPlmi_In32(PMC_TAP_VERSION);


	PmcVersion = ((PmcVersion & PMC_TAP_VERSION_PMC_VERSION_MASK) >>
				PMC_TAP_VERSION_PMC_VERSION_SHIFT);
	if (PmcVersion == XPLMI_PMC_VERSION_1_0) {
		PmcIroFreq = XPLMI_PMC_IRO_FREQ_320_MHZ;
	}
	else {
		RawVoltage = Xil_In32(XPLMI_SYSMON_SUPPLY0_ADDR);
		RawVoltage &= XPLMI_SYSMON_SUPPLYX_MASK;
		/* Update IR0 frequency to 400MHz for MP and HP parts */
		XPlmi_Out32(EFUSE_CTRL_WR_LOCK, XPLMI_EFUSE_CTRL_UNLOCK_VAL);
		if (RawVoltage >= XPlmi_GetRawVoltage(XPLMI_VCC_PMC_MP_MIN)) {
			PmcIroFreq = XPLMI_PMC_IRO_FREQ_400_MHZ;
			XPlmi_Out32(EFUSE_CTRL_ANLG_OSC_SW_1LP,
				XPLMI_EFUSE_IRO_TRIM_400MHZ);
		}
		else {
			PmcIroFreq = XPLMI_PMC_IRO_FREQ_320_MHZ;
			XPlmi_Out32(EFUSE_CTRL_ANLG_OSC_SW_1LP,
				XPLMI_EFUSE_IRO_TRIM_320MHZ);
		}
		XPlmi_Out32(EFUSE_CTRL_WR_LOCK, XPLMI_EFUSE_CTRL_LOCK_VAL);
	}
	Status = (int)Xil_SetMBFrequency(PmcIroFreq);

	return Status;
}

/*****************************************************************************/
/**
* @brief	It initializes the IO module structures and PIT timers.
*
* @return	XST_SUCCESS on success and error code failure
*
*****************************************************************************/
int XPlmi_StartTimer(void)
{
	int Status =  XST_FAILURE;
	u32 Pit3ResetValue;

	/*
	 * Initialize the IO Module so that it's ready to use,
	 * specify the device ID that is generated in xparameters.h
	 */
	Status = XIOModule_Initialize(&IOModule, IOMODULE_DEVICE_ID);
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

	/* Initialize and start the timer
	 *  Use PIT1 and PIT2 in prescaler mode
	 *  Setting for Prescaler mode
	 */
	XPlmi_Out32(IOModule.BaseAddress + (u32)XGO_OUT_OFFSET,
		MB_IOMODULE_GPO1_PIT1_PRESCALE_SRC_MASK);
	XPlmi_InitPitTimer((u8)XPLMI_PIT2, XPLMI_PIT2_RESET_VALUE);
	XPlmi_InitPitTimer((u8)XPLMI_PIT1, XPLMI_PIT1_RESET_VALUE);
	XPlmi_InitPitTimer((u8)XPLMI_PIT3, Pit3ResetValue);
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
	{XPlmi_SchedulerHandler},
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
	{XPlmi_ErrIntrHandler},
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
* @brief	This function connects the interrupt handler of the IO Module to the
* processor.
*
* @return	XST_SUCCESS if handlers are registered properly
*
****************************************************************************/
int XPlmi_SetUpInterruptSystem(void)
{
	int Status =  XST_FAILURE;
	u8 IntrNum;

	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the device occurs, the device driver handler performs the specific
	 * interrupt processing for the device
	 */
	for (IntrNum = XIN_IOMODULE_EXTERNAL_INTERRUPT_INTR;
		IntrNum < XPAR_IOMODULE_INTC_MAX_INTR_SIZE; IntrNum++) {
		Status = XIOModule_Connect(&IOModule, IntrNum,
			(XInterruptHandler) g_TopLevelInterruptTable[IntrNum].Handler,
			(void *)(u32)IntrNum);
		if (Status != XST_SUCCESS)
		{
			Status = XPlmi_UpdateStatus(XPLMI_ERR_IOMOD_CONNECT, Status);
			goto END;
		}
	}
	IntrNum = XIN_IOMODULE_PIT_3_INTERRUPT_INTR;
	Status = XIOModule_Connect(&IOModule, IntrNum,
			(XInterruptHandler) g_TopLevelInterruptTable[IntrNum].Handler,
			(void *)(u32)IntrNum);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_IOMOD_CONNECT, Status);
		goto END;
	}

	XIOModule_Enable(&IOModule, XPLMI_IOMODULE_PMC_GIC_IRQ);
	XIOModule_Enable(&IOModule, XPLMI_IOMODULE_PPU1_MB_RAM);
	XIOModule_Enable(&IOModule, XPLMI_IOMODULE_ERR_IRQ);
	XIOModule_Enable(&IOModule, XPLMI_IOMODULE_PMC_GPI);
	XIOModule_Enable(&IOModule, XPLMI_IOMODULE_PMC_PIT3_IRQ);

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
*
* @param    CallbackRef is presently the interrupt number that is received.
*
* @return   None
*
****************************************************************************/
static void XPlmi_IntrHandler(void *CallbackRef)
{
	/*
	 * Indicate Interrupt received
	 */
	XPlmi_Printf(DEBUG_GENERAL,
		"Received Interrupt: 0x%0x\n\r", (u32) CallbackRef);
}

/****************************************************************************/
/**
* @brief    This function will enable the interrupt.
*
* @param    IntrId Interrupt ID as specified in the xplmi_proc.h
*
* @return   None
*
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

		case XPLMI_IOMODULE_CFRAME_SEU:
			XIOModule_Enable(&IOModule, XPLMI_IOMODULE_CFRAME_SEU);
			break;

		default:
			break;
	}
}

/****************************************************************************/
/**
* @brief    This function will disable the interrupt.
*
* @param    IntrId Interrupt ID as specified in the xplmi_proc.h
*
* @return   XST_SUCCESS on success and error code on failure
*
****************************************************************************/
int XPlmi_PlmIntrDisable(u32 IntrId)
{
	int Status = XST_FAILURE;
	u32 PlmIntrId;
	u32 IoModIntrNum;

	if (IntrId >= (u32)XPLMI_MAX_EXT_INTR) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_INVALID_INTR_ID_DISABLE, 0);
		goto END;
	}

	PlmIntrId = PlmIntrMap[IntrId];
	IoModIntrNum = PlmIntrId & XPLMI_IOMODULE_MASK;

	switch (IoModIntrNum)
	{
		case XPLMI_IOMODULE_PMC_GIC_IRQ:
			XPlmi_GicIntrDisable(PlmIntrId);
			Status = XST_SUCCESS;
			break;

		case XPLMI_IOMODULE_CFRAME_SEU:
			XIOModule_Disable(&IOModule, (u8 )IoModIntrNum);
			Status = XST_SUCCESS;
			break;

		default:
			Status = XPlmi_UpdateStatus(XPLMI_ERR_IO_MOD_INTR_NUM_DISABLE, 0);
			break;
	}

END:
	return Status;
}

/****************************************************************************/
/**
* @brief    This function will clear the interrupt.
*
* @param    IntrId Interrupt ID as specified in the xplmi_proc.h
*
* @return   XST_SUCCESS on success and error code on failure
*
****************************************************************************/
int XPlmi_PlmIntrClear(u32 IntrId)
{
	int Status = XST_FAILURE;
	u32 PlmIntrId;
	u32 IoModIntrNum;

	if (IntrId >= (u32)XPLMI_MAX_EXT_INTR) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_INVALID_INTR_ID_CLEAR, 0);
		goto END;
	}

	PlmIntrId = PlmIntrMap[IntrId];
	IoModIntrNum = PlmIntrId & XPLMI_IOMODULE_MASK;

	switch (IoModIntrNum)
	{
		case XPLMI_IOMODULE_PMC_GIC_IRQ:
			XPlmi_GicIntrClearStatus(PlmIntrId);
			Status = XST_SUCCESS;
			break;

		case XPLMI_IOMODULE_CFRAME_SEU:
			XIOModule_Acknowledge(&IOModule, (u8 )IoModIntrNum);
			Status = XST_SUCCESS;
			break;

		default:
			Status = XPlmi_UpdateStatus(XPLMI_ERR_IO_MOD_INTR_NUM_CLEAR, 0);
			break;
	}

END:
	return Status;
}

/****************************************************************************/
/**
* @brief    This function will register IOModule handler.
*
* @param    IoModIntrNum IOModule interrupt Number
* @param    Handler to be registered for the interrupt
* @param    Data to be passed to handler
*
* @return   XST_SUCCESS on success and error code on failure
*
****************************************************************************/
static int XPlmi_IoModuleRegisterHandler(u32 IoModIntrNum,
			XInterruptHandler Handler, void *Data)
{
	int Status = XST_FAILURE;

	g_TopLevelInterruptTable[IoModIntrNum].Handler = Handler;
	Status = XIOModule_Connect(&IOModule, (u8)IoModIntrNum, Handler, Data);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_GENERAL, "IoModule Connect Failed:0x%0x\n\r",
			     Status);
		Status = XPlmi_UpdateStatus(XPLMI_ERR_REGISTER_IOMOD_HANDLER,
					    Status);
		goto END;
	}

END:
	return Status;
}

/****************************************************************************/
/**
* @brief    This function will register the handler and enable the interrupt.
*
* @param    IntrId Interrupt ID as specified in the xplmi_proc.h
* @param    Handler to be registered for the interrupt
* @param    Data to be passed to handler
*
* @return   XST_SUCCESS on success and error code on failure
*
****************************************************************************/
int XPlmi_RegisterHandler(u32 IntrId, GicIntHandler_t Handler, void *Data)
{
	int Status = XST_FAILURE;
	u32 PlmIntrId;
	u32 IoModIntrNum;

	if (IntrId >= (u32)XPLMI_MAX_EXT_INTR) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_INVALID_INTR_ID_REGISTER, 0);
		goto END;
	}

	PlmIntrId = PlmIntrMap[IntrId];
	IoModIntrNum = PlmIntrId & XPLMI_IOMODULE_MASK;

	switch (IoModIntrNum)
	{
		case XPLMI_IOMODULE_PMC_GIC_IRQ:
			Status = XPlmi_GicRegisterHandler(PlmIntrId, Handler, Data);
			break;

		case XPLMI_IOMODULE_CFRAME_SEU:
			Status = XPlmi_IoModuleRegisterHandler(IoModIntrNum,
				      (XInterruptHandler)(void*)Handler, Data);
			break;

		default:
			Status = XPlmi_UpdateStatus(XPLMI_ERR_IO_MOD_INTR_NUM_REGISTER, 0);
			break;
	}

END:
	return Status;
}
