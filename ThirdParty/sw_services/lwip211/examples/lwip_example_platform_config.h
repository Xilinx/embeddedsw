/*
 * Copyright (C) 2017 - 2019 Xilinx, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 */

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
