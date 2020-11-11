/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc. All rights reserved.
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
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi.h"
#include "xplmi_err.h"
#include "xplmi_sysmon.h"
#include "xplmi_wdt.h"
#include "xplmi_hw.h"

/************************** Constant Definitions *****************************/
/**************************** Type Definitions *******************************/
typedef int (*XPlmi_InitHandler)(void);

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void XPlmi_RunTimeConfigInit(void);

/************************** Variable Definitions *****************************/
u8 LpdInitialized = (u8)0U;

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

	XPlmi_RunTimeConfigInit();
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
}

/*****************************************************************************/
/**
 * @brief	This function calls all the PS LPD init functions of all the different
 * modules. As a part of init functions, modules can register the
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
#ifdef XPAR_XIPIPSU_0_DEVICE_ID
		XPlmi_IpiInit,
#endif
		XPlmi_SysMonInit,
		XPlmi_PsEmInit,
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
	XPlmi_PrintPlmBanner();
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
	static u8 IsBannerPrinted = (u8)FALSE;
	u32 BootMode;
	u32 MultiBoot;

	if ((u8)FALSE == IsBannerPrinted) {
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
		PsVersion = ((Version & PMC_TAP_VERSION_PS_VERSION_MASK) >>
				PMC_TAP_VERSION_PS_VERSION_SHIFT);
		PmcVersion = ((Version & PMC_TAP_VERSION_PMC_VERSION_MASK) >>
				PMC_TAP_VERSION_PMC_VERSION_SHIFT);
		BootMode = XPlmi_In32(CRP_BOOT_MODE_USER) &
				CRP_BOOT_MODE_USER_BOOT_MODE_MASK;
		MultiBoot = XPlmi_In32(PMC_GLOBAL_PMC_MULTI_BOOT);

		XPlmi_Printf(DEBUG_PRINT_ALWAYS, "Platform Version: v%u.%u "
				 "PMC: v%u.%u, PS: v%u.%u\n\r",
				(PmcVersion / 16U), (PmcVersion % 16U),
				(PmcVersion / 16U), (PmcVersion % 16U),
				(PsVersion / 16U), (PsVersion % 16U));
		XPlmi_Printf(DEBUG_PRINT_ALWAYS, "BOOTMODE: %u, MULTIBOOT: 0x%x"
				"\n\r", BootMode, MultiBoot);
		XPlmi_Printf(DEBUG_PRINT_ALWAYS,
			"****************************************\n\r");
		IsBannerPrinted = (u8)TRUE;
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
		XPlmi_DisableWdt();
	}

	LpdInitialized = (u8)0U;
}
