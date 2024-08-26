/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpuf_init.c
*
* This file contains the initialization functions to be called by PLM. This
* file will only be part of XilPuf when it is compiled with PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kpt  01/04/22 Initial release
* 2.1   skg  10/29/22 Added In Body comments
* 2.3   ng   11/22/23 Fixed doxygen grouping
*
* </pre>
*
******************************************************************************/
/**
 * @addtogroup xpuf_server_apis XilPuf Server APIs
 * @{
 */
/***************************** Include Files *********************************/
#include "xplmi_config.h"
#ifdef PLM_PUF
#include "xpuf_cmd.h"
#include "xpuf_init.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function registers the handlers for Xilpuf IPI commands
 *
 *****************************************************************************/
void XPuf_Init(void)
{
	/** - Initialize XilPUF commands. */
	XPuf_CmdsInit();
}
#endif
