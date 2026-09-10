/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xloader_ddr.h
*
* This is the header file which contains DDR interface declarations
* for the xilloader.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   03/12/2019 Initial release
* 1.01  kc   09/04/2019 Added support to use non blocking DMA in
*						DdrCopy function
* 1.02  bsv  04/09/2020 Code clean up of Xilloader
* 1.03  skd  07/14/2020 Added 64bit support for DDR source address
*       bsv  10/13/2020 Code clean up
* 1.04  bsv  08/26/2021 Code clean up
*       dd   09/11/2023 MISRA-C violation Rule 17.8 fixed
 *      pre  08/17/2026 Added DDR calibration data restore and status check APIs
 *      pre  09/06/2026 Include xplmi_cmd.h for XPlmi_Cmd
 *
 * </pre>
*
* @note
*
******************************************************************************/

#ifndef XLOADER_DDR_H
#define XLOADER_DDR_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_cmd.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XLoader_DdrInit(u32 DeviceFlags);
int XLoader_DdrCopy(u64 SrcAddr, u64 DestAddr, u32 Length, u32 FlagsVal);
int XLoader_DdrRelease(void);
int XLoader_CalibDataRestore(XPlmi_Cmd *Cmd);
#ifdef XPAR_CALIBDATA_STORAGEDEVICE
int XLoader_CheckDdrMcCalibStatus(u32 BaseAddr);
u32 XLoader_IsWarmBootFlagSet(u32 BaseAddr);
#endif

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_DDR_H */
