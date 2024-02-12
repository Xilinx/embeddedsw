/******************************************************************************
* Copyright 2013 - 2022 Xilinx, Inc. All Rights Reserved.
* Copyright 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/


/* @file xspi.h
 */
#ifndef XSPI_H
#define XSPI_H

#include "xparameters.h"

/* Xilinx QUAD SPI address */
#if defined(XPAR_AXI_QUAD_SPI_0_AXI_INTERFACE)
#define XSP_BASEADDR		XPAR_AXI_QUAD_SPI_0_AXI4_ADDRESS
#elif defined(XPAR_AXI_QUAD_SPI_0_BASEADDR)
#define XSP_BASEADDR		XPAR_AXI_QUAD_SPI_0_BASEADDR
#endif

/* Xilinx SPI address */
#if defined(XPAR_XSPI_0_AXI_INTERFACE)
#define XSP_BASEADDR		XPAR_XSPI_0_AXI4_ADDRESS
#elif defined(XPAR_XSPI_0_BASEADDR)
#define XSP_BASEADDR		XPAR_XSPI_0_BASEADDR
#endif

/* Xilinx SPI mode */
#if defined(XPAR_XSPI_0_SPI_MODE)
#define XSP_SPI_MODE	        XPAR_XSPI_0_SPI_MODE
#elif defined(XPAR_AXI_QUAD_SPI_0_SPI_MODE)
#define XSP_SPI_MODE	        XPAR_AXI_QUAD_SPI_0_SPI_MODE
#endif


/* Xilinx SPI fifo width */
#if defined(XPAR_AXI_QUAD_SPI_0_FIFO_SIZE)
#define XSP_SPI_FIFO_DEPTH	XPAR_AXI_QUAD_SPI_0_FIFO_SIZE
#elif defined(XPAR_XSPI_0_FIFO_SIZE)
#define XSP_SPI_FIFO_DEPTH	XPAR_XSPI_0_FIFO_SIZE
#endif

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
