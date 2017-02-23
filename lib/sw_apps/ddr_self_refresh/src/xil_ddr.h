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

#ifndef SRC_XIL_DDR_H_
#define SRC_XIL_DDR_H_

#define BIT(n)		(1 << (n))
#define ARRAY_SIZE(x)	(sizeof(x) / sizeof((x)[0]))

#define DDRC_BASE		0xFD070000
#define DDRC_MSTR		0XFD070000
#define DDRC_STAT		(DDRC_BASE + 4)
#define DDRC_MRCTRL0		(DDRC_BASE + 0X10)

#define DDRC_PWRCTL		(DDRC_BASE + 0x30)
#define DDRC_PWRTMG		(DDRC_BASE + 0X34)

#define DDRC_PSTAT		(DDRC_BASE + 0x3fc)
#define DDRC_PCTRL(n)		(DDRC_BASE + 0x490 + (0xb0 * n))

#define DDRQOS_BASE		0xFD090000
#define DDRQOS_DDR_CLK_CTRL	(DDRQOS_BASE + 0x700)

#define DDRC_STAT_OPMODE_MASK	7
#define DDRC_STAT_OPMODE_SHIFT	0
#define DDRC_STAT_OPMODE_INIT	0
#define DDRC_STAT_OPMODE_NORMAL	1
#define DDRC_STAT_OPMODE_SR	3

#define DDRC_PWRCTL_SR_SW	BIT(5)

#define DDRC_PSTAT_PORT_BUSY(n)	((BIT(0) | BIT(16)) << n)

#define DDRC_PCTRL_PORT_EN	BIT(0)

#define DDRQOS_DDR_CLK_CTRL_CLKACT	BIT(0)

#endif /* SRC_XIL_DDR_H_ */
