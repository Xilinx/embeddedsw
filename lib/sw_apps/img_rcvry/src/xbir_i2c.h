/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc. All rights reserved.
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
#define XBIR_IIC_SYS_BOARD_EEPROM_ADDRESS	(0x54U) /* 0x50 on System Board */
#define XBIR_IIC_CC_EEPROM_ADDRESS		(0x54U)	/* 0X51 on CC */

/* Error codes */
#define	XBIR_IIC_MUX_FAILED			(0x1U)
#define	XBIR_IIC_LKP_CONFIG_FAILED		(0x2U)
#define	XBIR_IIC_CONFIG_FAILED			(0x3U)
#define	XBIR_IIC_CONFIG_INIT_FAILED		(0x4U)
#define	XBIR_IIC_SLAVE_MONITOR_FAILED		(0x5U)
#define	XBIR_IIC_MASTER_SEND_POLLED_FAILED	(0x6U)
#define	XBIR_IIC_MASTER_RECV_POLLED_FAILED	(0x7U)
#define	XBIR_IIC_SET_SCLK_FAILED		(0x8U)
#define XBIR_IIC_EEPROM_HEADER_OFFSET	(0U)
#define XBIR_IIC_EEPROM_BOARD_OFFSET	(8U)

/************************** Variable Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int Xbir_IicEepromReadData (u8 *BufferPtr, u16 ByteCount, u8 EepromAddr);

#endif

int Xbir_IicInit (void);

#ifdef __cplusplus
}
#endif

#endif
