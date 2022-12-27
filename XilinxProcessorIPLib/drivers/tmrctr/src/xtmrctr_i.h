/******************************************************************************
* Copyright (C) 2002 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xtmrctr_i.h
* @addtogroup Overview
* @{
*
* This file contains data which is shared between files internal to the
* XTmrCtr component. It is intended for internal use only.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00b jhl  02/06/02 First release
* 1.10b mta  03/21/07 Updated to new coding style
* 2.00a ktn  10/30/09 _m is removed from all the macro definitions.
* </pre>
*
******************************************************************************/

#ifndef XTMRCTR_I_H		/* prevent circular inclusions */
#define XTMRCTR_I_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"

/************************** Constant Definitions *****************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

extern u8 XTmrCtr_Offsets[];

#ifdef __cplusplus
}
#endif

#endif
/** @} */
