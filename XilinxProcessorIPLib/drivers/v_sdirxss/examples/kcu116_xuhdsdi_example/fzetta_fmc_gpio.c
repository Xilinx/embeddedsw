/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 *****************************************************************************/

/*****************************************************************************/
/**
 * @file fzetta_fmc_gpio.c
 *
 * FMC configuration file
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
#include <stdio.h>
#include "xgpio.h"
#include "fzetta_fmc_gpio.h"

XGpio fzetta_fmc_GpioOutput; /* The driver instance for GPIO Device configured as O/P */
unsigned char fmc_init_done_flag = 0;

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
#ifndef SDT
int fzetta_fmc_gpio_init(u8 Dev_ID)
#else
int fzetta_fmc_gpio_init(UINTPTR BaseAddress)
#endif
{
	int Status;
	/*
	 * Initialize the GPIO driver so that it's ready to use,
	 * specify the device ID that is generated in xparameters.h
	 */
#ifndef SDT
	Status = XGpio_Initialize(&fzetta_fmc_GpioOutput, Dev_ID);
#else
	Status = XGpio_Initialize(&fzetta_fmc_GpioOutput, BaseAddress);
#endif
	 if (Status != XST_SUCCESS)  {
		  return XST_FAILURE;
	 }

	 /*
	  * Set the direction for all signals to be outputs
	  */
	 XGpio_SetDataDirection(&fzetta_fmc_GpioOutput, 1, 0x0);

	 return XST_SUCCESS;
}
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

int fzetta_fmc_spi_channel_select(u8 Channel) {
	if (Channel > 3) {
		  return XST_FAILURE;
	}

	XGpio_DiscreteWrite(&fzetta_fmc_GpioOutput, 1, fmc_init_done_flag | Channel);

	 return XST_SUCCESS;
}

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

void fzetta_fmc_init_done() {
	fmc_init_done_flag = 0x80;
	XGpio_DiscreteWrite(&fzetta_fmc_GpioOutput, 1, fmc_init_done_flag);
}
