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
#include "pm_notifier.h"
#include "pm_ddr.h"

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
	&pmPowerDomainPld_g.node,
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
	&pmSlaveTtc1_g.slv.node,
	&pmSlaveTtc2_g.slv.node,
	&pmSlaveTtc3_g.slv.node,
	&pmSlaveSata_g.slv.node,
	&pmSlaveApll_g.slv.node,
	&pmSlaveVpll_g.slv.node,
	&pmSlaveDpll_g.slv.node,
	&pmSlaveRpll_g.slv.node,
	&pmSlaveIOpll_g.slv.node,
	&pmSlaveGpuPP0_g.slv.node,
	&pmSlaveGpuPP1_g.slv.node,
	&pmSlaveUart0_g.node,
	&pmSlaveUart1_g.node,
	&pmSlaveSpi0_g.node,
	&pmSlaveSpi1_g.node,
	&pmSlaveI2C0_g.node,
	&pmSlaveI2C1_g.node,
	&pmSlaveSD0_g.node,
	&pmSlaveSD1_g.node,
	&pmSlaveCan0_g.node,
	&pmSlaveCan1_g.node,
	&pmSlaveEth0_g.node,
	&pmSlaveEth1_g.node,
	&pmSlaveEth2_g.node,
	&pmSlaveEth3_g.node,
	&pmSlaveAdma_g.node,
	&pmSlaveGdma_g.node,
	&pmSlaveDP_g.node,
	&pmSlaveNand_g.node,
	&pmSlaveQSpi_g.node,
	&pmSlaveGpio_g.node,
	&pmSlaveAFI_g.node,
	&pmSlaveDdr_g.node,
	&pmSlaveIpiApu_g.node,
	&pmSlaveIpiRpu0_g.node,
	&pmSlaveGpu_g.node,
	&pmSlavePcie_g.node,
	&pmSlavePcap_g.node,
	&pmSlaveRtc_g.node,
	&pmPowerDomainLpd_g.node,
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

	for (i = 0U; i < NODE_MAX; i++) {
		if (pmNodes[i]->nodeId == nodeId) {
			node = pmNodes[i];
			break;
		}
	}

	return node;
}

/**
 * PmNodeUpdateCurrState() - Call to update currState variable of the node
 * @node        Pointer to the node whose state has to be updated
 * @newState    New state value to be written in currState variable of the node
 */
void PmNodeUpdateCurrState(PmNode* const node, const PmStateId newState)
{
	if (newState == node->currState) {
		goto done;
	}
	node->currState = newState;

	PmNotifierEvent(node, EVENT_STATE_CHANGE);

done:
	return;
}

/**
 * PmNodeLookupConsumption() - Lookup the power consumption of a node in a
 *			       specific state
 * @node       Pointer to the node of the slave
 * @state      State of the slave
 *
 * @returns    Power consumption for the slave in the specified state
 */
static u32 PmNodeLookupConsumption(PmNode* const nodePtr, PmStateId state)
{
	u32 power = POWER_MISSING;

	if ((NULL == nodePtr) || (NULL == nodePtr->powerInfo) ||
	    (nodePtr->powerInfoCnt <= state)) {
		goto done;
	}

	power = nodePtr->powerInfo[state];

done:
	return power;
}

/**
 * PmNodeGetChildPos() - Find child's position in its parent array of children
 *                 matching a given node ID
 * @node        ID of the node
 *
 * @returns     Child's position in its parent's array of children
 */
static u32 PmNodeGetChildPos(PmNodeId node)
{
	u8 i;
	u32 childPos = ~0;
	PmNode* nodePtr = PmGetNodeById(node);

	if ((NULL == nodePtr) || (NULL == nodePtr->parent)) {
		goto done;
	}

	for (i = 0U; i < nodePtr->parent->childCnt; i++) {
		if (node == nodePtr->parent->children[i]->nodeId) {
			childPos = i;
			break;
		}
	}

done:
	return childPos;
}

/**
 * PmNodeGetPowerConsumption() - Get power consumption of a node in a specific
 *                               state
 * @node       Pointer to the node
 * @state      State of the node
 *
 * @returns    Power consumption for the node in a specific state
 *
 * If the requested component represents power island or domain, return the
 * power consumption as a sum of consumptions of the children.
 */
u32 PmNodeGetPowerConsumption(PmNode* const nodePtr, const PmStateId state)
{
	u32 childPwr;
	u32 childPos;
	PmPower* root;
	PmPower* currParent;
	PmNode* childNode;
	u32 power = 0U;

	if ((NULL == nodePtr) || (NULL == nodePtr->powerInfo) ||
	    (nodePtr->powerInfoCnt <= state)) {
		goto done;
	}

	power = PmNodeLookupConsumption(nodePtr, state);

	if ((false == NODE_IS_POWER(nodePtr->typeId)) ||
	    (true == NODE_IS_OFF(nodePtr))) {
		goto done;
	}

	/*
	 * If requested node is power island or domain, account
	 * power consumptions of components in hierarchy below
	 */
	root = (PmPower*)nodePtr->derived;
	currParent = root;
	childPos = 0U;

	/*
	 * Since MISRA-C doesn't allow recursion, we need an iterative
	 * algorithm to determine the power consumption for the complete tree.
	 * Using a depth-first approach:
	 */
	do {
		while (childPos < currParent->childCnt) {
			childNode = currParent->children[childPos];
			/* Every node may have a power consumption: */
			childPwr = PmNodeLookupConsumption(childNode,
							   childNode->currState);

			if (childPwr == POWER_MISSING) {
				power = POWER_MISSING;
				goto done;
			} else {
				power += childPwr;
			}

			if (true == NODE_IS_POWER(childNode->typeId)) {
				/* Work our way down to the lowest child */
				currParent = (PmPower*)childNode->derived;
				childPos = 0U;
			} else {
				childPos++;
			}
		}

		if (currParent == root) {
			break;
		}

		/* Now go back up one level, and continue where we left off */
		childPos = PmNodeGetChildPos(currParent->node.nodeId);
		if (currParent->childCnt <= childPos) {
			power = POWER_MISSING;
			goto done;
		}
		childPos++;
		currParent = currParent->node.parent;
	} while (1);

done:
	return power;
}

/**
 * PmNodeGetWakeLatency() - Get wake-up latency of a node
 * @node       Pointer to the node
 *
 * @returns    Wake-up latency for the node
 */
u32 PmNodeGetWakeLatency(PmNode* const nodePtr)
{
	u32 result = 0U;
	PmPower* parent = nodePtr->parent;
	const PmPower* power;
	const PmSlave* slave;
	const PmProc* proc;

	if (true == NODE_IS_SLAVE(nodePtr->typeId)) {
		slave = (PmSlave*)nodePtr->derived;
		result = PmGetLatencyFromState(slave, nodePtr->currState);
	} else if ((true == NODE_IS_PROC(nodePtr->typeId)) &&
		   (true == NODE_IS_OFF(nodePtr))) {
			proc = (PmProc*)nodePtr->derived;
			result = proc->pwrUpLatency;
	} else if ((true == NODE_IS_POWER(nodePtr->typeId)) &&
		   (true == NODE_IS_OFF(nodePtr))) {
		power = (PmPower*)nodePtr->derived;
		result = power->pwrUpLatency;
	}

	/*
	 * In case when parent power islands/domains are turned off,
	 * hierarchically account its powerUp latencies
	 */
	while ((NULL != parent) && (true == NODE_IS_OFF(&parent->node))) {
		result += parent->pwrUpLatency;
		parent = parent->node.parent;
	}

	return result;
}

/**
 * PmNodeDependsOnClock() - Check whether node currently depends on its clock
 */
bool PmNodeDependsOnClock(const PmNode* const node)
{
	bool deps = false;

	if (true == NODE_IS_SLAVE(node->typeId)) {
		const PmSlave* const slv = (PmSlave*)node->derived;

		if (0U != (PM_CAP_CLOCK & slv->slvFsm->states[node->currState])) {
			deps = true;
		}
	} else if (false == NODE_IS_OFF(node)) {
		deps = true;
	} else {
		/* Empty else required by MISRA */
	}

	return deps;
}
