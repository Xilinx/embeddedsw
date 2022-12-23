/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xocp_init.c
*
* This file contains the initialization functions to be called by PLM. This
* file will only be part of XilOcp when it is compiled with PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.1   am   12/21/22 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_config.h"
#ifdef PLM_OCP
#include "xocp_cmd.h"
#include "xocp_init.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief   This function registers the handlers for XilOcp IPI commands
 *
 *****************************************************************************/
void XOcp_Init(void)
{
	/**
	 *  Intialize XilOCP commands
	 */
	XOcp_CmdsInit();
}
#endif /* PLM_OCP */