/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file versal_net/xplmi_plat.c
*
* This file contains the PLMI versal_net platform specific code.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  bm   07/06/2022 Initial release
*       dc   07/12/2022 Added XPlmi_RomISR() API
*       kpt  07/21/2022 Added KAT APIs
*       bm   07/22/2022 Update EAM logic for In-Place PLM Update
*       bm   07/22/2022 Retain critical data structures after In-Place PLM Update
*       bm   07/22/2022 Shutdown modules gracefully during update
* 1.01  bm   11/07/2022 Clear SSS Cfg Error in SSSCfgSbiDma for Versal Net
*       ng   11/11/2022 Fixed doxygen file name error
*       kpt  01/04/2023 Added XPlmi_CheckandUpdateFipsState to update FIPS state
*       bm   03/11/2023 Modify XPlmi_PreInit to return Status
*		dd   03/28/2023 Updated doxygen comments
*       ng   03/30/2023 Updated algorithm and return values in doxygen comments
* 1.02  bm   04/28/2023 Update Pmc IRO frequency by detecting the part
*       dd   05/24/2023 Updated doxygen comments
*       ng   05/31/2023 Initialised IsKatRan state to False
*       bm   07/06/2023 Initialize address buffer list
*       ng   06/26/2023 Added support for system device-tree flow
*       sk   07/18/2023 Updated error codes in VerifyAddrRange function
*       sk   07/31/2023 Added redundant write for SSS Config
*       kpt  08/28/2023 Reread from efuse cache to enhance security
*       sk   09/07/2023 Added redundancy check in XPlmi_SetPmcIroFreq
*                       for updating the MB Freq
*       ng   09/22/2023 Fixed missing header for microblaze sleep
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_plat.h"
#include "xplmi_update.h"
#include "xplmi_ipi.h"
#include "xplmi_proc.h"
#include "xplmi_task.h"
#ifndef SDT
	#include "microblaze_sleep.h"
#else
	#include "sleep.h"
#endif
#include "xplmi_hw.h"
#include "xplmi_err_common.h"
#include "xplmi_wdt.h"
#include "xplmi_gic_interrupts.h"
#include "xcfupmc.h"
#include "xplmi_generic.h"
#include "xplmi.h"
#include "xplmi_proc.h"
#include "xil_util.h"
#include "xplmi_err.h"

/************************** Constant Definitions *****************************/
#define XPLMI_ROM_VERSION_1_0		(0x10U) /**< ROM version 1 */

#define XPLMI_SSSCFG_SHA0_MASK		(0x000F0000U) /**< SHA0 mask */
#define XPLMI_SSSCFG_SHA1_MASK		(0x0F000000U) /**< SHA1 mask */
#define XPLMI_SSSCFG_AES_MASK		(0x0000F000U) /**< AES mask */

#define XPLMI_SSS_SHA0_DMA0		(0x000C0000U) /**< SHA0 DMA0 */
#define XPLMI_SSS_SHA0_DMA1		(0x00070000U) /**< SHA0 DMA1 */
#define XPLMI_SSS_SHA1_DMA0		(0x0A000000U) /**< SHA1 DMA0 */
#define XPLMI_SSS_SHA1_DMA1		(0x0F000000U) /**< SHA1 DMA1 */

#define XPLMI_SSS_AES_DMA0		(0x0000E000U) /**< AES DMA0 */
#define XPLMI_SSS_AES_DMA1		(0x00005000U) /**< AES DMA1 */

#define XPLMI_LPDINITIALIZED_VER	(1U) /**< LPD initialized version */
#define XPLMI_LPDINITIALIZED_LCVER	(1U) /**< LPD initialized LC version */

#define XPLMI_UART_BASEADDR_VER		(1U) /**< UART base address version */
#define XPLMI_UART_BASEADDR_LCVER	(1U) /**< UART base address LC version */

#define XPLMI_TRACE_LOG_VERSION		(1U) /**< Trace log version */
#define XPLMI_TRACE_LOG_LCVERSION	(1U) /**< Trace log LC version */

#define XPLMI_BOARD_PARAMS_VERSION	(1U) /**< Board parameters version */
#define XPLMI_BOARD_PARAMS_LCVERSION	(1U) /**< Board parameters LC version */

#define XPLMI_PMC_VOLTAGE_MULTIPLIER	(32768.0f) /**< Voltage multiplier for Sysmon */

/* PMC IRO Frequency related macros */
#define XPLMI_PMC_IRO_FREQ_233_MHZ	(233000000U) /**< PMC IRO frequency 233Mhz */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
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
static void XPlmi_DisableClearIOmodule(void);
static void XPlmi_StopTimer(u8 Timer);
static void XPlmi_HwIntrHandler(void *CallbackRef);
static u32 XPlmi_GetIoIntrMask(void);
static void XPlmi_SetIoIntrMask(u32 Value);
static int XPlmi_UpdateFipsState(void);
static u32 XPlmi_IsHpPart(void);

/************************** Variable Definitions *****************************/
/* Structure for Top level interrupt table */
static XInterruptHandler g_TopLevelInterruptTable[] = {
	XPlmi_HwIntrHandler,
	XPlmi_IntrHandler,
	XPlmi_ErrIntrHandler,
};

 /* Default IRO frequency during ROM phase is 233MHz */
static u32 RomIroFreq = XPLMI_PMC_IRO_FREQ_233_MHZ;

/*****************************************************************************/
/**
 * @brief	This function provides pointer to g_TopLevelInterruptTable
 *
 * @return	Pointer to g_TopLevelInterruptTable structure
 *
 *****************************************************************************/
XInterruptHandler *XPlmi_GetTopLevelIntrTbl(void)
{
	return g_TopLevelInterruptTable;
}

/*****************************************************************************/
/**
 * @brief	This function provides size of g_TopLevelInterruptTable
 *
 * @return	Size g_TopLevelInterruptTable structure
 *
 *****************************************************************************/
u8 XPlmi_GetTopLevelIntrTblSize(void)
{
	return XPLMI_ARRAY_SIZE(g_TopLevelInterruptTable);
}

/*****************************************************************************/
/**
 * @brief	This function provides pointer to BoardParams
 *
 * @return	Pointer to BoardParams
 *
 *****************************************************************************/
XPlmi_BoardParams *XPlmi_GetBoardParams(void)
{
	static XPlmi_BoardParams BoardParams __attribute__ ((aligned(4U))) = {
		.Name = {0U,},
		.Len = 0U,
	};

	EXPORT_GENERIC_DS(BoardParams, XPLMI_BOARD_PARAMS_DS_ID,
		XPLMI_BOARD_PARAMS_VERSION, XPLMI_BOARD_PARAMS_LCVERSION,
		sizeof(BoardParams), (u32)(UINTPTR)&BoardParams);

	return &BoardParams;
}

/*****************************************************************************/
/**
 * @brief	This function provides LpdInitialized variable pointer
 *
 * @return	Pointer to LpdInitialized variable
 *
 *****************************************************************************/
u32 *XPlmi_GetLpdInitialized(void)
{
	static u32 LpdInitialized __attribute__ ((aligned(4U))) = 0U;

	EXPORT_GENERIC_DS(LpdInitialized, XPLMI_LPDINITIALIZED_DS_ID,
		XPLMI_LPDINITIALIZED_VER, XPLMI_LPDINITIALIZED_LCVER,
		sizeof(LpdInitialized), (u32)(UINTPTR)&LpdInitialized);

	return &LpdInitialized;
}

/*****************************************************************************/
/**
 * @brief	This function provides LpdInitialized variable pointer
 *
 * @return	Pointer to LpdInitialized variable
 *
 *****************************************************************************/
u32 *XPlmi_GetUartBaseAddr(void)
{
	static u32 UartBaseAddr __attribute__ ((aligned(4U))) =
		XPLMI_INVALID_UART_BASE_ADDR; /**< Base address of Uart */

	EXPORT_GENERIC_DS(UartBaseAddr, XPLMI_UART_BASEADDR_DS_ID,
		XPLMI_UART_BASEADDR_VER, XPLMI_UART_BASEADDR_LCVER,
		sizeof(UartBaseAddr), (u32)(UINTPTR)&UartBaseAddr);

	return &UartBaseAddr;
}

/*****************************************************************************/
/**
 * @brief	This function performs plmi pre-initializaton.
 *
 * @return
 *			- XST_SUCCESS always.
 *
 *****************************************************************************/
int XPlmi_PreInit(void)
{
	if (XPlmi_IsPlmUpdateDone() == (u8)TRUE) {
		XPlmi_RestoreWdt();
	}

	/* Initialize Address Buffer List */
	XPlmi_SetAddrBufferList();

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief	This function performs initialization of platform specific RCTA
 *		registers
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_RtcaPlatInit(void)
{
	/* Versal Net specific RTCA registers Init */
	return;
}

/*****************************************************************************/
/**
 * @brief	This function prints ROM version using ROM digest value.
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_PrintRomVersion(void)
{
	XPlmi_Printf(DEBUG_INFO, "ROM Version: v%u.%u\n\r",
		(XPLMI_ROM_VERSION_1_0 >> 4U), (XPLMI_ROM_VERSION_1_0 & 15U));
}

/*****************************************************************************/
/**
 * @brief	This function masks the secure stream switch value
 *
 * @param	InputSrc	- Input source to be selected for the resource
 * @param	OutputSrc	- Output source to be selected for the resource
 *
 *****************************************************************************/
void XPlmi_SssMask(u32 InputSrc, u32 OutputSrc)
{
	u32 Mask = 0U;
	u32 RegVal = XPlmi_In32(PMC_GLOBAL_PMC_SSS_CFG);

	if ((InputSrc == XPLMI_PMCDMA_0) || (OutputSrc == XPLMI_PMCDMA_0)) {
		if ((RegVal & XPLMI_SSSCFG_SBI_MASK) == XPLMI_SSS_SBI_DMA0) {
			Mask |= XPLMI_SSSCFG_SBI_MASK;
		}
		if ((RegVal & XPLMI_SSSCFG_SHA0_MASK) == XPLMI_SSS_SHA0_DMA0) {
			Mask |= XPLMI_SSSCFG_SHA0_MASK;
		}
		if ((RegVal & XPLMI_SSSCFG_SHA1_MASK) == XPLMI_SSS_SHA1_DMA0) {
			Mask |= XPLMI_SSSCFG_SHA1_MASK;
		}
		if ((RegVal & XPLMI_SSSCFG_AES_MASK) == XPLMI_SSS_AES_DMA0) {
			Mask |= XPLMI_SSSCFG_AES_MASK;
		}
		if ((RegVal & XPLMI_SSSCFG_DMA0_MASK) != 0U) {
			Mask |= XPLMI_SSSCFG_DMA0_MASK;
		}
	}

	if ((InputSrc == XPLMI_PMCDMA_1) || (OutputSrc == XPLMI_PMCDMA_1)) {
		if ((RegVal & XPLMI_SSSCFG_SBI_MASK) == XPLMI_SSS_SBI_DMA1) {
			Mask |= XPLMI_SSSCFG_SBI_MASK;
		}
		if ((RegVal & XPLMI_SSSCFG_SHA0_MASK) == XPLMI_SSS_SHA0_DMA1) {
			Mask |= XPLMI_SSSCFG_SHA0_MASK;
		}
		if ((RegVal & XPLMI_SSSCFG_SHA1_MASK) == XPLMI_SSS_SHA1_DMA1) {
			Mask |= XPLMI_SSSCFG_SHA1_MASK;
		}
		if ((RegVal & XPLMI_SSSCFG_AES_MASK) == XPLMI_SSS_AES_DMA1) {
			Mask |= XPLMI_SSSCFG_AES_MASK;
		}
		if ((RegVal & XPLMI_SSSCFG_DMA1_MASK) != 0U) {
			Mask |= XPLMI_SSSCFG_DMA1_MASK;
		}
	}

	RegVal &= ~Mask;
	XSECURE_REDUNDANT_IMPL(XPlmi_Out32, PMC_GLOBAL_PMC_SSS_CFG, RegVal);
}

/*****************************************************************************/
/**
 * @brief	This function provides TraceLog instance
 *
 * @return	Pointer to TraceLog variable
 *
 *****************************************************************************/
XPlmi_CircularBuffer *XPlmi_GetTraceLogInst(void)
{
	/* Trace log buffer */
	static XPlmi_CircularBuffer TraceLog __attribute__ ((aligned(4U))) = {
		.StartAddr = XPLMI_TRACE_LOG_BUFFER_ADDR,
		.Len = XPLMI_TRACE_LOG_BUFFER_LEN,
		.Offset = 0x0U,
		.IsBufferFull = (u32)FALSE,
	};

	EXPORT_GENERIC_DS(TraceLog, XPLMI_TRACELOG_DS_ID, XPLMI_TRACE_LOG_VERSION,
		XPLMI_TRACE_LOG_LCVERSION, sizeof(TraceLog), (u32)(UINTPTR)&TraceLog);
	return &TraceLog;
}

/*****************************************************************************/
/**
 * @brief	This function is used for shutdown operation before In-place
 *		PLM Update
 *
 * @param	Op is the operation information
 *
 * @return	XST_SUCCESS on success and XST_FAILURE on failure
 *
 *****************************************************************************/
int XPlmi_GenericHandler(XPlmi_ModuleOp Op)
{
	int Status = XST_FAILURE;
	u8 Index;
	static u8 GenericHandlerState = XPLMI_MODULE_NORMAL_STATE;

	if (Op.Mode == XPLMI_MODULE_SHUTDOWN_INITIATE) {
		if (GenericHandlerState == XPLMI_MODULE_NORMAL_STATE) {
			GenericHandlerState = XPLMI_MODULE_SHUTDOWN_INITIATED_STATE;
			Status = XST_SUCCESS;
		}
	}
	else if (Op.Mode == XPLMI_MODULE_SHUTDOWN_COMPLETE) {
		if (GenericHandlerState == XPLMI_MODULE_SHUTDOWN_COMPLETED_STATE) {
			Status = XST_SUCCESS;
			goto END;
		}
		if (GenericHandlerState != XPLMI_MODULE_SHUTDOWN_INITIATED_STATE) {
			goto END;
		}

		/* Clear KAT status */
		XPlmi_Out32(XPLMI_RTCFG_SECURE_STATE_ADDR, 0U);
		XPlmi_Out32(XPLMI_RTCFG_SECURE_STATE_PLM_ADDR, 0U);

		/* Disable all the Error Actions */
		for (Index = 0U; Index < XPLMI_PMC_MAX_ERR_CNT; Index++) {
			(void)EmDisableErrAction(
			GET_PMC_IRQ_MASK(GET_PMC_ERR_ACTION_OFFSET(Index)),
			MASK32_ALL_HIGH);
		}
		for (Index = 0U; Index < XPLMI_PSM_MAX_ERR_CNT; Index++) {
			(void)XPlmi_EmDisablePsmErrors(
			GET_PSM_ERR_ACTION_OFFSET(Index), MASK32_ALL_HIGH);
		}
		/* Stop Timers */
		XPlmi_StopTimer(XPLMI_PIT3);

		/* Disable & Acknowledge Interrupts */
		microblaze_disable_interrupts();

		/* Disable IPI interrupt */
		XPlmi_PlmIntrDisable(XPLMI_IOMODULE_PMC_IPI);
		/* Clear SBI interrupt */
		XPlmi_GicIntrClearStatus(XPLMI_SBI_GICP_INDEX, XPLMI_SBI_GICPX_INDEX);
		/* Clear IPI interrupt */
		XPlmi_PlmIntrClear(XPLMI_IOMODULE_PMC_IPI);
		/* Disable and Clear all Iomodule interrupts */
		XPlmi_DisableClearIOmodule();
		GenericHandlerState = XPLMI_MODULE_SHUTDOWN_COMPLETED_STATE;

		Status = XST_SUCCESS;
	}
	else if (Op.Mode == XPLMI_MODULE_SHUTDOWN_ABORT) {
		if (GenericHandlerState == XPLMI_MODULE_SHUTDOWN_INITIATED_STATE) {
			GenericHandlerState = XPLMI_MODULE_NORMAL_STATE;
			Status = XST_SUCCESS;
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function processes and provides SrcAddr and DestAddr for
 * 		cfi readback
 *
 * @param	SlrType is the type of Slr passed in readback cmd
 * @param	SrcAddr is the pointer to the SrcAddr variable
 * @param	DestAddrRead is the pointer to the DestAddrRead variable
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_GetReadbackSrcDest(u32 SlrType, u64 *SrcAddr, u64 *DestAddrRead)
{
	(void)SlrType;
	if ((SrcAddr != NULL) && (DestAddrRead != NULL)) {
		*SrcAddr = (u64)CFU_FDRO_2_ADDR;
		*DestAddrRead = (u64)CFU_STREAM_2_ADDR;
	}
}

/*****************************************************************************/
/**
 * @brief	This will add the GIC interrupt task handler to the TaskQueue.
 *
 * @param	PlmIntrId is the GIC interrupt ID of the task
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_GicAddTask(u32 PlmIntrId)
{
	/* Add task to the task queue */
	XPlmi_GicIntrAddTask(PlmIntrId | XPLMI_IOMODULE_PPU1_HW_INT |
		(XPLMI_HW_INT_GIC_IRQ << XPLMI_HW_SW_INTR_SHIFT));
}

/****************************************************************************/
/**
* @brief    This function is the Ipi interrupt handler for the device.
*
* @param    CallbackRef is a dummy argument
*
* @return   None
*
****************************************************************************/
void XPlmi_IpiIntrHandler(void *CallbackRef)
{
#ifdef XPLMI_IPI_DEVICE_ID
	XPlmi_TaskNode *Task = NULL;
	(void)CallbackRef;
	u16 IpiIntrVal;
	u16 IpiMaskVal;
	u16 IpiIndexMask;
	u8 IpiIndex;

	XPlmi_PlmIntrClear(XPLMI_IOMODULE_PMC_IPI);
	IpiIntrVal = (u16)Xil_In32(IPI_PMC_ISR);
	IpiMaskVal = (u16)Xil_In32(IPI_PMC_IMR);
	XPlmi_Out32(IPI_PMC_IDR, IpiIntrVal);

	/*
	 * Check IPI source channel and add channel specific task to
	 * task queue according to the channel priority
	 */
	for (IpiIndex = 0U; IpiIndex < XPLMI_IPI_MASK_COUNT; ++IpiIndex) {
		IpiIndexMask = (u16)1U << IpiIndex;
		if (((IpiIntrVal & IpiIndexMask) != 0U) &&
			((IpiMaskVal & IpiIndexMask) == 0U)) {
			Task = XPlmi_GetTaskInstance(NULL, NULL,
					XPlmi_GetIpiIntrId(IpiIndex));
			if (Task == NULL) {
				XPlmi_Printf(DEBUG_GENERAL, "IPI Interrupt"
						" add task error\n\r");
				break;
			}
			XPlmi_TaskTriggerNow(Task);
		}
	}

#endif
}

/*****************************************************************************/
/**
 * @brief	This function registers and enables IPI interrupt
 *
 * @return	XST_SUCCESS on success and XST_FAILURE on failure
 *
 *****************************************************************************/
int XPlmi_RegisterNEnableIpi(void)
{
	int Status = XST_FAILURE;

	Status = XPlmi_RegisterHandler(XPLMI_IOMODULE_PMC_IPI,
		(GicIntHandler_t)(void *)XPlmi_IpiIntrHandler, (void *)XPLMI_IPI_INTR_ID);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	XPlmi_PlmIntrEnable(XPLMI_IOMODULE_PMC_IPI);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides
 *
 * @return	XST_SUCCESS on success and XST_FAILURE on failure
 *
 *****************************************************************************/
static void XPlmi_HwIntrHandler(void *CallbackRef)
{
	u32 HwIntStatus;
	u32 HwIntMask;

	HwIntStatus = XPlmi_In32(PMC_GLOBAL_PPU1_HW_INT_ADDR);
	HwIntMask = XPlmi_In32(PMC_GLOBAL_PPU1_HW_INT_MASK_ADDR);

	if ((HwIntStatus & PMC_GLOBAL_PPU1_HW_INT_GICP_IRQ_MASK) ==
			PMC_GLOBAL_PPU1_HW_INT_GICP_IRQ_MASK) {
		if ((HwIntMask & PMC_GLOBAL_PPU1_HW_INT_GICP_IRQ_MASK) == 0U) {
			XPlmi_GicIntrHandler(CallbackRef);
			XPlmi_Out32(PMC_GLOBAL_PPU1_HW_INT_ADDR,
				PMC_GLOBAL_PPU1_HW_INT_GICP_IRQ_MASK);
			HwIntStatus &= ~PMC_GLOBAL_PPU1_HW_INT_GICP_IRQ_MASK;
		}
	}
	/* Call XPlmiIntrHandler if any other interrupt is set */
	if ((HwIntStatus & (~PMC_GLOBAL_PPU1_HW_INT_GICP_IRQ_MASK)) != 0U){
		if ((HwIntMask & (~PMC_GLOBAL_PPU1_HW_INT_GICP_IRQ_MASK)) == 0U){
			XPlmi_IntrHandler(CallbackRef);
			XPlmi_Out32(PMC_GLOBAL_PPU1_HW_INT_ADDR,
				HwIntStatus & (~HwIntMask));
		}
	}
}

/*****************************************************************************/
/**
 * @brief	This function registers and enables IPI interrupt
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_EnableIomoduleIntr(void)
{
	XPlmi_PlmIntrEnable(XPLMI_IOMODULE_PPU1_HW_INT);
	XPlmi_PlmIntrEnable(XPLMI_IOMODULE_ERR_IRQ);
	XPlmi_PlmIntrEnable(XPLMI_IOMODULE_PMC_PIT3_IRQ);
	XPlmi_Out32(PMC_GLOBAL_PPU1_HW_INT_ENABLE_ADDR,
			PMC_GLOBAL_PPU1_HW_INT_GICP_IRQ_MASK);
	XPlmi_Out32(PMC_GLOBAL_PPU1_HW_INT_ENABLE_ADDR,
			PMC_GLOBAL_PPU1_HW_INT_MB_DATA_MASK);
	XPlmi_Out32(PMC_GLOBAL_PPU1_HW_INT_ENABLE_ADDR,
			PMC_GLOBAL_PPU1_HW_INT_MB_INSTR_MASK);
	XPlmi_Out32(PMC_GLOBAL_PPU1_PL_INT_ENABLE_ADDR,
			PMC_GLOBAL_PPU1_PL_INT_GPI_MASK);
}

/*****************************************************************************/
/**
* @brief	This function provides the Iro Frequency used in ROM
*
* @return	RomIroFreq value
*
*****************************************************************************/
u32 XPlmi_GetRomIroFreq(void)
{
	return RomIroFreq;
}


/*****************************************************************************/
/**
* @brief	This function provides check if its hp part
*
* @return	- TRUE
*		- FALSE
*
*****************************************************************************/
static u32 XPlmi_IsHpPart(void) {

	volatile u32 RawVoltage;
	u32 IsHpPart = (u32)FALSE;

	RawVoltage = Xil_In32(XPLMI_SYSMON_SUPPLY0_ADDR);
	RawVoltage &= XPLMI_SYSMON_SUPPLYX_MASK;

	if (RawVoltage >= XPlmi_GetRawVoltage(XPLMI_VCC_PMC_HP_MIN)) {
		IsHpPart = (u32)TRUE;
	}

	return IsHpPart;
}

/*****************************************************************************/
/**
* @brief	This functions sets the PMC IRO frequency.
*
* @return	- XST_SUCCESS on success
*		- XST_FAILURE on failure
*
*****************************************************************************/
int XPlmi_SetPmcIroFreq(void)
{
	int Status = XST_FAILURE;
	u32 *PmcIroFreq = XPlmi_GetPmcIroFreq();

	/** Set PMC IRO Frequency to the value used during ROM phase */
	*PmcIroFreq = XPLMI_PMC_IRO_FREQ_233_MHZ;
	if ((XPlmi_In32(EFUSE_CTRL_ANLG_OSC_SW_1LP) == XPLMI_EFUSE_IRO_TRIM_FAST)) {
		RomIroFreq = XPLMI_PMC_IRO_FREQ_400_MHZ;
		*PmcIroFreq = XPLMI_PMC_IRO_FREQ_400_MHZ;
		goto END;
	}
	/** Set PMC IRO frequency to be used during PLM phase */
	/** Update IR0 frequency to 400MHz for HP parts */
	/** Added redundacy to make it single glitch immune */
	if((XPlmi_IsHpPart() == (u32)TRUE) && (XPlmi_IsHpPart() == (u32)TRUE)) {
		XPlmi_Out32(EFUSE_CTRL_WR_LOCK, XPLMI_EFUSE_CTRL_UNLOCK_VAL);
		*PmcIroFreq = XPLMI_PMC_IRO_FREQ_400_MHZ;
		XPlmi_Out32(EFUSE_CTRL_ANLG_OSC_SW_1LP,
			XPLMI_EFUSE_IRO_TRIM_FAST);
		XPlmi_Out32(EFUSE_CTRL_WR_LOCK, XPLMI_EFUSE_CTRL_LOCK_VAL);
	}

END:
	/** Update PPU1 MB frequency used in BSP for timing calculations */
	Status = (int)Xil_SetMBFrequency(*PmcIroFreq);
	return Status;
}

/*****************************************************************************/
/**
* @brief	It stops the PIT timers.
*
* @param	Timer argument to select. This can be OR of all three timers too.
*
*****************************************************************************/
static void XPlmi_StopTimer(u8 Timer)
{
	XIOModule *IOModule = XPlmi_GetIOModuleInst();

	XIOModule_Timer_Stop(IOModule, Timer);
}

/****************************************************************************/
/**
* @brief    This function will disable and clear the IOmodule interrupts
*
* @return   None
*
****************************************************************************/
static void XPlmi_DisableClearIOmodule(void)
{
	XIOModule *IOModule = XPlmi_GetIOModuleInst();

	XIomodule_Out32(IOModule->BaseAddress + XIN_IER_OFFSET, 0U);
	XIomodule_Out32(IOModule->BaseAddress + XIN_IAR_OFFSET, 0xFFFFFFFFU);
}

/*****************************************************************************/
/**
* @brief	This function returns the current enabled interrupt mask of
* 			IOmodule.
*
* @return	Current interrupt enable mask
*
*****************************************************************************/
static u32 XPlmi_GetIoIntrMask(void)
{
	XIOModule *IOModule = XPlmi_GetIOModuleInst();

	return (IOModule->CurrentIER);
}

/*****************************************************************************/
/**
* @brief	This function enables the IOModule interrupts with provided value.
*
* @param	Value to be written to the IER register of IOModule
*
* @return	None.
*
*****************************************************************************/
static void XPlmi_SetIoIntrMask(u32 Value)
{
	XIOModule *IOModule = XPlmi_GetIOModuleInst();

	XPlmi_Out32(IOModule->BaseAddress + XIN_IER_OFFSET, Value);
}
/*****************************************************************************/
/**
* @brief	This functions provides the PIT1 and PIT2 reset values
*
* @return
* 			- XST_SUCCESS if success.
* 			- XPLMI_ERR_IOMOD_INIT if IOModule driver look up fails.
*
*****************************************************************************/
int XPlmi_GetPitResetValues(u32 *Pit1ResetValue, u32 *Pit2ResetValue)
{
	int Status = XST_FAILURE;
	XIOModule_Config *CfgPtr;

	if (XPlmi_IsPlmUpdateDone() == (u8)TRUE) {
		CfgPtr = XIOModule_LookupConfig(IOMODULE_DEVICE);
		if (CfgPtr == NULL) {
			Status = XPlmi_UpdateStatus(XPLMI_ERR_IOMOD_INIT, 0U);
			goto END;
		}

		*Pit2ResetValue = XPlmi_In32(CfgPtr->BaseAddress + XTC_TCR_OFFSET +
					XTC_TIMER_COUNTER_OFFSET);
		*Pit1ResetValue = XPlmi_In32(CfgPtr->BaseAddress + XTC_TCR_OFFSET);
	}
	else {
		*Pit2ResetValue = XPLMI_PIT2_RESET_VALUE;
		*Pit1ResetValue = XPLMI_PIT1_RESET_VALUE;
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
* @brief	This function is used to check if the given address range is
* valid. This function can be called before loading any elf or assigning any
* buffer in that address range
*
* @param	StartAddr is the starting address
* @param	EndAddr is the ending address
*
* @return	XST_SUCCESS on success and error code on failure
*
*****************************************************************************/
int XPlmi_VerifyAddrRange(u64 StartAddr, u64 EndAddr)
{
	int Status = XST_FAILURE;

	if (EndAddr < StartAddr) {
		Status = XPLMI_ERROR_INVALID_ADDRESS;
		goto END;
	}

	if (XPlmi_IsLpdInitialized() == (u8)TRUE) {
		if ((StartAddr >= (u64)XPLMI_PSM_RAM_BASE_ADDR) &&
			(EndAddr <= (u64)XPLMI_PSM_RAM_HIGH_ADDR)) {
			/* PSM RAM is valid */
			Status = XST_SUCCESS;
		}
		else if ((StartAddr >= (u64)XPLMI_TCM0_BASE_ADDR) &&
			(EndAddr <= (u64)XPLMI_TCM0_HIGH_ADDR)) {
			/* TCM0 is valid */
			Status = XST_SUCCESS;
		}
		else if ((StartAddr >= (u64)XPLMI_TCM1_BASE_ADDR) &&
			(EndAddr <= (u64)XPLMI_TCM1_HIGH_ADDR)) {
			/* TCM1 is valid */
			Status = XST_SUCCESS;
		}
		else if ((StartAddr >= (u64)XPLMI_OCM_BASE_ADDR) &&
			(EndAddr <= (u64)XPLMI_OCM_HIGH_ADDR)) {
			/* OCM is valid */
			Status = XST_SUCCESS;
		}
		else {
			/* Rest of the Addr range is treated as invalid */
			Status = XPLMI_ERROR_INVALID_ADDRESS;
		}
	} else {
			Status = XPLMI_ERROR_LPD_NOT_INITIALIZED;
	}

	if ((EndAddr <= (u64)XPLMI_OCM_HIGH_ADDR) ||
		(StartAddr > (u64)XPLMI_4GB_END_ADDR)) {
		if ((StartAddr >= (u64)XPLMI_RSVD_BASE_ADDR) &&
			(EndAddr <= (u64)XPLMI_RSVD_HIGH_ADDR)) {
			Status = XPLMI_ERROR_INVALID_ADDRESS;
		}
		else {
			/* Addr range less than OCM high addr or greater
				than 2GB is considered valid */
			Status = XST_SUCCESS;
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to check and wait for DMA done
 *
 * @param	DestAddr is the address of destination
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
XPlmi_WaitForDmaDone_t XPlmi_GetPlmiWaitForDone(u64 DestAddr)
{
	(void)DestAddr;

	return (XPlmi_WaitForDmaDone_t)XPmcDma_WaitForDone;
}

/*****************************************************************************/
/**
 * @brief	This function provides Gic interrupt id
 *
 * @param	GicPVal indicates GICP source
 * @param	GicPxVal indicates GICPx source

 * @return	Readback length
 *
 *****************************************************************************/
u32 XPlmi_GetGicIntrId(u32 GicPVal, u32 GicPxVal)
{
	u32 IntrId;

	IntrId = (GicPVal << XPLMI_GICP_INDEX_SHIFT) |
			(GicPxVal << XPLMI_GICPX_INDEX_SHIFT);
	IntrId |= (XPLMI_HW_INT_GIC_IRQ << XPLMI_HW_SW_INTR_SHIFT);

	return IntrId | XPLMI_IOMODULE_PPU1_HW_INT;
}

/*****************************************************************************/
/**
 * @brief	This function provides IPI interrupt id
 *
 * @param	BufferIndex is the Ipi target buffer index

 * @return	IPI interrupt id
 *
 *****************************************************************************/
u32 XPlmi_GetIpiIntrId(u32 BufferIndex)
{
	return XPLMI_IPI_INTR_ID | (BufferIndex << XPLMI_IPI_INDEX_SHIFT);
}

/*****************************************************************************/
/**
 * @brief	This function raises an interrupt request to ROM and waits for
 * 			completion. Before calling this API all pre-requsites for ROM service shall
 * 			be completed.
 *
 * @param	RomServiceReq variable of enum type XPlmi_RomIntr
 *
 * @return
 * 			- XST_SUCCESS if success.
 * 			- XPLMI_ERR_INVALID_ROM_INT_REQ on invalid interrupt request for ROM.
 *
 ******************************************************************************/
int XPlmi_RomISR(XPlmi_RomIntr RomServiceReq)
{
	int Status = XST_FAILURE;
	u32 IntrMask;
	u32 IoMask;

	if ((RomServiceReq >= XPLMI_INVALID_INT) ||
		(RomServiceReq == XPLMI_PLM_UPDT_REQ)) {
		Status = XPLMI_ERR_INVALID_ROM_INT_REQ;
		goto END;
	}
	IntrMask = (u32)1 << RomServiceReq;
	XPlmi_Out32(PMC_GLOBAL_ROM_INT_REASON, IntrMask);
	/* Generate ROM interrupt */
	XPlmi_Out32(PMC_GLOBAL_ROM_INT, IntrMask);

	/* For DME request keeping Microblaze into sleep state */
	if (RomServiceReq == XPLMI_DME_CHL_SIGN_GEN) {
		/* Disable Interrupts */
		microblaze_disable_interrupts();
		/* Storing current interrupt enable mask of IOModule's IER */
		IoMask = XPlmi_GetIoIntrMask();
		XPlmi_DisableClearIOmodule();
		mb_sleep();
	}
	Status = (int)Xil_WaitForEvent((UINTPTR)PMC_GLOBAL_ROM_INT_REASON,
		IntrMask, IntrMask, XPLMI_ROM_SERVICE_TIMEOUT);

	if (RomServiceReq == XPLMI_DME_CHL_SIGN_GEN) {
		XPlmi_SetIoIntrMask(IoMask);
		microblaze_enable_interrupts();
		XPlmi_PpuWakeUpDis();
	}

	XPlmi_Out32(PMC_GLOBAL_ROM_INT_REASON, IntrMask);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function return FIPS mode.
 *
 * @return
 *			TRUE  If FIPS mode is enabled
 *			FALSE If FIPS mode is disabled
 *
 *****************************************************************************/
u8 XPlmi_IsFipsModeEn(void)
{
	u8 FipsModeEn = TRUE;

	FipsModeEn = (u8)((XPlmi_In32(EFUSE_CACHE_DME_FIPS_CTRL) & EFUSE_CACHE_DME_FIPS_MODE_MASK) >>
					XPLMI_EFUSE_FIPS_MODE_SHIFT);

	return FipsModeEn;
}

/*****************************************************************************/
/**
 * @brief	This function returns ROM KAT status.
 *
 * @return	ROM KAT status
 *
 *****************************************************************************/
u32 XPlmi_GetRomKatStatus(void)
{
	return (XPlmi_In32(XPLMI_RTCFG_SECURE_STATE_ADDR) & XPLMI_ROM_KAT_MASK);
}

/*****************************************************************************/
/**
 * @brief	This function sets KAT status from RTC area.
 *
 * @param	PlmKatStatus is the pointer to the variable which holds kat status
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_GetBootKatStatus(volatile u32 *PlmKatStatus)
{
	volatile u8 CryptoKatEn = TRUE;
	volatile u8 CryptoKatEnTmp = TRUE;
	volatile u8 FipsModeEn = TRUE;
	volatile u8 FipsModeEnTmp = TRUE;

	CryptoKatEn = XPlmi_IsCryptoKatEn();
	CryptoKatEnTmp = XPlmi_IsCryptoKatEn();
	if((CryptoKatEn == TRUE) || (CryptoKatEnTmp == TRUE)) {
		*PlmKatStatus = XPlmi_GetKatStatus();
		FipsModeEn = XPlmi_IsFipsModeEn();
		FipsModeEnTmp = XPlmi_IsFipsModeEn();
		if ((FipsModeEn != TRUE) && (FipsModeEnTmp != TRUE)) {
			*PlmKatStatus |= XPlmi_GetRomKatStatus();
			XPlmi_UpdateKatStatus(*PlmKatStatus);
		}
	} else {
		*PlmKatStatus = XPLMI_KAT_MASK;
	}
}

/*****************************************************************************/
/**
 * @brief	This function clears SSS Cfg error set during ROM PCR Extension
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_ClearSSSCfgErr(void)
{
	static u32 SSSCfgErrCleared = (u32)FALSE;
	u32 PmcIsr;

	if ((SSSCfgErrCleared != TRUE) && (XPlmi_IsPlmUpdateDone() != TRUE)) {
		PmcIsr = XPlmi_In32(PMC_GLOBAL_ISR);
		if ((PmcIsr & PMC_GLOBAL_SSS_CFG_ERR_MASK) ==
				PMC_GLOBAL_SSS_CFG_ERR_MASK) {
			XPlmi_Out32(PMC_GLOBAL_ISR, PMC_GLOBAL_SSS_CFG_ERR_MASK);
			SSSCfgErrCleared = (u32)TRUE;
		}
	}
}

/*****************************************************************************/
/**
 * @brief	This function returns XPlmi_FipsKatMask instance.
 *
 * @return	pointer to the XPlmi_FipsKatMask instance
 *
 *****************************************************************************/
XPlmi_FipsKatMask* XPlmi_GetFipsKatMaskInstance(void)
{
	static XPlmi_FipsKatMask FipsKatMask = {0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU,
					0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU};

	return &FipsKatMask;
}

/*****************************************************************************/
/**
 * @brief	This function monitors the KAT status and updates the FIPS state in
 *          RTCA.
 *
 * @return
 * 			- XST_SUCCESS on success
 * 			- XST_FAILURE on failure
 *
 *****************************************************************************/
static int XPlmi_UpdateFipsState(void)
{
	int Status = XST_FAILURE;
	XPlmi_FipsKatMask *FipsKatMask = XPlmi_GetFipsKatMaskInstance();
	u32 RomKatStatus = XPlmi_GetRomKatStatus();
	u32 PlmKatStatus = XPlmi_GetKatStatus();
	u32 DDRKatStatus = XPlmi_In32(XPLMI_RTCFG_SECURE_DDR_KAT_ADDR);
	u32 HnicCpm5nPcideKatStatus = XPlmi_In32(XPLMI_RTCFG_SECURE_HNIC_CPM5N_PCIDE_KAT_ADDR);
	u32 PKI0KatStatus = XPlmi_In32(XPLMI_RTCFG_SECURE_PKI_KAT_ADDR_0);
	u32 PKI1KatStatus = XPlmi_In32(XPLMI_RTCFG_SECURE_PKI_KAT_ADDR_1);
	u32 PKI2KatStatus = XPlmi_In32(XPLMI_RTCFG_SECURE_PKI_KAT_ADDR_2);

	if (((FipsKatMask->RomKatMask & RomKatStatus) == FipsKatMask->RomKatMask) &&
		((FipsKatMask->PlmKatMask & PlmKatStatus) == FipsKatMask->PlmKatMask) &&
		((FipsKatMask->DDRKatMask & DDRKatStatus) == FipsKatMask->DDRKatMask) &&
		((FipsKatMask->HnicCpm5NPcideKatMask & HnicCpm5nPcideKatStatus) == FipsKatMask->HnicCpm5NPcideKatMask) &&
		((FipsKatMask->PKI0KatMask & PKI0KatStatus) == FipsKatMask->PKI0KatMask) &&
		((FipsKatMask->PKI1KatMask & PKI1KatStatus) == FipsKatMask->PKI1KatMask) &&
		((FipsKatMask->PKI2KatMask & PKI2KatStatus) == FipsKatMask->PKI2KatMask)) {
		Status = Xil_SecureRMW32(XPLMI_RTCFG_PLM_KAT_ADDR, XPLMI_SECURE_FIPS_STATE_MASK, XPLMI_SECURE_FIPS_STATE_MASK);
	}
	else {
		Status = Xil_SecureRMW32(XPLMI_RTCFG_PLM_KAT_ADDR, XPLMI_SECURE_FIPS_STATE_MASK, 0x0U);
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function checks and updates the FIPS state in RTCA
 *
 * @return
 * 			- XST_SUCCESS on success
 * 			- XST_FAILURE on failure
 *
 *****************************************************************************/
int XPlmi_CheckAndUpdateFipsState(void)
{
	int Status = XST_FAILURE;
	u8 FipsModeEn = XPlmi_IsFipsModeEn();
	u8 FipsModeEnTmp = XPlmi_IsFipsModeEn();

	if ((FipsModeEn == TRUE) || (FipsModeEnTmp == TRUE)) {
		Status = XPlmi_UpdateFipsState();
	}
	else {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sets or clears crypto bit in RTCA based on mask
 *
 * @param	Mask Mask to set or clear crypto status
 * @param   Val  Value to set or clear crypto status
 *
 *****************************************************************************/
void XPlmi_UpdateCryptoStatus(u32 Mask, u32 Val)
{
	XPlmi_UtilRMW(XPLMI_RTCFG_PLM_CRYPTO_STATUS_ADDR, Mask, Val);
}

/*****************************************************************************/
/**
 * @brief	This function returns crypto status flag
 *
 * @param	Mask Mask to read crypto status
 *
 * @return  Crypto status value
 *
 *****************************************************************************/
u32 XPlmi_GetCryptoStatus(u32 Mask)
{
	return (XPlmi_In32(XPLMI_RTCFG_PLM_CRYPTO_STATUS_ADDR) & Mask);
}

/*****************************************************************************/
/**
 * @brief	This function will return KAT status of given mask.
 *
 * @param	PlmKatMask contains the KAT mask
 *
 * @return
 *			TRUE  If KAT ran
 *			FALSE If KAT didn't ran
 *
 *****************************************************************************/
u8 XPlmi_IsKatRan(u32 PlmKatMask)
{
	volatile u8 CryptoKatEn = XPlmi_IsCryptoKatEn();
	volatile u8 CryptoKatEnTmp = XPlmi_IsCryptoKatEn();
	u8 IsKatRan = FALSE;

	if ((CryptoKatEn == TRUE) || (CryptoKatEnTmp == TRUE)) {
		IsKatRan = (((XPlmi_In32(XPLMI_RTCFG_PLM_KAT_ADDR) & PlmKatMask) != 0U)?
					(u8)TRUE: (u8)FALSE);
	}
	else {
		IsKatRan = TRUE;
	}

	return IsKatRan;
}
