/******************************************************************************
* Copyright (c) 2020-2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xis_config.h
*
* This is the header file which contains all configuration data.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00  skd  01/13/23 Initial release
*
* </pre>
*
******************************************************************************/

#ifndef XIS_CONFIG_H
#define XIS_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xparameters.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/*
 * XIS_GET_BOARD_PARAMS
 * It will read the board name from EEPROM and based on board name
 * it will update the multiboot value
 *
 */

#ifdef XPAR_XIICPS_NUM_INSTANCES

#define XIS_GET_BOARD_PARAMS
#define XIS_MUX_ADDR 			(0x74U)
#define XIS_I2C_MUX_INDEX		(0x1U)
#define XIS_EEPROM_ADDRESS		(0x54U)

#endif

#ifdef __cplusplus
}
#endif

#endif
