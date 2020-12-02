/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
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
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* X.XX  XX     YY/MM/DD
* 1.00  RHe    16/06/20 Initial release. 
* </pre>
*
******************************************************************************/

#ifndef TI_LMK03318_H
#define TI_LMK03318_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xparameters.h"
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
int TI_LMK03318_Init(void *IicPtr, u8 I2CSlaveAddress);
int TI_LMK03318_PowerDown(void *IicPtr, u8 I2CSlaveAddress);
int TI_LMK03318_SetClock(void *IicPtr, u8 I2CSlaveAddress,
				int FIn, int FOut, u8 FreeRun);
int TI_LMK03318_EnableBypass(void *IicPtr, u8 I2CSlaveAddress,
				u8 InPort, u8 OutPort);
void TI_LMK03318_RegisterDump(void *IicPtr, u8 I2CSlaveAddress);
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
