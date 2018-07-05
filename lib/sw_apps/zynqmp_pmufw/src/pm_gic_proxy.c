/*
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */

#include "xpfw_config.h"
#ifdef ENABLE_PM

#include "pm_gic_proxy.h"
#include "pm_slave.h"
#include "lpd_slcr.h"
#include "pm_periph.h"
#include "pm_master.h"

/* GIC Proxy base address */
#define GIC_PROXY_BASE_ADDR		LPD_SLCR_GICP0_IRQ_STATUS
#define GIC_PROXY_GROUP_OFFSET(g)	(0x14U * (g))

/* GIC Proxy register offsets */
#define GIC_PROXY_IRQ_STATUS_OFFSET	0x0U
#define GIC_PROXY_IRQ_ENABLE_OFFSET	0x8U
#define GIC_PROXY_IRQ_DISABLE_OFFSET	0xCU

#define PM_GIC_PROXY_IS_ENABLED		0x1U

/**
 * PmWakeEventGicProxySet() - Set GIC Proxy wake event as the wake source
 * @wake	Wake event
 * @ipiMask	IPI mask of the master which sets the wake source
 * @enable	Flag: for enable non-zero value, for disable value zero
 */
static void PmWakeEventGicProxySet(PmWakeEvent* const wake, const u32 ipiMask,
				   const u32 enable)
{
	PmWakeEventGicProxy* gicWake = (PmWakeEventGicProxy*)wake->derived;

	/* Only APU's interrupts are routed through GIC Proxy */
	if (ipiMask != pmMasterApu_g.ipiMask) {
		goto done;
	}

	if (0U == enable) {
		pmGicProxy.groups[gicWake->group].setMask &= ~gicWake->mask;
	} else {
		u32 addr = GIC_PROXY_BASE_ADDR +
			   GIC_PROXY_GROUP_OFFSET(gicWake->group) +
			   GIC_PROXY_IRQ_STATUS_OFFSET;

		/* Write 1 into status register to clear interrupt */
		XPfw_Write32(addr, gicWake->mask);

		/* Remember which interrupt in the group needs to be enabled */
		pmGicProxy.groups[gicWake->group].setMask |= gicWake->mask;
	}

done:
	return;
}

/**
 * PmGicProxyEnable() - Enable all interrupts that are requested
 */
static void PmGicProxyEnable(void)
{
	u32 g;

	/* Always enable APU's IPI as the wake-up source (callback wake-up) */
	PmWakeEventGicProxySet(pmSlaveIpiApu_g.wake, pmMasterApu_g.ipiMask, 1U);

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
	.clear = PmGicProxyClear,
	.enable = PmGicProxyEnable,
	.flags = 0U,
};

/*
 * This event class doesn't have config method because the wake events are not
 * individually controlled. Instead, all GIC Proxy events are enabled when FPD
 * gets powered down and disabled when APU wakes, using the PmGicProxy methods.
 */
PmWakeEventClass pmWakeEventClassGicProxy_g = {
	.set = PmWakeEventGicProxySet,
	.config = NULL,
};

#endif
