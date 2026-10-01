/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xwdtps_g.c
* @brief Provides the configuration table for watchdog timer devices.
*
* This file contains a table that specifies the configuration of the watchdog
* timer devices in the system. Each device should have an entry in the table.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- ---------------------------------------------
* 1.00a ecm/jz 01/15/10 First release
* 3.00  kvn    02/13/15 Modified code for MISRA-C:2012 compliance.
* 3.10  vn     09/30/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

/** @addtogroup wdtps Overview
 * @{
 */

/***************************** Include Files *********************************/

#include "xwdtps.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/**
 * @brief Holds configuration information for each watchdog timer device in
 * the system.
 */
XWdtPs_Config XWdtPs_ConfigTable[XPAR_XWDTPS_NUM_INSTANCES] = {
	{
		(u16)XPAR_XWDTPS_0_DEVICE_ID,
		(u32)XPAR_XWDTPS_0_BASEADDR
	},
#ifdef XPAR_XWDTPS_1_DEVICE_ID
	{
		(u16)XPAR_XWDTPS_1_DEVICE_ID,
		(u32)XPAR_XWDTPS_1_BASEADDR
	}
#endif
};
/** @} */
