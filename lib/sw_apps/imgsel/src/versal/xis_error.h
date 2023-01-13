/******************************************************************************
* Copyright (c) 2020-2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xis_error.h
*
* This is the main header file which contains error numbers.
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

#ifndef XIS_ERROR_H
#define XIS_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
typedef enum {
	XIS_IICPS_MUX_ERROR = 0x1,
	XIS_IICPS_LKP_CONFIG_ERROR, 			/**< 0x2 */
	XIS_IICPS_CONFIG_ERROR, 				/**< 0x3 */
	XIS_IICPS_CONFIG_INIT_ERROR,			/**< 0x4 */
	XIS_IICPS_SLAVE_MONITOR_ERROR,			/**< 0x5 */
	XIS_IICPS_MASTER_SEND_POLLED_ERROR,		/**< 0x6 */
	XIS_IICPS_MASTER_RECV_POLLED_ERROR,		/**< 0x7 */
	XIS_IICPS_SET_SCLK_ERROR,				/**< 0x8 */
	XIS_IICPS_TIMEOUT,						/**< 0x9 */

	XIS_MUX_INIT_ERROR,						/**< 0xA */
	XIS_EEPROM_READ_ERROR,					/**< 0xB */
	XIS_EEPROM_WRITE_ERROR,					/**< 0xC */

	XIS_BOARD_NAME_NOTFOUND_ERROR			/**< 0xD */
} XIsError;

#ifdef __cplusplus
}
#endif

#endif
