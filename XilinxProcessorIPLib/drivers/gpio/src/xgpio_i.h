/******************************************************************************
* Copyright (C) 2002 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XGPIO_I_H
#define XGPIO_I_H

/******************************************************************************/
/**
* @file xgpio_i.h
* @brief Declares internal identifiers shared among XGpio driver files.
*
* Contains internal identifiers, which are shared among the files of the
* driver. It is intended for internal use only.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a rmm  03/13/02 First release
* 2.11a mta  03/21/07 Updated to new coding style
* 4.14  vn   09/10/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

/** @addtogroup gpio_api GPIO APIs
 * @{
 */

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

/**
 * @brief Holds the configuration entry for each GPIO instance.
 */
extern XGpio_Config XGpio_ConfigTable[];

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* end of protection macro */
