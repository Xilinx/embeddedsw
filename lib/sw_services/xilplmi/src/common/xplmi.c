/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
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
* 1.06  bm   07/06/2022 Refactor versal and versal_net code
*       kpt  07/19/2022 Added APIs to update or get KAT status from RTC area
*       bm   07/22/2022 Update EAM logic for In-Place PLM Update
* 1.07  ng   11/11/2022 Updated doxygen comments
*       kpt  01/04/2023 Added XPlmi_SetFipsKatMask command
*       bm   01/14/2023 Remove bypassing of PLM Set Alive during boot
*       bm   03/11/2023 Added status check for XPlmi_PreInit
* 1.08  sk   07/18/2023 Warn out for uart init fail
*       sk   07/26/2023 Added redundant check for XPlmi_IsPlmUpdateDone
*       dd   09/12/2023 MISRA-C violation Rule 10.3 fixed
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
#include "xplmi_plat.h"
#include "xplmi_wdt.h"
#include "xil_util.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void XPlmi_PrintEarlyLog(void);

/************************** Variable Definitions *****************************/

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

	Status = XPlmi_PreInit();
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_PRE_INIT, Status);
		goto END;
	}

	Status = XPlmi_SetUpInterruptSystem();
	if (Status != XST_SUCCESS) {
		goto END;
	}
	XPlmi_GenericInit();
	Xil_RegisterPlmHandler(XPlmi_SetPlmLiveStatus);

END:
	return Status;
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

	/* In-Place PLM Update is applicable only for versalnet */
	if ((XPlmi_IsPlmUpdateDone() == (u8)TRUE) || (XPlmi_IsPlmUpdateDoneTmp() == (u8)TRUE)) {
		XPlmi_Out32(XPLMI_RTCFG_SECURE_STATE_ADDR, DevSecureState);
		Status = XST_SUCCESS;
		goto END;
	}

	/**
	 * Clear the Runtime configuration area
	*/
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

	/**
	 * Set the default PLM Run time configuration values
	*/
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
	XPlmi_Out32(XPLMI_RTCFG_IMG_STORE_ADDRESS_HIGH, XPLMI_RTCFG_IMG_STORE_ADDRESS_HIGH_INVALID);
	XPlmi_Out32((XPLMI_RTCFG_IMG_STORE_ADDRESS_LOW), XPLMI_RTCFG_IMG_STORE_ADDRESS_LOW_INVALID);
	XPlmi_Out32(XPLMI_RTCFG_IMG_STORE_SIZE, XPLMI_RTCFG_IMG_STORE_SIZE_INVALID);
	/**
	 * Initialize platform specific RTCA Registers
	 */
	XPlmi_RtcaPlatInit();

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
		Status |=  XPLMI_WARNING_MAJOR_MASK;
		XPlmi_LogPlmErr(Status);
	}
#endif
	/* For versal, PLM Update is not applicable, and this API returns FALSE */
	if (XPlmi_IsPlmUpdateDone() != (u8)TRUE) {
		Status = XPlmi_PsEmInit();
		if (Status != XST_SUCCESS) {
			goto END;
		}
		XPlmi_SetLpdInitialized(LPD_INITIALIZED);
		XPlmi_PrintEarlyLog();
	}

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
	XPlmi_Printf(DEBUG_PRINT_ALWAYS, XPLMI_PLM_BANNER);
	XPlmi_Printf(DEBUG_PRINT_ALWAYS,
		"Release %s.%s   %s  -  %s\n\r",
		SDK_RELEASE_YEAR, SDK_RELEASE_QUARTER, __DATE__, __TIME__);

	/* For versal, PLM Update is not applicable, and this API returns FALSE */
	if (XPlmi_IsPlmUpdateDone() != (u8)TRUE) {
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
	}
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
 * @brief	This function resets LpdInitialized variable to 0.
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_ResetLpdInitialized(void)
{
	u32 *LpdInitializedPtr = XPlmi_GetLpdInitialized();

	if ((*LpdInitializedPtr & LPD_WDT_INITIALIZED) == LPD_WDT_INITIALIZED) {
		XPlmi_DisableWdt(XPLMI_WDT_EXTERNAL);
	}

	*LpdInitializedPtr = (u32)0U;
}

/*****************************************************************************/
/**
 * @brief	This function sets LpdInitialized variable with given flag.
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_SetLpdInitialized(u32 Flag)
{
	u32 *LpdInitializedPtr = XPlmi_GetLpdInitialized();

	*LpdInitializedPtr |= Flag;
}

/*****************************************************************************/
/**
 * @brief	This function clears LpdInitialized variable with given flag.
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_UnSetLpdInitialized(u32 Flag)
{
	u32 *LpdInitializedPtr = XPlmi_GetLpdInitialized();

	*LpdInitializedPtr &= (u32)(~Flag);
}

/*****************************************************************************/
/**
 * @brief	This function sets XPLMI_RTCFG_PLM_KAT_ADDR with
 *          PlmKatStatus.
 *
 * @param	PlmKatStatus contains the KAT status updated by PLM
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_UpdateKatStatus(u32 PlmKatStatus)
{
	XPlmi_UtilRMW(XPLMI_RTCFG_PLM_KAT_ADDR, XPLMI_KAT_MASK, PlmKatStatus);
	/* Applicable only for VersalNet */
	(void)XPlmi_CheckAndUpdateFipsState();
}

/*****************************************************************************/
/**
 * @brief	This function is called to get KAT status.
 *
 * @return	None
 *
 *****************************************************************************/
u32 XPlmi_GetKatStatus(void)
{
	return (XPlmi_In32(XPLMI_RTCFG_PLM_KAT_ADDR) & XPLMI_KAT_MASK);
}

/*****************************************************************************/
/**
 * @brief	This function sets XPLMI_RTCFG_PLM_KAT_ADDR  with
 *          PlmKatMask.
 *
 * @param   PlmKatMask contains the kat mask that needs to be set
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_SetKatMask(u32 PlmKatMask)
{
	u32 PlmKatStatus = XPlmi_GetKatStatus();

	PlmKatStatus |= PlmKatMask;
	XPlmi_UpdateKatStatus(PlmKatStatus);
}

/*****************************************************************************/
/**
 * @brief	This function clears XPLMI_RTCFG_PLM_KAT_ADDR with
 *          PlmKatMask.
 *
 * @param   PlmKatMask contains the kat mask that needs to be cleared
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_ClearKatMask(u32 PlmKatMask)
{
	u32 PlmKatStatus = XPlmi_GetKatStatus();

	PlmKatStatus &= ~PlmKatMask;
	XPlmi_UpdateKatStatus(PlmKatStatus);
}

/*****************************************************************************/
/**
 * @brief	This function will return the crypto kat enable status from efuse
 *          cache.
 *
 * @return
 *			TRUE  If crypto kat bit is set
 *			FALSE If crypto kat bit is not set
 *
 *****************************************************************************/
u8 XPlmi_IsCryptoKatEn(void)
{
	u8 CryptoKatEn = (u8)((XPlmi_In32(EFUSE_CACHE_MISC_CTRL) &
						XPLMI_EFUSE_CACHE_CRYPTO_KAT_EN_MASK) >>
						XPLMI_EFUSE_CACHE_CRYPTO_KAT_EN_SHIFT);

	return CryptoKatEn;
}
