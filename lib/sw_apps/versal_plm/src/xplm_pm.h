/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplm_pm.n
*
* This file contains the header functions of wrapper xilpm
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   07/20/2018 Initial release
* 1.01  rp   08/08/2019 Added code to send PM notify callback through IPI
* 1.02  kc   03/23/2020 Minor code cleanup
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XPLM_PM_H
#define XPLM_PM_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

int XPlm_PmInit(void);
int XPlm_ProcessPlmCdo(void *Arg);

#ifdef __cplusplus
}
#endif

#endif  /* XPLM_PM_H */
