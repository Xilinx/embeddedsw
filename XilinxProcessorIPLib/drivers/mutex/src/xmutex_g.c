/******************************************************************************
* Copyright (C) 2007 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xmutex_g.c
* @brief Defines the Mutex configuration table.
*
* This file contains a configuration table that specifies the configuration
* of Mutex devices in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 1.00a ecm  06/01/07 Cleanup, new coding standard, check into XCS
* 4.12  tb   09/11/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

/**
* @addtogroup mutex_api MUTEX APIs
* @{
*/

/***************************** Include Files *********************************/
#include "xmutex.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Prototypes ******************************/

XMutex_Config XMutex_ConfigTable[] = {
	{
		XPAR_MUTEX_0_DEVICE_ID,
		XPAR_MUTEX_0_BASEADDR,
		XPAR_MUTEX_0_NUM_MUTEX,
		XPAR_MUTEX_0_ENABLE_USER
	}
};
/** @} */
