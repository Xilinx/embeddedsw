/*
 * Copyright (C) 2009 - 2022 Xilinx, Inc.
 * Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc.
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
#if defined(SDT) || __MICROBLAZE__
#include "arch/cc.h"
#include "platform.h"
#include "platform_config.h"
#include "xil_cache.h"
#include "xparameters.h"
#include "xil_exception.h"
#include "lwip/tcp.h"
#ifdef STDOUT_IS_16550
#include "xuartns550_l.h"
#endif

#include "lwip/tcp.h"
#include "xil_printf.h"

#ifdef SDT
#include "xiltimer.h"
#include "xinterrupt_wrap.h"
#else
#include "xintc.h"
#endif

#if LWIP_DHCP_DOES_ACD_CHECK
#include "lwip/acd.h"
#endif

#if LWIP_DHCP==1
volatile int dhcp_timoutcntr = 240;
void dhcp_fine_tmr();
void dhcp_coarse_tmr();
#endif

volatile int TcpFastTmrFlag = 0;
volatile int TcpSlowTmrFlag = 0;

volatile u64_t tickcntr = 0;
void timer_callback()
{
	/* we need to call tcp_fasttmr & tcp_slowtmr at intervals specified
	 * by lwIP.
	 * It is not important that the timing is absoluetly accurate.
	 */
        static int Tcp_Fasttimer = 0;
        static int Tcp_Slowtimer = 0;
#if LWIP_DHCP==1
	static int dhcp_timer = 0;
        static int dhcp_finetimer = 0;
#if LWIP_DHCP_DOES_ACD_CHECK == 1
        static int acd_timer = 0;
#endif
#endif

        tickcntr++;

        Tcp_Fasttimer++;
        Tcp_Slowtimer++;

#if LWIP_DHCP==1
        dhcp_finetimer++;
        dhcp_timer++;
        dhcp_timoutcntr--;
#if LWIP_DHCP_DOES_ACD_CHECK == 1
        acd_timer++;
#endif
#endif

	if(Tcp_Fasttimer % 5 == 0)
	{
		TcpFastTmrFlag = 1;
	}

	if(Tcp_Slowtimer % 10 == 0)
	{
		TcpSlowTmrFlag = 1;
	}

#if LWIP_DHCP==1
	if(dhcp_finetimer % 10 == 0)
	{
		dhcp_fine_tmr();
	}
	if (dhcp_timer >= 1200)
	{
		dhcp_coarse_tmr();
		dhcp_timer = 0;
	}

#if LWIP_DHCP_DOES_ACD_CHECK == 1
        if(acd_timer % 2 == 0)
        {
                acd_tmr();
        }
#endif /* LWIP_DHCP_DOES_ACD_CHECK */

#endif /*LWIP_DHCP */
}

#ifndef SDT
static XIntc intc;

void platform_setup_interrupts()
{
	XIntc *intcp;
	intcp = &intc;

	XIntc_Initialize(intcp, XPAR_INTC_0_DEVICE_ID);
	XIntc_Start(intcp, XIN_REAL_MODE);

	/* Start the interrupt controller */
	XIntc_MasterEnable(XPAR_INTC_0_BASEADDR);

#ifdef __MICROBLAZE__
	microblaze_register_handler((XInterruptHandler)XIntc_InterruptHandler, intcp);
#endif

	platform_setup_timer();

#ifdef XPAR_ETHERNET_MAC_IP2INTC_IRPT_MASK
	/* Enable timer and EMAC interrupts in the interrupt controller */
	XIntc_EnableIntr(XPAR_INTC_0_BASEADDR,
#ifdef __MICROBLAZE__
			 PLATFORM_TIMER_INTERRUPT_MASK |
#endif
			 XPAR_ETHERNET_MAC_IP2INTC_IRPT_MASK);
#endif


#ifdef XPAR_INTC_0_LLTEMAC_0_VEC_ID
#ifdef __MICROBLAZE__
	XIntc_Enable(intcp, PLATFORM_TIMER_INTERRUPT_INTR);
#endif
	XIntc_Enable(intcp, XPAR_INTC_0_LLTEMAC_0_VEC_ID);
#endif


#ifdef XPAR_INTC_0_AXIETHERNET_0_VEC_ID
	XIntc_Enable(intcp, PLATFORM_TIMER_INTERRUPT_INTR);
	XIntc_Enable(intcp, XPAR_INTC_0_AXIETHERNET_0_VEC_ID);
#endif


#ifdef XPAR_INTC_0_EMACLITE_0_VEC_ID
#ifdef __MICROBLAZE__
	XIntc_Enable(intcp, PLATFORM_TIMER_INTERRUPT_INTR);
#endif
	XIntc_Enable(intcp, XPAR_INTC_0_EMACLITE_0_VEC_ID);
#endif


}
#endif

void enable_caches()
{
#ifdef __MICROBLAZE__
#ifdef XPAR_MICROBLAZE_USE_ICACHE
	Xil_ICacheEnable();
#endif
#ifdef XPAR_MICROBLAZE_USE_DCACHE
	Xil_DCacheEnable();
#endif
#endif
}

void disable_caches()
{
	Xil_DCacheDisable();
	Xil_ICacheDisable();
}

void init_platform()
{
	enable_caches();

#ifdef STDOUT_IS_16550
	XUartNs550_SetBaud(STDOUT_BASEADDR, XPAR_XUARTNS550_CLOCK_HZ, 9600);
	XUartNs550_SetLineControlReg(STDOUT_BASEADDR, XUN_LCR_8_DATA_BITS);
#endif

#ifndef SDT
	platform_setup_interrupts();
#else
	init_timer();
#endif

}

#ifdef SDT
void TimerCounterHandler(void *CallBackRef, u32_t TmrCtrNumber)
{
	timer_callback();
}

void init_timer()
{
	/* Calibrate the platform timer for 50 ms */
	XTimer_SetInterval(50);
	XTimer_SetHandler(TimerCounterHandler, 0, XINTERRUPT_DEFAULT_PRIORITY);
}
/* Timer ticking for SDT Flow */
u64_t get_time_ms()
{
	return tickcntr ;
}
#else
/* Timer ticking for Microblaze */
u64_t get_time_ms()
{
	return tickcntr;
}
#endif

void cleanup_platform()
{
	disable_caches();
}
#endif
