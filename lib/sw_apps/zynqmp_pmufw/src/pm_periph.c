/*
 * Copyright (C) 2014 - 2015 Xilinx, Inc.  All rights reserved.
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

#include "pm_periph.h"
#include "pm_common.h"
#include "pm_node.h"
#include "pm_master.h"
#include "xpfw_rom_interface.h"
#include "lpd_slcr.h"

/*
 * States of a slave with its own power island (PI), who has dependencies to the
 * power parent (power domain) and have no wake-up capability through GIC Proxy.
 * In this case, those are graphics processors
 */
#define PM_GPP_SLAVE_STATE_OFF	0U
#define PM_GPP_SLAVE_STATE_ON	1U

/* Always-on slaves, have only one state */
#define PM_AON_SLAVE_STATE	0U

/*
 * Without clock/reset control, from PM perspective ttc has only one state.
 * It is in LPD, which is never turned off, does not sit in power island,
 * therefore has no off state.
 */
static const u32 pmAonFsmStates[] = {
	[PM_AON_SLAVE_STATE] = PM_CAP_WAKEUP | PM_CAP_ACCESS | PM_CAP_CONTEXT,
};

static const PmSlaveFsm slaveAonFsm = {
	.states = pmAonFsmStates,
	.statesCnt = ARRAY_SIZE(pmAonFsmStates),
	.trans = NULL,
	.transCnt = 0U,
	.enterState = NULL,
};

static u32 PmSlaveAonPowers[] = {
	DEFAULT_POWER_ON,
};

/* Definitions for the typical LPD slave with gateable clock */

#define PM_LPD_SLAVE_STATE_IDLE	0U
#define PM_LPD_SLAVE_STATE_RUN	1U

static const u32 pmLpdSlaveStates[] = {
	[PM_LPD_SLAVE_STATE_IDLE] = PM_CAP_CONTEXT,
	[PM_LPD_SLAVE_STATE_RUN] = PM_CAP_CONTEXT | PM_CAP_WAKEUP |
				   PM_CAP_ACCESS | PM_CAP_CLOCK,
};

static const PmStateTran pmLpdSlaveTransitions[] = {
	{
		.fromState = PM_LPD_SLAVE_STATE_RUN,
		.toState = PM_LPD_SLAVE_STATE_IDLE,
		.latency = PM_DEFAULT_LATENCY,
	}, {
		.fromState = PM_LPD_SLAVE_STATE_IDLE,
		.toState = PM_LPD_SLAVE_STATE_RUN,
		.latency = PM_DEFAULT_LATENCY,
	},
};

static u32 pmLpdSlavePowers[] = {
	DEFAULT_POWER_OFF,
	DEFAULT_POWER_ON,
};

static const PmSlaveFsm pmLpdSlaveFsm = {
	.states = pmLpdSlaveStates,
	.statesCnt = ARRAY_SIZE(pmLpdSlaveStates),
	.trans = pmLpdSlaveTransitions,
	.transCnt = ARRAY_SIZE(pmLpdSlaveTransitions),
	.enterState = NULL,
};

static PmGicProxyWake pmRtcWake = {
	.mask = LPD_SLCR_GICP0_IRQ_MASK_SRC27_MASK |
		LPD_SLCR_GICP0_IRQ_MASK_SRC26_MASK,
	.group = 0U,
};

PmSlave pmSlaveRtc_g = {
	.node = {
		.derived = &pmSlaveRtc_g,
		.nodeId = NODE_RTC,
		.typeId = PM_TYPE_SLAVE,
		.parent = NULL,
		.clocks = NULL,
		.currState = PM_AON_SLAVE_STATE,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = PmSlaveAonPowers,
		.powerInfoCnt = ARRAY_SIZE(PmSlaveAonPowers),
	},
	.reqs = NULL,
	.wake = &pmRtcWake,
	.slvFsm = &slaveAonFsm,
	.flags = 0U,
};

static PmGicProxyWake pmTtc0Wake = {
	.mask = LPD_SLCR_GICP1_IRQ_MASK_SRC6_MASK |
		LPD_SLCR_GICP1_IRQ_MASK_SRC5_MASK |
		LPD_SLCR_GICP1_IRQ_MASK_SRC4_MASK,
	.group = 1U,
};

PmSlaveTtc pmSlaveTtc0_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveTtc0_g,
			.nodeId = NODE_TTC_0,
			.typeId = PM_TYPE_TTC,
			.parent = &pmPowerDomainLpd_g,
			.currState = PM_AON_SLAVE_STATE,
			.latencyMarg = MAX_LATENCY,
			.ops = NULL,
			.powerInfo = PmSlaveAonPowers,
			.powerInfoCnt = ARRAY_SIZE(PmSlaveAonPowers),
		},
		.reqs = NULL,
		.wake = &pmTtc0Wake,
		.slvFsm = &slaveAonFsm,
		.flags = 0U,
	},
};

static PmGicProxyWake pmTtc1Wake = {
	.mask = LPD_SLCR_GICP1_IRQ_MASK_SRC9_MASK |
		LPD_SLCR_GICP1_IRQ_MASK_SRC8_MASK |
		LPD_SLCR_GICP1_IRQ_MASK_SRC7_MASK,
	.group = 1U,
};

PmSlaveTtc pmSlaveTtc1_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveTtc1_g,
			.nodeId = NODE_TTC_1,
			.typeId = PM_TYPE_TTC,
			.parent = &pmPowerDomainLpd_g,
			.currState = PM_AON_SLAVE_STATE,
			.latencyMarg = MAX_LATENCY,
			.ops = NULL,
			.powerInfo = PmSlaveAonPowers,
			.powerInfoCnt = ARRAY_SIZE(PmSlaveAonPowers),
		},
		.reqs = NULL,
		.wake = &pmTtc1Wake,
		.slvFsm = &slaveAonFsm,
		.flags = 0U,
	},
};

static PmGicProxyWake pmTtc2Wake = {
	.mask = LPD_SLCR_GICP1_IRQ_MASK_SRC12_MASK |
		LPD_SLCR_GICP1_IRQ_MASK_SRC11_MASK |
		LPD_SLCR_GICP1_IRQ_MASK_SRC10_MASK,
	.group = 1U,
};

PmSlaveTtc pmSlaveTtc2_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveTtc2_g,
			.nodeId = NODE_TTC_2,
			.typeId = PM_TYPE_TTC,
			.parent = &pmPowerDomainLpd_g,
			.currState = PM_AON_SLAVE_STATE,
			.latencyMarg = MAX_LATENCY,
			.ops = NULL,
			.powerInfo = PmSlaveAonPowers,
			.powerInfoCnt = ARRAY_SIZE(PmSlaveAonPowers),
		},
		.reqs = NULL,
		.wake = &pmTtc2Wake,
		.slvFsm = &slaveAonFsm,
		.flags = 0U,
	},
};

static PmGicProxyWake pmTtc3Wake = {
	.mask = LPD_SLCR_GICP1_IRQ_MASK_SRC15_MASK |
		LPD_SLCR_GICP1_IRQ_MASK_SRC14_MASK |
		LPD_SLCR_GICP1_IRQ_MASK_SRC13_MASK,
	.group = 1U,
};

PmSlaveTtc pmSlaveTtc3_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveTtc3_g,
			.nodeId = NODE_TTC_3,
			.typeId = PM_TYPE_TTC,
			.parent = &pmPowerDomainLpd_g,
			.currState = PM_AON_SLAVE_STATE,
			.latencyMarg = MAX_LATENCY,
			.ops = NULL,
			.powerInfo = PmSlaveAonPowers,
			.powerInfoCnt = ARRAY_SIZE(PmSlaveAonPowers),
		},
		.reqs = NULL,
		.wake = &pmTtc3Wake,
		.slvFsm = &slaveAonFsm,
		.flags = 0U,
	},
};

static PmGicProxyWake pmUart0Wake = {
	.mask = LPD_SLCR_GICP0_IRQ_MASK_SRC21_MASK,
	.group = 0U,
};

PmSlave pmSlaveUart0_g = {
	.node = {
		.derived = &pmSlaveUart0_g,
		.nodeId = NODE_UART_0,
		.typeId = PM_TYPE_SLAVE,
		.parent = &pmPowerDomainLpd_g,
		.clocks = NULL,
		.currState = PM_LPD_SLAVE_STATE_RUN,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = pmLpdSlavePowers,
		.powerInfoCnt = ARRAY_SIZE(pmLpdSlavePowers),
	},
	.reqs = NULL,
	.wake = &pmUart0Wake,
	.slvFsm = &pmLpdSlaveFsm,
	.flags = 0U,
};

static PmGicProxyWake pmUart1Wake = {
	.mask = LPD_SLCR_GICP0_IRQ_MASK_SRC22_MASK,
	.group = 0U,
};

PmSlave pmSlaveUart1_g = {
	.node = {
		.derived = &pmSlaveUart1_g,
		.nodeId = NODE_UART_1,
		.typeId = PM_TYPE_SLAVE,
		.parent = &pmPowerDomainLpd_g,
		.clocks = NULL,
		.currState = PM_LPD_SLAVE_STATE_RUN,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = pmLpdSlavePowers,
		.powerInfoCnt = ARRAY_SIZE(pmLpdSlavePowers),
	},
	.reqs = NULL,
	.wake = &pmUart1Wake,
	.slvFsm = &pmLpdSlaveFsm,
	.flags = 0U,
};

static PmGicProxyWake pmSpi0Wake = {
	.mask = LPD_SLCR_GICP0_IRQ_MASK_SRC19_MASK,
	.group = 0U,
};

PmSlave pmSlaveSpi0_g = {
	.node = {
		.derived = &pmSlaveSpi0_g,
		.nodeId = NODE_SPI_0,
		.typeId = PM_TYPE_SLAVE,
		.parent = &pmPowerDomainLpd_g,
		.clocks = NULL,
		.currState = PM_LPD_SLAVE_STATE_RUN,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = pmLpdSlavePowers,
		.powerInfoCnt = ARRAY_SIZE(pmLpdSlavePowers),
	},
	.reqs = NULL,
	.wake = &pmSpi0Wake,
	.slvFsm = &pmLpdSlaveFsm,
	.flags = 0U,
};

static PmGicProxyWake pmSpi1Wake = {
	.mask = LPD_SLCR_GICP0_IRQ_MASK_SRC20_MASK,
	.group = 0U,
};

PmSlave pmSlaveSpi1_g = {
	.node = {
		.derived = &pmSlaveSpi1_g,
		.nodeId = NODE_SPI_1,
		.typeId = PM_TYPE_SLAVE,
		.parent = &pmPowerDomainLpd_g,
		.clocks = NULL,
		.currState = PM_LPD_SLAVE_STATE_RUN,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = pmLpdSlavePowers,
		.powerInfoCnt = ARRAY_SIZE(pmLpdSlavePowers),
	},
	.reqs = NULL,
	.wake = &pmSpi1Wake,
	.slvFsm = &pmLpdSlaveFsm,
	.flags = 0U,
};

static PmGicProxyWake pmI2C0Wake = {
	.mask = LPD_SLCR_GICP0_IRQ_MASK_SRC17_MASK,
	.group = 0U,
};

PmSlave pmSlaveI2C0_g = {
	.node = {
		.derived = &pmSlaveI2C0_g,
		.nodeId = NODE_I2C_0,
		.typeId = PM_TYPE_SLAVE,
		.parent = &pmPowerDomainLpd_g,
		.clocks = NULL,
		.currState = PM_LPD_SLAVE_STATE_RUN,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = pmLpdSlavePowers,
		.powerInfoCnt = ARRAY_SIZE(pmLpdSlavePowers),
	},
	.reqs = NULL,
	.wake = &pmI2C0Wake,
	.slvFsm = &pmLpdSlaveFsm,
	.flags = 0U,
};

static PmGicProxyWake pmI2C1Wake = {
	.mask = LPD_SLCR_GICP0_IRQ_MASK_SRC18_MASK,
	.group = 0U,
};

PmSlave pmSlaveI2C1_g = {
	.node = {
		.derived = &pmSlaveI2C1_g,
		.nodeId = NODE_I2C_1,
		.typeId = PM_TYPE_SLAVE,
		.parent = &pmPowerDomainLpd_g,
		.clocks = NULL,
		.currState = PM_LPD_SLAVE_STATE_RUN,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = pmLpdSlavePowers,
		.powerInfoCnt = ARRAY_SIZE(pmLpdSlavePowers),
	},
	.reqs = NULL,
	.wake = &pmI2C1Wake,
	.slvFsm = &pmLpdSlaveFsm,
	.flags = 0U,
};

static PmGicProxyWake pmSD0Wake = {
	.mask = LPD_SLCR_GICP1_IRQ_MASK_SRC18_MASK |
		LPD_SLCR_GICP1_IRQ_MASK_SRC16_MASK,
	.group = 1U,
};

PmSlave pmSlaveSD0_g = {
	.node = {
		.derived = &pmSlaveSD0_g,
		.nodeId = NODE_SD_0,
		.typeId = PM_TYPE_SLAVE,
		.parent = &pmPowerDomainLpd_g,
		.clocks = NULL,
		.currState = PM_LPD_SLAVE_STATE_RUN,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = pmLpdSlavePowers,
		.powerInfoCnt = ARRAY_SIZE(pmLpdSlavePowers),
	},
	.reqs = NULL,
	.wake = &pmSD0Wake,
	.slvFsm = &pmLpdSlaveFsm,
	.flags = 0U,
};

static PmGicProxyWake pmSD1Wake = {
	.mask = LPD_SLCR_GICP1_IRQ_MASK_SRC19_MASK |
		LPD_SLCR_GICP1_IRQ_MASK_SRC17_MASK,
	.group = 1U,
};

PmSlave pmSlaveSD1_g = {
	.node = {
		.derived = &pmSlaveSD1_g,
		.nodeId = NODE_SD_1,
		.typeId = PM_TYPE_SLAVE,
		.parent = &pmPowerDomainLpd_g,
		.clocks = NULL,
		.currState = PM_LPD_SLAVE_STATE_RUN,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = pmLpdSlavePowers,
		.powerInfoCnt = ARRAY_SIZE(pmLpdSlavePowers),
	},
	.reqs = NULL,
	.wake = &pmSD1Wake,
	.slvFsm = &pmLpdSlaveFsm,
	.flags = 0U,
};

static PmGicProxyWake pmCan0Wake = {
	.mask = LPD_SLCR_GICP0_IRQ_MASK_SRC23_MASK,
	.group = 0U,
};

PmSlave pmSlaveCan0_g = {
	.node = {
		.derived = &pmSlaveCan0_g,
		.nodeId = NODE_CAN_0,
		.typeId = PM_TYPE_SLAVE,
		.parent = &pmPowerDomainLpd_g,
		.clocks = NULL,
		.currState = PM_LPD_SLAVE_STATE_RUN,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = pmLpdSlavePowers,
		.powerInfoCnt = ARRAY_SIZE(pmLpdSlavePowers),
	},
	.reqs = NULL,
	.wake = &pmCan0Wake,
	.slvFsm = &pmLpdSlaveFsm,
	.flags = 0U,
};

static PmGicProxyWake pmCan1Wake = {
	.mask = LPD_SLCR_GICP0_IRQ_MASK_SRC24_MASK,
	.group = 0U,
};

PmSlave pmSlaveCan1_g = {
	.node = {
		.derived = &pmSlaveCan1_g,
		.nodeId = NODE_CAN_1,
		.typeId = PM_TYPE_SLAVE,
		.parent = &pmPowerDomainLpd_g,
		.clocks = NULL,
		.currState = PM_LPD_SLAVE_STATE_RUN,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = pmLpdSlavePowers,
		.powerInfoCnt = ARRAY_SIZE(pmLpdSlavePowers),
	},
	.reqs = NULL,
	.wake = &pmCan1Wake,
	.slvFsm = &pmLpdSlaveFsm,
	.flags = 0U,
};

static PmGicProxyWake pmEth0Wake = {
	.mask = LPD_SLCR_GICP1_IRQ_MASK_SRC25_MASK |
		LPD_SLCR_GICP1_IRQ_MASK_SRC24_MASK,
	.group = 1U,
};

PmSlave pmSlaveEth0_g = {
	.node = {
		.derived = &pmSlaveEth0_g,
		.nodeId = NODE_ETH_0,
		.typeId = PM_TYPE_SLAVE,
		.parent = &pmPowerDomainLpd_g,
		.clocks = NULL,
		.currState = PM_LPD_SLAVE_STATE_RUN,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = pmLpdSlavePowers,
		.powerInfoCnt = ARRAY_SIZE(pmLpdSlavePowers),
	},
	.reqs = NULL,
	.wake = &pmEth0Wake,
	.slvFsm = &pmLpdSlaveFsm,
	.flags = 0U,
};

static PmGicProxyWake pmEth1Wake = {
	.mask = LPD_SLCR_GICP1_IRQ_MASK_SRC27_MASK |
		LPD_SLCR_GICP1_IRQ_MASK_SRC26_MASK,
	.group = 1U,
};

PmSlave pmSlaveEth1_g = {
	.node = {
		.derived = &pmSlaveEth1_g,
		.nodeId = NODE_ETH_1,
		.typeId = PM_TYPE_SLAVE,
		.parent = &pmPowerDomainLpd_g,
		.clocks = NULL,
		.currState = PM_LPD_SLAVE_STATE_RUN,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = pmLpdSlavePowers,
		.powerInfoCnt = ARRAY_SIZE(pmLpdSlavePowers),
	},
	.reqs = NULL,
	.wake = &pmEth1Wake,
	.slvFsm = &pmLpdSlaveFsm,
	.flags = 0U,
};

static PmGicProxyWake pmEth2Wake = {
	.mask = LPD_SLCR_GICP1_IRQ_MASK_SRC29_MASK |
		LPD_SLCR_GICP1_IRQ_MASK_SRC28_MASK,
	.group = 1U,
};

PmSlave pmSlaveEth2_g = {
	.node = {
		.derived = &pmSlaveEth2_g,
		.nodeId = NODE_ETH_2,
		.typeId = PM_TYPE_SLAVE,
		.parent = &pmPowerDomainLpd_g,
		.clocks = NULL,
		.currState = PM_LPD_SLAVE_STATE_RUN,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = pmLpdSlavePowers,
		.powerInfoCnt = ARRAY_SIZE(pmLpdSlavePowers),
	},
	.reqs = NULL,
	.wake = &pmEth2Wake,
	.slvFsm = &pmLpdSlaveFsm,
	.flags = 0U,
};

static PmGicProxyWake pmEth3Wake = {
	.mask = LPD_SLCR_GICP1_IRQ_MASK_SRC31_MASK |
		LPD_SLCR_GICP1_IRQ_MASK_SRC30_MASK,
	.group = 1U,
};

PmSlave pmSlaveEth3_g = {
	.node = {
		.derived = &pmSlaveEth3_g,
		.nodeId = NODE_ETH_3,
		.typeId = PM_TYPE_SLAVE,
		.parent = &pmPowerDomainLpd_g,
		.clocks = NULL,
		.currState = PM_LPD_SLAVE_STATE_RUN,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = pmLpdSlavePowers,
		.powerInfoCnt = ARRAY_SIZE(pmLpdSlavePowers),
	},
	.reqs = NULL,
	.wake = &pmEth3Wake,
	.slvFsm = &pmLpdSlaveFsm,
	.flags = 0U,
};

static PmGicProxyWake pmAdmaWake = {
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
		.typeId = PM_TYPE_SLAVE,
		.parent = &pmPowerDomainLpd_g,
		.clocks = NULL,
		.currState = PM_LPD_SLAVE_STATE_RUN,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = pmLpdSlavePowers,
		.powerInfoCnt = ARRAY_SIZE(pmLpdSlavePowers),
	},
	.reqs = NULL,
	.wake = &pmAdmaWake,
	.slvFsm = &pmLpdSlaveFsm,
	.flags = 0U,
};

static PmGicProxyWake pmNandWake = {
	.mask = LPD_SLCR_GICP0_IRQ_MASK_SRC14_MASK,
	.group = 0U,
};

PmSlave pmSlaveNand_g = {
	.node = {
		.derived = &pmSlaveNand_g,
		.nodeId = NODE_NAND,
		.typeId = PM_TYPE_SLAVE,
		.parent = &pmPowerDomainLpd_g,
		.clocks = NULL,
		.currState = PM_LPD_SLAVE_STATE_RUN,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = pmLpdSlavePowers,
		.powerInfoCnt = ARRAY_SIZE(pmLpdSlavePowers),
	},
	.reqs = NULL,
	.wake = &pmNandWake,
	.slvFsm = &pmLpdSlaveFsm,
	.flags = 0U,
};

static PmGicProxyWake pmQSpiWake = {
	.mask = LPD_SLCR_GICP0_IRQ_MASK_SRC15_MASK,
	.group = 0U,
};

PmSlave pmSlaveQSpi_g = {
	.node = {
		.derived = &pmSlaveQSpi_g,
		.nodeId = NODE_QSPI,
		.typeId = PM_TYPE_SLAVE,
		.parent = &pmPowerDomainLpd_g,
		.clocks = NULL,
		.currState = PM_LPD_SLAVE_STATE_RUN,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = pmLpdSlavePowers,
		.powerInfoCnt = ARRAY_SIZE(pmLpdSlavePowers),
	},
	.reqs = NULL,
	.wake = &pmQSpiWake,
	.slvFsm = &pmLpdSlaveFsm,
	.flags = 0U,
};

static PmGicProxyWake pmGpioWake = {
	.mask = LPD_SLCR_GICP0_IRQ_MASK_SRC16_MASK,
	.group = 0U,
};

PmSlave pmSlaveGpio_g = {
	.node = {
		.derived = &pmSlaveGpio_g,
		.nodeId = NODE_GPIO,
		.typeId = PM_TYPE_SLAVE,
		.parent = &pmPowerDomainLpd_g,
		.clocks = NULL,
		.currState = PM_AON_SLAVE_STATE,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = PmSlaveAonPowers,
		.powerInfoCnt = ARRAY_SIZE(PmSlaveAonPowers),
	},
	.reqs = NULL,
	.wake = &pmGpioWake,
	.slvFsm = &slaveAonFsm,
	.flags = 0U,
};

/* Definitions for typical FPD slave with controllable clock */
#define PM_FPD_SLAVE_STATE_OFF	0U
#define PM_FPD_SLAVE_STATE_RUN	1U

static const u32 pmFpdSlaveStates[] = {
	[PM_FPD_SLAVE_STATE_OFF] = 0U,
	[PM_FPD_SLAVE_STATE_RUN] = PM_CAP_CONTEXT | PM_CAP_POWER |
				   PM_CAP_ACCESS | PM_CAP_WAKEUP | PM_CAP_CLOCK,
};

static const PmStateTran pmFpdSlaveTransitions[] = {
	{
		.fromState = PM_FPD_SLAVE_STATE_RUN,
		.toState = PM_FPD_SLAVE_STATE_OFF,
		.latency = PM_DEFAULT_LATENCY,
	}, {
		.fromState = PM_FPD_SLAVE_STATE_OFF,
		.toState = PM_FPD_SLAVE_STATE_RUN,
		.latency = PM_DEFAULT_LATENCY,
	},
};

static u32 pmFpdSlavePowers[] = {
	DEFAULT_POWER_OFF,
	DEFAULT_POWER_ON,
};

static const PmSlaveFsm pmFpdSlaveFsm = {
	.states = pmFpdSlaveStates,
	.statesCnt = ARRAY_SIZE(pmFpdSlaveStates),
	.trans = pmFpdSlaveTransitions,
	.transCnt = ARRAY_SIZE(pmFpdSlaveTransitions),
	.enterState = NULL,
};

static PmGicProxyWake pmSataWake = {
	.mask = LPD_SLCR_GICP4_IRQ_MASK_SRC5_MASK,
	.group = 4U,
};

PmSlaveSata pmSlaveSata_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveSata_g,
			.nodeId = NODE_SATA,
			.typeId = PM_TYPE_SLAVE,
			.parent = &pmPowerDomainFpd_g,
			.currState = PM_FPD_SLAVE_STATE_RUN,
			.latencyMarg = MAX_LATENCY,
			.ops = NULL,
			.powerInfo = pmFpdSlavePowers,
			.powerInfoCnt = ARRAY_SIZE(pmFpdSlavePowers),
		},
		.reqs = NULL,
		.wake = &pmSataWake,
		.slvFsm = &pmFpdSlaveFsm,
		.flags = 0U,
	},
};

/*
 * States of a GPP slave (GPPs have its own power island (PI) and dependencies
 * to the power parent (FPD)
 */
static const u32 PmGppStates[] = {
	[PM_GPP_SLAVE_STATE_OFF] = 0U,
	[PM_GPP_SLAVE_STATE_ON] = PM_CAP_ACCESS | PM_CAP_CONTEXT | PM_CAP_POWER,
};

/* GPP slave transitions (from which to which state slave can transits) */
static const PmStateTran PmGppTransitions[] = {
	{
		.fromState = PM_GPP_SLAVE_STATE_ON,
		.toState = PM_GPP_SLAVE_STATE_OFF,
		.latency = PM_DEFAULT_LATENCY,
	}, {
		.fromState = PM_GPP_SLAVE_STATE_OFF,
		.toState = PM_GPP_SLAVE_STATE_ON,
		.latency = PM_DEFAULT_LATENCY,
	},
};

/**
 * PmGppFsmHandler() - FSM handler of a GPP slave
 * @slave       Slave whose state should be changed
 * @nextState   State the slave should enter
 *
 * @return      Status of performing transition action
 */
static int PmGppFsmHandler(PmSlave* const slave, const PmStateId nextState)
{
	int status = XST_PM_INTERNAL;
	PmSlaveGpp* gpp = (PmSlaveGpp*)slave->node.derived;

	switch (slave->node.currState) {
	case PM_GPP_SLAVE_STATE_ON:
		if (PM_GPP_SLAVE_STATE_OFF == nextState) {
			/* ON -> OFF*/
			status = gpp->PwrDn();
		} else {
			status = XST_NO_FEATURE;
		}
		break;
	case PM_GPP_SLAVE_STATE_OFF:
		if (PM_GPP_SLAVE_STATE_ON == nextState) {
			/* OFF -> ON */
			status = gpp->PwrUp();
		} else {
			status = XST_NO_FEATURE;
		}
		break;
	default:
		PmDbg("ERROR: Unknown state #%d\r\n", slave->node.currState);
		break;
	}

	return status;
}

static const PmSlaveFsm slaveGPPFsm = {
	.states = PmGppStates,
	.statesCnt = ARRAY_SIZE(PmGppStates),
	.trans = PmGppTransitions,
	.transCnt = ARRAY_SIZE(PmGppTransitions),
	.enterState = PmGppFsmHandler,
};

PmSlaveGpp pmSlaveGpuPP0_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveGpuPP0_g,
			.nodeId = NODE_GPU_PP_0,
			.typeId = PM_TYPE_SLAVE,
			.parent = &pmPowerDomainFpd_g,
			.currState = PM_FPD_SLAVE_STATE_RUN,
			.latencyMarg = MAX_LATENCY,
			.ops = NULL,
			.powerInfo = pmFpdSlavePowers,
			.powerInfoCnt = ARRAY_SIZE(pmFpdSlavePowers),
		},
		.reqs = NULL,
		.wake = NULL,
		.slvFsm = &slaveGPPFsm,
		.flags = 0U,
	},
	.PwrDn = XpbrPwrDnPp0Handler,
	.PwrUp = XpbrPwrUpPp0Handler,
};

PmSlaveGpp pmSlaveGpuPP1_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveGpuPP1_g,
			.nodeId = NODE_GPU_PP_1,
			.typeId = PM_TYPE_SLAVE,
			.parent = &pmPowerDomainFpd_g,
			.currState = PM_FPD_SLAVE_STATE_RUN,
			.latencyMarg = MAX_LATENCY,
			.ops = NULL,
			.powerInfo = pmFpdSlavePowers,
			.powerInfoCnt = ARRAY_SIZE(pmFpdSlavePowers),
		},
		.reqs = NULL,
		.wake = NULL,
		.slvFsm = &slaveGPPFsm,
		.flags = 0U,
	},
	.PwrDn = XpbrPwrDnPp1Handler,
	.PwrUp = XpbrPwrUpPp1Handler,
};

PmSlave pmSlaveGpu_g = {
	.node = {
		.derived = &pmSlaveGpu_g,
		.nodeId = NODE_GPU,
		.typeId = PM_TYPE_SLAVE,
		.parent = &pmPowerDomainFpd_g,
		.currState = PM_FPD_SLAVE_STATE_RUN,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = pmFpdSlavePowers,
		.powerInfoCnt = ARRAY_SIZE(pmFpdSlavePowers),
	},
	.reqs = NULL,
	.wake = NULL,
	.slvFsm = &pmFpdSlaveFsm,
	.flags = 0U,
};

PmSlave pmSlavePcie_g = {
	.node = {
		.derived = &pmSlavePcie_g,
		.nodeId = NODE_PCIE,
		.typeId = PM_TYPE_SLAVE,
		.parent = &pmPowerDomainFpd_g,
		.currState = PM_FPD_SLAVE_STATE_RUN,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = pmFpdSlavePowers,
		.powerInfoCnt = ARRAY_SIZE(pmFpdSlavePowers),
	},
	.reqs = NULL,
	.wake = NULL,
	.slvFsm = &pmFpdSlaveFsm,
	.flags = 0U,
};

PmSlave pmSlavePcap_g = {
	.node = {
		.derived = &pmSlavePcap_g,
		.nodeId = NODE_PCAP,
		.typeId = PM_TYPE_SLAVE,
		.parent = &pmPowerDomainLpd_g,
		.clocks = NULL,
		.currState = PM_LPD_SLAVE_STATE_IDLE,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = pmLpdSlavePowers,
		.powerInfoCnt = ARRAY_SIZE(pmLpdSlavePowers),
	},
	.reqs = NULL,
	.wake = NULL,
	.slvFsm = &pmLpdSlaveFsm,
	.flags = 0U,
};

PmSlave pmSlaveGdma_g = {
	.node = {
		.derived = &pmSlaveGdma_g,
		.nodeId = NODE_GDMA,
		.typeId = PM_TYPE_SLAVE,
		.parent = &pmPowerDomainFpd_g,
		.currState = PM_FPD_SLAVE_STATE_RUN,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = pmFpdSlavePowers,
		.powerInfoCnt = ARRAY_SIZE(pmFpdSlavePowers)
	},
	.reqs = NULL,
	.wake = NULL,
	.slvFsm = &pmFpdSlaveFsm,
	.flags = 0U,
};

PmSlave pmSlaveDP_g = {
	.node = {
		.derived = &pmSlaveDP_g,
		.nodeId = NODE_DP,
		.typeId = PM_TYPE_SLAVE,
		.parent = &pmPowerDomainFpd_g,
		.currState = PM_FPD_SLAVE_STATE_RUN,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = pmFpdSlavePowers,
		.powerInfoCnt = ARRAY_SIZE(pmFpdSlavePowers)
	},
	.reqs = NULL,
	.wake = NULL,
	.slvFsm = &pmFpdSlaveFsm,
	.flags = 0U,
};

PmSlave pmSlaveAFI_g = {
	.node = {
		.derived = &pmSlaveAFI_g,
		.nodeId = NODE_AFI,
		.typeId = PM_TYPE_SLAVE,
		.parent = &pmPowerDomainLpd_g,
		.currState = PM_FPD_SLAVE_STATE_RUN,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = pmFpdSlavePowers,
		.powerInfoCnt = ARRAY_SIZE(pmFpdSlavePowers)
	},
	.reqs = NULL,
	.wake = NULL,
	.slvFsm = &pmFpdSlaveFsm,
	.flags = 0U,
};

static PmGicProxyWake pmIpiApuWake = {
	.mask = LPD_SLCR_GICP1_IRQ_MASK_SRC3_MASK,
	.group = 1U,
};

PmSlave pmSlaveIpiApu_g = {
	.node = {
		.derived = &pmSlaveIpiApu_g,
		.nodeId = NODE_IPI_APU,
		.typeId = PM_TYPE_SLAVE,
		.parent = &pmPowerDomainLpd_g,
		.clocks = NULL,
		.currState = PM_AON_SLAVE_STATE,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = PmSlaveAonPowers,
		.powerInfoCnt = ARRAY_SIZE(PmSlaveAonPowers)
	},
	.reqs = NULL,
	.wake = &pmIpiApuWake,
	.slvFsm = &slaveAonFsm,
	.flags = 0U,
};

PmSlave pmSlaveIpiRpu0_g = {
	.node = {
		.derived = &pmSlaveIpiRpu0_g,
		.nodeId = NODE_IPI_RPU_0,
		.typeId = PM_TYPE_SLAVE,
		.parent = &pmPowerDomainLpd_g,
		.clocks = NULL,
		.currState = PM_AON_SLAVE_STATE,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = PmSlaveAonPowers,
		.powerInfoCnt = ARRAY_SIZE(PmSlaveAonPowers)
	},
	.reqs = NULL,
	.wake = NULL,
	.slvFsm = &slaveAonFsm,
	.flags = 0U,
};
