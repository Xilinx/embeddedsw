/******************************************************************************
 * Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
 *       sk   02/04/25 Updated BootModeVal variable as volatile
 *       ng   02/11/25 Add Secure lockdown and tamper response support
 *       ng   02/22/25 Removed the print during error logging
 * </pre>
 *
 ******************************************************************************/

/**
 * @addtogroup spartanup_plm_apis SpartanUP PLM APIs
 * @{
 */

/***************************** Include Files *********************************/
#include "xil_io.h"
#include "xplm_error.h"
#include "xplm_load.h"
#include "xplm_hw.h"
#include "xplm_hooks.h"
#include "xplm_dma.h"

/************************** Constant Definitions *****************************/
/** @cond spartanup_plm_internal */
#define XPLM_EFUSE_CACHE_CRC_EN_ADDR	(0x04161150U)
#define XPLM_EFUSE_LCKDWN_EN_MASK	(0x8U)
#define XPLM_PMCL_RESET_VAL		(1U)

/* Masks for PMC FW ERR first and last */
#define XPLM_PMC_FW_ERR_FIRST_ERR_MASK	(0xFFF000U)
#define XPLM_PMC_FW_ERR_LAST_ERR_MASK	(0x000FFFU)

/* Shift for PMC FW ERR First error */
#define XPLM_PMC_FW_ERR_FIRST_ERR_SHIFT	(12U)

/* Multiboot max offset */
#define XPLM_MULTIBOOT_OFFSET	(0x8000U)

/* CCU payload to set the gts_usr_b bit to tristate PL IO. */
#define XPLM_TIO_CCU_SYNC_WORD	(0xAA995566U)
#define XPLM_TIO_CCU_NO_OP	(0x20000000U)
#define XPLM_TIO_CCU_MASK	(0x3000C001U)
#define XPLM_TIO_CCU_CTL	(0x3000A001U)
#define XPLM_TIO_CCU_CMD	(0x30008001U)
#define XPLM_TIO_CCU_DESYNCH	(0xDU)

/** @endcond */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
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
	volatile u32 BootModeVal = 0U;
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
 * @brief	This function is called for logging PLM error into FW_ERR BOOT_ERR register.
 *
 * @param	ErrStatus is the error code to be written to the FW_ERR register
 *
 *****************************************************************************/
void XPlm_LogPlmErr(u32 ErrStatus)
{
	u32 PlmFirstError;
	u32 RomFirstError;

	PlmFirstError = Xil_In32(PMC_GLOBAL_PMC_FW_ERR) & XPLM_PMC_FW_ERR_FIRST_ERR_MASK;
	RomFirstError = Xil_In32(PMC_GLOBAL_PMC_BOOT_ERR) & XPLM_PMC_FW_ERR_FIRST_ERR_MASK;

	/** - Store PLM error to FW_ERR register. */
	if (PlmFirstError == XPLM_ZERO) {
		XPlm_UtilRMW(PMC_GLOBAL_PMC_FW_ERR, XPLM_PMC_FW_ERR_FIRST_ERR_MASK,
			     (ErrStatus << XPLM_PMC_FW_ERR_FIRST_ERR_SHIFT));
	} else {
		XPlm_UtilRMW(PMC_GLOBAL_PMC_FW_ERR, XPLM_PMC_FW_ERR_LAST_ERR_MASK, ErrStatus);
	}

	/** - Store PLM error to BOOT_ERR register. */
	if (RomFirstError == XPLM_ZERO) {
		XPlm_UtilRMW(PMC_GLOBAL_PMC_BOOT_ERR, XPLM_PMC_FW_ERR_FIRST_ERR_MASK,
			     (ErrStatus << XPLM_PMC_FW_ERR_FIRST_ERR_SHIFT));
	} else {
		XPlm_UtilRMW(PMC_GLOBAL_PMC_BOOT_ERR, XPLM_PMC_FW_ERR_LAST_ERR_MASK, ErrStatus);
	}
}

/*****************************************************************************/
/**
 * @brief This function is called for logging PLM stage to PERSISTENT GLOBAL GEN STORAGE 0 register
 *
 * @param	Stage is the number to log to FW_STATUS register
 *
 *****************************************************************************/
void XPlm_LogPlmStage(XPlm_Stages Stage)
{
	/* Log PLM Stage to FW_STATUS registers. */
	XPlm_UtilRMW(PMC_GLOBAL_PMC_FW_STATUS, XPLM_FW_STATUS_STAGE_MASK, Stage);
}

/*****************************************************************************/
/**
 * @brief	This function initiates a secure lockdown sequence. The function performs various
 * security measures including tristating all chip IO, clearing crypto engines, disabling external
 * interfaces, resetting the capture control unit, triggering PL house clean, and triggering MBist
 * and Scan clear.
 *
 * @param Data	A pointer to a u32 value cast to void*. The value should be either
 * 		XPLM_TAMPER_EVENT_EAM (0xF0) to indicate a tamper event or XPLM_EVENT_MULTIBOOT_LIMIT (0x0F)
 * 		to indicate a multiboot failure.
 *
 *****************************************************************************/
void XPlm_TriggerSecLockdown(void *Data)
{
	u32 SldEvent = (u32)(UINTPTR)Data;
	u32 EnableIOTristate = Xil_In32(XPLM_RTCFG_SECURE_LOCKDOWN_TIO) & XPLM_RTCFG_SEC_LOCKDOWN_TIO_EN_MASK;
	const u32 GtsUsrBCcuPayload[] = {XPLM_TIO_CCU_SYNC_WORD, XPLM_TIO_CCU_NO_OP, XPLM_TIO_CCU_MASK, XPLM_ONE, XPLM_TIO_CCU_CTL, XPLM_ZERO, XPLM_TIO_CCU_CMD, XPLM_TIO_CCU_DESYNCH, XPLM_TIO_CCU_NO_OP};

	XPlm_Printf(DEBUG_PRINT_ALWAYS, "Triggering secure lockdown\n\r");

	/** - Tristate all chip IO based on the user configuration. */
	if ((SldEvent != XPLM_EVENT_MULTIBOOT_LIMIT) && (EnableIOTristate == XPLM_RTCFG_SEC_LOCKDOWN_TIO_EN_MASK)) {
		/* Allow the writes to CCU_FGCR register. */
		Xil_Out32(CCU_APB_CCU_PROTECT, CCU_APB_CCU_PROTECT_WRITE_ALLOWED);

		/* Update the CCU_MASK register with the mask to set GTS_CFG_B bit in CCU_FGCR register. */
		Xil_Out32(CCU_APB_CCU_MASK, CCU_APB_CCU_FGCR_GTS_CFG_B_MASK);

		/* Set the GTS_CFG_B bit in CCU_FGCR register. */
		XSECURE_REDUNDANT_IMPL(XPlm_UtilRMW, CCU_APB_CCU_FGCR, CCU_APB_CCU_FGCR_GTS_CFG_B_MASK, CCU_APB_CCU_FGCR_GTS_CFG_B_MASK);

		/* Disable the writes to CCU_FGCR register. */
		Xil_Out32(CCU_APB_CCU_PROTECT, CCU_APB_CCU_PROTECT_WRITE_DISABLE);

		/* Send the CCU payload to set the GTS_USR_B bit. */
		(void)XPlm_DmaXfr((u32)GtsUsrBCcuPayload, XPLM_CCU_WR_STREAM_BASEADDR, XPLM_ARRAY_SIZE(GtsUsrBCcuPayload), XPLM_DMA_INCR_MODE);
	}

	/** - Clear crypto engines. */
	HooksTbl->XRom_ClearCrypto();
	/* Stop the PUF engine. */
	XSECURE_REDUNDANT_IMPL(Xil_Out32, PUF_COMMAND, PUF_COMMAND_CMD_STOP);

	/** - Disable all external interfaces. */
	/* Unlock the PMC_TAP registers */
	Xil_Out32(PMC_TAP_PMC_TAP_LOCK, XPLM_ZERO);

	/* Enable security gate */
	XSECURE_REDUNDANT_IMPL(Xil_Out32, PMC_TAP_TAP_SECURITY, XPLM_ZERO);

	/* Lock the PMC_TAP registers */
	XSECURE_REDUNDANT_IMPL(Xil_Out32, PMC_TAP_PMC_TAP_LOCK, PMC_TAP_PMC_TAP_LOCK_DEFVAL);

	/* Disable SBI interface and readback. */
	XSECURE_REDUNDANT_IMPL(XPlm_UtilRMW, SLAVE_BOOT_SBI_CTRL, (SLAVE_BOOT_SBI_CTRL_MCAP_DIS_MASK | SLAVE_BOOT_SBI_CTRL_ENABLE_MASK), SLAVE_BOOT_SBI_CTRL_MCAP_DIS_MASK);
	XSECURE_REDUNDANT_IMPL(Xil_Out32, SLAVE_BOOT_SBI_RDBK, SLAVE_BOOT_SBI_RDBK_DIS_MASK);

	/** - Reset capture control unit. */
	Xil_Out32(PMC_GLOBAL_RST_CCU, 1U);
	Xil_Out32(PMC_GLOBAL_RST_CCU, PMC_GLOBAL_RST_CCU_RESET_DEFVAL);

	/** - Trigger PL house clean. */
	(void)HooksTbl->XRom_PlHouseClean();

	/** - Trigger MBist and Scan clear. */
	HooksTbl->XRom_MBistNScanClear();
}

/*****************************************************************************/
/**
 * @brief This function checks the efuse bits and if secure lockdown is enabled,
 * triggers secure lockdown. Otherwise does nothing.
 *
 *****************************************************************************/
static void XPlm_CheckNTriggerSecLockdown(void)
{
	u32 LckDwnEn = 0U;
	u32 LckDwnEnTmp = 0U;

	/** - Trigger secure lockdown - @ref XPlm_TriggerSecLockdown, if lockdown is enabled in efuse. */
	LckDwnEn = Xil_In32(XPLM_EFUSE_CACHE_CRC_EN_ADDR) & XPLM_EFUSE_LCKDWN_EN_MASK;
	LckDwnEnTmp = Xil_In32(XPLM_EFUSE_CACHE_CRC_EN_ADDR) & XPLM_EFUSE_LCKDWN_EN_MASK;
	if ((LckDwnEn != 0U) || (LckDwnEnTmp != 0U)) {
		XPlm_TriggerSecLockdown((void *)(UINTPTR)XPLM_EVENT_MULTIBOOT_LIMIT);
	}
}

/** @} end of spartanup_plm_apis group*/
