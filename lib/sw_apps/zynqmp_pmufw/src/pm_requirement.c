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

#include "pm_requirement.h"
#include "pm_master.h"
#include "pm_slave.h"
#include "pm_sram.h"
#include "pm_usb.h"
#include "pm_pll.h"
#include "pm_periph.h"
#include "pm_ddr.h"

/* Static resource allocation */
PmRequirement pmReqData[] = {
	{
		.slave = &pmSlaveRtc_g,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlavePcap_g,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlavePcie_g,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveGpu_g,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveTcm1B_g.slv,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveTcm1A_g.slv,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveTcm0B_g.slv,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveTcm0A_g.slv,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveIpiApu_g,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = PM_MASTER_USING_SLAVE_MASK,
		.defaultReq = PM_CAP_ACCESS,
		.currReq = PM_CAP_ACCESS,
		.nextReq = PM_CAP_ACCESS,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveDdr_g,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = PM_MASTER_USING_SLAVE_MASK,
		.defaultReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.currReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.nextReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveAFI_g,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveGpio_g,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveQSpi_g,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveNand_g,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveDP_g,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveGdma_g,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveAdma_g,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveEth3_g,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveEth2_g,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveEth1_g,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveEth0_g,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveCan1_g,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveCan0_g,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveSD1_g,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveSD0_g,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveI2C1_g,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveI2C0_g,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveSpi1_g,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveSpi0_g,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveUart1_g,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveUart0_g,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveGpuPP1_g.slv,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveGpuPP0_g.slv,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveIOpll_g.slv,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveRpll_g.slv,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveDpll_g.slv,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveVpll_g.slv,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveApll_g.slv,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveSata_g,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveTtc3_g,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveTtc2_g,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveTtc1_g,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveTtc0_g,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveUsb1_g.slv,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveUsb0_g.slv,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveL2_g.slv,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = PM_MASTER_USING_SLAVE_MASK,
		.defaultReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.currReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.nextReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveOcm3_g.slv,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = PM_MASTER_USING_SLAVE_MASK,
		.defaultReq = 0U,
		.currReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.nextReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveOcm2_g.slv,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = PM_MASTER_USING_SLAVE_MASK,
		.defaultReq = 0U,
		.currReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.nextReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveOcm1_g.slv,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = PM_MASTER_USING_SLAVE_MASK,
		.defaultReq = 0U,
		.currReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.nextReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveOcm0_g.slv,
		.master = &pmMasterApu_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = PM_MASTER_USING_SLAVE_MASK,
		.defaultReq = 0U,
		.currReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.nextReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveIpiRpu0_g,
		.master = &pmMasterRpu0_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = PM_MASTER_USING_SLAVE_MASK,
		.defaultReq = PM_CAP_ACCESS,
		.currReq = PM_CAP_ACCESS,
		.nextReq = PM_CAP_ACCESS,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveDdr_g,
		.master = &pmMasterRpu0_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveIOpll_g.slv,
		.master = &pmMasterRpu0_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveRpll_g.slv,
		.master = &pmMasterRpu0_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveDpll_g.slv,
		.master = &pmMasterRpu0_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveVpll_g.slv,
		.master = &pmMasterRpu0_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveApll_g.slv,
		.master = &pmMasterRpu0_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveSata_g,
		.master = &pmMasterRpu0_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveTtc0_g,
		.master = &pmMasterRpu0_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveUsb1_g.slv,
		.master = &pmMasterRpu0_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveUsb0_g.slv,
		.master = &pmMasterRpu0_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveTcm1B_g.slv,
		.master = &pmMasterRpu0_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = PM_MASTER_USING_SLAVE_MASK,
		.defaultReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.currReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.nextReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveTcm1A_g.slv,
		.master = &pmMasterRpu0_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = PM_MASTER_USING_SLAVE_MASK,
		.defaultReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.currReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.nextReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveTcm0B_g.slv,
		.master = &pmMasterRpu0_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = PM_MASTER_USING_SLAVE_MASK,
		.defaultReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.currReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.nextReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveTcm0A_g.slv,
		.master = &pmMasterRpu0_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = PM_MASTER_USING_SLAVE_MASK,
		.defaultReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.currReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.nextReq = PM_CAP_ACCESS | PM_CAP_CONTEXT,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveOcm3_g.slv,
		.master = &pmMasterRpu0_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveOcm2_g.slv,
		.master = &pmMasterRpu0_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveOcm1_g.slv,
		.master = &pmMasterRpu0_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	}, {
		.slave = &pmSlaveOcm0_g.slv,
		.master = &pmMasterRpu0_g,
		.nextSlave = NULL,
		.nextMaster = NULL,
		.info = 0U,
		.defaultReq = 0U,
		.currReq = 0U,
		.nextReq = 0U,
		.latencyReq = MAX_LATENCY,
	},
};

/**
 * PmRequirementLink() - Link requirement struct into master's and slave's lists
 * @req	Pointer to the requirement structure to be linked in lists
 */
static void PmRequirementLink(PmRequirement* const req)
{
	/* The req structure is becoming master's head of requirements list */
	req->nextSlave = req->master->reqs;
	req->master->reqs = req;

	/* The req is becoming the head of slave's requirements list as well */
	req->nextMaster = req->slave->reqs;
	req->slave->reqs = req;
}

/**
 * PmRequirementInit() - Initialize requirements data structures
 */
void PmRequirementInit(void)
{
	u32 i;

	for (i = 0U; i < ARRAY_SIZE(pmReqData); i++) {
		PmRequirementLink(&pmReqData[i]);
	}
}


/**
 * PmRequirementSchedule() - Schedule requirements of the master for slave
 * @masterReq   Pointer to master requirement structure (for a slave)
 * @caps        Required capabilities of slave
 *
 * @return      Status of the operation
 *              - XST_SUCCESS if requirement is successfully scheduled
 *              - XST_NO_FEATURE if there is no state with requested
 *                capabilities
 *
 * @note        Slave state will be updated according to the saved requirements
 *              after all processors/master suspends.
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
int PmRequirementUpdateScheduled(const PmMaster* const master, const bool swap)
{
	int status = XST_SUCCESS;
	PmRequirement* req = master->reqs;

	PmDbg("%s\r\n", PmStrNode(master->nid));

	while (NULL != req) {
		if (req->currReq != req->nextReq) {
			u32 tmpReq = req->nextReq;

			if (true == swap) {
				if (0U != req->defaultReq) {
					/* Master has default requirements for
					 * this slave, default has priority over
					 * current requirements.
					 */
					req->nextReq = req->defaultReq;
				} else {
					/* Save current requirements as next */
					req->nextReq = req->currReq;
				}
			}

			req->currReq = tmpReq;

			/* Update slave setting */
			status = PmUpdateSlave(req->slave);
			/* if rom works correctly, status should be always ok */
			if (XST_SUCCESS != status) {
				PmDbg("ERROR setting slave node %s\r\n",
				      PmStrNode(req->slave->node.nodeId));
				break;
			}
		}
		req = req->nextSlave;
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
	PmRequirement* req = master->reqs;

	while (NULL != req) {
		if (req->currReq != req->nextReq) {
			/* Drop the scheduled request by making it constant */
			PmDbg("%s\r\n", PmStrNode(req->slave->node.nodeId));
			req->nextReq = req->currReq;
		}
		req = req->nextSlave;
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
void PmRequirementRequestDefault(const PmMaster* const master)
{
	PmRequirement* req = master->reqs;

	while (NULL != req) {
		if (0U != req->defaultReq) {
			/* Set flag to state that master is using slave */
			req->info |= PM_MASTER_USING_SLAVE_MASK;
			req->nextReq = req->defaultReq;
		}
		req = req->nextSlave;
	}
}

/**
 * PmRequirementReleaseAll() - Called when a processor is forced to power down
 * @master  Master whose processor was forced to power down
 *
 * @return  Status of the operation of releasing all slaves used by the master
 *          and changing their state to the lowest possible.
 */
int PmRequirementReleaseAll(const PmMaster* const master)
{
	int status = XST_SUCCESS;
	PmRequirement* req = master->reqs;

	while (NULL != req) {
		if (0U != (PM_MASTER_USING_SLAVE_MASK & req->info)) {
			/* Clear flag - master is not using slave anymore */
			req->info &= ~PM_MASTER_USING_SLAVE_MASK;
			/* Release current and next requirements */
			req->currReq = 0U;
			req->nextReq = 0U;
			/* Update slave setting */
			status = PmUpdateSlave(req->slave);
			/* if pmu rom works correctly, status should be always ok */
			if (XST_SUCCESS != status) {
				PmDbg("ERROR setting slave node %s\r\n",
				      PmStrNode(req->slave->node.nodeId));
				break;
			}
		}
		req = req->nextSlave;
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
	PmRequirement* req = master->reqs;

	while (NULL != req) {
		if (nodeId == req->slave->node.nodeId) {
			break;
		}
		req = req->nextSlave;
	}

	return req;
}
