/******************************************************************************
* Copyright (C) 2002 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xuartlite_g.c
*
* @brief Configuration table for UART Lite devices in the system.
*
* Each device in the system should have an entry in the table.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a ecm  08/31/01 First release
* 1.00b jhl  02/21/02 Repartitioned the driver for smaller files
* 3.14  vn   09/09/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

/** @addtogroup uartlite Overview
 * @{
 */

/***************************** Include Files *********************************/

#include "xuartlite.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/**
 * @brief Holds the configuration table for UART Lite devices.
 */
XUartLite_Config XUartLite_ConfigTable[XPAR_XUARTLITE_NUM_INSTANCES] =
{
	{
		XPAR_UARTLITE_0_DEVICE_ID,	/* Unique ID of device */
		XPAR_UARTLITE_0_BASEADDR,	/* Device base address */
		XPAR_UARTLITE_0_BAUDRATE,	/* Fixed baud rate */
		XPAR_UARTLITE_0_USE_PARITY,	/* Fixed parity */
		XPAR_UARTLITE_0_ODD_PARITY,	/* Fixed parity type */
		XPAR_UARTLITE_0_DATA_BITS	/* Fixed data bits */
	},
};


/** @} */
