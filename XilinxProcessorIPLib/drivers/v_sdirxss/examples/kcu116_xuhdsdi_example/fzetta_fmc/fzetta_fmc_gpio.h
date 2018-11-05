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
