/******************************************************************************
* Copyright (C) 2017 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xresetps_sinit.c
*
* @brief Implements the static (compile-time) initialization of the driver.
*
* <pre>
* MODIFICATION HISTORY:
* Ver   Who    Date     Changes
* ----- ------ -------- ----------------------------------------------
* 1.00  cjp    09/05/17 First release
* 1.1   Nava   04/20/18 Fixed compilation warnings.
* 1.2   cjp    04/27/18 Updated for clockps interdependency
* 1.5   sd     07/07/23 Added SDT support.
* 1.8   vlt    12/16/25 Update Doxygen comments to include SDT flow details.
* 1.8   vlt    03/14/26 Updated BaseAddress type from u32 to UINTPTR
*                       to support 64-bit addressing.
* 1.9   vn     09/10/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

/** @addtogroup resetps Overview
 * @{
 */

/***************************** Include Files *********************************/

#include "xresetps.h"
#ifndef SDT
#include "xparameters.h"
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/*************************** Variable Definitions ****************************/
/**
 * @brief Holds the configuration entry for each reset instance.
 */
#ifndef SDT
extern XResetPs_Config XResetPs_ConfigTable[XPAR_XRESETPS_NUM_INSTANCES];
#else
extern XResetPs_Config XResetPs_ConfigTable[];
#endif

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
*
* @brief Looks up the device configuration by unique device ID or base address.
*
* XResetPs_ConfigTable[] contains the configuration information for each
* device in the system.
*
* @if SDT
* @param[in]   BaseAddress Base address of the device.
* @else
* @param[in]   DeviceId Unique ID of the device.
* @endif
*
* @return      A pointer to the configuration found, or NULL if the specified
*              device ID or base address is not found. See xresetps.h for the
*              definition of XResetPs_Config.
*
* @note        In the XSCT/classic flow, the function uses DeviceId to look up
*              the device configuration.
*
******************************************************************************/
#ifndef SDT
XResetPs_Config *XResetPs_LookupConfig(u16 DeviceId)
{
	XResetPs_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; Index < (u32)XPAR_XRESETPS_NUM_INSTANCES; Index++) {
		if (XResetPs_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XResetPs_ConfigTable[Index];
			break;
		}
	}
	return (XResetPs_Config *)CfgPtr;
}
#else
XResetPs_Config *XResetPs_LookupConfig(UINTPTR BaseAddress)
{
	XResetPs_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; XResetPs_ConfigTable[Index].Name != NULL; Index++) {
		if ((XResetPs_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XResetPs_ConfigTable[Index];
			break;
		}
	}
	return (XResetPs_Config *)CfgPtr;
}
#endif
/** @} */
