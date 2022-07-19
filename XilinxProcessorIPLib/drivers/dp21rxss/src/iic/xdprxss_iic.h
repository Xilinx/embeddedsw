/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdprxss_iic.h
*
* This is the header file for Xilinx DisplayPort Receiver Subsystem sub-core,
* is IIC.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- -----------------------------------------------------
* 1.00 sha 05/18/15 Initial release.
* </pre>
*
******************************************************************************/
#ifndef XDPRXSS_IIC_H_
#define XDPRXSS_IIC_H_		/**< Prevent circular inclusions
				  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#ifdef XPAR_XIIC_NUM_INSTANCES
#include "xiic.h"
#endif
#ifdef XPAR_XIICPS_NUM_INSTANCES
#include "xiicps.h"
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
