/******************************************************************************
* Copyright (c) 2020 - 2021 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xbir_i2c.h
*
* This is the main header file which contains definitions for the i2c.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date      Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.00  bsv   07/02/20   First release
*
* </pre>
*
******************************************************************************/

#ifndef XBIR_I2C_H
#define XBIR_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xparameters.h"
#if defined(XPAR_XIICPS_NUM_INSTANCES)
#include "xiicps.h"

/************************** Constant Definitions *****************************/
/* The values are specific to ZCU102 */
#define XBIR_IIC_SYS_BOARD_EEPROM_ADDRESS	(0x50U)
#define XBIR_IIC_CC_EEPROM_ADDRESS		(0x51U)

/************************** Variable Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XBIR_LATCH_TIME_FOR_PHY_RESET_IN_US	(200L)

/************************** Function Prototypes ******************************/
int Xbir_IicEepromReadData (u8 *BufferPtr, u16 ByteCount, u8 EepromAddr);
int Xbir_I2cExpanderReset(void);

#endif

int Xbir_IicInit (void);

#ifdef __cplusplus
}
#endif

#endif
