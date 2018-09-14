/******************************************************************************
*
* Copyright (C) 2017 - 2018 Xilinx, Inc.  All rights reserved.
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
******************************************************************************/

#ifndef __PLATFORM_CONFIG_H_
#define __PLATFORM_CONFIG_H_

#define SELECT_TFTPAPP

#ifdef SELECT_TFTPAPP
#define TFTP_APP
#endif

#if SELECT_STDOUT16550
#define STDOUT_IS_16550
#endif

#if SELECT_USESOFTETH
#if defined (__arm__) && !defined (ARMR5)
#define USE_SOFTETH_ON_ZYNQ 1
#endif
#endif

#ifdef XPAR_XEMACPS_3_BASEADDR
#define PLATFORM_EMAC_BASEADDR XPAR_XEMACPS_3_BASEADDR
#endif
#ifdef XPAR_XEMACPS_2_BASEADDR
#define PLATFORM_EMAC_BASEADDR XPAR_XEMACPS_2_BASEADDR
#endif
#ifdef XPAR_XEMACPS_1_BASEADDR
#define PLATFORM_EMAC_BASEADDR XPAR_XEMACPS_1_BASEADDR
#endif
#ifdef XPAR_XEMACPS_0_BASEADDR
#define PLATFORM_EMAC_BASEADDR XPAR_XEMACPS_0_BASEADDR
#endif

#ifdef XPAR_AXI_TIMER_0_BASEADDR
#define PLATFORM_TIMER_BASEADDR XPAR_AXI_TIMER_0_BASEADDR
#define PLATFORM_TIMER_INTERRUPT_INTR XPAR_AXI_TIMER_0_INTR
#define PLATFORM_TIMER_INTERRUPT_MASK (1 << XPAR_AXI_TIMER_0_INTR)
#endif

#if defined (__arm__) && !defined (ARMR5)
#define PLATFORM_ZYNQ
#endif
#if defined (ARMR5) || (__aarch64__) || (ARMA53_32)
#define PLATFORM_ZYNQMP
#endif

#endif /* __PLATFORM_CONFIG_H_ */
