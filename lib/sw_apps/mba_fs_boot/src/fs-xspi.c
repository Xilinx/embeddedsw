/******************************************************************************
*
* (c) Copyright 2013-2016 Xilinx, Inc. All rights reserved.
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
*******************************************************************************/
#include "fs-boot.h"
#ifdef CONFIG_PRIMARY_FLASH_SPI
#include "fs-xspi.h"
#include "xio.h"

struct spi_xfer_s {
	int mtxlen;
	int mrxlen;
	volatile unsigned char *txbuf;
	volatile unsigned char *rxbuf;
};

int xspi_init(int polarity, int phase)
{
	unsigned int cr;

	XIo_Out32((XSP_BASEADDR + XSP_CR_OFFSET),
		XSP_CR_MASTER_MODE_MASK | XSP_CR_MANUAL_SS_MASK |
		XSP_CR_TXFIFO_RESET_MASK | XSP_CR_RXFIFO_RESET_MASK |
		XSP_CR_TRANS_INHIBIT_MASK);

	/* Reset SPI controller */
	XIo_Out32((XSP_BASEADDR + XSP_SRR_OFFSET), XSP_SRR_RESET_MASK);

	XIo_Out32((XSP_BASEADDR + XSP_SSR_OFFSET), 0xffffffff);

	cr = XSP_CR_MASTER_MODE_MASK | XSP_CR_MANUAL_SS_MASK |
		XSP_CR_TRANS_INHIBIT_MASK | XSP_CR_ENABLE_MASK ;
	if (polarity != 0)
		cr |= XSP_CR_CLK_POLARTY_MASK;
	if (phase != 0)
		cr |= XSP_CR_CLK_PHASE_MASK;

	XIo_Out32((XSP_BASEADDR + XSP_CR_OFFSET), cr);
	return 0;
}

void xspi_xfersetup(int cs)
{
	XIo_Out32((XSP_BASEADDR + XSP_SSR_OFFSET), ~(1 << cs));
}

void xspi_xferterminate()
{
	XIo_Out32((XSP_BASEADDR + XSP_SSR_OFFSET), 0xffffffff);
}

#if SPI_NUM_TRANSFER_BITS == 8
void xspi_tx(struct spi_xfer_s *xfer)
{
	XIo_Out32((XSP_BASEADDR + XSP_DTR_OFFSET), (unsigned int)(*(xfer->txbuf)));
	xfer->txbuf++;
	xfer->mtxlen++;
}

void xspi_rx(struct spi_xfer_s *xfer)
{
	*(xfer->rxbuf) = (unsigned char)(XIo_In32(XSP_BASEADDR + XSP_DRR_OFFSET) & 0x000000ff);
	xfer->rxbuf++;
	xfer->mrxlen++;
}
#elif SPI_NUM_TRANSFER_BITS == 16
void xspi_tx(struct spi_xfer_s *xfer)
{
	XIo_Out32((XSP_BASEADDR + XSP_DTR_OFFSET), (unsigned int)(*((unsigned short *)(xfer->txbuf))));
	xfer->txbuf += 2;
	xfer->mtxlen += 2;
}

void xspi_rx(struct spi_xfer_s *xfer)
{
	*((unsigned short *)(xfer->rxbuf)) = (unsigned short)(XIo_In32(XSP_BASEADDR + XSP_DRR_OFFSET) & 0x0000ffff);
	xfer->rxbuf += 2;
	xfer->mrxlen += 2;
}
#else
void xspi_tx(struct spi_xfer_s *xfer)
{
	XIo_Out32((XSP_BASEADDR + XSP_DTR_OFFSET), *((unsigned int *)(xfer->txbuf)));
	xfer->txbuf += 4;
	xfer->mtxlen += 4;
}

void xspi_rx(struct spi_xfer_s *xfer)
{
	*((unsigned int *)(xfer->rxbuf)) = XIo_In32(XSP_BASEADDR + XSP_DRR_OFFSET);
	xfer->rxbuf += 4;
	xfer->mrxlen += 4;
}
#endif

int xspi_xfer(unsigned char *cmd, int cmdlen, unsigned char *rx, int endflag)
{
	unsigned int sr, cr, i;
	struct spi_xfer_s xfer;

	xfer.mrxlen = 0;
	xfer.txbuf = cmd;
	xfer.rxbuf = rx;

	cr = XIo_In32(XSP_BASEADDR + XSP_CR_OFFSET);

	for(xfer.mtxlen = 0; xfer.mtxlen < cmdlen;) {
		sr = XIo_In32(XSP_BASEADDR + XSP_SR_OFFSET);
		i = 0;
		while((sr & XSP_SR_TX_FULL_MASK) == 0 && xfer.mtxlen < cmdlen &&
		       i < XSP_SPI_FIFO_DEPTH) {
			if (cmd != 0) {
				xspi_tx(&xfer);
			} else {
				XIo_Out32((XSP_BASEADDR + XSP_DTR_OFFSET), 0);
#if SPI_NUM_TRANSFER_BITS == 8
				xfer.mtxlen++;
#elif SPI_NUM_TRANSFER_BITS == 16
				xfer.mtxlen += 2;
#else
				xfer.mtxlen += 4;
#endif
			}
			i++;
		}

		XIo_Out32((XSP_BASEADDR + XSP_CR_OFFSET),
			(cr & (~XSP_CR_TRANS_INHIBIT_MASK)));
		do {
			sr = XIo_In32(XSP_BASEADDR + XSP_SR_OFFSET);
		} while((sr & XSP_SR_TX_EMPTY_MASK) == 0);

		sr = XIo_In32(XSP_BASEADDR + XSP_SR_OFFSET);
		while((sr & XSP_SR_RX_EMPTY_MASK) == 0) {
			if (xfer.mrxlen < cmdlen && xfer.rxbuf != 0) {
				xspi_rx(&xfer);
			} else {
				XIo_In32(XSP_BASEADDR + XSP_DRR_OFFSET);
			}
			sr = XIo_In32(XSP_BASEADDR + XSP_SR_OFFSET);
		}
	}

	if (endflag != 0) {
		XIo_Out32((XSP_BASEADDR + XSP_CR_OFFSET),
			  (cr | XSP_CR_TRANS_INHIBIT_MASK));
		XIo_Out32((XSP_BASEADDR + XSP_SSR_OFFSET), 0xffffffff);
	}

	return xfer.mrxlen;
}
#endif
