/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xis_i2c.h
*
* This is the main header file which contains definitions for the i2c.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who             Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00  Ana       07/02/20      First release
*
* </pre>
*
******************************************************************************/

#ifndef XIS_I2C_H
#define XIS_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#ifdef XIS_GET_BOARD_PARAMS
#include "xiicps.h"

/************************** Variable Definitions *****************************/
typedef struct board
{
    char *Name; /* Name of the board */
    int Offset; /* Multiboot Offset value */
} Boards_List;

/**************************** Type Definitions *******************************/
/*
 * The AddressType should be u8 as the address pointer in the on-board
 * EEPROM is 1 byte.
 */
typedef u16 AddressType;

/***************** Macros (Inline Functions) Definitions *********************/
/* Board Parameters will present at 0xD0 Address location
 * for ZCU102,104,106,111 boards
 */
#define XIS_EEPROM_BOARD_ADDR_OFFSET_1			(0xD0U)
#define XIS_EEPROM_OFFSET_1_WRITE_BYTES   		(0x01U)
/* Board Parameters will present at 0x00 Address location
 * for ZCU208,216 boards
 */
#define XIS_EEPROM_BOARD_ADDR_OFFSET_2	        (0x00U)
#define XIS_EEPROM_OFFSET_2_WRITE_BYTES  		(0x02U)
#define XIS_IIC_SCLK_RATE						(100000U)
#define XIS_PAGE_SIZE_16						(16U)
#define XIS_DELAY								(10000U)

/************************** Function Prototypes ******************************/
/*
 * Functions defined in xis_i2c.c
 */
int XIs_IicPsMuxInit(void);
int XIs_EepromReadData(u8 *BufferPtr, u16 ReadAddress,
						u16 ByteCount, u32 WrBfrOffset);

#endif /* end of XIS_GET_BOARD_PARAMS */

#ifdef __cplusplus
}
#endif

#endif