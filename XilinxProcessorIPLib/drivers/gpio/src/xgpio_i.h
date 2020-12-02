/******************************************************************************
* Copyright (C) 2002 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
* @file xgpio_i.h
* @addtogroup gpio_v4_7
* @{
*
* This header file contains internal identifiers, which are those shared
* between the files of the driver. It is intended for internal use only.
*
* NOTES:
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a rmm  03/13/02 First release
* 2.11a mta  03/21/07 Updated to new coding style
* </pre>
******************************************************************************/

#ifndef XGPIO_I_H		/* prevent circular inclusions */
#define XGPIO_I_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xgpio.h"

/************************** Constant Definitions ****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions ****************************/

extern XGpio_Config XGpio_ConfigTable[];

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
