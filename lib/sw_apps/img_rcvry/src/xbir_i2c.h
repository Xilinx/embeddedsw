/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 2.00  skd   07/28/22   Added support to work with kv260 and kr260
*                        starter kit xsa
* 3.00  sd    01/27/24   Clean up
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
#if (XPAR_XIICPS_NUM_INSTANCES == 2U)
#ifndef SDT
#define XBIR_I2C_EEPROM_DEVICE	XPAR_XIICPS_1_DEVICE_ID
#else
#define XBIR_I2C_EEPROM_DEVICE	XPAR_XIICPS_1_BASEADDR
#endif
#else
#ifndef SDT
#define XBIR_I2C_EEPROM_DEVICE	XPAR_XIICPS_0_DEVICE_ID
#else
#define XBIR_I2C_EEPROM_DEVICE	XPAR_XIICPS_0_BASEADDR
#endif
#endif

/************************** Variable Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XBIR_IIC_SOM_EEPROM_ADDRESS			(0x50U)
#define XBIR_IIC_SC_EEPROM_ADDRESS			(0x54U)
#define XBIR_IIC_CC_EEPROM_ADDRESS			(0x51U)
#define XBIR_LATCH_TIME_FOR_PHY_RESET_IN_US	(200L)
#define XBIR_IIC_SCLK_RATE					(100000U)
#define XBIR_MAX_DELAY						(10000000U)
#define XBIR_I2C_GPIO_EXPANDER				(0x11U)
#define XBIR_GEM1_RESET_MASK				(0x40U)

/************************** Function Prototypes ******************************/
int Xbir_IicEepromReadData (u8 *BufferPtr, u16 ByteCount, u8 EepromAddr);
int Xbir_I2cExpanderReset(void);

#endif

int Xbir_IicInit (void);

#ifdef __cplusplus
}
#endif

#endif
