/******************************************************************************
* Copyright (C) 2002 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xuartns550_g.c
* @brief Defines the configuration table for NS16550 devices.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a ecm  08/16/01 First release
* 1.00b jhl  03/11/02 Repartitioned driver for smaller files.
* 1.11a sv   03/20/07 Updated to use the new coding guidelines.
* 3.15  vn   09/09/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

/** @addtogroup uartns550 Overview
 * @{
 */

/***************************** Include Files *********************************/

#include "xuartns550.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/**
 * @brief Configuration table for UART 16550/16450 devices.
 *
 * Each device in the system should have an entry in this table.
 */
XUartNs550_Config XUartNs550_ConfigTable[] =
{
	{
		XPAR_UARTNS550_0_DEVICE_ID,
		XPAR_UARTNS550_0_BASEADDR,
		XPAR_UARTNS550_0_CLOCK_HZ
	}
};
/** @} */
