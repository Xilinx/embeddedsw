/**************************************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
 *       am   01/22/25 Added key transfer support
 *       ma   02/21/25 Added functionality to trigger NON FATAL error when an exception is received
 *       am   02/22/25 Resolved key transfer done bit not being set
 *       am   04/04/25 Increased timeout for KV interrupt status poll
 *       am   04/10/25 Removed poll status interrupt timeout to wait indefinitely
 *       am   04/18/25 Suppressed unused variable warning
 * 1.3   am   05/18/25 Changed wait condition from implicit to explicit comparison
 *       am   06/16/25 Fixed consecutive PMC to ASU key transfers failing
 *       rmv  09/09/25 Removed XASUFW_KILO macro as it not needed and fix doxygen comments
 *       kp   03/16/26 Added ASU RAM ECC initialization and interrupt handling
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
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
#include "xasufw_hw.h"
#include "xasufw_memory.h"
#include "xaes.h"
#include "xasufw_error_manager.h"
#include "xasu_def.h"
#include "xasu_sharedmem.h"
#include "xasufw_alginfo.h"
#include "xasufw_kat.h"
#include "xasufw_hw.h"
#include "xil_error_node.h"

/************************************ Constant Definitions ***************************************/
#define MB_IOMODULE_GPO1_PIT1_PRESCALE_SRC_MASK	(0x2U) /**< IO Module PIT1 prescaler source mask */
#define XASUFW_ASU_IRO_FREQ_IN_HZ	(500000000U) /**< ASU IRO frequency 500Mhz */
#define XASUFW_IOMODULE_IPI_INTRNUM	(28U) /**< IPI interrupt number in IO Module */
#define XASUFW_IOMODULE_DATA_ECC_INTRNUM  (16U) /**< Data RAM ECC interrupt number in
						      IO Module (riscv_interrupt[0]) */
#define XASUFW_IOMODULE_INSTR_ECC_INTRNUM (17U) /**< Instr RAM ECC interrupt number in
						      IO Module (riscv_interrupt[1]) */
#define XASUFW_PIT3_TIMER_TICK		(10U) /**< PIT3 timer tick in milli-seconds */

#define XASUFW_PIT1_RESET_VALUE		(0xFFFFFFFDU) /**< PIT1 reset value */
#define XASUFW_PIT2_RESET_VALUE		(0xFFFFFFFEU) /**< PIT2 reset value */
#define XASUFW_PIT1_CYCLE_VALUE		((u64)XASUFW_PIT1_RESET_VALUE + 1U) /**< PIT1 cycle value */
#define XASUFW_PIT2_CYCLE_VALUE		(XASUFW_PIT2_RESET_VALUE + 1U) /**< PIT2 cycle value */
#define XASUFW_PIT1			(0U) /**< ASUFW PIT1 */
#define XASUFW_PIT2			(1U) /**< ASUFW PIT2 */
#define XASUFW_PIT3			(2U) /**< ASUFW PIT3 */
#define XASUFW_PIT_FREQ_DIVISOR		(100U) /**< ASUFW PIT frequency divisor */
#define XASUFW_MEGA			(1000000U) /**< Value for mega */
#define XASUFW_WORD_SIZE_IN_BITS	(32U) /**< Word size in bits */

/**
 * @brief	Initializes the version, NIST compliance status, KAT status and feature
 * 		capabilities for a crypto module.
 *
 * @param	MODULE	Module short name (e.g., AES, SHA2, ECC) used to construct module-specific
 * 			constants.
 * @param	XASUFW_NIST_FLAG	NIST compliance status flag (XASUFW_NIST_COMPLIANT or
 * 				XASUFW_NIST_NON_COMPLIANT).
 * @param	FEATURE_CAPS	Compile-time feature capability bitmask for the module.
 *
 * The macro builds the module-specific constants from MODULE and forwards them to
 * XAsufw_SetCryptoModuleInfo(), which writes the fields through a single resolved slot pointer.
 */
#define XASUFW_INIT_CRYPTO_MODULE_INFO(MODULE, XASUFW_NIST_FLAG, FEATURE_CAPS) \
	XAsufw_SetCryptoModuleInfo(XASU_MODULE_##MODULE##_ID, \
		XASUFW_ALG_BUILD_VERSION(XASUFW_##MODULE##_MAJOR_VERSION, \
					 XASUFW_##MODULE##_MINOR_VERSION), \
		(u8)(XASUFW_NIST_FLAG), FEATURE_CAPS)

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/*************************************************************************************************/
/**
 * @brief	This function writes the crypto algorithm information of a module to its module
 * 		info slot in the RTCA.
 *
 * @param	ModuleId	Module ID whose information is to be written.
 * @param	Version		Algorithm version built from the module major and minor versions.
 * @param	NistStatus	NIST compliance status of the module.
 * @param	FeatureCaps	Feature capability bitmask of the module.
 *
 *************************************************************************************************/
static inline void XAsufw_SetCryptoModuleInfo(u32 ModuleId, u32 Version, u8 NistStatus,
					      u16 FeatureCaps)
{
	XAsu_CryptoAlgInfo *ModuleInfoPtr = XAsu_GetModuleInfoSlot(ModuleId);

	ModuleInfoPtr->Version = Version;
	ModuleInfoPtr->NistStatus = NistStatus;
	ModuleInfoPtr->KatStatus = XASU_STATUS_NOT_RUN;
	ModuleInfoPtr->FeatureCaps = FeatureCaps;
}

/************************************ Function Prototypes ****************************************/
static void XAsufw_ExceptionEnable(void);
static void XAsufw_ExceptionHandler(void *Data);
static void XAsufw_InitPitTimer(u8 Timer, u32 ResetValue);
static void XAsufw_Pit3TimerHandler(const void *Data);
static s32 XAsufw_RegisterNotifier(u32 EventErrMask, u32 Enable);
static void XAsufw_RamEccCtrl(u32 BaseAddr);
#ifdef XASUFW_RAM_ECC_ENABLE
static void XAsufw_RamEccIntrHandler(void *CallbackRef);
#endif

/************************************ Variable Definitions ***************************************/
static XIOModule IOModule; /**< Instance of the IO Module */

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

	XAsufw_Printf(DEBUG_INFO, "Exception Init Start\r\n");

	/** Initialize processor registers related to interrupts and exceptions. */
	Xil_ExceptionInit();

	/** Register exception handlers. */
	for (Index = XIL_EXCEPTION_ID_FIRST; Index <= XIL_EXCEPTION_ID_LAST; Index++) {
		Status = XAsufw_UpdateErrorStatus(XASUFW_ERR_EXCEPTION, (s32)Index);
		Xil_ExceptionRegisterHandler(Index, XAsufw_ExceptionHandler, (void *)Status);
	}

	/** Register the IO module interrupt handler with the exception table. */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler)XIOModule_DeviceInterruptHandler,
				     (void *) XASUFW_IOMODULE_DEVICE_ID);

	/** Enable processor interrupts and exceptions. */
	Xil_ExceptionEnable();

	XAsufw_Printf(DEBUG_INFO, "Exception Init Done\r\n");
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

#ifdef XASUFW_RAM_ECC_ENABLE
	/**
	 * On RISC-V, a RAM ECC uncorrectable error (UE) aborts the bus transaction
	 * causing a load access fault (mcause=0x5) before the ECC interrupt fires.
	 * Call the unified ECC interrupt handler to detect and report UE.
	 */
	XAsufw_RamEccIntrHandler((void *)(UINTPTR)ASU_RAM_INSTR_ECC_CTRL_BASEADDR);
	XAsufw_RamEccIntrHandler((void *)(UINTPTR)ASU_RAM_DATA_ECC_CTRL_BASEADDR);
#endif

	/** Trigger Fatal error to PLM which performs secure lockdown with IO tri-state. */
	XAsufw_SendErrorToPlm(XASUFW_FATAL_ERROR, (s32)Data);

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
 * @param	Data	Interrupt number of the PIT3 timer interrupt.
 *
 *************************************************************************************************/
static void XAsufw_Pit3TimerHandler(const void *Data)
{
	(void)Data;
	/** Update TaskTimeNow every time the scheduler handler is called for every 10ms */
	TaskTimeNow += XASUFW_PIT3_TIMER_TICK;
}

/*************************************************************************************************/
/**
 * @brief	This function configures the ECC controller of instruction or data RAM at the given
 *		base address. ONOFF.ECC_EN is set or cleared based on XASUFW_RAM_ECC_CTRL_CONFIG.
 *		When XASUFW_RAM_ECC_ENABLE is defined, any pending CE/UE status is also cleared
 *		and CE/UE interrupts are enabled.
 *
 * @param	BaseAddr	Base address of the ECC controller.
 *
 *************************************************************************************************/
static void XAsufw_RamEccCtrl(u32 BaseAddr)
{
	/** Program ONOFF.ECC_EN to enable or disable ECC checking as per xparameter config. */
	XAsufw_WriteReg(BaseAddr + ASU_RAM_ECC_CTRL_ONOFF_OFFSET, XASUFW_RAM_ECC_CTRL_CONFIG);

#ifdef XASUFW_RAM_ECC_ENABLE
	/** Clear any pending CE/UE status before enabling interrupts. */
	XAsufw_WriteReg(BaseAddr + ASU_RAM_ECC_CTRL_STATUS_OFFSET,
			ASU_RAM_ECC_CTRL_STATUS_UE_MASK | ASU_RAM_ECC_CTRL_STATUS_CE_MASK);

	/** Enable both CE and UE interrupts. */
	XAsufw_WriteReg(BaseAddr + ASU_RAM_ECC_CTRL_EN_IRQ_OFFSET,
			ASU_RAM_ECC_CTRL_EN_IRQ_UE_MASK | ASU_RAM_ECC_CTRL_EN_IRQ_CE_MASK);
#endif
}

#ifdef XASUFW_RAM_ECC_ENABLE
/*************************************************************************************************/
/**
 * @brief	This is the unified interrupt handler for ASU RAM ECC events. It handles both
 *		correctable errors (CE) reported as non-fatal and uncorrectable errors (UE)
 *		reported as fatal. This handler is called from the IO Module interrupt context
 *		for CE events, and from XAsufw_ExceptionHandler for UE events since UE aborts
 *		the bus transaction causing a load access fault (mcause=0x5) before the ECC
 *		interrupt fires.
 *
 * @param	CallbackRef	Pointer containing the base address of the ECC controller that
 *				triggered the interrupt (cast to void *).
 *
 *************************************************************************************************/
static void XAsufw_RamEccIntrHandler(void *CallbackRef)
{
	u32 BaseAddr = (u32)(UINTPTR)CallbackRef;
	u32 EccStatus = XAsufw_ReadReg(BaseAddr + ASU_RAM_ECC_CTRL_STATUS_OFFSET);
	u32 FfAddr;
	u32 Ffd;
	u32 Ffe;
	u32 CeCnt;

	/** Handle uncorrectable error (fatal). */
	if ((EccStatus & ASU_RAM_ECC_CTRL_STATUS_UE_MASK) != 0U) {
		FfAddr = XAsufw_ReadReg(BaseAddr + ASU_RAM_ECC_CTRL_UE_FFA_OFFSET);
		Ffd = XAsufw_ReadReg(BaseAddr + ASU_RAM_ECC_CTRL_UE_FFD_OFFSET);
		Ffe = XAsufw_ReadReg(BaseAddr + ASU_RAM_ECC_CTRL_UE_FFE_OFFSET);

		XAsufw_Printf(DEBUG_PRINT_ALWAYS, "RAM ECC UE at controller 0x%08x, "
			       "first failing addr: 0x%08x, first failing data: 0x%08x, "
			       "first failing ECC: 0x%08x\r\n",
			       BaseAddr, FfAddr, Ffd, Ffe);

		/** Clear UE status. */
		XAsufw_WriteReg(BaseAddr + ASU_RAM_ECC_CTRL_STATUS_OFFSET,
				ASU_RAM_ECC_CTRL_STATUS_UE_MASK);

		/** Report fatal error to PLM. */
		XAsufw_SendErrorToPlm(XASUFW_FATAL_ERROR, (s32)XASUFW_ERR_RAM_ECC_UE);

		/** Enters infinite loop just in case control reaches here. */
		while (XASU_TRUE) {
			;
		}
	}

	/** Handle correctable error (non-fatal). */
	if ((EccStatus & ASU_RAM_ECC_CTRL_STATUS_CE_MASK) != 0U) {
		FfAddr = XAsufw_ReadReg(BaseAddr + ASU_RAM_ECC_CTRL_CE_FFA_OFFSET);
		CeCnt = XAsufw_ReadReg(BaseAddr + ASU_RAM_ECC_CTRL_CE_CNT_OFFSET);
		Ffd = XAsufw_ReadReg(BaseAddr + ASU_RAM_ECC_CTRL_CE_FFD_OFFSET);
		Ffe = XAsufw_ReadReg(BaseAddr + ASU_RAM_ECC_CTRL_CE_FFE_OFFSET);

		XAsufw_Printf(DEBUG_PRINT_ALWAYS, "RAM ECC CE at controller 0x%08x, "
			       "first failing addr: 0x%08x, count: %u, "
			       "first failing data: 0x%08x, "
			       "first failing ECC: 0x%08x\r\n",
			       BaseAddr, FfAddr, CeCnt, Ffd, Ffe);

		/** Clear CE status. */
		XAsufw_WriteReg(BaseAddr + ASU_RAM_ECC_CTRL_STATUS_OFFSET,
				ASU_RAM_ECC_CTRL_STATUS_CE_MASK);

		/** Report non-fatal error to PLM. */
		XAsufw_SendErrorToPlm(XASUFW_NON_FATAL_ERROR, (s32)XASUFW_ERR_RAM_ECC_CE);
	}
}
#endif

/*************************************************************************************************/
/**
 * @brief	This function configures ECC for both the instruction and data RAM controllers via
 *		XAsufw_RamEccCtrl().
 *
 *************************************************************************************************/
void XAsufw_RamEccInit(void)
{
	XAsufw_RamEccCtrl(ASU_RAM_INSTR_ECC_CTRL_BASEADDR);
	XAsufw_RamEccCtrl(ASU_RAM_DATA_ECC_CTRL_BASEADDR);
}

/*************************************************************************************************/
/**
 * @brief	This function is called during boot up of ASUFW to initialize IO module and
 * 		PIT timer, and to start IO module.
 *
 * @return
 * 	- XASUFW_SUCCESS, if initialization of IOModule is successful.
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
 * @brief	This function marks the IO module driver as stopped by wrapping XIOModule_Stop(),
 *		which clears the driver's IsStarted flag so that the IO module can be
 *		re-initialized cleanly by the next ASUFW image. It does not disable interrupt
 *		sources in hardware; callers that need to quiesce interrupts must disable or
 *		otherwise quiesce the relevant interrupt sources separately (e.g. via
 *		XAsufw_DisableInterruptSystem()) before invoking this function. Intended to be
 *		called from ASUFW teardown paths such as the in-place update flow, before
 *		control is transferred away from the running firmware image.
 *
 *************************************************************************************************/
void XAsufw_StopIoModule(void)
{
	XIOModule_Stop(&IOModule);
}

/*************************************************************************************************/
/**
 * @brief	This function is called during boot up of ASUFW which will connect the IO module
 * 		interrupt handlers to the processor interrupts. This function also calls
 * 		XAsufw_ExceptionEnable function to initialize and enable processor interrupts
 * 		and exceptions.
 *
 * @return
 * 	- XASUFW_SUCCESS, if setup of IOModule interrupts is successful.
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

	/** Enable the IO Module IPI interrupt. */
	XIOModule_Enable(&IOModule, IntrNum);

#ifdef XASUFW_RAM_ECC_ENABLE
	/** Connect instruction RAM ECC interrupt to its handler. */
	Status = XIOModule_Connect(&IOModule, XASUFW_IOMODULE_INSTR_ECC_INTRNUM,
				   (XInterruptHandler)XAsufw_RamEccIntrHandler,
				   (void *)(UINTPTR)ASU_RAM_INSTR_ECC_CTRL_BASEADDR);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(XASUFW_IOMODULE_CONNECT_FAILED, Status);
		goto END;
	}

	/** Enable the IO Module instruction RAM ECC interrupt. */
	XIOModule_Enable(&IOModule, XASUFW_IOMODULE_INSTR_ECC_INTRNUM);

	/** Connect data RAM ECC interrupt to its handler. */
	Status = XIOModule_Connect(&IOModule, XASUFW_IOMODULE_DATA_ECC_INTRNUM,
				   (XInterruptHandler)XAsufw_RamEccIntrHandler,
				   (void *)(UINTPTR)ASU_RAM_DATA_ECC_CTRL_BASEADDR);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(XASUFW_IOMODULE_CONNECT_FAILED, Status);
		goto END;
	}

	/** Enable the IO Module data RAM ECC interrupt. */
	XIOModule_Enable(&IOModule, XASUFW_IOMODULE_DATA_ECC_INTRNUM);
#endif

	/** Enable interrupts and exceptions. */
	XAsufw_ExceptionEnable();

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is used to read the 64-bit timer value. It reads from PIT1 and PIT2
 * 		and makes it 64-bit.
 *
 * @return
 * 	- Returns 64-bit timer value.
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
 * @param	TimerValStart 	Timer value captured at the beginning.
 * @param	PerfTime	Variable to hold the time elapsed
 *
 * @note The PIT timer starts with higher value and counts down. So TimerValStart will be
 * 		greater than TimerValEnd.
 *
 *************************************************************************************************/
void XAsufw_MeasurePerfTime(u64 TimerValStart, XAsufw_PerfTime *PerfTime)
{
	u64 TimerValEnd;
	u64 TDiff;
	u32 AsuIroFreqMHz = XASUFW_ASU_IRO_FREQ_IN_HZ / XASUFW_MEGA;

	/** Capture the timer value at the end of the measured interval. */
	TimerValEnd = XAsufw_GetTimerValue();

	/** Capture elapsed time. */
	TDiff = TimerValStart - TimerValEnd;

	/** Calculate whole microseconds. */
	PerfTime->TPerfUs = TDiff / AsuIroFreqMHz;

	/** Calculate the fractional part in microseconds. */
	PerfTime->TPerfUsFrac = TDiff % AsuIroFreqMHz;
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
	XAsufw_Printf(DEBUG_PRINT_ALWAYS, "[%llu.%06llu]", (u64)PerfTime.TPerfUs,
		(u64)PerfTime.TPerfUsFrac);
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
	XAsufw_WriteReg(XASU_RTCA_FW_VERSION_ADDR, XASU_FW_VERSION_VALUE);
}

/*************************************************************************************************/
/**
 * @brief	This function writes crypto algorithm information
 * 		(NIST compliant status,Version Info) of each module to the reserved RTCA memory.
 *
 *************************************************************************************************/
void XAsufw_UpdateModulesInfo(void)
{
	XASUFW_INIT_CRYPTO_MODULE_INFO(TRNG, XASUFW_NIST_COMPLIANT, XASUFW_TRNG_FEATURE_CAPS);
	XASUFW_INIT_CRYPTO_MODULE_INFO(SHA2, XASUFW_NIST_COMPLIANT, 0U);
	XASUFW_INIT_CRYPTO_MODULE_INFO(SHA3, XASUFW_NIST_COMPLIANT, 0U);
	XASUFW_INIT_CRYPTO_MODULE_INFO(ECC, XASUFW_NIST_COMPLIANT, XASUFW_ECC_FEATURE_CAPS);
	XASUFW_INIT_CRYPTO_MODULE_INFO(RSA, XASUFW_NIST_COMPLIANT, XASUFW_RSA_FEATURE_CAPS);
	XASUFW_INIT_CRYPTO_MODULE_INFO(AES, XASUFW_NIST_COMPLIANT, XASUFW_AES_FEATURE_CAPS);
#ifdef XASU_HMAC_ENABLE
	XASUFW_INIT_CRYPTO_MODULE_INFO(HMAC, XASUFW_NIST_COMPLIANT, 0U);
#endif
#ifdef XASU_KDF_ENABLE
	XASUFW_INIT_CRYPTO_MODULE_INFO(KDF, XASUFW_NIST_COMPLIANT, 0U);
#endif
#ifdef XASU_ECIES_ENABLE
	XASUFW_INIT_CRYPTO_MODULE_INFO(ECIES, XASUFW_NIST_NON_COMPLIANT, 0U);
#endif
#ifdef XASU_KEYWRAP_ENABLE
	XASUFW_INIT_CRYPTO_MODULE_INFO(KEYWRAP, XASUFW_NIST_COMPLIANT, 0U);
#endif
#ifdef XASU_OCP_ENABLE
	XASUFW_INIT_CRYPTO_MODULE_INFO(OCP, XASUFW_NIST_COMPLIANT, 0U);
#endif
#ifdef XASU_KEYMANAGER_ENABLE
	XASUFW_INIT_CRYPTO_MODULE_INFO(KEYMANAGER, XASUFW_NIST_COMPLIANT, 0U);
#endif
#ifdef XASU_LMS_ENABLE
	XASUFW_INIT_CRYPTO_MODULE_INFO(LMS, XASUFW_NIST_COMPLIANT, 0U);
#endif
#ifdef XASU_MLDSA_ENABLE
	XASUFW_INIT_CRYPTO_MODULE_INFO(MLDSA, XASUFW_NIST_COMPLIANT, 0U);
#endif
#ifdef XASU_MLKEM_ENABLE
	XASUFW_INIT_CRYPTO_MODULE_INFO(MLKEM, XASUFW_NIST_COMPLIANT, 0U);
#endif
}

/*************************************************************************************************/
/**
 * @brief	This function disables the IO Module interrupts.
 *
 *************************************************************************************************/
void XAsufw_DisableInterruptSystem(void)
{
	/** Disable IPI interrupt. */
	XIOModule_Disable(&IOModule, XASUFW_IOMODULE_IPI_INTRNUM);

	/** Disable PIT3 interrupt. */
	XIOModule_Disable(&IOModule, XIN_IOMODULE_PIT_3_INTERRUPT_INTR);
}

/*************************************************************************************************/
/**
 * @brief	This function registers notifier to get events notification from PLM.
 *
 * @param	EventErrMask	Event error mask.
 * @param	Enable		Enable/Disable flag for notifier registration.
 *
 * @return
 *	- XASUFW_SUCCESS, if notifier registration is successful.
 *	- XASUFW_FAILURE, if any other failure.
 *	- XASUFW_REGISTER_NOTIFIER_SEND_IPI_FAILED, if IPI send is failed.
 *	- XASUFW_REGISTER_NOTIFIER_READ_IPI_FAILED, if IPI read is failed.
 *
 *************************************************************************************************/
static s32 XAsufw_RegisterNotifier(u32 EventErrMask, u32 Enable)
{
	s32 Status = XASUFW_FAILURE;
	u32 Payload[XASUFW_REGISTER_NOTIFIER_PAYLOAD_SIZE] = {0U};
	u32 Response[XASUFW_REGISTER_NOTIFIER_RESP_SIZE] = {0U};

	/**
	 * Prepare payload to register notifier for events specified in EventErrMask. Payload will be
	 * sent to PLM through IPI to register the notifier.
	 */
	Payload[XASUFW_BUFFER_INDEX_ZERO] =
		XASUFW_PLM_IPI_HEADER(XOCP_ASU_REG_NOTIFIER_PAYLOAD_SIZE,
				XASU_PLM_REG_NOTIFIER_CMD_ID, XASUFW_XILPM_MODULE_ID);
	Payload[XASUFW_BUFFER_INDEX_ONE] = XIL_NODETYPE_EVENT_ERROR_SW_ERR;
	Payload[XASUFW_BUFFER_INDEX_TWO] = EventErrMask;
	Payload[XASUFW_BUFFER_INDEX_FOUR] = Enable;

	Status = XAsufw_SendIpiToPlm(Payload, XASUFW_REGISTER_NOTIFIER_PAYLOAD_SIZE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_REGISTER_NOTIFIER_SEND_IPI_FAILED);
		goto END;
	}

	Status = XAsufw_ReadIpiRespFromPlm(Response, XASUFW_REGISTER_NOTIFIER_RESP_SIZE);
	if ((Status != XASUFW_SUCCESS) ||
	    (Response[XASUFW_BUFFER_INDEX_ZERO] != (u32)XASUFW_SUCCESS)) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_REGISTER_NOTIFIER_READ_IPI_FAILED);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function enables or disables event notifiers for ASU and OCP subsystem updates.
 *
 * @param	Enable	Enable/Disable flag for notifier registration
 * 			(XASUFW_REGISTER_NOTIFIER_ENABLE or XASUFW_REGISTER_NOTIFIER_DISABLE).
 *
 * @return
 *	- XASUFW_SUCCESS, if notifier operation is successful.
 *	- XASUFW_FAILURE, if any other failure.
 *
 *************************************************************************************************/
s32 XAsufw_EnableDisableEventNotifiers(u32 Enable)
{
	s32 Status = XASUFW_FAILURE;
	u32 EventMask = XIL_EVENT_ERROR_MASK_ASU_UPDATE;

#ifdef XASU_OCP_ENABLE
	/** Include OCP subsystem update event mask if OCP is enabled. */
	EventMask |= XIL_EVENT_ERROR_MASK_OCP_SUBSYS_UPDATE;
#endif
	Status = XAsufw_RegisterNotifier(EventMask, Enable);

	return Status;
}

/** @} */
