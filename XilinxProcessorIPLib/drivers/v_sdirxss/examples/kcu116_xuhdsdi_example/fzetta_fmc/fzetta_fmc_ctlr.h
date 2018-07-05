/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 *****************************************************************************/

/*****************************************************************************/
/**
 *
 * @file fzetta_fmc_ctlr.h
 *
 * FMC configuration file
 *
 * This file configures the FMC card for KCU116 SDI Tx to SDI Rx loopback
 * design
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date       Changes
 * ---- ---- ---------- --------------------------------------------------
 * 1.0  jsr   03/07/2018 Initial version
 * </pre>
 *
 ******************************************************************************/
#ifndef FZETTA_FMC_CTLR_H_
#define FZETTA_FMC_CTLR_H_

#include <stdio.h>
#include "xparameters.h"
#include "fzetta_fmc_gpio.h"
#include "fzetta_fmc_iic.h"
#include "fzetta_fmc_spi.h"
#include "fzetta_fmc_init_table.h"
#include "fzetta_fmc_ctlr.h"

#define DEV_ID_REG 0x01

/*****************************************************************************/
/**
 *
 * This function Initializes CfgPtr and Device for FZetta Control
.
 *
 * @param	None.
 *
 * @return	XST_SUCCESS if initialization is successful else XST_FAILURE
 *
 * @note	None.
 *
 ******************************************************************************/

int fzetta_fmc_ip_init();

/*****************************************************************************/
/**
 *
 * This function Perform register write according to FMC device type to access.
 *
 * @params	Dev 		= device type (IIC_Dev or SPI_Dev)
 * @params	Channel 	= SDI Channel to access
 * @params	Slave_Sel	= SPI slave type (SPI_RCLKR, SPI_DRVR, SPI_RCVR)
 * @params	RegAddr         = 7-bit register to access
 * @params	RegData		= data to write
 *
 * @return	XST_SUCCESS if register write is successful else XST_FAILURE
 *
 * @note	None.
 *
 ******************************************************************************/


int fzetta_fmc_register_write(fzetta_dev_type *Dev, u8 *Channel, spi_slave_sel *Slave_Sel, u8 *RegAddr, u8 *RegData);
/*****************************************************************************/
/**
 *
 * This function Perform register read according to FMC device type to access.
 *
 * @params	Dev 		= device type (IIC_Dev or SPI_Dev)
 * @params	Channel 	= SDI Channel to access
 * @params	Slave_Sel	= SPI slave type (SPI_RCLKR, SPI_DRVR, SPI_RCVR)
 * @params	RegAddr         = 7-bit register to access
 *
 * @return	XST_SUCCESS if register read is successful else XST_FAILURE
 *
 * @note	None.
 *
 ******************************************************************************/


u8 fzetta_fmc_register_read(fzetta_dev_type Dev, u8 Channel, spi_slave_sel Slave_Sel, u8 RegAddr);
/*****************************************************************************/
/**
 *
 * This function  Perform FZETTA FMC IIC and SPI devices Initialization
 * based on Initialization table
 *
 * @params	fzetta_fmc_reg_init = Register list
 *
 * @return	XST_SUCCESS if initialization is successful else XST_FAILURE
 *
 * @note	None.
 *
 ******************************************************************************/


int fzetta_fmc_dev_init(fzetta_fmc_reg *fzetta_fmc_reg_init);
/*****************************************************************************/
/**
 *
 * This function  Perform Reclocker Device Errate Initialization
 *
 * @params      None
 *
 * @return	XST_SUCCESS if initialization is successful else XST_FAILURE
 *
 * @note	None.
 *
 ******************************************************************************/

int fzetta_fmc_dev_errata_init();
/*****************************************************************************/
/**
 *
 * This function  Perform  Stop IIC, GPIO and SPI IP
 *
 * @params      None
 *
 * @return	XST_SUCCESS if fmc stop is successful else XST_FAILURE
 *
 * @note	None.
 *
 ******************************************************************************/

int fzetta_fmc_stop();
/*****************************************************************************/
/**
 *
 * This function  Perform  Initialize FMC and its control IPs
 *
 * @params      None
 *
 * @return	XST_SUCCESS if initialization is successful else XST_FAILURE
 *
 * @note	None.
 *
 ******************************************************************************/

int fzetta_fmc_init();

#endif /* FZETTA_FMC_CTLR_H_ */
