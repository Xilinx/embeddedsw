/******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2023-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file ti_lmk03318.h
* @addtogroup TI_LMK03318
* @{
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver  Who      Date      Changes
* ---- ---      --------  --------------------------------------------------.
* 1.00  ND      18/10/22  Common DP 2.1 tx only application for zcu102 and
* 						  vcu118
* 1.01	ND		26/02/24  Added support for 13.5 and 20G
* </pre>
*
******************************************************************************/

#ifndef TI_LMK03318_H
#define TI_LMK03318_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"

/************************** Constant Definitions *****************************/

#define TI_LMK03318_FVCO_MAX 5400000000 //!< Maximum VCO Operating Frequency in Hz
#define TI_LMK03318_FVCO_MIN 4800000000 //!< Minimum VCO Operating Frequency in Hz

#define TI_LMK03318_FOUT_MAX 800000000  //!< Maximum Output Frequency in Hz
#define TI_LMK03318_FOUT_MIN   1000000  //!< Minimum Output Frequency in Hz

#define TI_LMK03318_FIN_MAX 300000000  //!< Maximum Input Frequency in Hz
#define TI_LMK03318_FIN_MIN   1000000  //!< Minimum Input Frequency in Hz

#define TI_LMK03318_FPD_MAX 150000000  //!< Maximum Phase Detector Frequency in Hz
#define TI_LMK03318_FPD_MIN   1000000  //!< Minimum Phase Detector Frequency in Hz

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int TI_LMK03318_Init(u32 I2CBaseAddress, u8 I2CSlaveAddress);
int TI_LMK03318_PowerDown(u32 I2CBaseAddress, u8 I2CSlaveAddress);
int TI_LMK03318_SetClock(u32 I2CBaseAddress, u8 I2CSlaveAddress,
				int FIn, int FOut, u8 FreeRun);
int TI_LMK03318_EnableBypass(u32 I2CBaseAddress, u8 I2CSlaveAddress,
				u8 InPort, u8 OutPort);
void TI_LMK03318_RegisterDump(u32 I2CBaseAddress, u8 I2CSlaveAddress);
//static int TI_LMK03318_SetRegister(u32 I2CBaseAddress, u8 I2CSlaveAddress,
//				u8 RegisterAddress, u8 Value);
//static u8 TI_LMK03318_GetRegister(u32 I2CBaseAddress, u8 I2CSlaveAddress,
//				u8 RegisterAddress);
//static int TI_LMK03318_ModifyRegister(u32 I2CBaseAddress, u8 I2CSlaveAddress,
//				u16 RegisterAddress, u8 Value, u8 Mask);

/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* TI_LMK03318_H */
/** @} */
