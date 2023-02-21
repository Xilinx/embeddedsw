/******************************************************************************
* Copyright (c) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xbir_util.h
*
* This file contains list of APIs provided by utility module (xbir_util.c file)
*
******************************************************************************/
#ifndef XBIR_UTIL_H
#define XBIR_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_io.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
/*****************************************************************************/
/**
 * @brief
 * This function does a masked write to input register
 *
 * @param	Reg	is the register address to be written
 * @param	Mask represents the bits to be modified
 * @param	Val is the value to be written
 *
 * @return	None
 *
 *****************************************************************************/
static inline void Xbir_MaskWrite (u32 Reg, u32 Mask, u32 Val)
{
	u32 RegVal = Xil_In32(Reg) & (~Mask);

	RegVal |= (Val & Mask);
	Xil_Out32(Reg, RegVal);
}

/************************** Function Prototypes ******************************/
const char* Xbir_UtilGetFileExt (const char *FileName);
u8 Xbir_UtilIsNumber (const char *Str);

#ifdef __cplusplus
}
#endif

#endif
