/******************************************************************************
 *
 * Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
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
