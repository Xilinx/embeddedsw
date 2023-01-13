/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022-2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xis_loader.h
*
* This file contains the wrapper code xilloader
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

#ifndef XIS_LOADER_H
#define XIS_LOADER_H

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
int XPlm_LoadBootPdi(void);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XIS_LOADER_H */
