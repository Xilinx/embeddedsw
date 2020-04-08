/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnandpsu_g.c
* @addtogroup nandpsu_v1_6
* @{
*
* This file contains a configuration table where each entry is a configuration
* structure for an XNandPsu device in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date	   Changes
* ----- ----   ----------  -----------------------------------------------
* 1.0   nm     05/06/2014  First release
* 1.0   nm     06/02/2014  Changed the copyright to new copyright
* </pre>
*
******************************************************************************/

/***************************** Include Files ********************************/
#include "xparameters.h"
#include "xnandpsu.h"
/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/

/**
 * Each XNandPsu device in the system has an entry in this table.
 */
XNandPsu_Config XNandPsu_ConfigTable[] = {
	{
		0U,
		(u32)XPAR_XNANDPSU_0_BASEADDR
	}
};
/** @} */
