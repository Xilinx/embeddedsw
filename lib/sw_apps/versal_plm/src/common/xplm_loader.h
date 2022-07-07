/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplm_loader.h
*
* This file contains the declarations of platform loader wrapper functions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   08/20/2018 Initial release
* 1.01  ma   08/24/2019 Added code to force bootmode to SBI
*                       for slave SLRs
* 1.02  ana  02/29/2020 Implemented KAT support for crypto engines
*       kc   03/23/2020 Minor code cleanup
* 1.03  kc   06/12/2020 Added IPI mask to PDI CDO commands to get
*                       subsystem information
* 1.04  bm   12/16/2020 Removed KAT related macros
* 1.05  td   07/08/2021 Fix doxygen warnings
*       bsv  08/13/2021 Remove unnecessary header file
* 1.07  bsv  11/08/2021 Move XLoader_IsJtagSbiMode to Xilloader
* 1.08  bm   07/06/2022 Refactor versal and versal_net code
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XPLM_LOADER_H
#define XPLM_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_hw.h"
#include "xloader.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XPlm_LoadBootPdi(void *Arg);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPLM_LOADER_H */
