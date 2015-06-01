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
 * Global array of all nodes, and GetbyId function
 *********************************************************************/

#include "pm_node.h"
#include "pm_power.h"
#include "pm_proc.h"
#include "pm_slave.h"
#include "pm_sram.h"
#include "pm_usb.h"
#include "pm_periph.h"
#include "pm_pll.h"

static PmNode* const pmNodes[NODE_MAX] = {
	&pmApuProcs_g[PM_PROC_APU_0].node,
	&pmApuProcs_g[PM_PROC_APU_1].node,
	&pmApuProcs_g[PM_PROC_APU_2].node,
	&pmApuProcs_g[PM_PROC_APU_3].node,
	&pmRpuProcs_g[PM_PROC_RPU_0].node,
	&pmRpuProcs_g[PM_PROC_RPU_1].node,
	&pmPowerIslandRpu_g.node,
	&pmPowerIslandApu_g.node,
	&pmPowerDomainFpd_g.node,
	&pmSlaveL2_g.slv.node,
	&pmSlaveOcm0_g.slv.node,
	&pmSlaveOcm1_g.slv.node,
	&pmSlaveOcm2_g.slv.node,
	&pmSlaveOcm3_g.slv.node,
	&pmSlaveTcm0A_g.slv.node,
	&pmSlaveTcm0B_g.slv.node,
	&pmSlaveTcm1A_g.slv.node,
	&pmSlaveTcm1B_g.slv.node,
	&pmSlaveUsb0_g.slv.node,
	&pmSlaveUsb1_g.slv.node,
	&pmSlaveTtc0_g.slv.node,
	&pmSlaveSata_g.slv.node,
	&pmSlaveApll_g.slv.node,
	&pmSlaveVpll_g.slv.node,
	&pmSlaveDpll_g.slv.node,
	&pmSlaveRpll_g.slv.node,
	&pmSlaveIOpll_g.slv.node,
};

/**
 * PmGetNodeById() - Find node that matches a given node ID
 * @nodeId      ID of the node to find
 *
 * @returns     Pointer to PmNode structure (or NULL if not found)
 */
PmNode* PmGetNodeById(const u32 nodeId)
{
	u32 i;
	PmNode* node = NULL;

	for (i=0; i < NODE_MAX; i++) {
		if (pmNodes[i]->nodeId == nodeId) {
			node = pmNodes[i];
			break;
		}
	}

	return node;
}
