/*******************************************************************************
* Copyright (C) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file si_5344.h
* 
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* X.XX  XX     YY/MM/DD
* 1.00  MG     16/07/27 Initial release. 
* </pre>
*
******************************************************************************/

#ifndef SI_5344_H
#define SI_5344_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"

/************************** Constant Definitions *****************************/
 
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
void SI_5344_RegisterDump(u32 I2CBaseAddress, u8 I2CSlaveAddress);
int SI_5344_PowerDown(u32 I2CBaseAddress, u8 I2CSlaveAddress);
int SI_5344_SetFrequencyConfig(u32 I2CBaseAddress, u8 I2CSlaveAddress,
			u8 Freerun, u8 ConfigSelect);

/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* SI_5344_H */
