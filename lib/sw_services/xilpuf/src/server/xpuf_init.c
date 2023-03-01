/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpuf_init.c
* @addtogroup xpuf_server_api_ XilPuf Server API
* @{
* @details
*
* This file contains the initialization functions to be called by PLM. This
* file will only be part of XilPuf when it is compiled with PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kpt 01/04/2022 Initial release
* 2.1   skg 10/29/2022 Added In Body comments
*
* </pre>
*
* @note
*
******************************************************************************/

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
 *
 *****************************************************************************/
void XPuf_Init(void)
{
	/**
	 *  Intialize XilPUF commands
	 */
	XPuf_CmdsInit();
}
#endif
