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

/*********************************************************************
 * Master related data and function definitions:
 * 1. Data structures for masters and array of their requirements for
 *    slaves capabilities
 * 2. Functions for managing requirements and accessing master data
 *********************************************************************/

#include "pm_master.h"
#include "pm_proc.h"
#include "pm_defs.h"
#include "pm_sram.h"
#include "pm_usb.h"
#include "pm_pll.h"
#include "pm_periph.h"
#include "pm_callbacks.h"
#include "pm_notifier.h"
#include "ipi_buffer.h"
#include "pm_system.h"

#define PM_REQUESTED_SUSPEND        0x1U
#define TO_ACK_CB(ack, status) (REQUEST_ACK_CB_STANDARD == (ack))

/* Requirement of APU master */
PmRequirement pmApuReq_g[PM_MASTER_APU_SLAVE_MAX] = {
	[PM_MASTER_APU_SLAVE_OCM0] = {
		.slave = &pmSlaveOcm0_g.slv,
		.requestor = &pmMasterApu_g,
		.info = PM_MASTER_USING_SLAVE_MASK,
		.defaultReq = 0U,
		.currReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.nextReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_OCM1] = {
		.slave = &pmSlaveOcm1_g.slv,
		.requestor = &pmMasterApu_g,
		.info = PM_MASTER_USING_SLAVE_MASK,
		.defaultReq = 0U,
		.currReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.nextReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_OCM2] = {
		.slave = &pmSlaveOcm2_g.slv,
		.requestor = &pmMasterApu_g,
		.info = PM_MASTER_USING_SLAVE_MASK,
		.defaultReq = 0U,
		.currReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.nextReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_OCM3] = {
		.slave = &pmSlaveOcm3_g.slv,
		.requestor = &pmMasterApu_g,
		.info = PM_MASTER_USING_SLAVE_MASK,
		.defaultReq = 0U,
		.currReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.nextReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_L2] = {
		.slave = &pmSlaveL2_g.slv,
		.requestor = &pmMasterApu_g,
		.info = PM_MASTER_USING_SLAVE_MASK,
		.defaultReq = 0U,
		.currReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.nextReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_USB0] = {
		.slave = &pmSlaveUsb0_g.slv,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_USB1] = {
		.slave = &pmSlaveUsb1_g.slv,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_TTC0] = {
		.slave = &pmSlaveTtc0_g.slv,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_TTC1] = {
		.slave = &pmSlaveTtc1_g.slv,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_TTC2] = {
		.slave = &pmSlaveTtc2_g.slv,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_TTC3] = {
		.slave = &pmSlaveTtc3_g.slv,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_SATA] = {
		.slave = &pmSlaveSata_g.slv,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_APLL] = {
		.slave = &pmSlaveApll_g.slv,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_VPLL] = {
		.slave = &pmSlaveVpll_g.slv,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_DPLL] = {
		.slave = &pmSlaveDpll_g.slv,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_RPLL] = {
		.slave = &pmSlaveRpll_g.slv,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_IOPLL] = {
		.slave = &pmSlaveIOpll_g.slv,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_GPUPP0] = {
		.slave = &pmSlaveGpuPP0_g.slv,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_GPUPP1] = {
		.slave = &pmSlaveGpuPP1_g.slv,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_UART0] = {
		.slave = &pmSlaveUart0_g,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_UART1] = {
		.slave = &pmSlaveUart1_g,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_SPI0] = {
		.slave = &pmSlaveSpi0_g,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_SPI1] = {
		.slave = &pmSlaveSpi1_g,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_I2C0] = {
		.slave = &pmSlaveI2C0_g,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_I2C1] = {
		.slave = &pmSlaveI2C1_g,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_SD0] = {
		.slave = &pmSlaveSD0_g,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_SD1] = {
		.slave = &pmSlaveSD1_g,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_CAN0] = {
		.slave = &pmSlaveCan0_g,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_CAN1] = {
		.slave = &pmSlaveCan1_g,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_ETH0] = {
		.slave = &pmSlaveEth0_g,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_ETH1] = {
		.slave = &pmSlaveEth1_g,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_ETH2] = {
		.slave = &pmSlaveEth2_g,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_ETH3] = {
		.slave = &pmSlaveEth3_g,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_ADMA] = {
		.slave = &pmSlaveAdma_g,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_GDMA] = {
		.slave = &pmSlaveGdma_g,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_DP] = {
		.slave = &pmSlaveDP_g,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_NAND] = {
		.slave = &pmSlaveNand_g,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_QSPI] = {
		.slave = &pmSlaveQSpi_g,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_GPIO] = {
		.slave = &pmSlaveGpio_g,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_APU_SLAVE_AFI] = {
		.slave = &pmSlaveAFI_g,
		.requestor = &pmMasterApu_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
};

/* Requirement of RPU_0 master */
PmRequirement pmRpu0Req_g[PM_MASTER_RPU_0_SLAVE_MAX] = {
	[PM_MASTER_RPU_0_SLAVE_OCM0] = {
		.slave = &pmSlaveOcm0_g.slv,
		.requestor = &pmMasterRpu0_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_RPU_0_SLAVE_OCM1] = {
		.slave = &pmSlaveOcm1_g.slv,
		.requestor = &pmMasterRpu0_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_RPU_0_SLAVE_OCM2] = {
		.slave = &pmSlaveOcm2_g.slv,
		.requestor = &pmMasterRpu0_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_RPU_0_SLAVE_OCM3] = {
		.slave = &pmSlaveOcm3_g.slv,
		.requestor = &pmMasterRpu0_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_RPU_0_SLAVE_TCM0A] = {
		.slave = &pmSlaveTcm0A_g.slv,
		.requestor = &pmMasterRpu0_g,
		.info = PM_MASTER_USING_SLAVE_MASK,
		.defaultReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.currReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.nextReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_RPU_0_SLAVE_TCM0B] = {
		.slave = &pmSlaveTcm0B_g.slv,
		.requestor = &pmMasterRpu0_g,
		.info = PM_MASTER_USING_SLAVE_MASK,
		.defaultReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.currReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.nextReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_RPU_0_SLAVE_TCM1A] = {
		.slave = &pmSlaveTcm1A_g.slv,
		.requestor = &pmMasterRpu0_g,
		.info = PM_MASTER_USING_SLAVE_MASK,
		.defaultReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.currReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.nextReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_RPU_0_SLAVE_TCM1B] = {
		.slave = &pmSlaveTcm1B_g.slv,
		.requestor = &pmMasterRpu0_g,
		.info = PM_MASTER_USING_SLAVE_MASK,
		.defaultReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.currReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.nextReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_RPU_0_SLAVE_USB0] = {
		.slave = &pmSlaveUsb0_g.slv,
		.requestor = &pmMasterRpu0_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_RPU_0_SLAVE_USB1] = {
		.slave = &pmSlaveUsb1_g.slv,
		.requestor = &pmMasterRpu0_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_RPU_0_SLAVE_TTC0] = {
		.slave = &pmSlaveTtc0_g.slv,
		.requestor = &pmMasterRpu0_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_RPU_0_SLAVE_SATA] = {
		.slave = &pmSlaveSata_g.slv,
		.requestor = &pmMasterRpu0_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_RPU_0_SLAVE_APLL] = {
		.slave = &pmSlaveApll_g.slv,
		.requestor = &pmMasterRpu0_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_RPU_0_SLAVE_VPLL] = {
		.slave = &pmSlaveVpll_g.slv,
		.requestor = &pmMasterRpu0_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_RPU_0_SLAVE_DPLL] = {
		.slave = &pmSlaveDpll_g.slv,
		.requestor = &pmMasterRpu0_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_RPU_0_SLAVE_RPLL] = {
		.slave = &pmSlaveRpll_g.slv,
		.requestor = &pmMasterRpu0_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
	[PM_MASTER_RPU_0_SLAVE_IOPLL] = {
		.slave = &pmSlaveIOpll_g.slv,
		.requestor = &pmMasterRpu0_g,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
};

PmMaster pmMasterApu_g = {
	.procs = pmApuProcs_g,
	.procsCnt = PM_PROC_APU_MAX,
	.nid = NODE_APU,
	.ipiMask = IPI_PMU_0_IER_APU_MASK,
	.pmuBuffer = IPI_BUFFER_PMU_BASE + IPI_BUFFER_TARGET_APU_OFFSET,
	.buffer = IPI_BUFFER_APU_BASE + IPI_BUFFER_TARGET_PMU_OFFSET,
	.reqs = pmApuReq_g,
	.reqsCnt = ARRAY_SIZE(pmApuReq_g),
	.permissions = IPI_PMU_0_IER_RPU_0_MASK | IPI_PMU_0_IER_RPU_1_MASK,
	.suspendRequest = {
		.initiator = NULL,
		.acknowledge = 0U,
	},
};

PmMaster pmMasterRpu0_g = {
	.procs = &pmRpuProcs_g[PM_PROC_RPU_0],
	.procsCnt = 1U,
	.nid = NODE_RPU,
	.ipiMask = IPI_PMU_0_IER_RPU_0_MASK,
	.pmuBuffer = IPI_BUFFER_PMU_BASE + IPI_BUFFER_TARGET_RPU_0_OFFSET,
	.buffer = IPI_BUFFER_RPU_0_BASE + IPI_BUFFER_TARGET_PMU_OFFSET,
	.reqs = pmRpu0Req_g,
	.reqsCnt = ARRAY_SIZE(pmRpu0Req_g),
	.permissions = IPI_PMU_0_IER_APU_MASK | IPI_PMU_0_IER_RPU_1_MASK,
	.suspendRequest = {
		.initiator = NULL,
		.acknowledge = 0U,
	},
};

PmMaster pmMasterRpu1_g = {
	.procs = &pmRpuProcs_g[PM_PROC_RPU_1],
	.procsCnt = 1U,
	.nid = NODE_RPU_0, /* placeholder for request suspend, not used */
	.ipiMask = IPI_PMU_0_IER_RPU_1_MASK,
	.pmuBuffer = IPI_BUFFER_PMU_BASE + IPI_BUFFER_TARGET_RPU_1_OFFSET,
	.buffer = IPI_BUFFER_RPU_1_BASE + IPI_BUFFER_TARGET_PMU_OFFSET,
	.reqs = NULL,   /* lockstep mode is assumed for now */
	.reqsCnt = 0U,
	.permissions = IPI_PMU_0_IER_APU_MASK | IPI_PMU_0_IER_RPU_0_MASK,
	.suspendRequest = {
		.initiator = NULL,
		.acknowledge = 0U,
	},
};

PmMaster *const pmAllMasters[PM_MASTER_MAX] = {
	&pmMasterApu_g,
	&pmMasterRpu0_g,
	&pmMasterRpu1_g,
};

/**
 * PmRequirementSchedule() - Schedule requirements of the master for slave.
 *                           Slave state will be updated according to the
 *                           requirement once primary processor goes to sleep.
 * @masterReq   Pointer to master requirement structure (for a slave)
 * @caps        Required capabilities of slave to be set once primary core
 *              goes to sleep.
 *
 * @return      Status of the operation
 *              - XST_SUCCESS if requirement is successfully scheduled
 *              - XST_NO_FEATURE if there is no state with requested
 *                capabilities
 */
int PmRequirementSchedule(PmRequirement* const masterReq, const u32 caps)
{
	int status;

	/* Check if slave has a state with requested capabilities */
	status = PmCheckCapabilities(masterReq->slave, caps);
	if (XST_SUCCESS != status) {
		goto done;
	}

	/* Schedule setting of the requirement for later */
	masterReq->nextReq = caps;

done:
	return status;
}

/**
 * PmRequirementUpdate() - Set slaves capabilities according to the master's
 * requirements
 * @masterReq   Pointer to structure keeping informations about the
 *              master's requirement
 * @caps        Capabilities of a slave requested by the master
 *
 * @return      Status of the operation
 */
int PmRequirementUpdate(PmRequirement* const masterReq, const u32 caps)
{
	int status;
	u32 tmpCaps;

	/* Check if slave has a state with requested capabilities */
	status = PmCheckCapabilities(masterReq->slave, caps);

	if (XST_SUCCESS != status) {
		goto done;
	}

	/* Configure requested capabilities */
	tmpCaps = masterReq->currReq;
	masterReq->currReq = caps;
	status = PmUpdateSlave(masterReq->slave);

	if (XST_SUCCESS == status) {
		/* All capabilities requested in active state are constant */
		masterReq->nextReq = masterReq->currReq;
	} else {
		/* Remember the last setting, will report an error */
		masterReq->currReq = tmpCaps;
	}

done:
	return status;
}

/**
 * PmRequirementUpdateScheduled() - Triggers the setting for scheduled
 *                                  requirements
 * @master  Master which changed the state and whose scheduled requirements are
 *          triggered
 * @swap    Flag stating should current/default requirements be saved as next
 *
 * a) swap=false
 * Set scheduled requirements of a master without swapping current/default and
 * next requirements - means the current requirements will be dropped and
 * default requirements has no effect. Upon every self suspend, master has to
 * explicitly re-request slave requirements.
 * b) swap=true
 * Set scheduled requirements of a master with swapping current/default and
 * next requirements (swapping means the current/default requirements will be
 * saved as next, and will be configured once master wakes-up). If the master
 * has default requirements, default requirements are saved as next instead of
 * current requirements. Default requirements has priority over current
 * requirements.
 */
static int PmRequirementUpdateScheduled(const PmMaster* const master,
					    const bool swap)
{
	int status;
	unsigned int i;

	PmDbg("%s\n", PmStrNode(master->procs[0].node.nodeId));

	for (i = 0; i < master->reqsCnt; i++) {
		if (master->reqs[i].currReq != master->reqs[i].nextReq) {
			u32 tmpReq = master->reqs[i].nextReq;

			if (true == swap) {
				if (0U != master->reqs[i].defaultReq) {
					/* Master has default requirements for
					 * this slave, default has priority over
					 * current requirements.
					 */
					master->reqs[i].nextReq =
						master->reqs[i].defaultReq;
				} else {
					/* Save current requirements as next */
					master->reqs[i].nextReq =
						master->reqs[i].currReq;
				}
			}

			master->reqs[i].currReq = tmpReq;

			/* Update slave setting */
			status = PmUpdateSlave(master->reqs[i].slave);
			/* if rom works correctly, status should be always ok */
			if (XST_SUCCESS != status) {
				PmDbg("ERROR setting slave node %s\n",
				      PmStrNode(master->reqs[i].slave->node.nodeId));
				break;
			}
		}
	}

	return status;
}

/**
 * PmRequirementCancelScheduled() - Called when master aborts suspend, to cancel
 * scheduled requirements (slave capabilities requests)
 * @master  Master whose scheduled requests should be cancelled
 */
void PmRequirementCancelScheduled(const PmMaster* const master)
{
	unsigned int i;

	for (i = 0; i < master->reqsCnt; i++) {
		if (master->reqs[i].currReq != master->reqs[i].nextReq) {
			/* Drop the scheduled request by making it constant */
			PmDbg("%s\n",
			      PmStrNode(master->reqs[i].slave->node.nodeId));
			master->reqs[i].nextReq = master->reqs[i].currReq;
		}
	}
}

/**
 * PmRequirementRequestDefault() - Request default requirements for master
 * @master      Master whose default requirements are requested
 *
 * When waking up from forced power down, master might have some default
 * requirements to be configured before it enters active state (example TCM for
 * RPU). This function loops all slaves, find those for which this master has
 * default requirements and updates next requirements in master/slave
 * requirement structure.
 */
static void PmRequirementRequestDefault(const PmMaster* const master)
{
	unsigned int i;

	for (i = 0; i < master->reqsCnt; i++) {
		if (0U != master->reqs[i].defaultReq) {
			/* Set flag to state that master is using slave */
			master->reqs[i].info |= PM_MASTER_USING_SLAVE_MASK;
			master->reqs[i].nextReq = master->reqs[i].defaultReq;
		}
	}
}

/**
 * PmRequirementReleaseAll() - Called when master primary processor is forced to
 *                             power down, so all requirements of the processor
 *                             are automatically released.
 * @master  Master whose primary processor was forced to power down
 *
 * @return  Status of the operation of releasing all slaves used by the master
 *          and changing their state to the lowest possible.
 */
static int PmRequirementReleaseAll(const PmMaster* const master)
{
	int status;
	unsigned int i;

	for (i = 0; i < master->reqsCnt; i++) {
		if (0U != (PM_MASTER_USING_SLAVE_MASK & master->reqs[i].info)) {
			/* Clear flag - master is not using slave anymore */
			master->reqs[i].info &= ~PM_MASTER_USING_SLAVE_MASK;
			/* Release current and next requirements */
			master->reqs[i].currReq = 0U;
			master->reqs[i].nextReq = 0U;
			/* Update slave setting */
			status = PmUpdateSlave(master->reqs[i].slave);
			/* if pmu rom works correctly, status should be always ok */
			if (XST_SUCCESS != status) {
				PmDbg("ERROR setting slave node %s\n",
				      PmStrNode(master->reqs[i].slave->node.nodeId));
				break;
			}
		}
	}

	return status;
}

/**
 * PmGetRequirementForSlave() - Get pointer to the master's request structure for
 *          a given slave
 * @master  Master whose request structure should be found
 * @nodeId  Slave nodeId
 *
 * @return  Pointer to the master's request structure dedicated to a slave with
 *          given node. If such structure is not found, it means the master is
 *          not allowed to use the slave.
 */
PmRequirement* PmGetRequirementForSlave(const PmMaster* const master,
					const PmNodeId nodeId)
{
	u32 i;
	PmRequirement *req = NULL;

	for (i = 0; i < master->reqsCnt; i++) {
		if (master->reqs[i].slave->node.nodeId == nodeId) {
			req = &master->reqs[i];
			break;
		}
	}

	return req;
}

/**
 * PmEnableAllMasterIpis() - Iterate through all masters and enable their IPI
 *                           interrupt
 */
void PmEnableAllMasterIpis(void)
{
	u8 i;

	for (i = 0U; i < ARRAY_SIZE(pmAllMasters); i++) {
		XPfw_RMW32(IPI_PMU_0_IER,
			   pmAllMasters[i]->ipiMask,
			   pmAllMasters[i]->ipiMask);
	}
}

/**
 * PmGetMasterByIpiMask() - Use to get pointer to master structure by ipi mask
 * @mask    IPI Mask of a master (requestor) in IPI registers
 *
 * @return  Pointer to a PmMaster structure or NULL if master is not found
 */
const PmMaster* PmGetMasterByIpiMask(const u32 mask)
{
	u32 i;
	const PmMaster *mst = NULL;

	for (i = 0U; i < ARRAY_SIZE(pmAllMasters); i++) {
		if (mask & pmAllMasters[i]->ipiMask) {
			mst = pmAllMasters[i];
			break;
		}
	}

	return mst;
}

/**
 * PmGetProcOfThisMaster() - Get processor pointer with given node id, if
 *          such processor exist within the master
 * @master  Master within which the search is performed
 * @nodeId  Node of the processor to be found
 *
 * @return  Pointer to processor with the given node id (which is within the
 *          master), or NULL if such processor is not found.
 */
PmProc* PmGetProcOfThisMaster(const PmMaster* const master,
			      const PmNodeId nodeId)
{
	u32 i;
	PmProc *proc = NULL;

	for (i = 0U; i < master->procsCnt; i++) {
		if (nodeId == master->procs[i].node.nodeId) {
			proc = &master->procs[i];
		}
	}

	return proc;
}

/**
 * PmGetProcOfOtherMaster() - Get pointer to the processor with given node id,
 *          by excluding given master from the search
 * @master  Master to be excluded from search
 * @nodeId  Node id of the processor to be found
 *
 * @return  Pointer to processor that is not within the master and which has
 *          given node id, or NULL if such processor is not found
 */
PmProc* PmGetProcOfOtherMaster(const PmMaster* const master,
			       const PmNodeId nodeId)
{
	u32 i;
	PmProc *proc = NULL;

	for (i = 0U; i < ARRAY_SIZE(pmAllMasters); i++) {
		u32 p;

		if (master == pmAllMasters[i]) {
			continue;
		}

		for (p = 0; p < pmAllMasters[i]->procsCnt; p++) {
			if (nodeId == pmAllMasters[i]->procs[p].node.nodeId) {
				proc = &pmAllMasters[i]->procs[p];
				goto done;
			}
		}
	}

done:
	return proc;
}

/**
 * PmGetProcByNodeId() - Get a pointer to processor structure by the node id
 * @nodeId  Node of the processor to be found
 *
 * @return  Pointer to a processor structure whose node is provided, or
 *          NULL if processor is not found
 */
PmProc* PmGetProcByNodeId(const PmNodeId nodeId)
{
	u32 i;
	PmProc *proc = NULL;

	for (i = 0U; i < ARRAY_SIZE(pmAllMasters); i++) {
		u32 p;

		for (p = 0U; p < pmAllMasters[i]->procsCnt; p++) {
			if (nodeId == pmAllMasters[i]->procs[p].node.nodeId) {
				proc = &pmAllMasters[i]->procs[p];
				goto done;
			}
		}
	}

done:
	return proc;
}

/**
 * PmGetProcByWfiStatus() - Get processor struct by wfi interrupt status
 * @mask    WFI interrupt mask read from GPI2 register
 *
 * @return  Pointer to a processor structure whose wfi mask is provided, or
 *          NULL if processor is not found
 */
PmProc* PmGetProcByWfiStatus(const u32 mask)
{
	u32 i;
	PmProc *proc = NULL;

	for (i = 0U; i < ARRAY_SIZE(pmAllMasters); i++) {
		u32 p;

		for (p = 0U; p < pmAllMasters[i]->procsCnt; p++) {
			if (mask & pmAllMasters[i]->procs[p].wfiStatusMask) {
				proc = &pmAllMasters[i]->procs[p];
				goto done;
			}
		}
	}

done:
	return proc;
}

/**
 * PmGetProcByWakeStatus() - Get proc struct by wake interrupt status
 * @mask    GIC wake mask read from GPI1 register
 *
 * @return  Pointer to a processor structure whose wake mask is provided
 */
PmProc* PmGetProcByWakeStatus(const u32 mask)
{
	u32 i;
	PmProc *proc = NULL;

	for (i = 0U; i < ARRAY_SIZE(pmAllMasters); i++) {
		u32 p;

		for (p = 0U; p < pmAllMasters[i]->procsCnt; p++) {
			if (mask & pmAllMasters[i]->procs[p].wakeStatusMask) {
				proc = &pmAllMasters[i]->procs[p];
				goto done;
			}
		}
	}

done:
	return proc;
}

/**
 * PmEnableProxyWake() - Enable scheduled wake-up sources in GIC Proxy
 * @master  Pointer to master whose scheduled wake-up sources should be enabled
 *
 * When FPD is powered down, wake-up sources should be enabled in GIC Proxy,
 * if APU's primary processor is gently put into a sleep. If APU is forced to
 * power down, this function will return without enabling GIC Proxy, because
 * after forced power down the processor can only be woken-up by an explicit
 * wake-up request through PM API. The check whether the processor is in sleep
 * state is performed in this function and not in pm_power from where this
 * function is called in order to keep pm_power independent from (not-aware of)
 * processor states.
 */
void PmEnableProxyWake(PmMaster* const master)
{
	u32 i;

	if (master->procs->node.currState != PM_PROC_STATE_SLEEP) {
		goto done;
	}

	PmDbg("%s\n", PmStrNode(master->procs->node.nodeId));

	for (i = 0; i < master->reqsCnt; i++) {
		if (master->reqs[i].info & PM_MASTER_WAKEUP_REQ_MASK) {
			PmSlaveWakeEnable(master->reqs[i].slave);
		}
	}

done:
	return;
}

/**
 * PmWakeUpCancelScheduled() - Cancel scheduled wake-up sources of the master
 * @master  Pointer to a master whose scheduled wake-up sources should be
 *          cancelled
 */
static void PmWakeUpCancelScheduled(PmMaster* const master)
{
	u32 i;

	PmDbg("%s\n", PmStrNode(master->procs->node.nodeId));

	for (i = 0; i < master->reqsCnt; i++) {
		master->reqs[i].info &= ~PM_MASTER_WAKEUP_REQ_MASK;
	}
}

/**
 * PmWakeUpDisableAll() - Disable all wake-up sources of this master
 * @master  Pointer to a master whose wake-up sources are to be disabled
 */
static void PmWakeUpDisableAll(PmMaster* const master)
{
	u32 i;

	PmDbg("for %s\n", PmStrNode(master->procs->node.nodeId));
	for (i = 0; i < master->reqsCnt; i++) {
		if (0U != (master->reqs[i].info & PM_MASTER_WAKEUP_REQ_MASK)) {
			unsigned int r;
			bool hasOtherReq = false;

			master->reqs[i].info &= ~PM_MASTER_WAKEUP_REQ_MASK;
			/*
			 * Check if there are other masters waiting for slave's
			 * wake-up.
			 */
			for (r = 0U; r < master->reqs[i].slave->reqsCnt; r++) {
				if (0U != (master->reqs[i].slave->reqs[r]->info &
					   PM_MASTER_WAKEUP_REQ_MASK)) {
					hasOtherReq = true;
					break;
				}
			}
			if (false == hasOtherReq) {
				/*
				 * No other masters waiting for wake, disable
				 * wake event.
				 */
				PmSlaveWakeDisable(master->reqs[i].slave);
			}
		}
	}
}

/**
 * PmCanRequestSuspend() - Check whether master is privileged to request another
 *                         master to suspend
 * @reqMaster   Master which requests another master to suspend
 * @respMaster  Master whose suspend is requested and which is extected to
 *              response to the request by initiating its own self suspend
 *
 * @return      Check result
 *              - True if master has privilege to request suspend
 *              - False if master has no privilege
 */
bool PmCanRequestSuspend(const PmMaster* const reqMaster,
			 const PmMaster* const respMaster)
{
	return 0U != (reqMaster->permissions & respMaster->ipiMask);
}

/**
 * PmIsRequestedToSuspend() - Check whether the master is requested from some
 *                            other master to suspend
 * @master      Master to check for
 *
 * @return      Check result
 *              - True if master is requested to suspend
 *              - False if no other master has requested this master to suspend
 */
bool PmIsRequestedToSuspend(const PmMaster* const master)
{
	return NULL != master->suspendRequest.initiator;
}

/**
 * PmMasterSuspendAck() - Acknowledge to the suspend request of another master
 * @mst		Master which is responding to the suspend request
 * @response	Status which is acknowledged as a response (whether the suspend
 *		operation is performed successfully)
 * @return	Status of the operation of sending acknowledge:
 *		- XST_SUCCESS if before calling this function the caller checked
 *		  that PmIsRequestedToSuspend returns true (the acknowledge
 *		  may need to be sent)
 *		- XST_FAILURE otherwise - this function didn't suppose to be
 *		  called
 */
int PmMasterSuspendAck(PmMaster* const mst, const int response)
{
	int status = XST_SUCCESS;

	if (NULL == mst->suspendRequest.initiator) {
		status = XST_FAILURE;
		goto done;
	}

	if (REQUEST_ACK_CB_STANDARD == mst->suspendRequest.acknowledge) {
		/* If shutdown is being processed drop the callback */
		if (true == PmSystemShutdownProcessing()) {
			goto done;
		}

		PmAcknowledgeCb(mst->suspendRequest.initiator,
				mst->procs->node.nodeId, response,
				mst->procs->node.currState);
	} else if (REQUEST_ACK_BLOCKING == mst->suspendRequest.acknowledge) {
		IPI_RESPONSE1(mst->buffer, response);
	} else {
		/* No acknowledge */
	}
	mst->suspendRequest.initiator = NULL;

done:
	return status;
}

/**
 * PmMasterNotify() - Notify master channel of a state change in its primary processor
 * @master      Pointer to master object which needs to be notified
 * @event       Processor Event to notify the master about
 *
 * @return      Status of potential changing of slave states or success.
 *
 * Primary processor has changed its state, notify master to update its requirements
 * accordingly.
 */
int PmMasterNotify(PmMaster* const master, const PmProcEvent event)
{
	int status = XST_SUCCESS;

	switch (event) {
	case PM_PROC_EVENT_SLEEP:
		status = PmRequirementUpdateScheduled(master, true);
		break;
	case PM_PROC_EVENT_ABORT_SUSPEND:
		PmRequirementCancelScheduled(master);
		PmWakeUpCancelScheduled(master);
		break;
	case PM_PROC_EVENT_WAKE:
		if (PM_PROC_STATE_SLEEP == master->procs->node.currState) {
			PmWakeUpDisableAll(master);
		} else if (PM_PROC_STATE_FORCEDOFF ==
			   master->procs->node.currState) {
			PmRequirementRequestDefault(master);
		} else {
		}
		status = PmRequirementUpdateScheduled(master, false);
		break;
	case PM_PROC_EVENT_FORCE_PWRDN:
		status = PmRequirementReleaseAll(master);
		PmWakeUpCancelScheduled(master);
		PmNotifierUnregisterAll(master);
		break;
	default:
		status = XST_PM_INTERNAL;
		PmDbg("ERROR: undefined event #%d\n", event);
		break;
	}

	return status;
}

/**
 * PmMasterGetPlaceholder() - Check whether there is a master which holds nodeId
 * @nodeId      Id of the node whose placeholder should be found
 *
 * @return      Pointer to the master if such exist, otherwise NULL
 */
PmMaster* PmMasterGetPlaceholder(const PmNodeId nodeId)
{
	PmMaster* holder = NULL;
	u32 i;

	/* Find the master with the node placeholder */
	for (i = 0U; i < ARRAY_SIZE(pmAllMasters); i++) {
		if (nodeId == pmAllMasters[i]->nid) {
			holder = pmAllMasters[i];
			break;
		}
	}

	return holder;
}

/**
 * PmSetupInitialMasterRequirements() - Setup initial state for all masters
 *
 * During the initial Linux boot process Linux does not call PmRequestNode
 * for the slaves it uses, hence the PMU-FW pre-allocates all of the slaves
 * Linux uses to the APU.
 *
 * If bare metal code is run on the APU then such code must take care of
 * releasing any unused pre-allocated peripherals during init.
 */
void PmSetupInitialMasterRequirements(void)
{
	PmRequirement* masterReq;
	const u32 apuDefaultSlaves[] = {
		NODE_ADMA,
		NODE_AFI,
		NODE_CAN_0,
		NODE_CAN_1,
		NODE_DP,
		NODE_GDMA,
		NODE_GPIO,
		NODE_I2C_0,
		NODE_I2C_1,
		NODE_NAND,
		NODE_QSPI,
		NODE_SATA,
		NODE_SD_0,
		NODE_SD_1,
		NODE_SPI_0,
		NODE_SPI_1,
		NODE_TTC_0,
		NODE_TTC_1,
		NODE_TTC_2,
		NODE_TTC_3,
		NODE_UART_0,
		NODE_UART_1,
		NODE_USB_0,
		NODE_USB_1,
	};
	int i;

	for (i = 0; i < ARRAY_SIZE(apuDefaultSlaves); i++) {
		masterReq = PmGetRequirementForSlave(&pmMasterApu_g,
						     apuDefaultSlaves[i]);
		if (NULL != masterReq) {
			masterReq->info |= PM_MASTER_USING_SLAVE_MASK;
			masterReq->currReq = PM_CAP_ACCESS;
			masterReq->nextReq = PM_CAP_ACCESS;
		}
	}
}
