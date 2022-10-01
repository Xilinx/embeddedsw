/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc. All rights reserved.
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
* 1.00  Ana  06/24/20 First release
* 1.01  td   06/10/21 Added check to ensure board param option is only enabled
*                     when UPDATE_A_B option is disabled and vice-versa
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

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/*
 * Below mentioned macros support two different mechanisms
 * We will support any one mechanism at a time
 *
 * XIS_UPDATE_A_B_MECHANISM
 * It will read the persistent registers from QSPI and based on persistent
 * registers data it will update the multiboot value
 *
 * XIS_GET_BOARD_PARAMS
 * It will read the board name from EEPROM and based on board name
 * it will update the multiboot value
 *
 */
//#define XIS_UPDATE_A_B_MECHANISM
//#define XIS_UART_ENABLE
#ifdef XPAR_XIICPS_NUM_INSTANCES
#ifndef XIS_UPDATE_A_B_MECHANISM
#define XIS_GET_BOARD_PARAMS
#endif
#define XIS_MUX_ADDR 			(0x74U)
#define XIS_I2C_MUX_INDEX		(0x1U)
#define XIS_EEPROM_ADDRESS		(0x54U)
#endif

#ifdef __cplusplus
}
#endif

#endif
