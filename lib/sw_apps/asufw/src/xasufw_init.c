/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
#include "xasufw_memory.h"
#include "xaes.h"
#include "xasufw_error_manager.h"
#include "xasu_def.h"
#include "xasufw_alginfo.h"
#include "xasufw_kat.h"
#include "xocp_dme.h"

/************************************ Constant Definitions ***************************************/
#define MB_IOMODULE_GPO1_PIT1_PRESCALE_SRC_MASK	(0x2U) /**< IO Module PIT1 prescaler source mask */
#define XASUFW_ASU_IRO_FREQ_IN_HZ	(500000000U) /**< ASU IRO frequency 500Mhz */
#define XASUFW_IOMODULE_IPI_INTRNUM	(28U) /**< IPI interrupt number in IO Module */
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
#define XASUFW_KEY_TX_PAYLOAD_SIZE 	(2U) /**< Key transfer payload size */
#define XASUFW_PAYLOAD_INDEX_0		(0U) /**< Key transfer payload index 0 */
#define XASUFW_PAYLOAD_INDEX_1		(1U) /**< Key transfer payload index 1 */
#define XASUFW_RESPONSE_INDEX_0		(0U) /**< Key transfer response index 0 */
#define XASUFW_RESPONSE_INDEX_1		(1U) /**< Key transfer response index 1 */

/**
 * @brief	Initializes the version, NIST compliance status, and clears KAT bits for a
 * 		crypto module.
 *
 * @param	MODULE	Module short name (e.g., AES, SHA2, ECC) used to construct module-specific
 * 			constants.
 *
 * This macro sets:
 *  - 'version' using XASUFW_ALG_BUILD_VERSION(MAJOR, MINOR)
 *  - 'NistStatus' to XASUFW_NIST_FLAG
 *  - 'KatStatusBits' to 0
 *
 */
#define XASUFW_INIT_CRYPTO_MODULE_INFO(MODULE, XASUFW_NIST_FLAG) \
	XAsu_CryptoAlgInfoPtr[XASU_MODULE_##MODULE##_ID].Version = \
		XASUFW_ALG_BUILD_VERSION(XASUFW_##MODULE##_MAJOR_VERSION, XASUFW_##MODULE##_MINOR_VERSION); \
	XAsu_CryptoAlgInfoPtr[XASU_MODULE_##MODULE##_ID].NistStatus = XASUFW_NIST_FLAG; \
	XAsu_CryptoAlgInfoPtr[XASU_MODULE_##MODULE##_ID].KatStatus = XASUFW_KAT_STATUS_NOT_RUN;

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static void XAsufw_ExceptionEnable(void);
static void XAsufw_ExceptionHandler(void *Data);
static void XAsufw_InitPitTimer(u8 Timer, u32 ResetValue);
static void XAsufw_Pit3TimerHandler(const void *Data);

/************************************ Variable Definitions ***************************************/
static XIOModule IOModule; /* Instance of the IO Module */
static u32 PufKekFlag = XASUFW_PUF_KEK_GEN_FAILURE; /**< PUF KEK generation status */

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

	/* TODO: Register separate handler for illegal instruction trap to execute secure lockdown */

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

	/** Trigger Non-Fatal error to PLM. */
	XAsufw_SendErrorToPlm(XASUFW_NON_FATAL_ERROR, (s32)Data);

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
}

/*************************************************************************************************/
/**
 * @brief	This function gets three 256-bit keys (two red/black and one KEK key) from PMXC to Key
 * receiver engine in ASU using 128-bit dedicated interface between PMXC and ASU.
 *
 * @return
 *		- XASUFW_SUCCESS, if all three keys are received successfully.
 *		- XASUFW_ERR_KV_INTERRUPT_DONE_TIMEOUT, if key transfer done timeout occurs.
 *		- XASUFW_FAILURE, if all three keys are not received.
 *
 *************************************************************************************************/
s32 XAsufw_PmcKeyTransfer(void)
{
	s32 Status = XASUFW_FAILURE;
	u32 Response[XASUFW_KEY_TX_PAYLOAD_SIZE] = {0U};
	u32 Payload[XASUFW_KEY_TX_PAYLOAD_SIZE];

	/** Transfer efuse key_0 is black or red based on user configuration. */
	XAsufw_WriteReg((XASU_XKEY_0_BASEADDR + XAES_EFUSE_KEY_0_BLACK_OR_RED_OFFSET),
		XASUFW_PMXC_EFUSE_USER_KEY_0);

	/** Transfer efuse key_1 is black or red based on user configuration. */
	XAsufw_WriteReg((XASU_XKEY_0_BASEADDR + XAES_EFUSE_KEY_1_BLACK_OR_RED_OFFSET),
		XASUFW_PMXC_EFUSE_USER_KEY_1);

	/** Indicate to PMXC and ASU that ASU is ready to accept key transfer. */
	XAsufw_WriteReg((XASU_XKEY_0_BASEADDR + XAES_ASU_PMXC_KEY_TRANSFER_READY_OFFSET),
		XAES_ASU_PMXC_KEY_TRANSFER_READY_MASK);

	Payload[XASUFW_PAYLOAD_INDEX_0] = XASUFW_PLM_IPI_HEADER(0U, XASUFW_PLM_ASU_KEY_TX_API_ID,
			XASUFW_PLM_ASU_MODULE_ID);
	Payload[XASUFW_PAYLOAD_INDEX_1] = XASU_RTCA_BH_IV_ADDR;

	/** Send Key transfer IPI request to PLM. */
	Status = XAsufw_SendIpiToPlm(Payload, XASUFW_KEY_TX_PAYLOAD_SIZE);
	if (Status != XASUFW_SUCCESS) {
		XAsufw_Printf(DEBUG_INFO, "Send IPI to PLM failed\r\n");
		goto END;
	}

	/** Wait till HW to set the KV interrupt status bit when key transfer(KT) is done. */
	while ((XAsufw_ReadReg(XASU_XKEY_0_BASEADDR + XAES_KV_INTERRUPT_STATUS_OFFSET) &
		XAES_KV_INTERRUPT_STATUS_DONE_MASK) == 0U);

	/** Disable ASU Key transfer. */
	XAsufw_WriteReg((XASU_XKEY_0_BASEADDR + XAES_ASU_PMXC_KEY_TRANSFER_READY_OFFSET),
		XAES_ASU_PMXC_KEY_TRANSFER_READY_DISABLE);

	/** Clear KV interrupt status. */
	XAsufw_WriteReg((XASU_XKEY_0_BASEADDR + XAES_KV_INTERRUPT_STATUS_OFFSET),
		XAES_KV_INTERRUPT_STATUS_CLEAR_MASK);

	/** Read Key transfer response received from PLM. */
	Status = XAsufw_ReadIpiRespFromPlm(Response, XASUFW_KEY_TX_PAYLOAD_SIZE);
	if ((Status != XASUFW_SUCCESS) || (Response[XASUFW_RESPONSE_INDEX_0] != (u32)XASUFW_SUCCESS)) {
		XAsufw_Printf(DEBUG_INFO, "Read IPI response from PLM failed\r\n");
	}

	PufKekFlag = Response[XASUFW_RESPONSE_INDEX_1];
END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function executes the key transfer task handler.
 *
 * @param	KeyTransferTask		Pointer to the key transfer task instance.
 *
 * @return
 *	- XASUFW_SUCCESS, if key transfer task is handled successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *
 *************************************************************************************************/
s32 XAsufw_RunKeyTransferTaskHandler(void *KeyTransferTask)
{
	s32 Status = XASUFW_FAILURE;
	XTask_TaskNode *SelfKeyTransferTask = (XTask_TaskNode *)KeyTransferTask;
	u8 AesKatStatus = *XAsu_GetKatStatusPtr(XASU_MODULE_AES_ID);

	/** Get keys from PMC. */
	Status = XAsufw_PmcKeyTransfer();
	if (XASUFW_SUCCESS != Status) {
		XAsufw_Printf(DEBUG_GENERAL, "ASUFW key transfer failed. Error: 0x%x\r\n", Status);
		goto END;
	}

	/** Generate DME KEK if PUF KEK generation and AES KAT are successful. */
	if (PufKekFlag == XASUFW_PUF_KEK_GEN_SUCCESS) {
#ifdef XASU_OCP_ENABLE
		if (AesKatStatus == XASUFW_KAT_STATUS_PASS) {
			Status = XOcp_GenerateDmeKek();
			if (Status != XASUFW_SUCCESS) {
				XAsufw_Printf(DEBUG_GENERAL, "ASUFW DME KEK generation failed. Error: 0x%x\r\n", Status);
			}
		}
#else
		(void)AesKatStatus;
		XAsufw_Printf(DEBUG_PRINT_ALWAYS, "ASUFW DME KEK generation failed as OCP is disabled\r\n");
#endif
	}

END:
	/** Delete self task after completion. */
	XTask_Delete(SelfKeyTransferTask);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function writes crypto algorithm information
 * 		(NIST compliant status,Version Info) of each module to the reserved RTCA memory.
 *
 *************************************************************************************************/
void XAsufw_UpdateModulesInfo(void)
{
	XAsu_CryptoAlgInfo *XAsu_CryptoAlgInfoPtr = GetModuleInfoPtr();

	XASUFW_INIT_CRYPTO_MODULE_INFO(TRNG, XASUFW_NIST_COMPLIANT);
	XASUFW_INIT_CRYPTO_MODULE_INFO(SHA2, XASUFW_NIST_COMPLIANT);
	XASUFW_INIT_CRYPTO_MODULE_INFO(SHA3, XASUFW_NIST_COMPLIANT);
	XASUFW_INIT_CRYPTO_MODULE_INFO(ECC, XASUFW_NIST_COMPLIANT);
	XASUFW_INIT_CRYPTO_MODULE_INFO(RSA, XASUFW_NIST_COMPLIANT);
	XASUFW_INIT_CRYPTO_MODULE_INFO(AES, XASUFW_NIST_COMPLIANT);
	XASUFW_INIT_CRYPTO_MODULE_INFO(HMAC, XASUFW_NIST_COMPLIANT);
	XASUFW_INIT_CRYPTO_MODULE_INFO(KDF, XASUFW_NIST_COMPLIANT);
	XASUFW_INIT_CRYPTO_MODULE_INFO(ECIES, XASUFW_NIST_NON_COMPLIANT);
	XASUFW_INIT_CRYPTO_MODULE_INFO(KEYWRAP, XASUFW_NIST_COMPLIANT);
}
/** @} */
