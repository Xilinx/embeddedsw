/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xresetps_g.c
* @addtogroup xresetps_v1_4
* @{
*
* This file contains a table that specifies the configuration of the reset
* controller devices in the system. Each device should have an entry in the
* table.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- ---------------------------------------------
* 1.00  cjp    09/05/17 First release
* 1.2   cjp    04/27/18 Updated for clockps interdependency
* 1.2   sd     07/20/18 Fixed Doxygen warnings
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xresetps.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/**
 * This table contains configuration information for each reset controller
 * device in the system.
 *
 * Note:
 * This is a dummy instance since reset system doesnot have a dedicated
 * controller
 */
XResetPs_Config XResetPs_ConfigTable[XPAR_XRESETPS_NUM_INSTANCES] = {
	{
		(u16)XPAR_XRESETPS_DEVICE_ID,
	}
};
/** @} */
