/*
 * Copyright (C) 2014 - 2016 Xilinx, Inc.  All rights reserved.
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
 */

#ifndef PM_GIC_H_
#define PM_GIC_H_

#include "pm_common.h"

typedef struct PmSlave PmSlave;

/*********************************************************************
 * Macros
 ********************************************************************/

/* FPD GIC Proxy irq masks */

/* GIC Proxy group 0 */
#define FPD_GICP_RTC_WAKE_IRQ_MASK	(1U << 26U)
#define FPD_GICP_CAN1_WAKE_IRQ_MASK	(1U << 24U)
#define FPD_GICP_CAN0_WAKE_IRQ_MASK	(1U << 23U)
#define FPD_GICP_UART1_WAKE_IRQ_MASK	(1U << 22U)
#define FPD_GICP_UART0_WAKE_IRQ_MASK	(1U << 21U)
#define FPD_GICP_SPI1_WAKE_IRQ_MASK	(1U << 20U)
#define FPD_GICP_SPI0_WAKE_IRQ_MASK	(1U << 19U)
#define FPD_GICP_I2C1_WAKE_IRQ_MASK	(1U << 18U)
#define FPD_GICP_I2C0_WAKE_IRQ_MASK	(1U << 17U)
#define FPD_GICP_GPIO_WAKE_IRQ_MASK	(1U << 16U)
#define FPD_GICP_SPI_WAKE_IRQ_MASK	(1U << 15U)
#define FPD_GICP_NAND_WAKE_IRQ_MASK	(1U << 14U)

/* GIC Proxy group 1 */
#define FPD_GICP_ETH3_WAKE_IRQ_MASK	(1U << 31U)
#define FPD_GICP_ETH2_WAKE_IRQ_MASK	(1U << 29U)
#define FPD_GICP_ETH1_WAKE_IRQ_MASK	(1U << 27U)
#define FPD_GICP_ETH0_WAKE_IRQ_MASK	(1U << 25U)
#define FPD_GICP_SD1_WAKE_IRQ_MASK	(1U << 19U)
#define FPD_GICP_SD0_WAKE_IRQ_MASK	(1U << 18U)
#define FPD_GICP_TTC3_WAKE_IRQ_MASK	(1U << 13U)
#define FPD_GICP_TTC2_WAKE_IRQ_MASK	(1U << 10U)
#define FPD_GICP_TTC1_WAKE_IRQ_MASK	(1U << 7U)
#define FPD_GICP_TTC0_WAKE_IRQ_MASK	(1U << 4U)
#define FPD_GICP_IPI_APU_WAKE_IRQ_MASK	(1U << 3U)

/* GIC Proxy group 2 */
#define FPD_GICP_USB1_WAKE_IRQ_MASK (1U << 12U)
#define FPD_GICP_USB0_WAKE_IRQ_MASK (1U << 11U)

/* GIC Proxy group 4 */
#define FPD_GICP_SATA_WAKE_IRQ_MASK (1U << 5U)


/**
 * PmGicProxyWake - Properties of the GIC Proxy wake event
 * @mask	Interrupt mask associated with the slave's wake event in the
 *		GIC Proxy group
 * @group	Index of the group containing the interrupt in the GIC Proxy
 */
typedef struct {
	const u32 mask;
	const u8 group;
} PmGicProxyWake;

/**
 * GicProxyGroup - Properties of a GIC Proxy group
 * @setMask	When GIC Proxy is enabled, enable the interrupts whose masks
 *		are set in this variable
 */
typedef struct {
	u32 setMask;
} PmGicProxyGroup;

/**
 * PmGicProxy - Structure containing GIC Proxy properties
 * @groups	Pointer to the array of GIC Proxy groups
 * @groupsCnt	Number of elements in the array of GIC Proxy groups
 * @setWake	Function to be called in order to inform the GIC Proxy about
 *		the set wake source being enabled/disabled for a slave
 * @clear	Clear all set wake-up sources (flags for all groups)
 * @enable	Function that enables GIC Proxy and all interrupts that are set
 *		as wake sources
 * @flags	GIC Proxy flags (is enabled or not)
 */
typedef struct {
	PmGicProxyGroup* const groups;
	int (*const setWake)(const PmSlave* const slv, const u32 enable);
	void (*const clear)(void);
	void (*const enable)(void);
	const u8 groupsCnt;
	u8 flags;
} PmGicProxy;

/*********************************************************************
 * Global data declarations
 ********************************************************************/
extern PmGicProxy pmGicProxy;

#endif
