/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsysmonpsv_direct.c
*
* @brief Implements the low-level register access APIs for SysMon.
*
* The xsysmonpsv_direct.c file implements the low-level SysMon register access
* functions.
*
* <pre>
*
* MODIFICATION HISTORY:
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------
* 3.0   cog    03/25/21 Driver Restructure
* 4.0   se     11/10/22 Secure and Non-Secure mode integration
* 5.3   se     03/12/26 Enhancements: NULL checks, range validation,
*                       standardize return values to XST_FAILURE
*       se     03/13/26 Fix secure mode and PCSR re-lock in SDT flow
* 5.4   vn     07/30/26 Fix Doxygen warnings
* 5.4   vn     09/29/26 Align Doxygen comments with AMD standards
*
* </pre>
*
******************************************************************************/

/**
 * @addtogroup sysmonpsv_api SYSMONPSV APIs
 * @{
 */

/***************************** Include Files *********************************/
#include "xsysmonpsv_lowlevel.h"
#include "xil_io.h"
#include "xsysmonpsv_common.h"
#include "xsysmonpsv_hw.h"

#if !defined(XSYSMONPSV_SECURE_MODE)
/******************************************************************************/
/**
 * @brief Reads the register value.
 *
 * @param[in]   InstancePtr Pointer to the driver instance.
 * @param[in]   Offset Offset address of the register.
 * @param[out]  Data Value read from the register.
 *
*******************************************************************************/
void XSysMonPsv_ReadReg32(XSysMonPsv *InstancePtr, u32 Offset, u32 *Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Data != NULL);
	*Data = Xil_In32(InstancePtr->Config.BaseAddress + Offset);
}

/******************************************************************************/
/**
 * @brief Writes the register value.
 *
 * @param[in]   InstancePtr Pointer to the driver instance.
 * @param[in]   Offset Offset address of the register.
 * @param[in]   Data Value to be written.
 *
*******************************************************************************/
void XSysMonPsv_WriteReg32(XSysMonPsv *InstancePtr, u32 Offset, u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_Out32(InstancePtr->Config.BaseAddress + XSYSMONPSV_PCSR_LOCK,
		  XSYSMONPSV_LOCK_CODE);
	Xil_Out32(InstancePtr->Config.BaseAddress + Offset, Data);
}

/******************************************************************************/
/**
 * @brief Updates the register value.
 *
 * @param[in]   InstancePtr Pointer to the driver instance.
 * @param[in]   Offset Offset address of the register.
 * @param[in]   Mask Bits to be masked.
 * @param[in]   Data Value to be written.
 *
*******************************************************************************/
void XSysMonPsv_UpdateReg32(XSysMonPsv *InstancePtr, u32 Offset, u32 Mask,
			   u32 Data)
{
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);

	XSysMonPsv_ReadReg32(InstancePtr, Offset, &Val);
	XSysMonPsv_WriteReg32(InstancePtr, Offset,
			     (Val & ~Mask) | (Mask & Data));
}
#endif

/** @} */
