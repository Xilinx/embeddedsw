/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsysmonpsv_sinit.c
*
* @brief Implements the static initialization functions for the XSysMonPsv
* driver.
*
* The xsysmonpsv_sinit.c file provides the minimum required functions for the
* XSysMonPsv driver.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date         Changes
* ----- -----  -------- -----------------------------------------------
* 1.0   aad    20/11/18 First release.
* 2.3   aad    07/26/21 Fixed doxygen comments.
* 3.0   cog    03/25/21 Driver Restructure
* 4.1   cog    07/18/23 Add support for SDT flow
* 4.2   cog    02/27/24 Fixed issue with XSysMonPsv_LookupConfig
* 5.4   vn     09/29/26 Align Doxygen comments with AMD standards
*
* </pre>
*
******************************************************************************/

/**
 * @addtogroup sysmonpsv_api SYSMONPSV APIs
 * @{
 */

/***************************** Include Files *********************************/

#include "lowlevel/xsysmonpsv_hw.h"
#include "xsysmonpsv.h"
#ifndef SDT
#include "xparameters.h"
#endif

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

/************************** Variable Definitions ****************************/
extern XSysMonPsv_Config XSysMonPsv_ConfigTable[]; /**< Config Table for
                                                          sysmon device */

/*****************************************************************************/
/**
*
* @brief Looks up the device configuration based on the unique device ID.
*
* The table XSysMonPsv_ConfigTable[] contains the configuration information
* for each device in the system.
*
* @return      A pointer to the configuration table entry corresponding to
*              the given device, or NULL if no match is found.
*
******************************************************************************/
XSysMonPsv_Config *XSysMonPsv_LookupConfig(void)
{
	XSysMonPsv_Config *CfgPtr = NULL;

	CfgPtr = &XSysMonPsv_ConfigTable[0];

	return CfgPtr;
}
/** @} */
