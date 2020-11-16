/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xis_gpio.h
*
* This is the main header file which contains definitions for the gpio.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who             Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00  Ana            10/11/20   First release
*
* </pre>
*
******************************************************************************/

#ifndef XIS_GPIO_H
#define XIS_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xis_main.h"

#if defined(XIS_UPDATE_A_B_MECHANISM) && defined(XPAR_XGPIOPS_NUM_INSTANCES)
#include "xgpiops.h"

/************************** Variable Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FW_UPDATE_BUTTON	(12U)

/************************** Function Prototypes ******************************/
int GpioInit(void);
u8 GetGpioStatus(void);
#endif /* end of XIS_UPDATE_A_B_MECHANISM */

#ifdef __cplusplus
}
#endif

#endif