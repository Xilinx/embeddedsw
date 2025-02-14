/******************************************************************************
 * Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xplm_init.c
 *
 * This file contains the PLM initialization related code.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  ng   05/31/24 Initial release
 * 1.01  ng   11/05/24 Add boot time measurements
 *       ng   02/05/25 Update SDK release year and quarter
 *       sk   02/04/25 Updated the XPlm_ExceptionHandler function
 *                     brief
 * </pre>
 *
 ******************************************************************************/

/**
 * @addtogroup spartanup_plm_apis SpartanUP PLM APIs
 * @{
 */

/***************************** Include Files *********************************/
#include "xplm_init.h"
#include "xplm_debug.h"
#include "xplm_error.h"
#include "xplm_hooks.h"
#include "xplm_load.h"
#include "xil_io.h"
#include "xil_types.h"
#include "xplm_qspi.h"
#include "xplm_dma.h"
#include "xplm_generic.h"
#include "xplm_ospi.h"
#include "xplm_status.h"
#include "xplm_util.h"
#include "xstatus.h"
#include "xplm_hw.h"
#include "xiomodule.h"

/************************** Constant Definitions *****************************/
/** @cond spartanup_plm_internal */
#define SDK_RELEASE_YEAR	"2025"
#define SDK_RELEASE_QUARTER	"1"

#define XPLM_TAP_INST_0_UNLOCK_MASK	(PMC_TAP_INST_MASK_0_JRDBK_MASK | PMC_TAP_INST_MASK_0_USER1_MASK | PMC_TAP_INST_MASK_0_USER2_MASK)
#define XPLM_TAP_INST_1_UNLOCK_MASK	(PMC_TAP_INST_MASK_1_USER3_MASK | PMC_TAP_INST_MASK_1_USER4_MASK)

#define XPLM_PMC_GLOBAL_FW_ERR_CR_NCR_MASK	(PMC_GLOBAL_PMCL_EAM_ERR_OUT1_EN_PMC_FW_NCR_ERR_MASK | PMC_GLOBAL_PMCL_EAM_ERR_OUT1_EN_PMC_FW_CR_ERR_MASK)
/** @endcond */

#define XPLM_PMC_IRO_FREQ_HZ		((u32)510000000U)
#define XPLM_PMC_IRO_FREQ_MHZ		(XPLM_PMC_IRO_FREQ_HZ / XPLM_MEGA)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void XPlm_ExceptionHandler(void *Data);

/************************** Variable Definitions *****************************/
/** Pointer to the Hooks table in PMC RAM. */
XRom_HooksTbl *HooksTbl = (XRom_HooksTbl *)XROM_HOOKS_TBL_BASE_ADDR;

/*****************************************************************************/
/**
 * @brief This function enables the exceptions and interrupts
 * Enable interrupts from the hardware
 *
 *****************************************************************************/
static void XPlm_ExceptionInit(void)
{
	u32 Status = (u32)XST_FAILURE;

	Xil_ExceptionDisable();
	Xil_ExceptionInit();
	/**
	 * - Register the @ref XPlm_ExceptionHandler as exception handler for all types of
	 * exceptions and pass the exception ID masked with @ref XPLM_ERR_EXCEPTION as data to it.
	 */
	for (u32 Index = XIL_EXCEPTION_ID_FIRST; Index <= XIL_EXCEPTION_ID_LAST; Index++) {
		Status = (u32)XPLM_ERR_EXCEPTION | Index;
		Xil_ExceptionRegisterHandler(Index, (Xil_ExceptionHandler)XPlm_ExceptionHandler, (void *)Status);
	}

	Xil_ExceptionEnable();
}

/*****************************************************************************/
/**
 * @brief This is a function handler for all exceptions.
 *
 * @param	Data Pointer to Error Status that needs to be updated in
 * Error Register. Status is initialized during exception initialization
 * having Index and exception error code.
 *
 *****************************************************************************/
static void XPlm_ExceptionHandler(void *Data)
{
	XPlm_Printf(DEBUG_PRINT_ALWAYS, "Received Exception\r\n mcause: 0x%08x\r\n", csrr(mcause));

	/** - Log the PLM error with the received input param "Data" as error and loop infinitely */
	XPlm_LogPlmErr((u32)Data);

	while (TRUE) {
		;
	}
}

/** @cond spartanup_plm_internal */
/*****************************************************************************/
/**
 * @brief	This function is used as handler to print the file name and line number during
 * assertions.
 *
 * @param	File	is the pointer to the string containing the file name
 * @param	Line	is the line number of the file
 *
 *****************************************************************************/
static void XPlm_PrintAssertInfo(const char8 *File, s32 Line)
{
	XPlm_Printf(DEBUG_INFO, "Assert : %s - L%d\n\r", File, Line);
	XPlm_LogPlmErr(XPLM_ERR_ASSERT);
}
/** @endcond */

/*****************************************************************************/
/**
 * @brief	Initialize the RTCA memory with default values.
 *
 *****************************************************************************/
static void XPlm_RtcaInit(void)
{
	/** - Initialize the RTCA registers to zero if not initialized by Reginit */
	for (u32 Addr = XPLM_RTCFG_BASEADDR; Addr < (XPLM_RTCFG_BASEADDR + XPLM_RTCFG_LENGTH_BYTES);
	     Addr += XPLM_WORD_LEN) {
		if (Xil_In32(Addr) == XPLM_REG_RTCA_RESET_VAL) {
			Xil_Out32(Addr, XPLM_ZERO);
		}
	}

	/** - Initialize Identification string, version and size with default values. */
	Xil_Out32(XPLM_RTCFG_ID_STRING_ADDR, XPLM_RTCFG_IDENTIFICATION);
	Xil_Out32(XPLM_RTCFG_VERSION_ADDR, XPLM_RTCFG_VER);
	Xil_Out32(XPLM_RTCFG_SIZE_ADDR, XPLM_RTCFG_SIZE);
}

/** @cond spartanup_plm_internal */
/*****************************************************************************/
/**
 * @brief	Print the PLM banner.
 *
 *****************************************************************************/
static void XPlm_PrintBanner(void)
{
	u32 BootMode = Xil_In32(PMC_GLOBAL_BOOT_MODE_USER) & PMC_GLOBAL_BOOT_MODE_USER_MASK;
	u32 MultiBoot = Xil_In32(PMC_GLOBAL_MULTI_BOOT);

	XPlm_Printf(DEBUG_PRINT_ALWAYS, "****************************************\n\r");
	XPlm_Printf(DEBUG_PRINT_ALWAYS, "Xilinx Spartan Ultrascale+ PLM\n\r");
	XPlm_Printf(DEBUG_PRINT_ALWAYS, "Release %s.%s   %s  -  %s\n\r", SDK_RELEASE_YEAR,
		    SDK_RELEASE_QUARTER, __DATE__, __TIME__);
	XPlm_Printf(DEBUG_PRINT_ALWAYS, "BOOTMODE: 0x%x, MULTIBOOT: 0x%x" "\n\r", BootMode, MultiBoot);
	XPlm_Printf(DEBUG_PRINT_ALWAYS, "****************************************\n\r");
}
/** @endcond */

/*****************************************************************************/
/**
 * @brief	This function returns pointer to IO module instance.
 *
 * @return	Pointer to IO Module instance.
 *
 *****************************************************************************/
XIOModule *XPlm_GetIOModuleInst(void)
{
	static XIOModule IOModule;
	return &IOModule;
}

/*****************************************************************************/
/**
 * @brief	This functions returns the operating frequency of PMCL.
 *
 * @return
 * 		- IRO frequency of PMC in Hz.
 *
 *****************************************************************************/
u32 XPlm_PmcIroFreq(void)
{
	u32 IroClkDiv;

	IroClkDiv = Xil_In32(PMC_GLOBAL_PMCL_MAIN_IRO_CLK_CTRL);
	IroClkDiv &= PMC_GLOBAL_PMCL_MAIN_IRO_CLK_CTRL_DIVISOR_MASK;
	IroClkDiv >>= PMC_GLOBAL_PMCL_MAIN_IRO_CLK_CTRL_DIVISOR_SHIFT;

	return (XPLM_PMC_IRO_FREQ_HZ / IroClkDiv);
}

/*****************************************************************************/
/**
 * @brief	This function captures the 32-bit timer count from both PIT1 & PIT2 and
 * returns the timer count as 64-bit value.
 *
 * @return	Returns 64 bit timer value
 *
 ******************************************************************************/
u64 XPlm_GetTimerValue(void)
{
	u64 TimerValue;
	u64 TPit1;
	u32 TPit2;
	XIOModule *IOModule = XPlm_GetIOModuleInst();

	TPit1 = XIOModule_GetValue(IOModule, (u8)XPLM_PIT1);
	TPit2 = XIOModule_GetValue(IOModule, (u8)XPLM_PIT2);

	/*
	 * Pit1 starts at 0 and preload the full value
	 * after pit2 expires. So, recasting TPit1 0 value
	 * to highest so that u64 comparison works for
	 * Tpit1 0 and TPit1 0xfffffffe
	 */
	if (TPit1 == 0U) {
		TPit1 = XPLM_PIT1_CYCLE_VALUE;
	}

	TimerValue = (TPit1 << 32U) | (u64)TPit2;

	return TimerValue;
}

/*****************************************************************************/
/**
 * @brief	This function prints the total time taken between two points for performance
 * measurement.
 *
 * @param	TCur is the current time
 * @param	TStart is the start time
 * @param	IroFreq is the frequency at which PMC IRO is running in MHz
 * @param	PerfTime is the pointer to variable holding the performance time
 *
 *****************************************************************************/
void XPlm_GetPerfTime(u64 TCur, u64 TStart, u32 IroFreq, XPlm_PerfTime *PerfTime)
{
	u64 PerfUs;
	u64 TDiff = TCur - TStart;
	u32 PmcIroFreqMHz = IroFreq / XPLM_MEGA;

	/* Convert TPerf into microseconds */
	PerfUs = TDiff / (u64)PmcIroFreqMHz;
	PerfTime->TPerfMsFrac = PerfUs % (u64)XPLM_KILO;
	PerfTime->TPerfMs = PerfUs / (u64)XPLM_KILO;
}

/*****************************************************************************/
/**
 * @brief	This function prints the ROM time.
 *
 *****************************************************************************/
void XPlm_PrintRomTime(void)
{
	u64 PmcRomTime;
	XPlm_PerfTime PerfTime;

	/* Get PMC ROM time */
	PmcRomTime = (u64)Xil_In32(PMC_GLOBAL_GLOBAL_GEN_STORAGE0);
	PmcRomTime |= (u64)Xil_In32(PMC_GLOBAL_GLOBAL_GEN_STORAGE1) << 32U;

	/* Print time stamp of PLM */
	XPlm_GetPerfTime(((u64)XPLM_PIT1_CYCLE_VALUE << 32U) | XPLM_PIT2_CYCLE_VALUE, PmcRomTime,
			 XPlm_PmcIroFreq(), &PerfTime);
	XPlm_Printf(DEBUG_PRINT_ALWAYS, "%u.%03u ms: ROM Time\r\n",
		(u32)PerfTime.TPerfMs, (u32)PerfTime.TPerfMsFrac);
}

/*****************************************************************************/
/**
 * @brief	This function prints the PLM time.
 *
 *****************************************************************************/
void XPlm_PrintPlmTime(void)
{
	u64 PmcRomTime;
	XPlm_PerfTime PerfTime;
	u32 TPit1;
	u32 TPit2;
	u64 TCur;

	TPit1 = Xil_In32(RV_IOMODULE_PIT1_COUNTER);
	TPit2 = Xil_In32(RV_IOMODULE_PIT2_COUNTER);

	/* Get PMC ROM time */
	PmcRomTime = (u64)Xil_In32(PMC_GLOBAL_GLOBAL_GEN_STORAGE0);
	PmcRomTime |= (u64)Xil_In32(PMC_GLOBAL_GLOBAL_GEN_STORAGE1) << 32U;

	Xil_Out32(PMC_GLOBAL_GLOBAL_GEN_STORAGE2, TPit2);
	Xil_Out32(PMC_GLOBAL_GLOBAL_GEN_STORAGE3, TPit1);

	if (TPit1 == XPLM_ZERO) {
		TPit1 = XPLM_PIT1_CYCLE_VALUE;
	}

	TCur = ((u64)TPit1 << 32U) | TPit2;

	/* Print time stamp of PLM */
	XPlm_GetPerfTime(PmcRomTime, TCur, XPlm_PmcIroFreq(), &PerfTime);
	XPlm_Printf(DEBUG_PRINT_ALWAYS, "%u.%03u ms: PLM Time\r\n",
		(u32)PerfTime.TPerfMs, (u32)PerfTime.TPerfMsFrac);
}

/*****************************************************************************/
/**
 * @brief	Perform Pre-Boot initialization tasks for Exceptions, RTCA,
 * Log buffer, and CDO commands handlers.
 *
 * @returns
 *		- Errors from @ref XPlm_Status_t.
 *
 *****************************************************************************/
u32 XPlm_Init(void)
{
	u32 Status = (u32)XST_FAILURE;
	u32 EamErr1Status;
	XRomBootRom *InstancePtr = HooksTbl->InstancePtr;
	XRomTmpVar *TmpInstancePtr = HooksTbl->XRom_GetTemporalInstance();

	XPlm_LogPlmStage(XPLM_PRE_BOOT_INIT_STAGE);

	/** - Enable EAM error outs for PMC FW CR and NCR errors. */
	XPlm_UtilRMW(PMC_GLOBAL_PMCL_EAM_ERR_OUT1_EN, XPLM_PMC_GLOBAL_FW_ERR_CR_NCR_MASK,
		     XPLM_PMC_GLOBAL_FW_ERR_CR_NCR_MASK);

	/** - Initialize exceptions. */
	XPlm_ExceptionInit();
	Xil_AssertSetCallback(XPlm_PrintAssertInfo);

	/** - Initialize RTCA memory. */
	XPlm_RtcaInit();

	/** - Initialize Log buffer to store PLM prints. */
	XPlm_InitDebugLogBuffer();
	XPlm_PrintBanner();

	/** - Store PMCL_EAM_ERR1_STATUS to RTCA and clear it. */
	EamErr1Status = Xil_In32(PMC_GLOBAL_PMCL_EAM_ERR1_STATUS);
	Xil_Out32(XPLM_RTCFG_EAM_ERR1_STATUS, EamErr1Status);
	Xil_Out32(PMC_GLOBAL_PMCL_EAM_ERR1_STATUS, PMC_GLOBAL_PMCL_EAM_ERR1_STATUS_FULLMASK);

	/** - Initialize DMA drivers. */
	Status = XPlm_DmaInit();
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/** - Configure handlers for CDO commands. */
	XPlm_GenericInit();

	if ((InstancePtr->AuthEnabled != XPLM_ZERO) || (TmpInstancePtr->AuthEnabledTmp != XPLM_ZERO) ||
	    (InstancePtr->Encstatus != XPLM_ZERO) || (TmpInstancePtr->EncstatusTmp != XPLM_ZERO)) {
		/** - Disable PL readback in secure boot. */
		XSECURE_REDUNDANT_IMPL(Xil_Out32, SLAVE_BOOT_SBI_RDBK, SLAVE_BOOT_SBI_RDBK_DIS_MASK);
		/** - Disable/Mask MONITOR_DRP TAP instruction in secure boot. */
		XSECURE_REDUNDANT_IMPL(XPlm_UtilRMW, PMC_TAP_INST_MASK_0, PMC_TAP_INST_MASK_0_MONITOR_DRP_MASK,
				       PMC_TAP_INST_MASK_0_MONITOR_DRP_MASK);
	}

	/** - Unmask JRDBK, and USER 1/2/3/4 TAP instructions. */
	XPlm_UtilRMW(PMC_TAP_INST_MASK_0, XPLM_TAP_INST_0_UNLOCK_MASK, XPLM_ZERO);
	XPlm_UtilRMW(PMC_TAP_INST_MASK_1, XPLM_TAP_INST_1_UNLOCK_MASK, XPLM_ZERO);

END:
	return Status;
}

/** @} end of spartanup_plm_apis group*/
