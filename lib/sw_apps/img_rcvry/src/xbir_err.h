/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xbir_err.h
*
* This file contains list of error codes generated across the application.
*
******************************************************************************/
#ifndef __XBIR_ERR_H_
#define __XBIR_ERR_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xparameters.h"

/************************** Constant Definitions *****************************/
#if defined(XPAR_XIICPS_NUM_INSTANCES)
#define	XBIR_ERROR_IIC_MUX				(0x2U)
#define	XBIR_ERROR_IIC_LKP_CONFIG		(0x3U)
#define	XBIR_ERROR_IIC_CONFIG			(0x4U)
#define	XBIR_ERROR_IIC_CONFIG_INIT		(0x5U)
#define	XBIR_ERROR_IIC_SLAVE_MONITOR	(0x6U)
#define	XBIR_ERROR_IIC_MASTER_SEND		(0x6U)
#define	XBIR_ERROR_IIC_MASTER_RECV		(0x7U)
#define	XBIR_ERROR_IIC_SET_SCLK			(0x8U)
#define XBIR_ERROR_I2C_WRITE_TIMEOUT	(0x9U)
#define XBIR_ERROR_I2C_READ_TIMEOUT		(0xAU)
#define XBIR_ERROR_IIC_SET_SCLK_TIMEOUT	(0xBU)
#endif

#define	XBIR_ERROR_QSPI_CONFIG			(0x20U)
#define	XBIR_ERROR_QSPI_CONFIG_INIT		(0x21U)
#define	XBIR_ERROR_QSPI_READ			(0x22U)
#define	XBIR_ERROR_QSPI_4BYTE_ENTER		(0x23U)
#define	XBIR_ERROR_INVALID_QSPI_CONN	(0x24U)
#define	XBIR_ERROR_QSPI_LENGTH			(0x25U)
#define	XBIR_ERROR_POLLED_TRANSFER		(0x26U)
#define	XBIR_ERROR_QSPI_MANUAL_START	(0x27U)
#define	XBIR_ERROR_QSPI_PRESCALER_CLK	(0x28U)
#define	XBIR_ERROR_QSPI_CONN_MODE		(0x29U)
#define XBIR_ERROR_QSPI_VENDOR			(0x2AU)

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif	/* XBIR_ERR_H */