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

#include "pm_gic_proxy.h"
#include "pm_slave.h"
#include "lpd_slcr.h"
#include "pm_periph.h"

/* GIC Proxy base address */
#define GIC_PROXY_BASE_ADDR		LPD_SLCR_GICP0_IRQ_STATUS
#define GIC_PROXY_GROUP_OFFSET(g)	(0x14U * (g))

/* GIC Proxy register offsets */
#define GIC_PROXY_IRQ_STATUS_OFFSET	0x0U
#define GIC_PROXY_IRQ_ENABLE_OFFSET	0x8U
#define GIC_PROXY_IRQ_DISABLE_OFFSET	0xCU

#define PM_GIC_PROXY_IS_ENABLED		0x1U

/**
 * PmGicProxySetWake() - Set/clear GIC Proxy wake
 * @slv		Pointer to the slave whose interrupt needs to be set/cleared
 * @enable	Flag stating that the slave's interrupt shall be enabled (!0)
 *		or disabled. The value of this flag is forwarded from the
 *		PmSetWakeupSource API.
 *
 * @return	XST_SUCCESS if processed correctly, XST_FAILURE if interrupt
 *		for the given slave is not found
 */
static int PmGicProxySetWake(const PmSlave* const slv, const u32 enable)
{
	int status = XST_FAILURE;
	u32 addr;

	if ((NULL == slv->wake) || (slv->wake->group >= pmGicProxy.groupsCnt)) {
		goto done;
	}

	if (0U != enable) {
		/* Calculate address of the status register */
		addr = GIC_PROXY_BASE_ADDR +
		       GIC_PROXY_GROUP_OFFSET(slv->wake->group) +
		       GIC_PROXY_IRQ_STATUS_OFFSET;

		/* Write 1 into status register to clear interrupt */
		XPfw_Write32(addr, slv->wake->mask);

		/* Remember which interrupt in the group needs to be enabled */
		pmGicProxy.groups[slv->wake->group].setMask |= slv->wake->mask;
	} else {
		/* Clear remembered flag, interrupt shall not be enabled */
		pmGicProxy.groups[slv->wake->group].setMask &= ~slv->wake->mask;
	}

	status = XST_SUCCESS;

done:
	return status;
}

/**
 * PmGicProxyEnable() - Enable all interrupts that are requested
 */
static void PmGicProxyEnable(void)
{
	u32 g;

	/* Always enable APU's IPI as the wake-up source (callback wake-up) */
	PmGicProxySetWake(&pmSlaveIpiApu_g, 1U);

	for (g = 0U; g < pmGicProxy.groupsCnt; g++) {
		u32 addr = GIC_PROXY_BASE_ADDR +
			   GIC_PROXY_GROUP_OFFSET(g) +
			   GIC_PROXY_IRQ_ENABLE_OFFSET;

		/* Clear GIC Proxy group interrupt */
		XPfw_Write32(LPD_SLCR_GICP_PMU_IRQ_STATUS, 1U << g);

		/* Enable GIC Proxy group interrupt */
		XPfw_Write32(LPD_SLCR_GICP_PMU_IRQ_ENABLE, 1U << g);

		/* Enable interrupts in the group that are set as wake */
		XPfw_Write32(addr, pmGicProxy.groups[g].setMask);
	}

	/* Enable GPI1 FPD GIC Proxy wake event */
	ENABLE_WAKE(PMU_LOCAL_GPI1_ENABLE_FPD_WAKE_GIC_PROX_MASK);

	pmGicProxy.flags |= PM_GIC_PROXY_IS_ENABLED;
}

/**
 * PmGicProxyDisable() - Disable all interrupts in the GIC Proxy
 */
static void PmGicProxyDisable(void)
{
	u32 g;

	for (g = 0U; g < pmGicProxy.groupsCnt; g++) {
		u32 disableAddr = GIC_PROXY_BASE_ADDR +
				  GIC_PROXY_GROUP_OFFSET(g) +
				  GIC_PROXY_IRQ_DISABLE_OFFSET;
		u32 statusAddr = GIC_PROXY_BASE_ADDR +
				 GIC_PROXY_GROUP_OFFSET(g) +
				 GIC_PROXY_IRQ_STATUS_OFFSET;

		/* Clear all interrupts in the GIC Proxy group */
		XPfw_Write32(statusAddr, ~0U);

		/* Disable all interrupts in the GIC Proxy group */
		XPfw_Write32(disableAddr, ~0U);

		/* Disable GIC Proxy group interrupt */
		XPfw_Write32(LPD_SLCR_GICP_PMU_IRQ_DISABLE, 1U << g);
	}

	/* Disable FPD GPI1 wake event */
	DISABLE_WAKE(PMU_LOCAL_GPI1_ENABLE_FPD_WAKE_GIC_PROX_MASK);

	pmGicProxy.flags &= ~PM_GIC_PROXY_IS_ENABLED;
}

/**
 * PmGicProxyClear() - Clear wake-up sources
 */
static void PmGicProxyClear(void)
{
	u32 g;

	for (g = 0U; g < pmGicProxy.groupsCnt; g++) {
		pmGicProxy.groups[g].setMask = 0U;
	}

	if (0U != (pmGicProxy.flags & PM_GIC_PROXY_IS_ENABLED)) {
		PmGicProxyDisable();
	}
}

/* FPD GIC Proxy has interrupts organized in 5 groups */
static PmGicProxyGroup pmGicProxyGroups[5];

PmGicProxy pmGicProxy = {
	.groups = pmGicProxyGroups,
	.groupsCnt = ARRAY_SIZE(pmGicProxyGroups),
	.setWake = PmGicProxySetWake,
	.clear = PmGicProxyClear,
	.enable = PmGicProxyEnable,
	.flags = 0U,
};
