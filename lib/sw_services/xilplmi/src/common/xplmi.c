/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi.c
*
* This file contains the PLMI module register functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/07/2019 Initial release
* 1.01  ma   08/01/2019 Added LPD init code
* 1.02  kc   02/19/2020 Moved code to support PLM banner from PLM app
*       bsv  04/04/2020 Code clean up
* 1.03  bsv  07/07/2020 Made functions used in single transaltion unit as
*						static
*       kc   07/28/2020 Moved LpdInitialized from xplmi_debug.c to xplmi.c
*       bm   09/08/2020 Added RunTime Configuration Init API to XPlmi_Init
*       bm   10/14/2020 Code clean up
*       td   10/19/2020 MISRA C Fixes
* 1.04  bm   10/28/2020 Added ROM Version Print
*       har  03/17/2021 Added code for run time initialization of Secure State
*                       registers
*       ma   03/24/2021 Print early logs
*       bm   03/24/2021 Added RTCA initialization for Error Status registers
*       har  03/31/2021 Added RTCA initialization for PDI ID
*       bsv  04/16/2021 Add provision to store Subsystem Id in XilPlmi
*       bm   05/05/2021 Added USR_ACCESS support for PLD0 image
*       ma   05/21/2021 Copy secure boot state from PMC GLOBAL GEN STORAGE2
                        register to RTCA Secure State offset
* 1.05  td   07/08/2021 Fix doxygen warnings
*       bsv  07/18/2021 Print PLM banner at the beginning of PLM execution
*       bsv  07/24/2021 Clear RTC area at the beginning of PLM
*       bsv  08/02/2021 Code clean up to reduce elf size
*       rb   07/29/2021 Update reset reason during Init
*       ma   08/23/2021 Do not clear Debug Log RTCA memory
*       ma   09/13/2021 Set PLM prints log level during RTCA init
*       tnt  11/11/2021 Add RTCA initialization for MIO Flush routine
*       tnt  12/17/2021 Add RTCA initialization for PL_POR HDIO WA
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi.h"
#include "xplmi_err_common.h"
#include "xplmi_wdt.h"
#include "xplmi_hw.h"

/************************** Constant Definitions *****************************/
#define XPLMI_DIGEST_PMC_1_0_ROM_1_0	(0x2B004AC7U) /**< PMC1 ROM version 1
														digest */
#define XPLMI_DIGEST_PMC_2_0_ROM_2_0	(0xB576B550U) /**< PMC2 ROM version 2
														digest */

#define XPLMI_ROM_VERSION_1_0		(0x10U) /**< ROM version 1 */
#define XPLMI_ROM_VERSION_2_0		(0x20U) /**< ROM version 2 */
#define XPLMI_INVALID_ROM_VERSION	(0x0U) /**< Invalid ROM version */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void XPlmi_PrintRomVersion(void);
static void XPlmi_PrintEarlyLog(void);
static void XPlmi_UpdateResetReason(void);

/************************** Variable Definitions *****************************/
u8 LpdInitialized = (u8)0U; /**< 1 if LPD is initialized */

/******************************************************************************/
/**
* @brief	This function  will initialize the PLMI module.
*
* @return   None.
*
****************************************************************************/
int XPlmi_Init(void)
{
	int Status = XST_FAILURE;

	XPlmi_UpdateResetReason();

	Status = XPlmi_SetUpInterruptSystem();
	if (Status != XST_SUCCESS) {
		goto END;
	}
	XPlmi_GenericInit();

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function updates reset reason.
 *
 * @return	None
 *
 *****************************************************************************/
static void XPlmi_UpdateResetReason(void)
{
	u32 AccResetReason = XPlmi_In32(PMC_GLOBAL_PERS_GEN_STORAGE2) &
				PERS_GEN_STORAGE2_ACC_RR_MASK;
	u32 ResetReason = XPlmi_In32(CRP_RESET_REASON) &
				CRP_RESET_REASON_MASK;

	/* Accumulate previous reset reasons and add last reset reason */
	AccResetReason |= (ResetReason << CRP_RESET_REASON_SHIFT) | ResetReason;

	/* Store Reset Reason to Persistent2 address */
	XPlmi_Out32(PMC_GLOBAL_PERS_GEN_STORAGE2, AccResetReason);

	/* Clear Reset Reason register, by writing the same value */
	XPlmi_Out32(CRP_RESET_REASON, ResetReason);
}

/*****************************************************************************/
/**
 * @brief	This function initializes the Runtime Configuration Area with
 * default values.
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_RunTimeConfigInit(void)
{
	int Status = XST_FAILURE;
	u32 DevSecureState = XPlmi_In32(PMC_GLOBAL_GLOBAL_GEN_STORAGE2);

	Status = XPlmi_MemSet((u64)XPLMI_RTCFG_BASEADDR, 0U,
		(XPLMI_RTCFG_DBG_LOG_BUF_OFFSET >> XPLMI_WORD_LEN_SHIFT));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPlmi_MemSet((u64)(XPLMI_RTCFG_BASEADDR + XPLMI_RTCFG_LOG_UART_OFFSET),
		0U,	((XPLMI_RTCFG_SIZE - XPLMI_RTCFG_LOG_UART_OFFSET)
				>> XPLMI_WORD_LEN_SHIFT));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XPlmi_Out32(XPLMI_RTCFG_RTCA_ADDR, XPLMI_RTCFG_IDENTIFICATION);
	XPlmi_Out32(XPLMI_RTCFG_VERSION_ADDR, XPLMI_RTCFG_VER);
	XPlmi_Out32(XPLMI_RTCFG_SIZE_ADDR, XPLMI_RTCFG_SIZE);
	XPlmi_Out32(XPLMI_RTCFG_IMGINFOTBL_ADDRLOW_ADDR,
				XPLMI_IMAGE_INFO_TBL_BUFFER_ADDR);
	XPlmi_Out32(XPLMI_RTCFG_IMGINFOTBL_ADDRHIGH_ADDR,
				XPLMI_RTCFG_IMGINFOTBL_ADDR_HIGH);
	XPlmi_Out32(XPLMI_RTCFG_IMGINFOTBL_LEN_ADDR,
				XPLMI_RTCFG_IMGINFOTBL_LEN);
	XPlmi_Out32(XPLMI_RTCFG_SECURESTATE_AHWROT_ADDR,
				XPLMI_RTCFG_SECURESTATE_AHWROT);
	XPlmi_Out32(XPLMI_RTCFG_SECURESTATE_SHWROT_ADDR,
				XPLMI_RTCFG_SECURESTATE_SHWROT);
	XPlmi_Out32(XPLMI_RTCFG_PDI_ID_ADDR, XPLMI_RTCFG_PDI_ID);
	XPlmi_Out32(XPLMI_RTCFG_SECURE_STATE_ADDR, DevSecureState);
	/* MIO flush RTCFG init */
	XPlmi_Out32(XPLMI_RTCFG_MIO_WA_BANK_500_ADDR, XPLMI_MIO_FLUSH_ALL_PINS);
	XPlmi_Out32(XPLMI_RTCFG_MIO_WA_BANK_501_ADDR, XPLMI_MIO_FLUSH_ALL_PINS);
	XPlmi_Out32(XPLMI_RTCFG_MIO_WA_BANK_502_ADDR, XPLMI_MIO_FLUSH_ALL_PINS);
	XPlmi_Out32(XPLMI_RTCFG_RST_PL_POR_WA, 0U);
	/* Set PLM prints log level */
	DebugLog->LogLevel = ((u8)XPlmiDbgCurrentTypes << XPLMI_LOG_LEVEL_SHIFT) |
		(u8)XPlmiDbgCurrentTypes;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function calls all the PS LPD init functions of all the
 * different modules. As a part of init functions, modules can register the
 * command handlers, interrupt handlers with the interface layer.
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_LpdInit(void)
{
	int Status = XST_FAILURE;

#ifdef DEBUG_UART_PS
	Status = XPlmi_InitUart();
	if (Status != XST_SUCCESS) {
		goto END;
	}
#endif
	Status = XPlmi_PsEmInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	LpdInitialized |= LPD_INITIALIZED;
	XPlmi_PrintEarlyLog();

END:
	return;
}

/*****************************************************************************/
/**
 * @brief	This function prints PLM banner
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_PrintPlmBanner(void)
{
	u32 Version;
	u8 PsVersion;
	u8 PmcVersion;
	u32 BootMode;
	u32 MultiBoot;
	u8 PmcVersionDecimal;

	/* Print the PLM Banner */
	XPlmi_Printf(DEBUG_PRINT_ALWAYS,
		"****************************************\n\r");
	XPlmi_Printf(DEBUG_PRINT_ALWAYS,
		"Xilinx Versal Platform Loader and Manager \n\r");
	XPlmi_Printf(DEBUG_PRINT_ALWAYS,
		"Release %s.%s   %s  -  %s\n\r",
		SDK_RELEASE_YEAR, SDK_RELEASE_QUARTER, __DATE__, __TIME__);

	/* Read the Version */
	Version = XPlmi_In32(PMC_TAP_VERSION);
	PsVersion = (u8)((Version & PMC_TAP_VERSION_PS_VERSION_MASK) >>
			PMC_TAP_VERSION_PS_VERSION_SHIFT);
	PmcVersion = (u8)((Version & PMC_TAP_VERSION_PMC_VERSION_MASK) >>
			PMC_TAP_VERSION_PMC_VERSION_SHIFT);
	BootMode = XPlmi_In32(CRP_BOOT_MODE_USER) &
			CRP_BOOT_MODE_USER_BOOT_MODE_MASK;
	MultiBoot = XPlmi_In32(PMC_GLOBAL_PMC_MULTI_BOOT);
	PmcVersionDecimal = (u8)(PmcVersion & XPLMI_PMC_VERSION_MASK);
	PmcVersion = (u8)(PmcVersion >> XPLMI_PMC_VERSION_SHIFT);

	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "Platform Version: v%u.%u "
		"PMC: v%u.%u, PS: v%u.%u\n\r", PmcVersion, PmcVersionDecimal,
		PmcVersion, PmcVersionDecimal,
		(PsVersion >> XPLMI_PMC_VERSION_SHIFT),
		(PsVersion & XPLMI_PMC_VERSION_MASK));
	XPlmi_PrintRomVersion();
	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "BOOTMODE: 0x%x, MULTIBOOT: 0x%x"
			"\n\r", BootMode, MultiBoot);
	XPlmi_Printf(DEBUG_PRINT_ALWAYS,
		"****************************************\n\r");
}

/*****************************************************************************/
/**
 * @brief	This function prints early log.
 *
 * @return	None
 *
 *****************************************************************************/
static void XPlmi_PrintEarlyLog(void)
{
	DebugLog->PrintToBuf = (u8)FALSE;
	/* Print early log */
	if (DebugLog->LogBuffer.IsBufferFull == (u32)FALSE) {
		XPlmi_OutByte64(DebugLog->LogBuffer.StartAddr +
			DebugLog->LogBuffer.Offset, 0U);
		XPlmi_Printf_WoTS(DEBUG_PRINT_ALWAYS, "%s",
			(UINTPTR)DebugLog->LogBuffer.StartAddr);
	}
	else {
		XPlmi_PrintPlmBanner();
	}
	DebugLog->PrintToBuf = (u8)TRUE;
}

/*****************************************************************************/
/**
 * @brief	This function prints ROM version using ROM digest value.
 *
 * @return	None
 *
 *****************************************************************************/
static void XPlmi_PrintRomVersion(void)
{
	u32 RomDigest;
	u8 RomVersion;

	RomDigest = XPlmi_In32(PMC_GLOBAL_ROM_VALIDATION_DIGEST_0);
	switch (RomDigest) {
		case XPLMI_DIGEST_PMC_1_0_ROM_1_0:
			RomVersion = XPLMI_ROM_VERSION_1_0;
			break;
		case XPLMI_DIGEST_PMC_2_0_ROM_2_0:
			RomVersion = XPLMI_ROM_VERSION_2_0;
			break;
		default:
			RomVersion = XPLMI_INVALID_ROM_VERSION;
			break;
	}

	if (RomVersion != XPLMI_INVALID_ROM_VERSION) {
		XPlmi_Printf(DEBUG_INFO, "ROM Version: v%u.%u\n\r",
			(RomVersion >> 4U), (RomVersion & 15U));
	}
}

/*****************************************************************************/
/**
 * @brief	This function resets LpdInitialized variable to 0.
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_ResetLpdInitialized(void)
{
	if ((LpdInitialized & LPD_WDT_INITIALIZED) == LPD_WDT_INITIALIZED) {
		XPlmi_DisableWdt();
	}

	LpdInitialized = (u8)0U;
}
