/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
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
