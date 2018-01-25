/******************************************************************************
* Copyright (c) 2013 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/* @file spi-flash.h
 */
#ifndef SPI_FLASH_H
#define SPI_FLASH_H
int spi_flash_probe();

int spi_flash_read(unsigned long offset, unsigned char *rx, int rxlen, char startflag, char endflag);

#endif
