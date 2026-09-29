/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef _XSYSMONPSV_LOWLEVEL_H_
#define _XSYSMONPSV_LOWLEVEL_H_

/*****************************************************************************/
/**
*
* @file xsysmonpsv_lowlevel.h
*
* @brief Declares the low-level register access functions for the driver.
*
* The xsysmonpsv_lowlevel.h header file declares low-level driver functions
* used to read and write SysMon registers.
*
* <pre>
* MODIFICATION HISTORY:
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------
* 3.0   cog    03/25/21 Driver Restructure
* 4.0   se     11/10/22 Secure and Non-Secure mode integration
* 5.4   vn     09/29/26 Align Doxygen comments with AMD standards
*
* </pre>
*
******************************************************************************/

/**
 * @addtogroup sysmonpsv_api SYSMONPSV APIs
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xsysmonpsv_driver.h"

/************************** Function Prototypes ******************************/
void XSysMonPsv_ReadReg32(XSysMonPsv *InstancePtr, u32 Offset, u32 *Data);
void XSysMonPsv_WriteReg32(XSysMonPsv *InstancePtr, u32 Offset, u32 Data);
void XSysMonPsv_UpdateReg32(XSysMonPsv *InstancePtr, u32 Offset, u32 Mask,
			   u32 Data);

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* _XSYSMONPSV_LOWLEVEL_H_ */
