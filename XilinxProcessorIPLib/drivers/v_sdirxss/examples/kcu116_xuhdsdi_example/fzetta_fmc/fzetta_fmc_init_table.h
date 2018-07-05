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
 * @file fzetta_fmc_init_table.h
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
#ifndef FZETTA_FMC_INIT_TABLE_H_
#define FZETTA_FMC_INIT_TABLE_H_

#include <stdio.h>
#include "xparameters.h"

typedef enum{
	IIC_Dev = 0,
	SPI_Dev
}fzetta_dev_type;

typedef struct {
	fzetta_dev_type dev;
	u8 				channel;
	spi_slave_sel 	slave_sel;
	u8 				regaddr;
	u8 				regdata;
}fzetta_fmc_reglist;

#define REGLIST_SIZE 25

typedef struct {
	u8 gpio_dev_id;
	u8 iic_dev_id;
	u8 spi_dev_id;
	fzetta_fmc_reglist reglist[REGLIST_SIZE];
}fzetta_fmc_reg;

fzetta_fmc_reglist rclkr_errata_id_80[2];
fzetta_fmc_reglist rclkr_errata_id_81[5];
fzetta_fmc_reglist rclkr_errata_id_82[3];
fzetta_fmc_reglist rcvr_errata_id_01[5];

fzetta_fmc_reg fzetta_fmc_table;
/*****************************************************************************/
/**
 *
 * This function has FIdus FMC Initialization Table.
 *
 * @param	None.
 *
 * @return	None
 *
 * @note	None.
 *
 ******************************************************************************/

void fzetta_fmc_table_init();

#endif /* FZETTA_FMC_INIT_TABLE_H_ */
