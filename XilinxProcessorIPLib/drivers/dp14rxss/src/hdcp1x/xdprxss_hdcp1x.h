/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdprxss_hdcp1x.h
*
* This is the header file for Xilinx DisplayPort Receiver Subsystem sub-core,
* is High-Bandwidth Content Protection (HDCP).
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- ----------------------------------------------------------
* 1.00 sha 05/18/15 Initial release.
* 2.00 sha 10/05/15 Protected HDCP, Timer Counter header file under macro
*                    number of instances.
* </pre>
*
******************************************************************************/
#ifndef XDPRXSS_HDCP1X_H_
#define XDPRXSS_HDCP1X_H_	/**< Prevent circular inclusions
				  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xparameters.h"
#if (XPAR_XHDCP_NUM_INSTANCES > 0)
#include "xhdcp1x.h"
#include "xtmrctr.h"
#endif

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
