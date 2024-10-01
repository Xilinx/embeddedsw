/******************************************************************************
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xplm_proc.c
 *
 * This file contains the PLM initialization related code.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  ng   05/31/24 Initial release
 * </pre>
 *
 ******************************************************************************/

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
#define SDK_RELEASE_YEAR	"2024"
#define SDK_RELEASE_QUARTER	"2"

#define XPLM_TAP_INST_0_UNLOCK_MASK	(PMC_TAP_INST_MASK_0_JRDBK_MASK |\
					PMC_TAP_INST_MASK_0_USER1_MASK |\
					PMC_TAP_INST_MASK_0_USER2_MASK)
#define XPLM_TAP_INST_1_UNLOCK_MASK	(PMC_TAP_INST_MASK_1_USER3_MASK |\
					PMC_TAP_INST_MASK_1_USER4_MASK)

#define XPLM_PMC_GLOBAL_FW_ERR_CR_NCR_MASK \
		(PMC_GLOBAL_PMCL_EAM_ERR_OUT1_EN_PMC_FW_NCR_ERR_MASK \
		| PMC_GLOBAL_PMCL_EAM_ERR_OUT1_EN_PMC_FW_CR_ERR_MASK)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void XPlm_ExceptionHandler(void *Data);

/************************** Variable Definitions *****************************/
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
	/** Register exception handlers */
	for (u32 Index = XIL_EXCEPTION_ID_FIRST; Index <= XIL_EXCEPTION_ID_LAST; Index++) {
		Status = (u32)XPLM_ERR_EXCEPTION | Index;
		Xil_ExceptionRegisterHandler(Index,
			(Xil_ExceptionHandler)XPlm_ExceptionHandler,
			(void *)Status);
	}

	Xil_ExceptionEnable();
}

/*****************************************************************************/
/**
 * @brief This is a function handler for all exceptions. It clears security
 * critical data by clearing AES keys and by placing SHA3 in reset.
 *
 * @param	Data Pointer to Error Status that needs to be updated in
 * Error Register. Status is initialized during exception initialization
 * having Index and exception error code.
 *
 *****************************************************************************/
static void XPlm_ExceptionHandler(void *Data) {
	// print mcause register, implement it in assembly
	XPlm_Printf(DEBUG_PRINT_ALWAYS, "Received Exception\r\n mcause: 0x%08x\r\n", csrr(mcause));

	XPlm_LogPlmErr((u32)Data);
	/* Just in case if control reaches here */
	while (TRUE) {
		;
	}
}

static void XPlm_PrintAssertInfo(const char8 *File, s32 Line) {
	XPlm_Printf(DEBUG_INFO, "Assert : %s - L%d\n\r", File, Line);
	XPlm_LogPlmErr(XPLM_ERR_ASSERT);
}

static void XPlm_RtcaInit(void){
	/** - Initialize the RTCA registers to zero if not initialized by Reginit */
	for (u32 Addr = XPLM_RTCFG_BASEADDR; Addr < (XPLM_RTCFG_BASEADDR + XPLM_RTCFG_LENGTH_BYTES);
			Addr += XPLM_WORD_LEN) {
		if(Xil_In32(Addr) == XPLM_REG_RTCA_RESET_VAL) {
			Xil_Out32(Addr, XPLM_ZERO);
		}
	}

	/** - Initialize Identification string, version and size with default values. */
	Xil_Out32(XPLM_RTCFG_ID_STRING_ADDR, XPLM_RTCFG_IDENTIFICATION);
	Xil_Out32(XPLM_RTCFG_VERSION_ADDR, XPLM_RTCFG_VER);
	Xil_Out32(XPLM_RTCFG_SIZE_ADDR, XPLM_RTCFG_SIZE);
}

static void XPlm_PrintBanner(void) {
	u32 BootMode = Xil_In32(PMC_GLOBAL_BOOT_MODE_USER) &
					PMC_GLOBAL_BOOT_MODE_USER_MASK;
	u32 MultiBoot = Xil_In32(PMC_GLOBAL_MULTI_BOOT);

	XPlm_Printf(DEBUG_PRINT_ALWAYS, "****************************************\n\r");
	XPlm_Printf(DEBUG_PRINT_ALWAYS, "Xilinx Spartan Ultrascale+ PLM\n\r");
	XPlm_Printf(DEBUG_PRINT_ALWAYS, "Release %s.%s   %s  -  %s\n\r",
		SDK_RELEASE_YEAR, SDK_RELEASE_QUARTER, __DATE__, __TIME__);
	XPlm_Printf(DEBUG_PRINT_ALWAYS, "BOOTMODE: 0x%x, MULTIBOOT: 0x%x"
				"\n\r",BootMode , MultiBoot);
	XPlm_Printf(DEBUG_PRINT_ALWAYS, "****************************************\n\r");
}

u32 XPlm_Init(void) {
	u32 Status = (u32)XST_FAILURE;
	u32 EamErr1Status;
	XRomBootRom *InstancePtr = HooksTbl->InstancePtr;
	XRomTmpVar *TmpInstancePtr = HooksTbl->XRom_GetTemporalInstance();

	XPlm_LogPlmStage(XPLM_PRE_BOOT_INIT_STAGE);

	XPlm_UtilRMW(PMC_GLOBAL_PMCL_EAM_ERR_OUT1_EN,
			XPLM_PMC_GLOBAL_FW_ERR_CR_NCR_MASK,
			XPLM_PMC_GLOBAL_FW_ERR_CR_NCR_MASK);
	XPlm_ExceptionInit();
	Xil_AssertSetCallback(XPlm_PrintAssertInfo);
	XPlm_RtcaInit();
	XPlm_InitDebugLogBuffer();
	XPlm_PrintBanner();

	/** - Store PMCL_EAM_ERR1_STATUS and clear it. */
	EamErr1Status = Xil_In32(PMC_GLOBAL_PMCL_EAM_ERR1_STATUS);
	Xil_Out32(XPLM_RTCFG_EAM_ERR1_STATUS, EamErr1Status);
	Xil_Out32(PMC_GLOBAL_PMCL_EAM_ERR1_STATUS, PMC_GLOBAL_PMCL_EAM_ERR1_STATUS_FULLMASK);

	Status = XPlm_DmaInit();
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/** - Initialize PLMI module. */
	XPlm_GenericInit();

	if ((InstancePtr->AuthEnabled != XPLM_ZERO) || (TmpInstancePtr->AuthEnabledTmp != XPLM_ZERO) ||
		(InstancePtr->Encstatus != XPLM_ZERO) || (TmpInstancePtr->EncstatusTmp != XPLM_ZERO)) {
		/** - Disable PL readback in secure boot. */
		XSECURE_REDUNDANT_IMPL(Xil_Out32, SLAVE_BOOT_SBI_RDBK,
				SLAVE_BOOT_SBI_RDBK_DIS_MASK);
		/** - Disable/Mask MONITOR_DRP TAP instruction in secure boot. */
		XSECURE_REDUNDANT_IMPL(XPlm_UtilRMW, PMC_TAP_INST_MASK_0,
				PMC_TAP_INST_MASK_0_MONITOR_DRP_MASK,
				PMC_TAP_INST_MASK_0_MONITOR_DRP_MASK);
	}

	/** - Unmask JRDBK, and USER 1/2/3/4 TAP instructions. */
	XPlm_UtilRMW(PMC_TAP_INST_MASK_0, XPLM_TAP_INST_0_UNLOCK_MASK, XPLM_ZERO);
	XPlm_UtilRMW(PMC_TAP_INST_MASK_1, XPLM_TAP_INST_1_UNLOCK_MASK, XPLM_ZERO);

END:
	return Status;
}
