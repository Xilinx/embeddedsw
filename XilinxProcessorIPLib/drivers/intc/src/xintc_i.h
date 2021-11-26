/******************************************************************************
* Copyright (C) 2002 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xintc_i.h
* @addtogroup intc_v3_14
* @{
*
* This file contains data which is shared between files and internal to the
* XIntc component. It is intended for internal use only.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00b jhl  02/06/02 First release
* 1.00b jhl  04/24/02 Moved register definitions to xintc_l.h
* 1.00c rpm  10/17/03 New release. Removed extern of global, single instance
*                     pointer.
* 1.10c mta  03/21/07 Updated to new coding style
* 2.00a ktn  10/20/09 Updated to use HAL Processor APIs.
* </pre>
*
******************************************************************************/

#ifndef XINTC_I_H		/* prevent circular inclusions */
#define XINTC_I_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xintc.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

extern u32 XIntc_BitPosMask[];

extern XIntc_Config XIntc_ConfigTable[];

#ifdef __cplusplus
}
#endif

#endif
/** @} */
