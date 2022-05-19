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
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi.h"
#include "xplmi_err.h"
#include "xplmi_wdt.h"
#include "xplmi_hw.h"
#include "xplmi_update.h"

/************************** Constant Definitions *****************************/
#define XPLMI_DIGEST_PMC_1_0_ROM_1_0	(0x2B004AC7U)
#define XPLMI_DIGEST_PMC_2_0_ROM_2_0	(0xB576B550U)

#define XPLMI_ROM_VERSION_1_0		(0x10U)
#define XPLMI_ROM_VERSION_2_0		(0x20U)
#define XPLMI_INVALID_ROM_VERSION	(0x0U)

/**************************** Type Definitions *******************************/
typedef int (*XPlmi_InitHandler)(void);

/***************** Macros (Inline Functions) Definitions *********************/
#define XPLMI_LPDINITIALIZED_VER 	(1U)
#define XPLMI_LPDINITIALIZED_LCVER 	(1U)
#define XPLMI_BANNER_VER		(1U)
#define XPLMI_BANNER_LCVER		(1U)

/************************** Function Prototypes ******************************/
static void XPlmi_RunTimeConfigInit(void);
static void XPlmi_PrintRomVersion(void);
static void XPlmi_PrintEarlyLog(void);

/************************** Variable Definitions *****************************/
u32 LpdInitialized __attribute__ ((aligned(4U))) = (u32)0U;

EXPORT_GENERIC_DS(LpdInitialized, XPLMI_LPDINITIALIZED_DS_ID, XPLMI_LPDINITIALIZED_VER,
		XPLMI_LPDINITIALIZED_LCVER, sizeof(LpdInitialized), (u32)(UINTPTR)&LpdInitialized);

/******************************************************************************/
/**
* @brief	This function  will initialize the PLMI module.
*
* @param    None
*
* @return   None.
*
****************************************************************************/
int XPlmi_Init(void)
{
	int Status = XST_FAILURE;

	if (XPlmi_IsPlmUpdateDone() != (u8)TRUE) {
		XPlmi_RunTimeConfigInit();
	}
	else {
		XPlmi_RestoreWdt();
	}

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
 * @brief	This function initializes the Runtime Configuration Area with
 * default values.
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
static void XPlmi_RunTimeConfigInit(void)
{
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
	XPlmi_Out32(XPLMI_RTCFG_PMC_ERR1_STATUS_ADDR, 0U);
	XPlmi_Out32(XPLMI_RTCFG_PMC_ERR2_STATUS_ADDR, 0U);
	XPlmi_Out32(XPLMI_RTCFG_PSM_ERR1_STATUS_ADDR, 0U);
	XPlmi_Out32(XPLMI_RTCFG_PSM_ERR2_STATUS_ADDR, 0U);
	XPlmi_Out32(XPLMI_RTCFG_PDI_ID_ADDR, XPLMI_RTCFG_PDI_ID);
	XPlmi_Out32(XPLMI_RTCFG_USR_ACCESS_ADDR, 0U);
}

/*****************************************************************************/
/**
 * @brief	This function calls all the PS LPD init functions of all the
 * different modules. As a part of init functions, modules can register the
 * command handlers, interrupt handlers with the interface layer.
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_LpdInit(void)
{
	int Status = XST_FAILURE;
	u32 Index;
	/*
	 * It contains all the PS LPD init functions to be run for every module that
	 * is present as a part of PLM.
	 */
	const XPlmi_InitHandler LpdInitList[] = {
#ifdef DEBUG_UART_PS
		XPlmi_InitUart,
#endif
#ifndef PLM_PM_EXCLUDE
		XPlmi_PsEmInit,
#endif
	};

	for (Index = 0U; Index < XPLMI_ARRAY_SIZE(LpdInitList); Index++) {
		Status = LpdInitList[Index]();
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XPLM_ERR_LPD_MOD, Status);
			break;
		}
	}
	if (XST_SUCCESS == Status) {
		LpdInitialized |= LPD_INITIALIZED;
	}
	if (XPlmi_IsPlmUpdateDone() != (u8)TRUE) {
		XPlmi_PrintEarlyLog();
	}
}

/*****************************************************************************/
/**
 * @brief	This function prints Early Log
 *
 *****************************************************************************/
void XPlmi_PrintEarlyLog(void)
{
	/* Print early log */
	if (DebugLog->LogBuffer.IsBufferFull == (u32)FALSE) {
		DebugLog->PrintToBuf = (u8)FALSE;
		XPlmi_OutByte64(DebugLog->LogBuffer.StartAddr + DebugLog->LogBuffer.Offset, 0U);
		XPlmi_Printf_WoTS(DEBUG_PRINT_ALWAYS, "%s", (UINTPTR)DebugLog->LogBuffer.StartAddr);
		DebugLog->PrintToBuf = (u8)TRUE;
	}
}

/*****************************************************************************/
/**
 * @brief	This function prints PLM banner
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_PrintPlmBanner(void)
{
	u32 Version;
	u32 PsVersion;
	u32 PmcVersion;
	static u32 IsBannerPrinted __attribute__ ((aligned(4U))) = (u8)FALSE;
	u32 BootMode;
	u32 MultiBoot;
	char *Platform = NULL;

	EXPORT_GENERIC_DS(IsBannerPrinted, XPLMI_BANNER_DS_ID, XPLMI_BANNER_VER,
		XPLMI_BANNER_LCVER, sizeof(IsBannerPrinted), (u32)(UINTPTR)&IsBannerPrinted);

	if ((u32)FALSE == IsBannerPrinted) {
		/* Print the PLM Banner */
		XPlmi_Printf(DEBUG_PRINT_ALWAYS,
			"****************************************\n\r");
		XPlmi_Printf(DEBUG_PRINT_ALWAYS,
			"Xilinx Versal_Net Platform Loader and Manager \n\r");
		XPlmi_Printf(DEBUG_PRINT_ALWAYS,
			"Release %s.%s   %s  -  %s\n\r",
			SDK_RELEASE_YEAR, SDK_RELEASE_QUARTER, __DATE__, __TIME__);

		/* Read the Version */
		Version = XPlmi_In32(PMC_TAP_VERSION);
		PsVersion = ((Version & PMC_TAP_VERSION_PS_VERSION_MASK) >>
				PMC_TAP_VERSION_PS_VERSION_SHIFT);
		PmcVersion = ((Version & PMC_TAP_VERSION_PMC_VERSION_MASK) >>
				PMC_TAP_VERSION_PMC_VERSION_SHIFT);
		BootMode = XPlmi_In32(CRP_BOOT_MODE_USER) &
				CRP_BOOT_MODE_USER_BOOT_MODE_MASK;
		MultiBoot = XPlmi_In32(PMC_GLOBAL_PMC_MULTI_BOOT);

		XPlmi_Printf(DEBUG_PRINT_ALWAYS, "Platform Version: v%u.%u "
				 "PMX: v%u.%u, PSX: v%u.%u\n\r",
				(PmcVersion / 16U), (PmcVersion % 16U),
				(PmcVersion / 16U), (PmcVersion % 16U),
				(PsVersion / 16U), (PsVersion % 16U));
		XPlmi_PrintRomVersion();
		switch (XPLMI_PLATFORM) {
			case PMC_TAP_VERSION_SILICON:
				Platform = "Silicon";
				break;
			case PMC_TAP_VERSION_SPP:
				Platform = "Protium";
				break;
			case PMC_TAP_VERSION_EMU:
				Platform = "Palladium";
				break;
			case PMC_TAP_VERSION_QEMU:
				Platform = "QEMU";
				break;
			default: Platform = "Undefined";
				 break;
		}
		XPlmi_Printf(DEBUG_PRINT_ALWAYS, "Platform Type: %s\n\r", Platform);
		XPlmi_Printf(DEBUG_PRINT_ALWAYS, "BOOTMODE: 0x%x, MULTIBOOT: 0x%x"
				"\n\r", BootMode, MultiBoot);
		XPlmi_Printf(DEBUG_PRINT_ALWAYS,
			"****************************************\n\r");
		IsBannerPrinted = (u32)TRUE;
	}
}

/*****************************************************************************/
/**
 * @brief	This function prints ROM version using ROM digest value.
 *
 * @param	None
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
			(RomVersion / 16U), (RomVersion % 16U));
	}
}

/*****************************************************************************/
/**
 * @brief	This function resets LpdInitialized variable to 0.
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_ResetLpdInitialized(void)
{
	if ((LpdInitialized & LPD_WDT_INITIALIZED) == LPD_WDT_INITIALIZED) {
		XPlmi_DisableWdt(XPLMI_WDT_EXTERNAL);
	}

	LpdInitialized = (u32)0U;
}
