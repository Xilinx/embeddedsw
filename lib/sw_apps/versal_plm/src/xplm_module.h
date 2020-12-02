/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplm_module.h
*
* This file contains the headers for startup code of PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ====  ==== ======== ======================================================-
* 1.00  kc   08/28/2018 Initial release
* 1.01  sn   07/04/2019 Added code to enable SysMon's over-temperature
*						interrupt
*       kc   08/01/2019 Added error management module in PLM
*       ma   08/01/2019 Removed LPD module init related code from PLM app
* 1.02  kc   03/23/2020 Minor code cleanup
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XPLM_MODULE_H
#define XPLM_MODULE_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XPlm_ModuleInit(void *Arg);

/************************** Variable Definitions *****************************/

/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPLM_MODULE_H */
