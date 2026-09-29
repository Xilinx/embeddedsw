/******************************************************************************
* Copyright (C) 2017 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xresetps_g.c
*
* @brief Specifies the configuration of the reset controller devices.
*
* Each device in the system should have an entry in the configuration table.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- ---------------------------------------------
* 1.00  cjp    09/05/17 First release
* 1.2   cjp    04/27/18 Updated for clockps interdependency
* 1.2   sd     07/20/18 Fixed Doxygen warnings
* 1.8   vlt    01/27/26 Fixed codespell issues
* 1.9  vn   09/10/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

/** @addtogroup resetps Overview
 * @{
 */

/***************************** Include Files *********************************/

#include "xresetps.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/**
 * @brief Configuration table for the reset controller.
 *
 * Contains configuration information for each reset controller device in the
 * system.
 *
 * @note        This is a dummy instance because the reset system does not
 *              have a dedicated controller.
 */
XResetPs_Config XResetPs_ConfigTable[XPAR_XRESETPS_NUM_INSTANCES] = {
	{
		(u16)XPAR_XRESETPS_DEVICE_ID,
	}
};
/** @} */
