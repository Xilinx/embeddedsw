/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplm_proc.h
*
* This file contains declarations for PROC C file in PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   03/27/2018 Initial release
* 1.01  kc   03/23/2020 Minor code cleanup
* 1.02  bm   07/17/2023 Removed XPlm_InitProc prototype, Added XPlm_ExceptionInit
*                       prototype
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPLM_PROC_H
#define XPLM_PROC_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
void XPlm_ExceptionInit(void);

#ifdef __cplusplus
}
#endif

#endif  /* XPLM_PROC_H */
