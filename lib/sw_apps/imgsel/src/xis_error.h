/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc. All rights reserved.
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
* Ver   Who             Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00  Ana	   07/02/20      First release
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

	XIS_BOARD_NAME_NOTFOUND_ERROR,			/**< 0xD */
	XIS_IDEN_STRING_MISMATCH_ERROR,			/**< 0xE */
	XIS_REGISTERS_LENGTH_MISMATCH_ERROR,	/**< 0xF */
	XIS_CHECKSUM_MISMATCH_ERROR,			/**< 0x10 */

	XIS_UART_CONFIG_ERROR,					/**< 0x15 */
	XIS_UART_CONFIG_INIT_ERROR,				/**< 0x16 */

	XIS_QSPI_CONFIG_ERROR = 0x20,			/**< 0x20 */
	XIS_QSPI_CONFIG_INIT_ERROR,				/**< 0x21 */
	XIS_QSPI_READ_ERROR,               		/**< 0x22 */
	XIS_QSPI_4BYTE_ENETER_ERROR,       		/**< 0x23 */
	XIS_INVALID_QSPI_CONNECTION_ERROR, 		/**< 0x24 */
	XIS_QSPI_LENGTH_ERROR,        			/**< 0x25 */
	XIS_POLLED_TRANSFER_ERROR,				/**< 0x26 */
	XIS_QSPI_MANUAL_START_ERROR,			/**< 0x27 */
	XIS_QSPI_PRESCALER_CLK_ERROR,			/**< 0x28 */
	XIS_UNSUPPORTED_QSPI_ERROR,				/**< 0x29 */
	XIS_UNSUPPORTED_QSPI_CONN_MODE_ERROR,	/**< 0x2A */

	XIS_GPIO_LKP_CONFIG_ERROR = 0x30,       /**< 0x30 */
	XIS_GPIO_CONFIG_ERROR,                  /**< 0x31 */
} XIsError;

#ifdef __cplusplus
}
#endif

#endif