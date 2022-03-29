/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsysmonpsv_direct.c
* @addtogroup sysmonpsv_v3_0
*
* This file contains the function implementation for low level sysmon APIs.
*
* @note		None.
*
* <pre>
*
* MODIFICATION HISTORY:
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------
* 3.0   cog    03/25/21 Driver Restructure
*
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsysmonpsv_lowlevel.h"
#include "xil_io.h"

/******************************************************************************/
/**
 * This function reads register value.
 *
 * @param	InstancePtr is a pointer to the driver instance.
 * @param	Offset is offset address for register.
 * @param	Data is value to be read in.
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
 * This function writes register value.
 *
 * @param	InstancePtr is a pointer to the driver instance.
 * @param	Offset is offset address of the register.
 * @param	Data is value to be written.
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
 * This function updates register value.
 *
 * @param	InstancePtr is a pointer to the driver instance.
 * @param	Offset offset addtress of the register.
 * @param	Mask is bits to be masked.
 * @param	Data is value to be written.
 *
 * @return	None.
 *
*******************************************************************************/
void XSysMonPsv_UpdateReg32(XSysMonPsv *InstancePtr, u32 Offset, u32 Mask,
			  u32 Data)
{
	u32 Val;

	XSysMonPsv_ReadReg32(InstancePtr, Offset, &Val);
	XSysMonPsv_WriteReg32(InstancePtr, Offset, (Val & ~Mask) | (Mask & Data));
}
