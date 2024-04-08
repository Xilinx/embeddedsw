/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsysmonpsv_direct.c
* @addtogroup sysmonpsv_api SYSMONPSV APIs
*
* The xsysmonpsv_direct.c file contains the function implementation for low level sysmon APIs.
*
*
* <pre>
*
* MODIFICATION HISTORY:
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------
* 3.0   cog    03/25/21 Driver Restructure
* 4.0   se     11/10/22 Secure and Non-Secure mode integration
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsysmonpsv_lowlevel.h"
#include "xil_io.h"

#if !defined(XSYSMONPSV_SECURE_MODE)
/******************************************************************************/
/**
 * Reads the register value.
 *
 * @param	InstancePtr Pointer to the driver instance.
 * @param	Offset Offset address for the register.
 * @param	Data Value to be read.
 *
 * @return	None.
 *
*******************************************************************************/
void XSysMonPsv_ReadReg32(XSysMonPsv *InstancePtr, u32 Offset, u32 *Data)
{
	*Data = Xil_In32(InstancePtr->Config.BaseAddress + Offset);
}

/******************************************************************************/
/**
 * Writes the register value.
 *
 * @param	InstancePtr Pointer to the driver instance.
 * @param	Offset Offset address of the register.
 * @param	Data Value to be written.
 *
 * @return	None.
 *
*******************************************************************************/
void XSysMonPsv_WriteReg32(XSysMonPsv *InstancePtr, u32 Offset, u32 Data)
{
	Xil_Out32(InstancePtr->Config.BaseAddress + Offset, Data);
}

/******************************************************************************/
/**
 * Updates the register value.
 *
 * @param	InstancePtr Pointer to the driver instance.
 * @param	Offset Offset addtress of the register.
 * @param	Mask Bits to be masked.
 * @param	Data Value to be written.
 *
 * @return	None.
 *
*******************************************************************************/
void XSysMonPsv_UpdateReg32(XSysMonPsv *InstancePtr, u32 Offset, u32 Mask,
			   u32 Data)
{
	u32 Val;

	XSysMonPsv_ReadReg32(InstancePtr, Offset, &Val);
	XSysMonPsv_WriteReg32(InstancePtr, Offset,
			     (Val & ~Mask) | (Mask & Data));
}
#endif
