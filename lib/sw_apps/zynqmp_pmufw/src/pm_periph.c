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

/*
 * Standard slave states (used for generic slaves with trivial on/off)
 * These slaves have no machanisms for controlling their own state, and their
 * off state is controlled by the power parent state.
 */
#define PM_STD_SLAVE_STATE_OFF	0U
#define PM_STD_SLAVE_STATE_ON	1U

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

static u32 PmSlaveStdPowers[] = {
	DEFAULT_POWER_OFF,
	DEFAULT_POWER_ON,
};

static PmWakeProperties pmTtc0Wake = {
	.proxyIrqMask = FPD_GICP_TTC0_WAKE_IRQ_MASK,
	.proxyGroup = &gicProxyGroups_g[FPD_GICP_GROUP1],
};

static PmRequirement* const pmTtc0Reqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_TTC0],
	&pmRpu0Req_g[PM_MASTER_RPU_0_SLAVE_TTC0],
};

PmSlaveTtc pmSlaveTtc0_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveTtc0_g,
			.nodeId = NODE_TTC_0,
			.typeId = PM_TYPE_TTC,
			.parent = NULL,
			.currState = PM_AON_SLAVE_STATE,
			.latencyMarg = MAX_LATENCY,
			.ops = NULL,
			.powerInfo = PmSlaveAonPowers,
			.powerInfoCnt = ARRAY_SIZE(PmSlaveAonPowers),
		},
		.reqs = pmTtc0Reqs,
		.reqsCnt = ARRAY_SIZE(pmTtc0Reqs),
		.wake = &pmTtc0Wake,
		.slvFsm = &slaveAonFsm,
	},
};

static PmWakeProperties pmTtc1Wake = {
	.proxyIrqMask = FPD_GICP_TTC1_WAKE_IRQ_MASK,
	.proxyGroup = &gicProxyGroups_g[FPD_GICP_GROUP1],
};

static PmRequirement* const pmTtc1Reqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_TTC1],
};

PmSlaveTtc pmSlaveTtc1_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveTtc1_g,
			.nodeId = NODE_TTC_1,
			.typeId = PM_TYPE_TTC,
			.parent = NULL,
			.currState = PM_AON_SLAVE_STATE,
			.latencyMarg = MAX_LATENCY,
			.ops = NULL,
			.powerInfo = PmSlaveAonPowers,
			.powerInfoCnt = ARRAY_SIZE(PmSlaveAonPowers),
		},
		.reqs = pmTtc1Reqs,
		.reqsCnt = ARRAY_SIZE(pmTtc1Reqs),
		.wake = &pmTtc1Wake,
		.slvFsm = &slaveAonFsm,
	},
};

static PmWakeProperties pmTtc2Wake = {
	.proxyIrqMask = FPD_GICP_TTC2_WAKE_IRQ_MASK,
	.proxyGroup = &gicProxyGroups_g[FPD_GICP_GROUP1],
};

static PmRequirement* const pmTtc2Reqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_TTC2],
};

PmSlaveTtc pmSlaveTtc2_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveTtc2_g,
			.nodeId = NODE_TTC_2,
			.typeId = PM_TYPE_TTC,
			.parent = NULL,
			.currState = PM_AON_SLAVE_STATE,
			.latencyMarg = MAX_LATENCY,
			.ops = NULL,
			.powerInfo = PmSlaveAonPowers,
			.powerInfoCnt = ARRAY_SIZE(PmSlaveAonPowers),
		},
		.reqs = pmTtc2Reqs,
		.reqsCnt = ARRAY_SIZE(pmTtc2Reqs),
		.wake = &pmTtc2Wake,
		.slvFsm = &slaveAonFsm,
	},
};

static PmWakeProperties pmTtc3Wake = {
	.proxyIrqMask = FPD_GICP_TTC3_WAKE_IRQ_MASK,
	.proxyGroup = &gicProxyGroups_g[FPD_GICP_GROUP1],
};

static PmRequirement* const pmTtc3Reqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_TTC3],
};

PmSlaveTtc pmSlaveTtc3_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveTtc3_g,
			.nodeId = NODE_TTC_3,
			.typeId = PM_TYPE_TTC,
			.parent = NULL,
			.currState = PM_AON_SLAVE_STATE,
			.latencyMarg = MAX_LATENCY,
			.ops = NULL,
			.powerInfo = PmSlaveAonPowers,
			.powerInfoCnt = ARRAY_SIZE(PmSlaveAonPowers),
		},
		.reqs = pmTtc3Reqs,
		.reqsCnt = ARRAY_SIZE(pmTtc3Reqs),
		.wake = &pmTtc3Wake,
		.slvFsm = &slaveAonFsm,
	},
};

static PmWakeProperties pmUart0Wake = {
	.proxyIrqMask = FPD_GICP_UART0_WAKE_IRQ_MASK,
	.proxyGroup = &gicProxyGroups_g[FPD_GICP_GROUP0],
};

static PmRequirement* const pmUart0Reqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_UART0],
};

PmSlave pmSlaveUart0_g = {
	.node = {
		.derived = &pmSlaveUart0_g,
		.nodeId = NODE_UART_0,
		.typeId = PM_TYPE_SLAVE,
		.parent = NULL,
		.currState = PM_AON_SLAVE_STATE,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = PmSlaveAonPowers,
		.powerInfoCnt = ARRAY_SIZE(PmSlaveAonPowers),
	},
	.reqs = pmUart0Reqs,
	.reqsCnt = ARRAY_SIZE(pmUart0Reqs),
	.wake = &pmUart0Wake,
	.slvFsm = &slaveAonFsm,
};

static PmWakeProperties pmUart1Wake = {
	.proxyIrqMask = FPD_GICP_UART1_WAKE_IRQ_MASK,
	.proxyGroup = &gicProxyGroups_g[FPD_GICP_GROUP0],
};

static PmRequirement* const pmUart1Reqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_UART1],
};

PmSlave pmSlaveUart1_g = {
	.node = {
		.derived = &pmSlaveUart1_g,
		.nodeId = NODE_UART_1,
		.typeId = PM_TYPE_SLAVE,
		.parent = NULL,
		.currState = PM_AON_SLAVE_STATE,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = PmSlaveAonPowers,
		.powerInfoCnt = ARRAY_SIZE(PmSlaveAonPowers),
	},
	.reqs = pmUart1Reqs,
	.reqsCnt = ARRAY_SIZE(pmUart1Reqs),
	.wake = &pmUart1Wake,
	.slvFsm = &slaveAonFsm,
};

static PmWakeProperties pmSpi0Wake = {
	.proxyIrqMask = FPD_GICP_SPI0_WAKE_IRQ_MASK,
	.proxyGroup = &gicProxyGroups_g[FPD_GICP_GROUP0],
};

static PmRequirement* const pmSpi0Reqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_SPI0],
};

PmSlave pmSlaveSpi0_g = {
	.node = {
		.derived = &pmSlaveSpi0_g,
		.nodeId = NODE_SPI_0,
		.typeId = PM_TYPE_SLAVE,
		.parent = NULL,
		.currState = PM_AON_SLAVE_STATE,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = PmSlaveAonPowers,
		.powerInfoCnt = ARRAY_SIZE(PmSlaveAonPowers),
	},
	.reqs = pmSpi0Reqs,
	.reqsCnt = ARRAY_SIZE(pmSpi0Reqs),
	.wake = &pmSpi0Wake,
	.slvFsm = &slaveAonFsm,
};

static PmWakeProperties pmSpi1Wake = {
	.proxyIrqMask = FPD_GICP_SPI1_WAKE_IRQ_MASK,
	.proxyGroup = &gicProxyGroups_g[FPD_GICP_GROUP0],
};

static PmRequirement* const pmSpi1Reqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_SPI1],
};

PmSlave pmSlaveSpi1_g = {
	.node = {
		.derived = &pmSlaveSpi1_g,
		.nodeId = NODE_SPI_1,
		.typeId = PM_TYPE_SLAVE,
		.parent = NULL,
		.currState = PM_AON_SLAVE_STATE,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = PmSlaveAonPowers,
		.powerInfoCnt = ARRAY_SIZE(PmSlaveAonPowers),
	},
	.reqs = pmSpi1Reqs,
	.reqsCnt = ARRAY_SIZE(pmSpi1Reqs),
	.wake = &pmSpi1Wake,
	.slvFsm = &slaveAonFsm,
};

static PmWakeProperties pmI2C0Wake = {
	.proxyIrqMask = FPD_GICP_I2C0_WAKE_IRQ_MASK,
	.proxyGroup = &gicProxyGroups_g[FPD_GICP_GROUP0],
};

static PmRequirement* const pmI2C0Reqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_I2C0],
};

PmSlave pmSlaveI2C0_g = {
	.node = {
		.derived = &pmSlaveI2C0_g,
		.nodeId = NODE_I2C_0,
		.typeId = PM_TYPE_SLAVE,
		.parent = NULL,
		.currState = PM_AON_SLAVE_STATE,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = PmSlaveAonPowers,
		.powerInfoCnt = ARRAY_SIZE(PmSlaveAonPowers),
	},
	.reqs = pmI2C0Reqs,
	.reqsCnt = ARRAY_SIZE(pmI2C0Reqs),
	.wake = &pmI2C0Wake,
	.slvFsm = &slaveAonFsm,
};

static PmWakeProperties pmI2C1Wake = {
	.proxyIrqMask = FPD_GICP_I2C1_WAKE_IRQ_MASK,
	.proxyGroup = &gicProxyGroups_g[FPD_GICP_GROUP0],
};

static PmRequirement* const pmI2C1Reqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_I2C1],
};

PmSlave pmSlaveI2C1_g = {
	.node = {
		.derived = &pmSlaveI2C1_g,
		.nodeId = NODE_I2C_1,
		.typeId = PM_TYPE_SLAVE,
		.parent = NULL,
		.currState = PM_AON_SLAVE_STATE,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = PmSlaveAonPowers,
		.powerInfoCnt = ARRAY_SIZE(PmSlaveAonPowers),
	},
	.reqs = pmI2C1Reqs,
	.reqsCnt = ARRAY_SIZE(pmI2C1Reqs),
	.wake = &pmI2C1Wake,
	.slvFsm = &slaveAonFsm,
};

static PmWakeProperties pmSD0Wake = {
	.proxyIrqMask = FPD_GICP_SD0_WAKE_IRQ_MASK,
	.proxyGroup = &gicProxyGroups_g[FPD_GICP_GROUP1],
};

static PmRequirement* const pmSD0Reqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_SD0],
};

PmSlave pmSlaveSD0_g = {
	.node = {
		.derived = &pmSlaveSD0_g,
		.nodeId = NODE_SD_0,
		.typeId = PM_TYPE_SLAVE,
		.parent = NULL,
		.currState = PM_AON_SLAVE_STATE,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = PmSlaveAonPowers,
		.powerInfoCnt = ARRAY_SIZE(PmSlaveAonPowers),
	},
	.reqs = pmSD0Reqs,
	.reqsCnt = ARRAY_SIZE(pmSD0Reqs),
	.wake = &pmSD0Wake,
	.slvFsm = &slaveAonFsm,
};

static PmWakeProperties pmSD1Wake = {
	.proxyIrqMask = FPD_GICP_SD1_WAKE_IRQ_MASK,
	.proxyGroup = &gicProxyGroups_g[FPD_GICP_GROUP1],
};

static PmRequirement* const pmSD1Reqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_SD1],
};

PmSlave pmSlaveSD1_g = {
	.node = {
		.derived = &pmSlaveSD1_g,
		.nodeId = NODE_SD_1,
		.typeId = PM_TYPE_SLAVE,
		.parent = NULL,
		.currState = PM_AON_SLAVE_STATE,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = PmSlaveAonPowers,
		.powerInfoCnt = ARRAY_SIZE(PmSlaveAonPowers),
	},
	.reqs = pmSD1Reqs,
	.reqsCnt = ARRAY_SIZE(pmSD1Reqs),
	.wake = &pmSD1Wake,
	.slvFsm = &slaveAonFsm,
};

static PmWakeProperties pmCan0Wake = {
	.proxyIrqMask = FPD_GICP_CAN0_WAKE_IRQ_MASK,
	.proxyGroup = &gicProxyGroups_g[FPD_GICP_GROUP0],
};

static PmRequirement* const pmCan0Reqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_CAN0],
};

PmSlave pmSlaveCan0_g = {
	.node = {
		.derived = &pmSlaveCan0_g,
		.nodeId = NODE_CAN_0,
		.typeId = PM_TYPE_SLAVE,
		.parent = NULL,
		.currState = PM_AON_SLAVE_STATE,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = PmSlaveAonPowers,
		.powerInfoCnt = ARRAY_SIZE(PmSlaveAonPowers),
	},
	.reqs = pmCan0Reqs,
	.reqsCnt = ARRAY_SIZE(pmCan0Reqs),
	.wake = &pmCan0Wake,
	.slvFsm = &slaveAonFsm,
};

static PmWakeProperties pmCan1Wake = {
	.proxyIrqMask = FPD_GICP_CAN1_WAKE_IRQ_MASK,
	.proxyGroup = &gicProxyGroups_g[FPD_GICP_GROUP0],
};

static PmRequirement* const pmCan1Reqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_CAN1],
};

PmSlave pmSlaveCan1_g = {
	.node = {
		.derived = &pmSlaveCan1_g,
		.nodeId = NODE_CAN_1,
		.typeId = PM_TYPE_SLAVE,
		.parent = NULL,
		.currState = PM_AON_SLAVE_STATE,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = PmSlaveAonPowers,
		.powerInfoCnt = ARRAY_SIZE(PmSlaveAonPowers),
	},
	.reqs = pmCan1Reqs,
	.reqsCnt = ARRAY_SIZE(pmCan1Reqs),
	.wake = &pmCan1Wake,
	.slvFsm = &slaveAonFsm,
};

static PmWakeProperties pmEth0Wake = {
	.proxyIrqMask = FPD_GICP_ETH0_WAKE_IRQ_MASK,
	.proxyGroup = &gicProxyGroups_g[FPD_GICP_GROUP1],
};

static PmRequirement* const pmEth0Reqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_ETH0],
};

PmSlave pmSlaveEth0_g = {
	.node = {
		.derived = &pmSlaveEth0_g,
		.nodeId = NODE_ETH_0,
		.typeId = PM_TYPE_SLAVE,
		.parent = NULL,
		.currState = PM_AON_SLAVE_STATE,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = PmSlaveAonPowers,
		.powerInfoCnt = ARRAY_SIZE(PmSlaveAonPowers),
	},
	.reqs = pmEth0Reqs,
	.reqsCnt = ARRAY_SIZE(pmEth0Reqs),
	.wake = &pmEth0Wake,
	.slvFsm = &slaveAonFsm,
};

static PmWakeProperties pmEth1Wake = {
	.proxyIrqMask = FPD_GICP_ETH1_WAKE_IRQ_MASK,
	.proxyGroup = &gicProxyGroups_g[FPD_GICP_GROUP1],
};

static PmRequirement* const pmEth1Reqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_ETH1],
};

PmSlave pmSlaveEth1_g = {
	.node = {
		.derived = &pmSlaveEth1_g,
		.nodeId = NODE_ETH_1,
		.typeId = PM_TYPE_SLAVE,
		.parent = NULL,
		.currState = PM_AON_SLAVE_STATE,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = PmSlaveAonPowers,
		.powerInfoCnt = ARRAY_SIZE(PmSlaveAonPowers),
	},
	.reqs = pmEth1Reqs,
	.reqsCnt = ARRAY_SIZE(pmEth1Reqs),
	.wake = &pmEth1Wake,
	.slvFsm = &slaveAonFsm,
};

static PmWakeProperties pmEth2Wake = {
	.proxyIrqMask = FPD_GICP_ETH2_WAKE_IRQ_MASK,
	.proxyGroup = &gicProxyGroups_g[FPD_GICP_GROUP1],
};

static PmRequirement* const pmEth2Reqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_ETH2],
};

PmSlave pmSlaveEth2_g = {
	.node = {
		.derived = &pmSlaveEth2_g,
		.nodeId = NODE_ETH_2,
		.typeId = PM_TYPE_SLAVE,
		.parent = NULL,
		.currState = PM_AON_SLAVE_STATE,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = PmSlaveAonPowers,
		.powerInfoCnt = ARRAY_SIZE(PmSlaveAonPowers),
	},
	.reqs = pmEth2Reqs,
	.reqsCnt = ARRAY_SIZE(pmEth2Reqs),
	.wake = &pmEth2Wake,
	.slvFsm = &slaveAonFsm,
};

static PmWakeProperties pmEth3Wake = {
	.proxyIrqMask = FPD_GICP_ETH3_WAKE_IRQ_MASK,
	.proxyGroup = &gicProxyGroups_g[FPD_GICP_GROUP1],
};

static PmRequirement* const pmEth3Reqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_ETH3],
};

PmSlave pmSlaveEth3_g = {
	.node = {
		.derived = &pmSlaveEth3_g,
		.nodeId = NODE_ETH_3,
		.typeId = PM_TYPE_SLAVE,
		.parent = NULL,
		.currState = PM_AON_SLAVE_STATE,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = PmSlaveAonPowers,
		.powerInfoCnt = ARRAY_SIZE(PmSlaveAonPowers),
	},
	.reqs = pmEth3Reqs,
	.reqsCnt = ARRAY_SIZE(pmEth3Reqs),
	.wake = &pmEth3Wake,
	.slvFsm = &slaveAonFsm,
};

static PmRequirement* const pmAdmaReqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_ADMA],
};

PmSlave pmSlaveAdma_g = {
	.node = {
		.derived = &pmSlaveAdma_g,
		.nodeId = NODE_ADMA,
		.typeId = PM_TYPE_SLAVE,
		.parent = NULL,
		.currState = PM_AON_SLAVE_STATE,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = PmSlaveAonPowers,
		.powerInfoCnt = ARRAY_SIZE(PmSlaveAonPowers),
	},
	.reqs = pmAdmaReqs,
	.reqsCnt = ARRAY_SIZE(pmAdmaReqs),
	.wake = NULL,
	.slvFsm = &slaveAonFsm,
};

static PmWakeProperties pmNandWake = {
	.proxyIrqMask = FPD_GICP_NAND_WAKE_IRQ_MASK,
	.proxyGroup = &gicProxyGroups_g[FPD_GICP_GROUP0],
};

static PmRequirement* const pmNandReqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_NAND],
};

PmSlave pmSlaveNand_g = {
	.node = {
		.derived = &pmSlaveNand_g,
		.nodeId = NODE_NAND,
		.typeId = PM_TYPE_SLAVE,
		.parent = NULL,
		.currState = PM_AON_SLAVE_STATE,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = PmSlaveAonPowers,
		.powerInfoCnt = ARRAY_SIZE(PmSlaveAonPowers),
	},
	.reqs = pmNandReqs,
	.reqsCnt = ARRAY_SIZE(pmNandReqs),
	.wake = &pmNandWake,
	.slvFsm = &slaveAonFsm,
};

static PmWakeProperties pmQSpiWake = {
	.proxyIrqMask = FPD_GICP_SPI_WAKE_IRQ_MASK,
	.proxyGroup = &gicProxyGroups_g[FPD_GICP_GROUP0],
};

static PmRequirement* const pmQSpiReqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_QSPI],
};

PmSlave pmSlaveQSpi_g = {
	.node = {
		.derived = &pmSlaveQSpi_g,
		.nodeId = NODE_QSPI,
		.typeId = PM_TYPE_SLAVE,
		.parent = NULL,
		.currState = PM_AON_SLAVE_STATE,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = PmSlaveAonPowers,
		.powerInfoCnt = ARRAY_SIZE(PmSlaveAonPowers),
	},
	.reqs = pmQSpiReqs,
	.reqsCnt = ARRAY_SIZE(pmQSpiReqs),
	.wake = &pmQSpiWake,
	.slvFsm = &slaveAonFsm,
};

static PmWakeProperties pmGpioWake = {
	.proxyIrqMask = FPD_GICP_GPIO_WAKE_IRQ_MASK,
	.proxyGroup = &gicProxyGroups_g[FPD_GICP_GROUP0],
};

static PmRequirement* const pmGpioReqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_GPIO],
};

PmSlave pmSlaveGpio_g = {
	.node = {
		.derived = &pmSlaveGpio_g,
		.nodeId = NODE_GPIO,
		.typeId = PM_TYPE_SLAVE,
		.parent = NULL,
		.currState = PM_AON_SLAVE_STATE,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = PmSlaveAonPowers,
		.powerInfoCnt = ARRAY_SIZE(PmSlaveAonPowers),
	},
	.reqs = pmGpioReqs,
	.reqsCnt = ARRAY_SIZE(pmGpioReqs),
	.wake = &pmGpioWake,
	.slvFsm = &slaveAonFsm,
};

/*
 * Standard slave with no private PM properties to be controlled.
 * It can be powered down with the power parent.
 */
static const u32 pmStdStates[] = {
	[PM_STD_SLAVE_STATE_OFF] = 0U,
	[PM_STD_SLAVE_STATE_ON] = PM_CAP_WAKEUP | PM_CAP_ACCESS |
				  PM_CAP_CONTEXT | PM_CAP_POWER,
};

/* Standard slave transitions (from which to which state Std slave transits) */
static const PmStateTran pmStdTransitions[] = {
	{
		.fromState = PM_STD_SLAVE_STATE_ON,
		.toState = PM_STD_SLAVE_STATE_OFF,
		.latency = PM_DEFAULT_LATENCY,
	}, {
		.fromState = PM_STD_SLAVE_STATE_OFF,
		.toState = PM_STD_SLAVE_STATE_ON,
		.latency = PM_DEFAULT_LATENCY,
	},
};

static const PmSlaveFsm slaveStdFsm = {
	.states = pmStdStates,
	.statesCnt = ARRAY_SIZE(pmStdStates),
	.trans = pmStdTransitions,
	.transCnt = ARRAY_SIZE(pmStdTransitions),
	.enterState = NULL,
};

static PmWakeProperties pmSataWake = {
	.proxyIrqMask = FPD_GICP_SATA_WAKE_IRQ_MASK,
	.proxyGroup = &gicProxyGroups_g[FPD_GICP_GROUP4],
};

static PmRequirement* const pmSataReqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_SATA],
	&pmRpu0Req_g[PM_MASTER_RPU_0_SLAVE_SATA],
};

PmSlaveSata pmSlaveSata_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveSata_g,
			.nodeId = NODE_SATA,
			.typeId = PM_TYPE_SATA,
			.parent = &pmPowerDomainFpd_g,
			.currState = PM_STD_SLAVE_STATE_ON,
			.latencyMarg = MAX_LATENCY,
			.ops = NULL,
			.powerInfo = PmSlaveStdPowers,
			.powerInfoCnt = ARRAY_SIZE(PmSlaveStdPowers),
		},
		.reqs = pmSataReqs,
		.reqsCnt = ARRAY_SIZE(pmSataReqs),
		.wake = &pmSataWake,
		.slvFsm = &slaveStdFsm,
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
		PmDbg("ERROR: Unknown state #%d\n", slave->node.currState);
		break;
	}
	if (XST_SUCCESS == status) {
		PmNodeUpdateCurrState(&slave->node, nextState);
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

static PmRequirement* const pmGpuPP0Reqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_GPUPP0],
};

PmSlaveGpp pmSlaveGpuPP0_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveGpuPP0_g,
			.nodeId = NODE_GPU_PP_0,
			.typeId = PM_TYPE_SLAVE,
			.parent = &pmPowerDomainFpd_g,
			.currState = PM_STD_SLAVE_STATE_ON,
			.latencyMarg = MAX_LATENCY,
			.ops = NULL,
			.powerInfo = PmSlaveStdPowers,
			.powerInfoCnt = ARRAY_SIZE(PmSlaveStdPowers),
		},
		.reqs = pmGpuPP0Reqs,
		.reqsCnt = ARRAY_SIZE(pmGpuPP0Reqs),
		.wake = NULL,
		.slvFsm = &slaveGPPFsm,
	},
	.PwrDn = XpbrPwrDnPp0Handler,
	.PwrUp = XpbrPwrUpPp0Handler,
};

static PmRequirement* const pmGpuPP1Reqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_GPUPP1],
};

PmSlaveGpp pmSlaveGpuPP1_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveGpuPP1_g,
			.nodeId = NODE_GPU_PP_1,
			.typeId = PM_TYPE_SLAVE,
			.parent = &pmPowerDomainFpd_g,
			.currState = PM_STD_SLAVE_STATE_ON,
			.latencyMarg = MAX_LATENCY,
			.ops = NULL,
			.powerInfo = PmSlaveStdPowers,
			.powerInfoCnt = ARRAY_SIZE(PmSlaveStdPowers),
		},
		.reqs = pmGpuPP1Reqs,
		.reqsCnt = ARRAY_SIZE(pmGpuPP1Reqs),
		.wake = NULL,
		.slvFsm = &slaveGPPFsm,
	},
	.PwrDn = XpbrPwrDnPp1Handler,
	.PwrUp = XpbrPwrUpPp1Handler,
};

static PmRequirement* const pmGdmaReqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_GDMA],
};

PmSlave pmSlaveGdma_g = {
	.node = {
		.derived = &pmSlaveGdma_g,
		.nodeId = NODE_GDMA,
		.typeId = PM_TYPE_SLAVE,
		.parent = &pmPowerDomainFpd_g,
		.currState = PM_STD_SLAVE_STATE_ON,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = PmSlaveStdPowers,
		.powerInfoCnt = ARRAY_SIZE(PmSlaveStdPowers)
	},
	.reqs = pmGdmaReqs,
	.reqsCnt = ARRAY_SIZE(pmGdmaReqs),
	.wake = NULL,
	.slvFsm = &slaveStdFsm,
};

static PmRequirement* const pmDPReqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_DP],
};

PmSlave pmSlaveDP_g = {
	.node = {
		.derived = &pmSlaveDP_g,
		.nodeId = NODE_DP,
		.typeId = PM_TYPE_SLAVE,
		.parent = &pmPowerDomainFpd_g,
		.currState = PM_STD_SLAVE_STATE_ON,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = PmSlaveStdPowers,
		.powerInfoCnt = ARRAY_SIZE(PmSlaveStdPowers)
	},
	.reqs = pmDPReqs,
	.reqsCnt = ARRAY_SIZE(pmDPReqs),
	.wake = NULL,
	.slvFsm = &slaveStdFsm,
};

static PmRequirement* const pmAFIReqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_AFI],
};

PmSlave pmSlaveAFI_g = {
	.node = {
		.derived = &pmSlaveAFI_g,
		.nodeId = NODE_AFI,
		.typeId = PM_TYPE_SLAVE,
		.parent = &pmPowerDomainFpd_g,
		.currState = PM_STD_SLAVE_STATE_ON,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = PmSlaveStdPowers,
		.powerInfoCnt = ARRAY_SIZE(PmSlaveStdPowers)
	},
	.reqs = pmAFIReqs,
	.reqsCnt = ARRAY_SIZE(pmAFIReqs),
	.wake = NULL,
	.slvFsm = &slaveStdFsm,
};

static PmRequirement* const pmIpiApuReqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_IPI_APU],
};

static PmWakeProperties pmIpiApuWake = {
	.proxyIrqMask = FPD_GICP_IPI_APU_WAKE_IRQ_MASK,
	.proxyGroup = &gicProxyGroups_g[FPD_GICP_GROUP1],
};

PmSlave pmSlaveIpiApu_g = {
	.node = {
		.derived = &pmSlaveIpiApu_g,
		.nodeId = NODE_IPI_APU,
		.typeId = PM_TYPE_SLAVE,
		.parent = NULL,
		.currState = PM_AON_SLAVE_STATE,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = PmSlaveAonPowers,
		.powerInfoCnt = ARRAY_SIZE(PmSlaveAonPowers)
	},
	.reqs = pmIpiApuReqs,
	.reqsCnt = ARRAY_SIZE(pmIpiApuReqs),
	.wake = &pmIpiApuWake,
	.slvFsm = &slaveAonFsm,
};

static PmRequirement* const pmIpiRpu0Reqs[] = {
	&pmRpu0Req_g[PM_MASTER_RPU_0_SLAVE_IPI_RPU_0],
};

PmSlave pmSlaveIpiRpu0_g = {
	.node = {
		.derived = &pmSlaveIpiRpu0_g,
		.nodeId = NODE_IPI_RPU_0,
		.typeId = PM_TYPE_SLAVE,
		.parent = NULL,
		.currState = PM_AON_SLAVE_STATE,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = PmSlaveAonPowers,
		.powerInfoCnt = ARRAY_SIZE(PmSlaveAonPowers)
	},
	.reqs = pmIpiRpu0Reqs,
	.reqsCnt = ARRAY_SIZE(pmIpiRpu0Reqs),
	.wake = NULL,
	.slvFsm = &slaveAonFsm,
};
