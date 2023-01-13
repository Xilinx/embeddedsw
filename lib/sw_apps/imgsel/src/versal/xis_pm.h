/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022-2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xis_pm.h
*
* This file contains the header functions of wrapper xilpm
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  skd  01/13/23 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XIS_PM_H
#define XIS_PM_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/
#define XPLM_NOCPLL_CFG_VAL		(0x7E5DCC65U)
#define XPLM_NOCPLL_CTRL_VAL		(0x34809U)
#define NOCPLL_TIMEOUT			(100000U)
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XPlm_PmInit(void);
int XPlm_ProcessPmcCdo(void);
int XPlm_ConfigureDefaultNPll(void);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XIS_PM_H */
