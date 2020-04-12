/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 *****************************************************************************/

/*****************************************************************************/
/**
 *
 * @file fzetta_fmc_gpio.h
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
#ifndef FZETTA_FMC_GPIO_H_
#define FZETTA_FMC_GPIO_H_

#include <stdio.h>
#include "xgpio.h"

XGpio fzetta_fmc_GpioOutput; /* The driver instance for GPIO Device configured as O/P */

/*****************************************************************************/
/**
 *
 * This function Initialize GPIO IP and set all IO direction to output.
 *
 * @param	Dev_ID  Device ID.
 *
 * @return	XST_SUCCESS if initialization is successful else XST_FAILURE
 *
 * @note	None.
 *
 ******************************************************************************/

int fzetta_fmc_gpio_init(u8 Dev_ID);
/*****************************************************************************/
/**
 *
 * This function Set 4-Channel MUX in FMC according to target channel.
 *
 * @param	GPIO[1:0] 	00 - Chan0
 * 				01 - Chan1
 * 				10 - Chan2
 * 				11 - Chan3
 *
 * @return	XST_SUCCESS if SPI channel select is successful else XST_FAILURE
 *
 * @note	None.
 *
 ******************************************************************************/


int fzetta_fmc_spi_channel_select(u8 Channel);
/*****************************************************************************/
/**
 *
 * This function Set bit7 of GPIO to 1 to declare init done
 *
 * @param	None
 *
 * @return	none
 *
 * @note	None.
 *
 ******************************************************************************/


void fzetta_fmc_init_done();

#endif /* FZETTA_FMC_GPIO_H_ */
