/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xtmr_inject_i.h
* @addtogroup Overview
* @{
*
* Contains data which is shared between the files of the XTMR_Inject component.
* It is intended for internal use only.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.0   sa   04/05/17 First release
* 1.9   ml   10/01/26 Fix doxygen 1.15.0 and compliance checker warnings
* </pre>
*
*****************************************************************************/

#ifndef XTMR_INJECT_I_H
#define XTMR_INJECT_I_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/

#include "xtmr_inject.h"
#include "xtmr_inject_l.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/

/* the configuration table */
extern XTMR_Inject_Config XTMR_Inject_ConfigTable[];

/************************** Function Prototypes *****************************/

#ifdef __cplusplus
}
#endif

#endif		/* end of protection macro */

/** @} */
