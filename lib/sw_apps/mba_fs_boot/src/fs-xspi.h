/******************************************************************************
* Copyright (c) 2013 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/


/* @file xspi.h
 */
#ifndef XSPI_H
#define XSPI_H

#include "xparameters.h"
#include "auto-config.h"

/* Xilinx SPI Macros */
#define XSP_BASEADDR		CONFIG_PRIMARY_FLASH_SPI_BASEADDR
#define XSP_SPI_MODE		CONFIG_FLASH_SPI_MODE
#define XSP_SPI_FIFO_DEPTH	CONFIG_FLASH_SPI_FIFO_DEPTH

#define XSP_SPI_QUAD_MODE	2
#define XSP_SPI_DUAL_MODE	1
#define XSP_SPI_SINGLE_MODE	0

#define XSP_SRR_OFFSET	0x40
#define XSP_CR_OFFSET	0x60
#define XSP_SR_OFFSET	0x64
#define XSP_DTR_OFFSET	0x68
#define XSP_DRR_OFFSET	0x6C
#define XSP_SSR_OFFSET	0x70

#define XSP_SRR_RESET_MASK	0xa

#define XSP_CR_ENABLE_MASK	0x2
#define XSP_CR_MASTER_MODE_MASK	0x4
#define XSP_CR_CLK_POLARTY_MASK	0x8
#define XSP_CR_CLK_PHASE_MASK	0x10
#define XSP_CR_TXFIFO_RESET_MASK	0x20
#define XSP_CR_RXFIFO_RESET_MASK	0x40
#define XSP_CR_MANUAL_SS_MASK	0x80
#define XSP_CR_TRANS_INHIBIT_MASK	0x100

#define XSP_SR_RX_EMPTY_MASK	0x1
#define XSP_SR_RX_FULL_MASK	0x2
#define XSP_SR_TX_EMPTY_MASK	0x4
#define XSP_SR_TX_FULL_MASK	0x8
#define XSP_SR_MODE_FAULT_MASK	0x10

#ifndef SPI_NUM_TRANSFER_BITS
#define SPI_NUM_TRANSFER_BITS 8
#endif

int xspi_init(int polarity, int phase);

void xspi_xfersetup(int cs);

void xspi_xferterminate();

int xspi_xfer(unsigned char *cmd, int cmdlen, unsigned char *rx, int endflag);

#endif
