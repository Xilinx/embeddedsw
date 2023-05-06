/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc. All rights reserved.
* (c) Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplm_sem_init.h
*
* This file contains the xilsem interfaces of PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  rm   09/22/2019 Initial release
* 1.01  kc   02/10/2020 Updated scheduler to add/remove tasks
*       kc   02/17/2020 Added configurable priority for scheduler tasks
*       kc   02/26/2020 Added XPLM_SEM macro to include/disable SEM
*                       functionality
*       kc   03/23/2020 Minor code cleanup
* 1.02  rb   01/28/2021 Added Sem PreInit prototype, updated header file
*       rb   03/09/2021 Updated Sem Init API call
* 1.03  ga   05/03/2023 Removed XPlm_SemInit prototype and updated
*                       copyright information
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XPLM_SEM_INIT_H
#define XPLM_SEM_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_hw.h"

#ifdef XPLM_SEM

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XPlm_SemScanInit(void *Arg);

/************************** Variable Definitions *****************************/

/*****************************************************************************/

#endif

#ifdef __cplusplus
}
#endif

#endif  /* XPLM_INIT_SEM_H */
