/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xplm_error.c
 *
 * This file contains error management code
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  ng   05/31/24 Initial release
 *       ng   09/18/24 Fixed multiboot offset check
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_io.h"
#include "xplm_error.h"
#include "xplm_load.h"
#include "xplm_hw.h"
#include "xplm_hooks.h"

/************************** Constant Definitions *****************************/
#define XPLM_EFUSE_CACHE_CRC_EN_ADDR	(0x04161150U)
#define XPLM_EFUSE_LCKDWN_EN_MASK	(0x8U)
#define XPLM_PMCL_RESET_VAL		(1U)

/* Masks for PMC FW ERR first and last */
#define XPLM_PMC_FW_ERR_FIRST_ERR_MASK	(0x3FFF0000U)
#define XPLM_PMC_FW_ERR_LAST_ERR_MASK	(0x00003FFFU)

/* Shift for PMC FW ERR First error */
#define XPLM_PMC_FW_ERR_FIRST_ERR_SHIFT	(16U)

/* Multiboot max offset */
#define XPLM_MULTIBOOT_OFFSET	(0x8000U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void XPlm_TriggerSecLockdown(void);
static void XPlm_CheckNTriggerSecLockdown(void);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function is used to log PLM error into FW_ERR register, execute secure
 * lockdown when LckDwnEn efuse bit is programmed, fallback to other boot pdi.
 *
 * @param	ErrStatus is the error code to be written to the FW_ERR register
 *
 *****************************************************************************/
void XPlm_ErrMgr(u32 ErrStatus)
{
	XRomBootRom *InstancePtr = HooksTbl->InstancePtr;
	u32 MultiBootVal = 0U;
	u32 BootModeVal = 0U;
	volatile u32 DebugMode = 0U;
	volatile u32 DebugModeTmp = 0U;
	volatile u32 CurrPlmStage;
	volatile u32 CurrPlmStageTmp;

	/** - Log PLM error to FW_ERR register. */
	XPlm_LogPlmErr(ErrStatus);

	/**
	 * - If the current boot stage is @ref XPLM_POST_BOOT_STAGE or above, then set FW_CR in
	 * FW_ERR register and enter infinite loop.
	 * - Otherwise set FW_NCR in FW_ERR and:
	 * 	- enter infinite loop, if debug mode is enabled in RTCA, or
	 * 	- trigger secure lockdown if it's JTAG/SMAP/Select Serial boot mode, or
	 * 	- increment multiboot offset if it's OSPI/QSPI boot mode or trigger secure lockdown
	 * 	if the multiboot offset exceeds the supported flash limit.
	 */
	CurrPlmStage = Xil_In32(PMC_GLOBAL_PMC_FW_STATUS) & XPLM_FW_STATUS_STAGE_MASK;
	CurrPlmStageTmp = Xil_In32(PMC_GLOBAL_PMC_FW_STATUS) & XPLM_FW_STATUS_STAGE_MASK;
	if ((CurrPlmStage >= XPLM_POST_BOOT_STAGE) && (CurrPlmStageTmp >= XPLM_POST_BOOT_STAGE)) {
		XPlm_UtilRMW(PMC_GLOBAL_PMC_FW_ERR, PMC_GLOBAL_PMC_FW_ERR_CR_MASK, PMC_GLOBAL_PMC_FW_ERR_CR_MASK);
	} else {
		DebugMode = Xil_In32(XPLM_RTCFG_DBG_CTRL);
		DebugModeTmp = Xil_In32(XPLM_RTCFG_DBG_CTRL);
		BootModeVal = Xil_In32(PMC_GLOBAL_BOOT_MODE_USER) & PMC_GLOBAL_BOOT_MODE_USER_MASK;

		if ((DebugMode == XPLM_SKIP_MULTIBOOT_RESET) && (DebugModeTmp == XPLM_SKIP_MULTIBOOT_RESET)) {
			XPlm_UtilRMW(PMC_GLOBAL_PMC_FW_ERR, PMC_GLOBAL_PMC_FW_ERR_NCR_MASK, PMC_GLOBAL_PMC_FW_ERR_NCR_MASK);
		} else if ((BootModeVal == XPLM_BOOT_MODE_JTAG) || (BootModeVal == XPLM_BOOT_MODE_SMAP)
			   || (BootModeVal == XPLM_BOOT_MODE_SELECT_SERIAL)) {
			XPlm_UtilRMW(PMC_GLOBAL_PMC_FW_ERR, PMC_GLOBAL_PMC_FW_ERR_NCR_MASK, PMC_GLOBAL_PMC_FW_ERR_NCR_MASK);

			XSECURE_REDUNDANT_IMPL(XPlm_CheckNTriggerSecLockdown);
		} else if ((BootModeVal == XPLM_BOOT_MODE_QSPI24) || (BootModeVal == XPLM_BOOT_MODE_QSPI32)
			   || (BootModeVal == XPLM_BOOT_MODE_OSPI)) {
			MultiBootVal = Xil_In32(PMC_GLOBAL_MULTI_BOOT);
			MultiBootVal += 1U;

			if (MultiBootVal >= (InstancePtr->DeviceData / XPLM_MULTIBOOT_OFFSET)) {
				XPlm_UtilRMW(PMC_GLOBAL_PMC_FW_ERR, PMC_GLOBAL_PMC_FW_ERR_NCR_MASK, PMC_GLOBAL_PMC_FW_ERR_NCR_MASK);

				XSECURE_REDUNDANT_IMPL(XPlm_CheckNTriggerSecLockdown);

				/* Wait forever */
				while (1U);
			}
			Xil_Out32(PMC_GLOBAL_MULTI_BOOT, MultiBootVal);
			Xil_Out32(PMC_GLOBAL_RST_PMCL, XPLM_PMCL_RESET_VAL);
		}
	}

	/* Wait forever */
	while (1U);
}

/*****************************************************************************/
/**
 * This function is called for logging PLM error into FW_ERR register
 *
 * @param	ErrStatus is the error code written to the FW_ERR register
 *
 *****************************************************************************/
void XPlm_LogPlmErr(u32 ErrStatus) {
	u32 FirstError;

	FirstError = Xil_In32(PMC_GLOBAL_PMC_FW_ERR) & XPLM_PMC_FW_ERR_FIRST_ERR_MASK;

	if (FirstError == XPLM_ZERO) {
		XPlm_UtilRMW(PMC_GLOBAL_PMC_FW_ERR, XPLM_PMC_FW_ERR_FIRST_ERR_MASK, (ErrStatus << XPLM_PMC_FW_ERR_FIRST_ERR_SHIFT));
	}
	else {
		XPlm_UtilRMW(PMC_GLOBAL_PMC_FW_ERR, XPLM_PMC_FW_ERR_LAST_ERR_MASK, ErrStatus);
	}
	/** - Print the PLM Error */
	XPlm_Printf(DEBUG_PRINT_ALWAYS, "PLM Error Status: 0x%08x\n\r", Xil_In32(PMC_GLOBAL_PMC_FW_ERR));
}

/*****************************************************************************/
/**
 * This function is called for logging PLM stage to PERSISTENT GLOBAL GEN STORAGE 0 register
 *
 * @param	Stage is the number to log to persistent global register
 *
 *****************************************************************************/
void XPlm_LogPlmStage(XPlm_Stages Stage) {
	/* Log PLM Stage to FW_STATUS registers. */
	XPlm_UtilRMW(PMC_GLOBAL_PMC_FW_STATUS, XPLM_FW_STATUS_STAGE_MASK, Stage);
}

/*****************************************************************************/
/**
 * This function triggers secure lockdown
 *
 *****************************************************************************/
static void XPlm_TriggerSecLockdown(void)
{
	XPlm_Printf(DEBUG_PRINT_ALWAYS, "Triggering secure lockdown\n\r");

	HooksTbl->XRom_ClearCrypto();

	/* Toggle CCU */
	Xil_Out32(PMC_GLOBAL_RST_CCU, 1U);
	Xil_Out32(PMC_GLOBAL_RST_CCU, PMC_GLOBAL_RST_CCU_RESET_DEFVAL);

	(void)HooksTbl->XRom_PlHouseClean();

	HooksTbl->XRom_MBistNScanClear();
}

/*****************************************************************************/
/**
 * This function checks the efuse bits and if secure lockdown is enabled,
 * triggers secure lockdown. Otherwise does nothing.
 *
 *****************************************************************************/
static void XPlm_CheckNTriggerSecLockdown(void)
{
	u32 LckDwnEn = 0U;
	u32 LckDwnEnTmp = 0U;

	LckDwnEn = Xil_In32(XPLM_EFUSE_CACHE_CRC_EN_ADDR) & XPLM_EFUSE_LCKDWN_EN_MASK;
	LckDwnEnTmp = Xil_In32(XPLM_EFUSE_CACHE_CRC_EN_ADDR) & XPLM_EFUSE_LCKDWN_EN_MASK;
	if ((LckDwnEn != 0U) || (LckDwnEnTmp != 0U)) {
		XPlm_TriggerSecLockdown();
	}
}