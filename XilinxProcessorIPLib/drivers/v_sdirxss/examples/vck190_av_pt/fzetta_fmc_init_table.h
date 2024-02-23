/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 *****************************************************************************/
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
} fzetta_dev_type;

typedef struct {
	fzetta_dev_type dev;
	u8 				channel;
	spi_slave_sel 	slave_sel;
	u8 				regaddr;
	u8 				regdata;
} fzetta_fmc_reglist;

#define REGLIST_SIZE 25

typedef struct {
#ifndef SDT
	u8 gpio_dev_id;
	u8 iic_dev_id;
	u8 spi_dev_id;
#else
	UINTPTR gpio_dev_baseaddress;
	UINTPTR iic_dev_baseaddress;
	UINTPTR spi_dev_baseaddress;
#endif
	fzetta_fmc_reglist reglist[REGLIST_SIZE];
} fzetta_fmc_reg;

extern fzetta_fmc_reglist rclkr_errata_id_80[2];
extern fzetta_fmc_reglist rclkr_errata_id_81[5];
extern fzetta_fmc_reglist rclkr_errata_id_82[3];
extern fzetta_fmc_reglist rcvr_errata_id_01[5];

extern fzetta_fmc_reg fzetta_fmc_table;
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
