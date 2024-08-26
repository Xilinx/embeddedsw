/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnvm_init.c
*
* This file contains the initialization functions to be called by PLM. This
* file will only be part of XilNvm when it is compiled with PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kal 07/05/2021 Initial release
* 2.4   bsv 09/09/2021 Added PLM_NVM macro
* 3.3   kpt 02/21/2024 Added support to extend secure state
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_NVM
#include "xnvm_cmd.h"
#include "xnvm_init.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function registers the handlers for Xilnvm IPI commands
 *
 * @param	GenericHandler Pointer to generic handler
 *
 * @return	- XST_SUCCESS - On success
 *
 *****************************************************************************/
void XNvm_Init(int (*GenericHandler)(void))
{
	XNvm_CmdsInit(GenericHandler);
}

#endif
