/******************************************************************************
* Copyright (c) 2009 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpseudo_asm_iccarm.c
*
* This file contains functions for ARM register handling.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 3.12a asa  11/2/13 First Release
* 9.6   ml   10/01/26 Fix doxygen 1.15.0 and compliance checker warnings
* </pre>
*
******************************************************************************/

/***************************** Include Files ********************************/

#include "xpseudo_asm_iccarm.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/

/************************** Function Prototypes *****************************/

/**
 * @brief Read the Current Program Status Register (CPSR).
 *
 * @return Current value of the CPSR register.
 */
s32 mfcpsr (void)
{
	s32 rval;
	asm("mrs %0, cpsr" : "=r"(rval));
	return rval;
}
