/******************************************************************************
* Copyright (C) 2002 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xtmrctr_l.c
* @addtogroup Overview
* @{
*
* This file contains low-level driver functions that can be used to access the
* device.  The user should refer to the hardware device specification for more
* details of the device operation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00b jhl  04/24/02 First release
* 1.10b mta  03/21/07 Updated to new coding style
* 2.00a ktn  10/30/09 Updated to use HAL API's
* </pre>
*
******************************************************************************/


/***************************** Include Files *********************************/

#include "xtmrctr_l.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

/* The following table contains the offset from the base address of a timer
 * counter device for each timer counter.  A single device may contain multiple
 * timer counters and the functions specify which one to operate on.
 */
u8 XTmrCtr_Offsets[] = { 0, XTC_TIMER_COUNTER_OFFSET };
/** @} */
