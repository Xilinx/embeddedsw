/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xbir_qspi.h
*
* This is the qspi header file which contains definitions for the qspi.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date       Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.00  bsv   07/02/20   First release
*
* </pre>
*
******************************************************************************/

#ifndef XBIR_QSPI_H
#define XBIR_QSPI_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/
#define	XBIR_QSPI_CONFIG_FAILED			(0x20U)
#define	XBIR_QSPI_CONFIG_INIT_FAILED		(0x21U)
#define	XBIR_ERROR_QSPI_READ			(0x22U)
#define	XBIR_QSPI_4BYTE_ENETER_ERROR		(0x23U)
#define	XBIR_ERROR_INVALID_QSPI_CONNECTION	(0x24U)
#define	XBIR_ERROR_QSPI_LENGTH			(0x25U)
#define	XBIR_POLLED_TRANSFER_FAILED		(0x26U)
#define	XBIR_ERROR_QSPI_MANUAL_START		(0x27U)
#define	XBIR_ERROR_QSPI_PRESCALER_CLK		(0x28U)
#define	XBIR_ERROR_UNSUPPORTED_QSPI_CONN_MODE	(0x29U)
#define XBIR_QSPI_FLASH_PAGE_SIZE		(256U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int Xbir_QspiInit (void);
int Xbir_QspiRead (u32 SrcAddr, u8 *DestAddr, u32 Length);
int Xbir_QspiWrite(u32 Addr, u8 *WrBuff, u32 Len);
int Xbir_QspiFlashErase(u32 Address, u32 Length);
int Xbir_QspiWrite(u32 Address, u8 *WrBuffer, u32 Length);

#ifdef __cplusplus
}
#endif

#endif  /* XBIR_QSPI_H */