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
#include "pm_usb.h"

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
	const PmWakeEventEth *eth0Wake = pmSlaveEth0_g.wake->derived;
	const PmWakeEventEth *eth1Wake = pmSlaveEth1_g.wake->derived;
	const PmWakeEventEth *eth2Wake = pmSlaveEth2_g.wake->derived;
	const PmWakeEventEth *eth3Wake = pmSlaveEth3_g.wake->derived;
	u32 regVal;

	/* Only APU's interrupts are routed through GIC Proxy */
	if (ipiMask != pmMasterApu_g.ipiMask) {
		goto done;
	}

	if (0U == enable) {
		pmGicProxy.groups[gicWake->group].setMask &= ~gicWake->mask;
	} else {

		/* Keep FPD ON if USB is set as wakeup source in USB3 mode */
		if (pmSlaveUsb0_g.slv.wake->derived == gicWake) {

			/*
			 * Check USB3_0_FPD_PIPE_CLK[0] to identify USB mode
			 * 0 => USB3.0 enabled
			 * 1 => USB2.0 only enabled
			 */
			regVal = XPfw_Read32(USB3_0_FPD_PIPE_CLK);
			if (0U == (regVal & USB3_0_FPD_PIPE_CLK_OPTION_MASK)) {
				pmPowerDomainFpd_g.power.useCount++;
			}
		} else if (eth0Wake->subWake->derived == gicWake) {
			/* Keep FPD ON if WOL is set as wakeup source in ETH0 mode */
			regVal = XPfw_Read32(PM_ETH_0_DEF_BASEADDR + PM_GEM_NETWORK_CONFIG_OFFSET);
			if ((PM_GEM_NETWORK_CONFIG_PCS_SELECT_MASK) ==
			    (regVal & PM_GEM_NETWORK_CONFIG_PCS_SELECT_MASK)) {
				pmPowerDomainFpd_g.power.useCount++;
			}
		} else if (eth1Wake->subWake->derived == gicWake) {
			/* Keep FPD ON if WOL is set as wakeup source in ETH1 mode */
			regVal = XPfw_Read32(PM_ETH_1_DEF_BASEADDR + PM_GEM_NETWORK_CONFIG_OFFSET);
			if ((PM_GEM_NETWORK_CONFIG_PCS_SELECT_MASK) ==
			    (regVal & PM_GEM_NETWORK_CONFIG_PCS_SELECT_MASK)) {
				pmPowerDomainFpd_g.power.useCount++;
			}
		} else if (eth2Wake->subWake->derived == gicWake) {
			/* Keep FPD ON if WOL is set as wakeup source in ETH2 mode */
			regVal = XPfw_Read32(PM_ETH_2_DEF_BASEADDR + PM_GEM_NETWORK_CONFIG_OFFSET);
			if ((PM_GEM_NETWORK_CONFIG_PCS_SELECT_MASK) ==
			    (regVal & PM_GEM_NETWORK_CONFIG_PCS_SELECT_MASK)) {
				pmPowerDomainFpd_g.power.useCount++;
			}
		} else if (eth3Wake->subWake->derived == gicWake) {
			/* Keep FPD ON if WOL is set as wakeup source in ETH3 mode */
			regVal = XPfw_Read32(PM_ETH_3_DEF_BASEADDR + PM_GEM_NETWORK_CONFIG_OFFSET);
			if ((PM_GEM_NETWORK_CONFIG_PCS_SELECT_MASK) ==
			    (regVal & PM_GEM_NETWORK_CONFIG_PCS_SELECT_MASK)) {
				pmPowerDomainFpd_g.power.useCount++;
			}
		} else {
			/* Required by MISRA */
		}

		u32 addr = GIC_PROXY_BASE_ADDR +
			   GIC_PROXY_GROUP_OFFSET((u32)gicWake->group) +
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
		XPfw_Write32(LPD_SLCR_GICP_PMU_IRQ_STATUS, (u32)1 << g);

		/* Enable GIC Proxy group interrupt */
		XPfw_Write32(LPD_SLCR_GICP_PMU_IRQ_ENABLE, (u32)1 << g);

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
		XPfw_Write32(LPD_SLCR_GICP_PMU_IRQ_DISABLE, (u32)1 << g);
	}

	/* Disable FPD GPI1 wake event */
	DISABLE_WAKE(PMU_LOCAL_GPI1_ENABLE_FPD_WAKE_GIC_PROX_MASK);

	pmGicProxy.flags &= ~(u8)PM_GIC_PROXY_IS_ENABLED;
}

/**
 * PmGicProxyClear() - Clear wake-up sources
 */
static void PmGicProxyClear(void)
{
	u32 g, regVal;
	const PmWakeEventGicProxy* usbWake = (PmWakeEventGicProxy*) pmSlaveUsb0_g.slv.wake->derived;
	const PmWakeEventEth *eth0WakeEvent = pmSlaveEth0_g.wake->derived;
	const PmWakeEventEth *eth1WakeEvent = pmSlaveEth1_g.wake->derived;
	const PmWakeEventEth *eth2WakeEvent = pmSlaveEth2_g.wake->derived;
	const PmWakeEventEth *eth3WakeEvent = pmSlaveEth3_g.wake->derived;
	const PmWakeEventGicProxy* eth0Wake = (PmWakeEventGicProxy*) eth0WakeEvent->subWake->derived;
	const PmWakeEventGicProxy* eth1Wake = (PmWakeEventGicProxy*) eth1WakeEvent->subWake->derived;
	const PmWakeEventGicProxy* eth2Wake = (PmWakeEventGicProxy*) eth2WakeEvent->subWake->derived;
	const PmWakeEventGicProxy* eth3Wake = (PmWakeEventGicProxy*) eth3WakeEvent->subWake->derived;

	for (g = 0U; g < pmGicProxy.groupsCnt; g++) {

		/*
		 * Decrease FPD use count which was incremented
		 * in PmWakeEventGicProxySet().
		 * */
		if ((g == usbWake->group) && (0U != (pmGicProxy.groups[g].setMask & usbWake->mask))) {
			/*
			 * Check USB3_0_FPD_PIPE_CLK[0] to identify USB mode
			 * 0 => USB3.0 enabled
			 * 1 => USB2.0 only enabled
			 */
			regVal = XPfw_Read32(USB3_0_FPD_PIPE_CLK);
			if (0U == (regVal & USB3_0_FPD_PIPE_CLK_OPTION_MASK)) {
				pmPowerDomainFpd_g.power.useCount--;
			}
		} else if ((g == eth0Wake->group) && (0U != (pmGicProxy.groups[g].setMask & eth0Wake->mask))) {
			regVal = XPfw_Read32(PM_ETH_0_DEF_BASEADDR + PM_GEM_NETWORK_CONFIG_OFFSET);
			if ((PM_GEM_NETWORK_CONFIG_PCS_SELECT_MASK) ==
			    (regVal & PM_GEM_NETWORK_CONFIG_PCS_SELECT_MASK)) {
				pmPowerDomainFpd_g.power.useCount--;
			}
		} else if ((g == eth1Wake->group) && (0U != (pmGicProxy.groups[g].setMask & eth1Wake->mask))) {
			regVal = XPfw_Read32(PM_ETH_1_DEF_BASEADDR + PM_GEM_NETWORK_CONFIG_OFFSET);
			if ((PM_GEM_NETWORK_CONFIG_PCS_SELECT_MASK) ==
			    (regVal & PM_GEM_NETWORK_CONFIG_PCS_SELECT_MASK)) {
				pmPowerDomainFpd_g.power.useCount--;
			}
		} else if ((g == eth2Wake->group) && (0U != (pmGicProxy.groups[g].setMask & eth2Wake->mask))) {
			regVal = XPfw_Read32(PM_ETH_2_DEF_BASEADDR + PM_GEM_NETWORK_CONFIG_OFFSET);
			if ((PM_GEM_NETWORK_CONFIG_PCS_SELECT_MASK) ==
			    (regVal & PM_GEM_NETWORK_CONFIG_PCS_SELECT_MASK)) {
				pmPowerDomainFpd_g.power.useCount--;
			}
		} else if ((g == eth3Wake->group) && (0U != (pmGicProxy.groups[g].setMask & eth3Wake->mask))) {
			regVal = XPfw_Read32(PM_ETH_3_DEF_BASEADDR + PM_GEM_NETWORK_CONFIG_OFFSET);
			if ((PM_GEM_NETWORK_CONFIG_PCS_SELECT_MASK) ==
			    (regVal & PM_GEM_NETWORK_CONFIG_PCS_SELECT_MASK)) {
				pmPowerDomainFpd_g.power.useCount--;
			}
		} else {
			/* Required by MISRA */
		}
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
