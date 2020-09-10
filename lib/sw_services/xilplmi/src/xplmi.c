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

/************************** Constant Definitions *****************************/
/**************************** Type Definitions *******************************/
typedef int (*XPlmiInit)(void);

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void XPlm_PrintPlmBanner(void);

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
int XPlmi_Init(void )
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

/*
 * It contains all the PS LPD init functions to be run for every module that
 * is present as a part of PLM.
 */
static const XPlmiInit LpdInitList[] = {
#ifdef DEBUG_UART_PS
	XPlmi_InitUart,
#endif
#ifdef XPAR_XIPIPSU_0_DEVICE_ID
	XPlmi_IpiInit,
#endif
	XPlmi_SysMonInit,
	XPlmi_PsEmInit,
};

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
void XPlmi_RunTimeConfigInit(void)
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
	XPlm_PrintPlmBanner();
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
void XPlm_PrintPlmBanner(void)
{
	u32 Version;
	u32 PsVersion;
	u32 PmcVersion;
	static u8 IsBannerPrinted = FALSE;

	if (FALSE == IsBannerPrinted) {
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
		XPlmi_Printf(DEBUG_PRINT_ALWAYS, "Platform Version: v%u.%u "
					 "PMC: v%u.%u, PS: v%u.%u\n\r",
					(PmcVersion / 16U), (PmcVersion % 16U),
					(PmcVersion / 16U), (PmcVersion % 16U),
					(PsVersion / 16U), (PsVersion % 16U));
#ifdef DEBUG_UART_MDM
		XPlmi_Printf(DEBUG_PRINT_ALWAYS, "STDOUT: MDM UART\n\r");
#else
		XPlmi_Printf(DEBUG_PRINT_ALWAYS, "STDOUT: PS UART\n\r");
#endif
		XPlmi_Printf(DEBUG_PRINT_ALWAYS,
                 "****************************************\n\r");
		IsBannerPrinted = TRUE;
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
