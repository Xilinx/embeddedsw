/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
* @file xsysmonpsv_lowlevel.h
* @addtogroup sysmonpsv_v3_0
*
* The user should refer to the hardware device specification for detailed
* information about the device.
*
* This header file contains low level driver functions that are use to read and
* write sysmon registers.
*
*
* <pre>
* MODIFICATION HISTORY:
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------
* 3.0   cog    03/25/21 Driver Restructure
*
* </pre>
*
******************************************************************************/

#ifndef _XSYSMONPSV_LOWLEVEL_H_
#define _XSYSMONPSV_LOWLEVEL_H_

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
#endif /* _XSYSMONPSV_LOWLEVEL_H_ */
