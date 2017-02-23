/******************************************************************************
 * (c) Copyright 2017 Xilinx, Inc. All rights reserved.
 *
 * This file contains confidential and proprietary information of Xilinx, Inc.
 * and is protected under U.S. and international copyright and other
 * intellectual property laws.
 *
 * DISCLAIMER
 * This disclaimer is not a license and does not grant any rights to the
 * materials distributed herewith. Except as otherwise provided in a valid
 * license issued to you by Xilinx, and to the maximum extent permitted by
 * applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
 * FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
 * MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
 * and (2) Xilinx shall not be liable (whether in contract or tort, including
 * negligence, or under any other theory of liability) for any loss or damage
 * of any kind or nature related to, arising under or in connection with these
 * materials, including for any direct, or any indirect, special, incidental,
 * or consequential loss or damage (including loss of data, profits, goodwill,
 * or any type of loss or damage suffered as a result of any action brought by
 * a third party) even if such damage or loss was reasonably foreseeable or
 * Xilinx had been advised of the possibility of the same.
 *
 * CRITICAL APPLICATIONS
 * Xilinx products are not designed or intended to be fail-safe, or for use in
 * any application requiring fail-safe performance, such as life-support or
 * safety devices or systems, Class III medical devices, nuclear facilities,
 * applications related to the deployment of airbags, or any other applications
 * that could lead to death, personal injury, or severe property or
 * environmental damage (individually and collectively, "Critical
 * Applications"). Customer assumes the sole risk and liability of any use of
 * Xilinx products in Critical Applications, subject only to applicable laws
 * and regulations governing limitations on product liability.
 *
 * THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
 * AT ALL TIMES.
 ******************************************************************************/

/*
 * Zynq DDR Self-refresh
 *   This DDR self refresh application  provides a simple
 *   demonstration of how to enter to/exit from DDR self refresh mode.
 *   This application runs on R5 out of TCM.
 */

#include <xil_cache.h>
#include <xil_exception.h>
#include <xil_printf.h>
#include <xil_types.h>
#include <xil_io.h>
#include <sleep.h>
#include "platform.h"
#include "xil_ddr.h"

#define SLEEP_TIME_SEC	5

static int ddrc_opmode_is(u32 m)
{
	u32 r = Xil_In32(DDRC_STAT);

	r &= DDRC_STAT_OPMODE_MASK;
	r >>= DDRC_STAT_OPMODE_SHIFT;

	return r == m;
}

static int ddrc_opmode_is_sr(void)
{
	return ddrc_opmode_is(DDRC_STAT_OPMODE_SR);
}

static void ddrc_enable_sr(void)
{
	u32 r;
	size_t i;

	/* disable AXI ports */
	for (i = 0; i < 6; i++) {
		while (Xil_In32(DDRC_PSTAT) & DDRC_PSTAT_PORT_BUSY(i))
			;
		r = Xil_In32(DDRC_PCTRL(i));
		r &= ~DDRC_PCTRL_PORT_EN;
		Xil_Out32(DDRC_PCTRL(i), r);
	}

	/* enable self refresh */
	r = Xil_In32(DDRC_PWRCTL);
	r |= DDRC_PWRCTL_SR_SW;
	Xil_Out32(DDRC_PWRCTL, r);

	while (!ddrc_opmode_is_sr())
		;

	while ((Xil_In32(DDRC_STAT) & (3 << 4)) != (2 << 4))
		;
}

static void ddr_clock_set(int en)
{
	u32 r = Xil_In32(DDRQOS_DDR_CLK_CTRL);

	if (en)
		r |= DDRQOS_DDR_CLK_CTRL_CLKACT;
	else
		r &= ~DDRQOS_DDR_CLK_CTRL_CLKACT;
	Xil_Out32(DDRQOS_DDR_CLK_CTRL, r);
}

static void ddr_clock_enable(void)
{
	ddr_clock_set(1);
}

static void ddr_clock_disable(void)
{
	ddr_clock_set(0);
}

void ddrc_disable_sr(void)
{
	unsigned int readVal;

	print("Bringing DRAM Out of self refresh \r\n");
	Xil_Out32(DDRC_PWRCTL, 0);
	do {
		readVal = Xil_In32(DDRC_STAT);
		readVal &= 3 << 4;
	} while (readVal);

	do {
		readVal = Xil_In32(DDRC_STAT);
		readVal &= DDRC_STAT_OPMODE_MASK;
		readVal >>= DDRC_STAT_OPMODE_SHIFT;
	} while (readVal != DDRC_STAT_OPMODE_NORMAL);
}

void enter_ddr_sr(void)
{
	print("enable SR in DDRC\n");
	ddrc_enable_sr();

	print("stop DDR clocks\n");
	ddr_clock_disable();

	print("DRAM in self-refresh\n");
}

void exit_ddr_sr(void)
{
	print("start DDR clocks\n");
	ddr_clock_enable();
	ddrc_disable_sr();
	print("DRAM out of self-refresh\n");
}

int main(void)
{
	init_platform();

	print("DRAM self refresh\n");

	Xil_DCacheDisable();

	/* Starting DDR self refresh */
	enter_ddr_sr();

	sleep(SLEEP_TIME_SEC);

	/* Exit from DDR self refresh */
	exit_ddr_sr();

	cleanup_platform();

	return 0;
}
