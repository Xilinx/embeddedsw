/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xprefsbl_error.h
*
* This is the main header file which contains error numbers.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who             Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00  Ana	   07/02/20      First release
*
* </pre>
*
******************************************************************************/

#ifndef XPREFSBL_ERROR_H
#define XPREFSBL_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
typedef enum {
	XPREFSBL_IICPS_MUX_ERROR = 0x1,
	XPREFSBL_IICPS_LKP_CONFIG_ERROR, 			/**< 0x2 */
	XPREFSBL_IICPS_CONFIG_ERROR, 				/**< 0x3 */
	XPREFSBL_IICPS_CONFIG_INIT_ERROR,			/**< 0x4 */
	XPREFSBL_IICPS_SLAVE_MONITOR_ERROR,			/**< 0x5 */
	XPREFSBL_IICPS_MASTER_SEND_POLLED_ERROR,	/**< 0x6 */
	XPREFSBL_IICPS_MASTER_RECV_POLLED_ERROR,	/**< 0x7 */
	XPREFSBL_IICPS_SET_SCLK_ERROR,				/**< 0x8 */
	XPREFSBL_IICPS_TIMEOUT,						/**< 0x9 */

	XPREFSBL_MUX_INIT_ERROR,					/**< 0xA */
	XPREFSBL_EEPROM_READ_ERROR,					/**< 0xB */
	XPREFSBL_EEPROM_WRITE_ERROR,				/**< 0xC */

	XPREFSBL_BOARD_NAME_NOTFOUND,				/**< 0xD */

	XPREFSBL_UART_CONFIG_ERROR,					/**< 0xE */
	XPREFSBL_UART_CONFIG_INIT_ERROR,			/**< 0xF */

	XPREFSBL_QSPI_CONFIG_ERROR = 0x20,			/**< 0x20 */
	XPREFSBL_QSPI_CONFIG_INIT_ERROR,			/**< 0x21 */
	XPREFSBL_QSPI_READ_ERROR,               	/**< 0x22 */
	XPREFSBL_QSPI_4BYTE_ENETER_ERROR,       	/**< 0x23 */
	XPREFSBL_INVALID_QSPI_CONNECTION_ERROR, 	/**< 0x24 */
	XPREFSBL_QSPI_LENGTH_ERROR,        			/**< 0x25 */
	XPREFSBL_POLLED_TRANSFER_ERROR,				/**< 0x26 */
	XPREFSBL_QSPI_MANUAL_START_ERROR,			/**< 0x27 */
	XPREFSBL_QSPI_PRESCALER_CLK_ERROR,			/**< 0x28 */
	XPREFSBL_UNSUPPORTED_QSPI_ERROR,			/**< 0x29 */
	XPREFSBL_UNSUPPORTED_QSPI_CONN_MODE_ERROR	/**< 0x2A */
} XPreFsblError;

#ifdef __cplusplus
}
#endif

#endif