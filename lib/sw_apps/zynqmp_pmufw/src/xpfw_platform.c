/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

#include "xpfw_platform.h"
#include "xparameters.h"
#include "xil_io.h"
#include "xil_types.h"


#include "crl_apb.h"

#include "xpfw_default.h"

#ifdef STDOUT_BASEADDRESS

#if (STDOUT_BASEADDRESS == XPAR_XUARTPS_0_BASEADDR)
/* Register definitions for UART0 */
#define UART_BASE	0XFF000000U
#define R_CRL_APB_REF_CTL CRL_APB_UART0_REF_CTRL
#define IOU_UART_RST_MASK	CRL_APB_RST_LPD_IOU2_UART0_RESET_MASK

#elif (STDOUT_BASEADDRESS == XPAR_XUARTPS_1_BASEADDR)

/* Register definitions for UART1 */
#define UART_BASE	0XFF010000U
#define R_CRL_APB_REF_CTL CRL_APB_UART1_REF_CTRL
#define IOU_UART_RST_MASK	CRL_APB_RST_LPD_IOU2_UART1_RESET_MASK

#else
#define SKIP_UARTINIT
#endif

#endif /* STDOUT_BASEADDRESS */

#ifndef SKIP_UARTINIT
#define R_UART_CR  (UART_BASE)
#define R_UART_MODE (UART_BASE + 0x04)
#define R_UART_IMR (UART_BASE + 0x10)
#define R_UART_BGEN (UART_BASE + 0x18)
#define R_UART_STS  (UART_BASE + 0x2C)
#define R_UART_TX  (UART_BASE + 0x30)
#define R_UART_BDIV (UART_BASE + 0x34)
#endif

void XPfw_PlatformInit(void)
{

#ifndef SKIP_UARTINIT
	/*
	 * FIXME: This function configures UART to baud 115200 and uses
	 * hardcoded values. Fix it to use Clock params from xparameters
	 * Also add MIO configs
	 */

	/* Release UART from Reset */
	XPfw_RMW32(CRL_APB_RST_LPD_IOU2, IOU_UART_RST_MASK,0 );
	/* SetUp the  Reference Clock for UART*/
	XPfw_Write32(R_CRL_APB_REF_CTL,0X01000500U);
	/* RX and TX Enable */
	XPfw_Write32(R_UART_CR, (UART0_CONTROL_REG0_TXEN_MASK|UART0_CONTROL_REG0_RXEN_MASK));
	/* 8bits, 1 stop bit, No Parity, normal mode, uart_ref_clk */
	XPfw_Write32(R_UART_MODE, 0X00000020U);
	/* For Baud Rate = 115200 */
	XPfw_Write32(R_UART_BGEN, 0X0000001FU);
	/* Baud Rate Divider */
	XPfw_Write32(R_UART_BDIV, 0X00000006U);
#endif

}
