/*
* Copyright (c) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */

#include "xpfw_config.h"
#ifdef ENABLE_PM

#include "pm_periph.h"
#include "pm_common.h"
#include "pm_node.h"
#include "pm_master.h"
#include "xpfw_rom_interface.h"
#include "lpd_slcr.h"
#include "pm_gic_proxy.h"
#include "pm_requirement.h"
#include "pm_sram.h"
#include "xpfw_default.h"

/* Always-on slave has only one state */
#define PM_AON_SLAVE_STATE	0U
#define PM_AON_SLAVE_MAX_STATE	1U

static const u8 pmAonFsmStates[PM_AON_SLAVE_MAX_STATE] = {
	[PM_AON_SLAVE_STATE] = PM_CAP_WAKEUP | PM_CAP_ACCESS | PM_CAP_CONTEXT,
};

static const PmSlaveFsm pmSlaveAonFsm = {
	DEFINE_SLAVE_STATES(pmAonFsmStates),
	.trans = NULL,
	.transCnt = 0U,
	.enterState = NULL,
};

static u8 pmSlaveAonPowers[] = {
	DEFAULT_POWER_ON,
};

#define PM_GENERIC_SLAVE_STATE_UNUSED	0U
#define PM_GENERIC_SLAVE_STATE_RUNNING	1U
#define PM_GENERIC_SLAVE_MAX_STATE	2U

/* Generic slaves state transition latency values */
#define PM_GENERIC_SLAVE_UNUSED_TO_RUNNING_LATENCY	304U
#define PM_GENERIC_SLAVE_RUNNING_TO_UNUSED_LATENCY	6U

static const u8 pmGenericSlaveStates[PM_GENERIC_SLAVE_MAX_STATE] = {
	[PM_GENERIC_SLAVE_STATE_UNUSED] = 0U,
	[PM_GENERIC_SLAVE_STATE_RUNNING] = PM_CAP_CONTEXT | PM_CAP_WAKEUP |
			PM_CAP_ACCESS | PM_CAP_CLOCK | PM_CAP_POWER,
};

static const PmStateTran pmGenericSlaveTransitions[] = {
	{
		.fromState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.toState = PM_GENERIC_SLAVE_STATE_UNUSED,
		.latency = PM_GENERIC_SLAVE_RUNNING_TO_UNUSED_LATENCY,
	}, {
		.fromState = PM_GENERIC_SLAVE_STATE_UNUSED,
		.toState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latency = PM_GENERIC_SLAVE_UNUSED_TO_RUNNING_LATENCY,
	},
};

static u8 pmGenericSlavePowers[] = {
	DEFAULT_POWER_OFF,
	DEFAULT_POWER_ON,
};

static const PmSlaveFsm pmGenericSlaveFsm = {
	DEFINE_SLAVE_STATES(pmGenericSlaveStates),
	DEFINE_SLAVE_TRANS(pmGenericSlaveTransitions),
	.enterState = NULL,
};

static PmWakeEventGicProxy pmRtcWake = {
	.wake = {
		.derived = &pmRtcWake,
		.class = &pmWakeEventClassGicProxy_g,
	},
	.mask = LPD_SLCR_GICP0_IRQ_MASK_SRC27_MASK |
		LPD_SLCR_GICP0_IRQ_MASK_SRC26_MASK,
	.group = 0U,
};

PmSlave pmSlaveRtc_g = {
	.node = {
		.derived = &pmSlaveRtc_g,
		.nodeId = NODE_RTC,
		.class = &pmNodeClassSlave_g,
		.parent = NULL,
		.clocks = NULL,
		.currState = PM_AON_SLAVE_STATE,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmSlaveAonPowers),
		DEFINE_NODE_NAME("rtc"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = &pmRtcWake.wake,
	.slvFsm = &pmSlaveAonFsm,
	.flags = 0U,
};

static PmWakeEventGicProxy pmTtc0Wake = {
	.wake = {
		.derived = &pmTtc0Wake,
		.class = &pmWakeEventClassGicProxy_g,
	},
	.mask = LPD_SLCR_GICP1_IRQ_MASK_SRC6_MASK |
		LPD_SLCR_GICP1_IRQ_MASK_SRC5_MASK |
		LPD_SLCR_GICP1_IRQ_MASK_SRC4_MASK,
	.group = 1U,
};

PmSlave pmSlaveTtc0_g = {
	.node = {
		.derived = &pmSlaveTtc0_g,
		.nodeId = NODE_TTC_0,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainLpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("ttc0"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = &pmTtc0Wake.wake,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

static PmWakeEventGicProxy pmTtc1Wake = {
	.wake = {
		.derived = &pmTtc1Wake,
		.class = &pmWakeEventClassGicProxy_g,
	},
	.mask = LPD_SLCR_GICP1_IRQ_MASK_SRC9_MASK |
		LPD_SLCR_GICP1_IRQ_MASK_SRC8_MASK |
		LPD_SLCR_GICP1_IRQ_MASK_SRC7_MASK,
	.group = 1U,
};

PmSlave pmSlaveTtc1_g = {
	.node = {
		.derived = &pmSlaveTtc1_g,
		.nodeId = NODE_TTC_1,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainLpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("ttc1"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = &pmTtc1Wake.wake,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

static PmWakeEventGicProxy pmTtc2Wake = {
	.wake = {
		.derived = &pmTtc2Wake,
		.class = &pmWakeEventClassGicProxy_g,
	},
	.mask = LPD_SLCR_GICP1_IRQ_MASK_SRC12_MASK |
		LPD_SLCR_GICP1_IRQ_MASK_SRC11_MASK |
		LPD_SLCR_GICP1_IRQ_MASK_SRC10_MASK,
	.group = 1U,
};

PmSlave pmSlaveTtc2_g = {
	.node = {
		.derived = &pmSlaveTtc2_g,
		.nodeId = NODE_TTC_2,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainLpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("ttc2"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = &pmTtc2Wake.wake,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

static PmWakeEventGicProxy pmTtc3Wake = {
	.wake = {
		.derived = &pmTtc3Wake,
		.class = &pmWakeEventClassGicProxy_g,
	},
	.mask = LPD_SLCR_GICP1_IRQ_MASK_SRC15_MASK |
		LPD_SLCR_GICP1_IRQ_MASK_SRC14_MASK |
		LPD_SLCR_GICP1_IRQ_MASK_SRC13_MASK,
	.group = 1U,
};

PmSlave pmSlaveTtc3_g = {
	.node = {
		.derived = &pmSlaveTtc3_g,
		.nodeId = NODE_TTC_3,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainLpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("ttc3"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = &pmTtc3Wake.wake,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

static PmWakeEventGicProxy pmUart0Wake = {
	.wake = {
		.derived = &pmUart0Wake,
		.class = &pmWakeEventClassGicProxy_g,
	},
	.mask = LPD_SLCR_GICP0_IRQ_MASK_SRC21_MASK,
	.group = 0U,
};

PmSlave pmSlaveUart0_g = {
	.node = {
		.derived = &pmSlaveUart0_g,
		.nodeId = NODE_UART_0,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainLpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("uart0"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = &pmUart0Wake.wake,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

static PmWakeEventGicProxy pmUart1Wake = {
	.wake = {
		.derived = &pmUart1Wake,
		.class = &pmWakeEventClassGicProxy_g,
	},
	.mask = LPD_SLCR_GICP0_IRQ_MASK_SRC22_MASK,
	.group = 0U,
};

PmSlave pmSlaveUart1_g = {
	.node = {
		.derived = &pmSlaveUart1_g,
		.nodeId = NODE_UART_1,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainLpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("uart1"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = &pmUart1Wake.wake,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

static PmWakeEventGicProxy pmSpi0Wake = {
	.wake = {
		.derived = &pmSpi0Wake,
		.class = &pmWakeEventClassGicProxy_g,
	},
	.mask = LPD_SLCR_GICP0_IRQ_MASK_SRC19_MASK,
	.group = 0U,
};

PmSlave pmSlaveSpi0_g = {
	.node = {
		.derived = &pmSlaveSpi0_g,
		.nodeId = NODE_SPI_0,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainLpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("spi0"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = &pmSpi0Wake.wake,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

static PmWakeEventGicProxy pmSpi1Wake = {
	.wake = {
		.derived = &pmSpi1Wake,
		.class = &pmWakeEventClassGicProxy_g,
	},
	.mask = LPD_SLCR_GICP0_IRQ_MASK_SRC20_MASK,
	.group = 0U,
};

PmSlave pmSlaveSpi1_g = {
	.node = {
		.derived = &pmSlaveSpi1_g,
		.nodeId = NODE_SPI_1,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainLpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("spi1"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = &pmSpi1Wake.wake,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

static PmWakeEventGicProxy pmI2C0Wake = {
	.wake = {
		.derived = &pmI2C0Wake,
		.class = &pmWakeEventClassGicProxy_g,
	},
	.mask = LPD_SLCR_GICP0_IRQ_MASK_SRC17_MASK,
	.group = 0U,
};

PmSlave pmSlaveI2C0_g = {
	.node = {
		.derived = &pmSlaveI2C0_g,
		.nodeId = NODE_I2C_0,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainLpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("i2c0"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = &pmI2C0Wake.wake,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

static PmWakeEventGicProxy pmI2C1Wake = {
	.wake = {
		.derived = &pmI2C1Wake,
		.class = &pmWakeEventClassGicProxy_g,
	},
	.mask = LPD_SLCR_GICP0_IRQ_MASK_SRC18_MASK,
	.group = 0U,
};

PmSlave pmSlaveI2C1_g = {
	.node = {
		.derived = &pmSlaveI2C1_g,
		.nodeId = NODE_I2C_1,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainLpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("i2c1"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = &pmI2C1Wake.wake,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

static PmWakeEventGicProxy pmSD0Wake = {
	.wake = {
		.derived = &pmSD0Wake,
		.class = &pmWakeEventClassGicProxy_g,
	},
	.mask = LPD_SLCR_GICP1_IRQ_MASK_SRC18_MASK |
		LPD_SLCR_GICP1_IRQ_MASK_SRC16_MASK,
	.group = 1U,
};

PmSlave pmSlaveSD0_g = {
	.node = {
		.derived = &pmSlaveSD0_g,
		.nodeId = NODE_SD_0,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainLpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("sd0"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = &pmSD0Wake.wake,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

static PmWakeEventGicProxy pmSD1Wake = {
	.wake = {
		.derived = &pmSD1Wake,
		.class = &pmWakeEventClassGicProxy_g,
	},
	.mask = LPD_SLCR_GICP1_IRQ_MASK_SRC19_MASK |
		LPD_SLCR_GICP1_IRQ_MASK_SRC17_MASK,
	.group = 1U,
};

PmSlave pmSlaveSD1_g = {
	.node = {
		.derived = &pmSlaveSD1_g,
		.nodeId = NODE_SD_1,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainLpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("sd1"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = &pmSD1Wake.wake,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

static PmWakeEventGicProxy pmCan0Wake = {
	.wake = {
		.derived = &pmCan0Wake,
		.class = &pmWakeEventClassGicProxy_g,
	},
	.mask = LPD_SLCR_GICP0_IRQ_MASK_SRC23_MASK,
	.group = 0U,
};

PmSlave pmSlaveCan0_g = {
	.node = {
		.derived = &pmSlaveCan0_g,
		.nodeId = NODE_CAN_0,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainLpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("can0"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = &pmCan0Wake.wake,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

static PmWakeEventGicProxy pmCan1Wake = {
	.wake = {
		.derived = &pmCan1Wake,
		.class = &pmWakeEventClassGicProxy_g,
	},
	.mask = LPD_SLCR_GICP0_IRQ_MASK_SRC24_MASK,
	.group = 0U,
};

PmSlave pmSlaveCan1_g = {
	.node = {
		.derived = &pmSlaveCan1_g,
		.nodeId = NODE_CAN_1,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainLpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("can1"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = &pmCan1Wake.wake,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

/* Size required by Ethernet in word of OCM */
#define ETH_OCM_REQ_SIZE		6
/* OCM address used for receive queue pointer */
#define RECV_Q_OCM_ADDR			0xFFFFFF80U

#define ETH_RECV_ENABLE_MASK		0x4U
#define ETH_RECV_Q_PTR_OFFSET		0x018U
#define ETH_RECV_Q1_PTR_OFFSET		0x480U
#define ETH_RECV_HIGH_PTR_OFFSET	0x4D4U

static u32 ocmData[ETH_OCM_REQ_SIZE];
static bool ocmStored;

/**
 * PmWakeEventEthConfig() - Configure propagation of ethernet wake event
 * @wake	Wake event
 * @ipiMask	IPI mask of the master which configures the wake
 * @enable	Flag: for enable non-zero value, for disable value zero
 */
static void PmWakeEventEthConfig(PmWakeEvent* const wake, const u32 ipiMask,
				 const u32 enable)
{
	u32 i;
	PmRequirement* req = PmRequirementGetNoMaster(&pmSlaveOcm3_g.slv);
	PmWakeEventEth* ethWake = (PmWakeEventEth*)wake->derived;

	/* Return if ethernet base address is not available */
	if (0U == ethWake->baseAddr) {
		return;
	}

	if ((0U == enable) && ethWake->wakeEnabled) {
		/* Disable GEM Rx in network contorl register */
		XPfw_RMW32(ethWake->baseAddr, ETH_RECV_ENABLE_MASK,
			   ~ETH_RECV_ENABLE_MASK);

		/* Restore receive queue pointer */
		XPfw_Write32(ethWake->baseAddr + ETH_RECV_Q_PTR_OFFSET,
			     ethWake->receiveQptr);
		XPfw_Write32(ethWake->baseAddr + ETH_RECV_Q1_PTR_OFFSET,
			     ethWake->receiveQ1ptr);
		XPfw_Write32(ethWake->baseAddr + ETH_RECV_HIGH_PTR_OFFSET,
			     ethWake->receiveHighptr);

		if (ocmStored) {
			/*
			 * Restore OCM Bank3's memory which is used as receive
			 * queue pointer
			 */
			for (i = 0; i < ETH_OCM_REQ_SIZE; i++) {
				XPfw_Write32(RECV_Q_OCM_ADDR + (i * 4U),
					     ocmData[i]);
			}
			ocmStored = false;
		}

		/* Enable GEM Rx in network control register */
		XPfw_RMW32(ethWake->baseAddr, ETH_RECV_ENABLE_MASK,
			   ETH_RECV_ENABLE_MASK);

		/* Change OCM Bank3 requirement to default */
		if (XST_SUCCESS != PmRequirementUpdate(req, req->defaultReq)) {
			PmWarn("Error in update OCM Bank3 requirement to default\r\n");
		}

		ethWake->wakeEnabled = false;
	}
}

/**
 * PmWakeEventEthSet() - Set Ethernet wake event as the wake source
 * @wake	Wake event
 * @ipiMask	IPI mask of the master which sets the wake source
 * @enable	Flag: for enable non-zero value, for disable value zero
 */
static void PmWakeEventEthSet(PmWakeEvent* const wake, const u32 ipiMask,
			      const u32 enable)
{
	u32 i;
	PmRequirement* req = PmRequirementGetNoMaster(&pmSlaveOcm3_g.slv);
	PmWakeEventEth* ethWake = (PmWakeEventEth*)wake->derived;

	/* Return if ethernet base address is not available */
	if (0U == ethWake->baseAddr) {
		return;
	}

	if (enable != 0U) {
		/* Keep OCM Bank3 ON while suspend */
		if (XST_SUCCESS != PmRequirementUpdate(req, PM_CAP_ACCESS)) {
			PmWarn("Error in requirement update for OCM Bank3\r\n");
		}

		if (0U == ocmStored) {
			/*
			 * Store OCM Bank-3's memory which is going to be used
			 * as receive pointer
			 */
			for (i = 0; i < ETH_OCM_REQ_SIZE; i++) {
				ocmData[i] = XPfw_Read32(RECV_Q_OCM_ADDR +
							 (i * 4U));
			}
			ocmStored = true;
		}

		/* Store receive queue pointer */
		ethWake->receiveQptr = XPfw_Read32(ethWake->baseAddr +
						   ETH_RECV_Q_PTR_OFFSET);
		ethWake->receiveQ1ptr = XPfw_Read32(ethWake->baseAddr +
						    ETH_RECV_Q1_PTR_OFFSET);
		ethWake->receiveHighptr = XPfw_Read32(ethWake->baseAddr +
						    ETH_RECV_HIGH_PTR_OFFSET);

		/* Disable GEM Rx in network contorl register */
		XPfw_RMW32(ethWake->baseAddr, ETH_RECV_ENABLE_MASK,
			   ~ETH_RECV_ENABLE_MASK);

		/* Prepare OCM memory to use as receive queue */
		XPfw_Write32(RECV_Q_OCM_ADDR, 0x3U);
		XPfw_Write32(RECV_Q_OCM_ADDR + 0x4U, 0x0U);
		XPfw_Write32(RECV_Q_OCM_ADDR + 0x8U, 0x0U);
		XPfw_Write32(RECV_Q_OCM_ADDR + 0xCU, 0x0U);
		XPfw_Write32(RECV_Q_OCM_ADDR + 0x10U, 0x0U);
		XPfw_Write32(RECV_Q_OCM_ADDR + 0x14U, 0x0U);

		/* Change receive queue pointer to OCM address */
		XPfw_Write32(ethWake->baseAddr + ETH_RECV_Q_PTR_OFFSET,
			     RECV_Q_OCM_ADDR);
		XPfw_Write32(ethWake->baseAddr + ETH_RECV_Q1_PTR_OFFSET,
			     RECV_Q_OCM_ADDR);
		XPfw_Write32(ethWake->baseAddr + ETH_RECV_HIGH_PTR_OFFSET, 0U);

		/* Enable GEM Rx in network control register */
		XPfw_RMW32(ethWake->baseAddr, ETH_RECV_ENABLE_MASK,
			   ETH_RECV_ENABLE_MASK);

		ethWake->subClass->set(ethWake->subWake, ipiMask, enable);
		ethWake->wakeEnabled = true;
	}
}

static PmWakeEventClass pmWakeEventClassEth_g = {
	.set = PmWakeEventEthSet,
	.config = PmWakeEventEthConfig,
};

static PmWakeEventGicProxy pmEth0GicWake = {
	.wake = {
		.derived = &pmEth0GicWake,
		.class = &pmWakeEventClassGicProxy_g,
	},
	.mask = LPD_SLCR_GICP1_IRQ_MASK_SRC26_MASK,
	.group = 1U,
};

static PmWakeEventEth pmEth0Wake = {
	.wake = {
		.derived = &pmEth0Wake,
		.class = &pmWakeEventClassEth_g,
	},
	.subClass = &pmWakeEventClassGicProxy_g,
	.subWake = &pmEth0GicWake.wake,
	.wakeEnabled = false,
	.receiveQptr = 0U,
	.receiveQ1ptr = 0U,
#ifdef XPMU_ETHERNET_0
	.baseAddr = XPMU_ETHERNET_0_BASEADDR,
#else
	.baseAddr = 0U,
#endif
};

PmSlave pmSlaveEth0_g = {
	.node = {
		.derived = &pmSlaveEth0_g,
		.nodeId = NODE_ETH_0,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainLpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("eth0"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = &pmEth0Wake.wake,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

static PmWakeEventGicProxy pmEth1GicWake = {
	.wake = {
		.derived = &pmEth1GicWake,
		.class = &pmWakeEventClassGicProxy_g,
	},
	.mask = LPD_SLCR_GICP1_IRQ_MASK_SRC28_MASK,
	.group = 1U,
};

static PmWakeEventEth pmEth1Wake = {
	.wake = {
		.derived = &pmEth1Wake,
		.class = &pmWakeEventClassEth_g,
	},
	.subClass = &pmWakeEventClassGicProxy_g,
	.subWake = &pmEth1GicWake.wake,
	.wakeEnabled = false,
	.receiveQptr = 0U,
	.receiveQ1ptr = 0U,
#ifdef XPMU_ETHERNET_1
	.baseAddr = XPMU_ETHERNET_1_BASEADDR,
#else
	.baseAddr = 0U,
#endif
};

PmSlave pmSlaveEth1_g = {
	.node = {
		.derived = &pmSlaveEth1_g,
		.nodeId = NODE_ETH_1,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainLpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("eth1"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = &pmEth1Wake.wake,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

static PmWakeEventGicProxy pmEth2GicWake = {
	.wake = {
		.derived = &pmEth2GicWake,
		.class = &pmWakeEventClassGicProxy_g,
	},
	.mask = LPD_SLCR_GICP1_IRQ_MASK_SRC30_MASK,
	.group = 1U,
};

static PmWakeEventEth pmEth2Wake = {
	.wake = {
		.derived = &pmEth2Wake,
		.class = &pmWakeEventClassEth_g,
	},
	.subClass = &pmWakeEventClassGicProxy_g,
	.subWake = &pmEth2GicWake.wake,
	.wakeEnabled = false,
	.receiveQptr = 0U,
	.receiveQ1ptr = 0U,
#ifdef XPMU_ETHERNET_2
	.baseAddr = XPMU_ETHERNET_2_BASEADDR,
#else
	.baseAddr = 0U,
#endif
};

PmSlave pmSlaveEth2_g = {
	.node = {
		.derived = &pmSlaveEth2_g,
		.nodeId = NODE_ETH_2,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainLpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("eth2"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = &pmEth2Wake.wake,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

static PmWakeEventGicProxy pmEth3GicWake = {
	.wake = {
		.derived = &pmEth3GicWake,
		.class = &pmWakeEventClassGicProxy_g,
	},
	.mask = LPD_SLCR_GICP2_IRQ_MASK_SRC0_MASK,
	.group = 2U,
};

static PmWakeEventEth pmEth3Wake = {
	.wake = {
		.derived = &pmEth3Wake,
		.class = &pmWakeEventClassEth_g,
	},
	.subClass = &pmWakeEventClassGicProxy_g,
	.subWake = &pmEth3GicWake.wake,
	.wakeEnabled = false,
	.receiveQptr = 0U,
	.receiveQ1ptr = 0U,
#ifdef XPMU_ETHERNET_3
	.baseAddr = XPMU_ETHERNET_3_BASEADDR,
#else
	.baseAddr = 0U,
#endif
};

PmSlave pmSlaveEth3_g = {
	.node = {
		.derived = &pmSlaveEth3_g,
		.nodeId = NODE_ETH_3,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainLpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("eth3"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = &pmEth3Wake.wake,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

static PmWakeEventGicProxy pmAdmaWake = {
	.wake = {
		.derived = &pmAdmaWake,
		.class = &pmWakeEventClassGicProxy_g,
	},
	.mask = LPD_SLCR_GICP2_IRQ_MASK_SRC19_MASK |
		LPD_SLCR_GICP2_IRQ_MASK_SRC18_MASK |
		LPD_SLCR_GICP2_IRQ_MASK_SRC17_MASK |
		LPD_SLCR_GICP2_IRQ_MASK_SRC16_MASK |
		LPD_SLCR_GICP2_IRQ_MASK_SRC15_MASK |
		LPD_SLCR_GICP2_IRQ_MASK_SRC14_MASK |
		LPD_SLCR_GICP2_IRQ_MASK_SRC13_MASK |
		LPD_SLCR_GICP2_IRQ_MASK_SRC12_MASK,
	.group = 2U,
};

PmSlave pmSlaveAdma_g = {
	.node = {
		.derived = &pmSlaveAdma_g,
		.nodeId = NODE_ADMA,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainLpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("adma"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = &pmAdmaWake.wake,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

static PmWakeEventGicProxy pmNandWake = {
	.wake = {
		.derived = &pmNandWake,
		.class = &pmWakeEventClassGicProxy_g,
	},
	.mask = LPD_SLCR_GICP0_IRQ_MASK_SRC14_MASK,
	.group = 0U,
};

PmSlave pmSlaveNand_g = {
	.node = {
		.derived = &pmSlaveNand_g,
		.nodeId = NODE_NAND,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainLpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("nand"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = &pmNandWake.wake,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

static PmWakeEventGicProxy pmQSpiWake = {
	.wake = {
		.derived = &pmQSpiWake,
		.class = &pmWakeEventClassGicProxy_g,
	},
	.mask = LPD_SLCR_GICP0_IRQ_MASK_SRC15_MASK,
	.group = 0U,
};

PmSlave pmSlaveQSpi_g = {
	.node = {
		.derived = &pmSlaveQSpi_g,
		.nodeId = NODE_QSPI,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainLpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("qspi"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = &pmQSpiWake.wake,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

static PmWakeEventGicProxy pmGpioWake = {
	.wake = {
		.derived = &pmGpioWake,
		.class = &pmWakeEventClassGicProxy_g,
	},
	.mask = LPD_SLCR_GICP0_IRQ_MASK_SRC16_MASK,
	.group = 0U,
};

PmSlave pmSlaveGpio_g = {
	.node = {
		.derived = &pmSlaveGpio_g,
		.nodeId = NODE_GPIO,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainLpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("gpio"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = &pmGpioWake.wake,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

PmSlave pmSlaveSata_g = {
	.node = {
		.derived = &pmSlaveSata_g,
		.nodeId = NODE_SATA,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainFpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("sata"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = NULL,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

PmSlave pmSlavePcie_g = {
	.node = {
		.derived = &pmSlavePcie_g,
		.nodeId = NODE_PCIE,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainFpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("pcie"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = NULL,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

PmSlave pmSlavePcap_g = {
	.node = {
		.derived = &pmSlavePcap_g,
		.nodeId = NODE_PCAP,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainLpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("pcap"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = NULL,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

PmSlave pmSlaveGdma_g = {
	.node = {
		.derived = &pmSlaveGdma_g,
		.nodeId = NODE_GDMA,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainFpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("gdma"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = NULL,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

PmSlave pmSlaveDP_g = {
	.node = {
		.derived = &pmSlaveDP_g,
		.nodeId = NODE_DP,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainFpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("dp"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = NULL,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

static PmWakeEventGicProxy pmIpiApuWake = {
	.wake = {
		.derived = &pmIpiApuWake,
		.class = &pmWakeEventClassGicProxy_g,
	},
	.mask = LPD_SLCR_GICP1_IRQ_MASK_SRC3_MASK,
	.group = 1U,
};

PmSlave pmSlaveIpiApu_g = {
	.node = {
		.derived = &pmSlaveIpiApu_g,
		.nodeId = NODE_IPI_APU,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainLpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("ipi_apu"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = &pmIpiApuWake.wake,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

PmSlave pmSlaveIpiRpu0_g = {
	.node = {
		.derived = &pmSlaveIpiRpu0_g,
		.nodeId = NODE_IPI_RPU_0,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainLpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("ipi_rpu0"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = NULL,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

PmSlave pmSlaveIpiRpu1_g = {
	.node = {
		.derived = &pmSlaveIpiRpu1_g,
		.nodeId = NODE_IPI_RPU_1,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainLpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("ipi_rpu1"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = NULL,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

PmSlave pmSlaveIpiPl0_g = {
	.node = {
		.derived = &pmSlaveIpiPl0_g,
		.nodeId = NODE_IPI_PL_0,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainLpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("ipi_pl0"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = NULL,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

PmSlave pmSlaveIpiPl1_g = {
	.node = {
		.derived = &pmSlaveIpiPl1_g,
		.nodeId = NODE_IPI_PL_1,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainLpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("ipi_pl1"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = NULL,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

PmSlave pmSlaveIpiPl2_g = {
	.node = {
		.derived = &pmSlaveIpiPl2_g,
		.nodeId = NODE_IPI_PL_2,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainLpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("ipi_pl2"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = NULL,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

PmSlave pmSlaveIpiPl3_g = {
	.node = {
		.derived = &pmSlaveIpiPl3_g,
		.nodeId = NODE_IPI_PL_3,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainLpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("ipi_pl3"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = NULL,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

PmSlave pmSlavePl_g = {
	.node = {
		.derived = &pmSlavePl_g,
		.nodeId = NODE_PL,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainPld_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
		DEFINE_NODE_NAME("pl"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = NULL,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

PmSlave pmSlaveFpdWdt_g = {
	.node = {
		.derived = &pmSlaveFpdWdt_g,
		.nodeId = NODE_SWDT_1,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainFpd_g.power,
		.clocks = NULL,
		.currState = PM_GENERIC_SLAVE_STATE_RUNNING,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(pmGenericSlavePowers),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = NULL,
	.slvFsm = &pmGenericSlaveFsm,
	.flags = 0U,
};

#endif
