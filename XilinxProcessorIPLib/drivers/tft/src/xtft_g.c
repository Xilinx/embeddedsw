/******************************************************************************
* Copyright (C) 2008 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/***************************************************************************/
/**
*
* @file xtft_g.c
* @brief Provides the XTft configuration table.
*
* This file contains a configuration table that specifies the parameters of
* TFT devices.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver    Who   Date      Changes
* -----  ----  --------  -----------------------------------------------
* 1.00a  sg    03/24/08  First release
* 4.00a  bss   01/25/13  Removed the XPAR_TFT_0_DCR_SPLB_SLAVE_IF and
*			 XPAR_TFT_0_DCR_BASEADDR as AXI TFT controller
*			 does not support them
* 6.8    vlt   01/27/26  Fixed codespell issues
* 6.9   tb   09/16/26 Align Doxygen comments with AMD standards.
* </pre>
*
****************************************************************************/

/**
* @addtogroup tft_api TFT APIs
* @{
*/

/***************************** Include Files *******************************/
#include "xtft.h"
#include "xparameters.h"

/************************** Constant Definitions ***************************/

/**************************** Type Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *******************/

/************************** Function Prototypes ****************************/

/************************** Variable Definitions ****************************/
/**
 * The configuration table for TFT devices in the table. Each
 * device should have an entry in this table.
 */
XTft_Config XTft_ConfigTable[] =
{
	{
		XPAR_TFT_0_DEVICE_ID,
		XPAR_TFT_0_BASEADDR,
		XPAR_TFT_0_DEFAULT_TFT_BASE_ADDR,
		XPAR_TFT_0_ADDR_WIDTH
	}
};

/************************** Function Definitions ****************************/
/** @} */
