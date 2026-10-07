/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xil_mmu.h
* This file only includes xil_mpu.h which contains Xil_SetTlbAttributes API
* defined for MPU in R5. R5 does not have mmu and for usage of similar API
* the file has been created.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 5.0	pkp  2/12/15 Initial version
* 9.6   ml   10/01/26 Fix doxygen 1.15.0 and compliance checker warnings
* </pre>
*
* @note
*
* None.
*
******************************************************************************/

#ifndef XIL_MMU_H
#define XIL_MMU_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/***************************** Include Files *********************************/

#include "xil_mpu.h"

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* XIL_MMU_H */
