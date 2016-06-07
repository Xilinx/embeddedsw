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
 * Power nodes (power islands and power domains) related structures,
 * transition actions, and FSM definition.
 *********************************************************************/

#include "pm_power.h"
#include "pm_common.h"
#include "pm_proc.h"
#include "pm_master.h"
#include "pm_sram.h"
#include "pm_periph.h"
#include "pm_pll.h"
#include "xpfw_rom_interface.h"
#include "crf_apb.h"
#include "pm_system.h"
#include "pm_ddr.h"
#include "apu.h"

/*
 * Note: PLL registers will never be saved/restored as part of CRF_APB module
 * context. PLLs have separate logic, which is part of PmSlavePll (pm_pll.h/c)
 */
static PmRegisterContext pmFpdContext[] = {
	{ .addr = CRF_APB_ERR_CTRL },
	{ .addr = CRF_APB_CRF_WPROT },
	{ .addr = CRF_APB_ACPU_CTRL, },
	{ .addr = CRF_APB_DBG_TRACE_CTRL },
	{ .addr = CRF_APB_DBG_FPD_CTRL },
	{ .addr = CRF_APB_DP_VIDEO_REF_CTRL },
	{ .addr = CRF_APB_DP_AUDIO_REF_CTRL },
	{ .addr = CRF_APB_DP_STC_REF_CTRL },
	{ .addr = CRF_APB_DDR_CTRL },
	{ .addr = CRF_APB_GPU_REF_CTRL },
	{ .addr = CRF_APB_SATA_REF_CTRL },
	{ .addr = CRF_APB_PCIE_REF_CTRL },
	{ .addr = CRF_APB_GDMA_REF_CTRL },
	{ .addr = CRF_APB_DPDMA_REF_CTRL },
	{ .addr = CRF_APB_TOPSW_MAIN_CTRL },
	{ .addr = CRF_APB_TOPSW_LSBUS_CTRL },
	{ .addr = CRF_APB_GTGREF0_REF_CTRL },
	{ .addr = CRF_APB_DBG_TSTMP_CTRL },
	{ .addr = CRF_APB_RST_FPD_TOP },
	{ .addr = CRF_APB_RST_FPD_APU },
	{ .addr = CRF_APB_RST_DDR_SS },
	{ .addr = APU_PWRCTL },
};

/**
 * PmFpdSaveContext() - Save context of CRF_APB module due to powering down FPD
 */
static void PmFpdSaveContext(void)
{
	u32 i;

	for (i = 0U; i < ARRAY_SIZE(pmFpdContext); i++) {
		pmFpdContext[i].value = XPfw_Read32(pmFpdContext[i].addr);
	}
}

/**
 * PmFpdRestoreContext() - Restore context of CRF_APB module (FPD has been
 *                         powered up)
 */
static void PmFpdRestoreContext(void)
{
	u32 i;

	for (i = 0U; i < ARRAY_SIZE(pmFpdContext); i++) {
		XPfw_Write32(pmFpdContext[i].addr, pmFpdContext[i].value);
	}
}

/**
 * PmPowerDownFpd() - Power down FPD domain
 *
 * @return      Status of the pmu-rom operations
 */
static int PmPowerDownFpd(void)
{
	int status;

	PmPllSuspendAll(&pmPowerDomainFpd_g);
	PmFpdSaveContext();

	status = XpbrPwrDnFpdHandler();
	/*
	 * When FPD is powered off, the APU-GIC will be affected too.
	 * GIC Proxy has to take over for all wake-up sources for
	 * the APU.
	 */
	PmEnableProxyWake(&pmMasterApu_g);

	return status;
}

/**
 * PmPwrDnHandler() - Power down island/domain
 * @nodePtr Pointer to node-structure of power island/dom to be powered off
 *
 * @return  Operation status of power down procedure (done by pmu-rom)
 */
static int PmPwrDnHandler(PmNode* const nodePtr)
{
	int status = XST_PM_INTERNAL;

	if (NULL == nodePtr) {
		goto done;
	}

	/* Call proper PMU-ROM handler as needed */
	switch (nodePtr->nodeId) {
	case NODE_FPD:
		status = PmPowerDownFpd();
		break;
	case NODE_APU:
		status = XST_SUCCESS;
		break;
	case NODE_RPU:
		status = XpbrPwrDnRpuHandler();
		break;
	default:
		PmDbg("unsupported node %s(%d)\n",
		      PmStrNode(nodePtr->nodeId), nodePtr->nodeId);
		break;
	}

	if (XST_SUCCESS == status) {
		PmNodeUpdateCurrState(nodePtr, PM_PWR_STATE_OFF);
	}

done:
	PmDbg("%s\n", PmStrNode(nodePtr->nodeId));
	return status;
}

/**
 * PmPwrUpHandler() - Power up island/domain
 * @nodePtr Pointer to node-structure of power island/dom to be powered on
 *
 * @return  Operation status of power up procedure (done by pmu-rom)
 */
static int PmPwrUpHandler(PmNode* const nodePtr)
{
	int status = XST_PM_INTERNAL;

	PmDbg("%s\n", PmStrNode(nodePtr->nodeId));

	if (NULL == nodePtr) {
		goto done;
	}

	/* Call proper PMU-ROM handler as needed */
	switch (nodePtr->nodeId) {
	case NODE_FPD:
		status = XpbrPwrUpFpdHandler();
		if (XST_SUCCESS == status) {
			PmFpdRestoreContext();
			PmPllResumeAll(&pmPowerDomainFpd_g);
		}
		break;
	case NODE_APU:
		status = XST_SUCCESS;
		break;
	case NODE_RPU:
	{
		u32 reg;

		status = XpbrPwrUpRpuHandler();

		/* release RPU island reset */
		reg = Xil_In32(CRL_APB_RST_LPD_TOP);
		reg &= ~CRL_APB_RST_LPD_TOP_RPU_PGE_RESET_MASK;
		Xil_Out32(CRL_APB_RST_LPD_TOP, reg);
		break;
	}
	default:
		PmDbg("ERROR - unsupported node %s(%d)\n",
		      PmStrNode(nodePtr->nodeId), nodePtr->nodeId);
		break;
	}
	if (XST_SUCCESS == status) {
		PmNodeUpdateCurrState(nodePtr, PM_PWR_STATE_ON);
	}

done:
	return status;
}

/* Children array definitions */
static PmNode* pmApuChildren[] = {
	&pmApuProcs_g[PM_PROC_APU_0].node,
	&pmApuProcs_g[PM_PROC_APU_1].node,
	&pmApuProcs_g[PM_PROC_APU_2].node,
	&pmApuProcs_g[PM_PROC_APU_3].node,
};

static PmNode* pmRpuChildren[] = {
	&pmRpuProcs_g[PM_PROC_RPU_0].node,
	&pmRpuProcs_g[PM_PROC_RPU_1].node,
};

static PmNode* pmFpdChildren[] = {
	&pmPowerIslandApu_g.node,
	&pmSlaveL2_g.slv.node,
	&pmSlaveSata_g.slv.node,
	&pmSlaveApll_g.slv.node,
	&pmSlaveVpll_g.slv.node,
	&pmSlaveDpll_g.slv.node,
	&pmSlaveGpuPP0_g.slv.node,
	&pmSlaveGpuPP1_g.slv.node,
	&pmSlaveGdma_g.node,
	&pmSlaveDP_g.node,
	&pmSlaveAFI_g.node,
	&pmSlaveDdr_g.node,
};

/* Operations for the Rpu power island */
static const PmNodeOps pmRpuNodeOps = {
	.sleep = PmPwrDnHandler,
	.wake = PmPwrUpHandler,
};

/* Operations for the Apu dummy power island */
static const PmNodeOps pmApuNodeOps = {
	.sleep = PmPwrDnHandler,
	.wake = PmPwrUpHandler,
};

/* Operations for the Fpd power domain */
static const PmNodeOps pmFpdNodeOps = {
	.sleep = PmPwrDnHandler,
	.wake = PmPwrUpHandler,
};

/* Dummy consumption for the power domains/islands */
static u32 PmDomainPowers[] = {
	DEFAULT_POWER_OFF,
	DEFAULT_POWER_OFF,
};

/*
 * Power Island and Power Domain definitions
 *
 * We only define those islands and domains containing more than 1 node.
 * For optimization reasons private power islands, such as APU0-island or
 * USB0-island are modeled as a feature of the node itself and are therefore
 * not described here.
 */
PmPower pmPowerIslandRpu_g = {
	.node = {
		.derived = &pmPowerIslandRpu_g,
		.nodeId = NODE_RPU,
		.typeId = PM_TYPE_PWR_ISLAND,
		.parent = NULL,
		.currState = PM_PWR_STATE_ON,
		.latencyMarg = MAX_LATENCY,
		.ops = &pmRpuNodeOps,
		.powerInfo = PmDomainPowers,
		.powerInfoCnt = ARRAY_SIZE(PmDomainPowers),
	},
	.children = pmRpuChildren,
	.childCnt = ARRAY_SIZE(pmRpuChildren),
	.pwrDnLatency = PM_POWER_ISLAND_LATENCY,
	.pwrUpLatency = PM_POWER_ISLAND_LATENCY,
};

/*
 * @Note: The APU power island does not physically exist, therefore it has
 * no operations and no latencies. The individual APU cores have their own
 * dedicated power islands, the transition latency is hence accounted for
 * in PmProc
 */
PmPower pmPowerIslandApu_g = {
	.node = {
		.derived = &pmPowerIslandApu_g,
		.nodeId = NODE_APU,
		.typeId = PM_TYPE_PWR_ISLAND,
		.parent = &pmPowerDomainFpd_g,
		.currState = PM_PWR_STATE_ON,
		.latencyMarg = MAX_LATENCY,
		.ops = &pmApuNodeOps,
		.powerInfo = PmDomainPowers,
		.powerInfoCnt = ARRAY_SIZE(PmDomainPowers),
	},
	.children = pmApuChildren,
	.childCnt = ARRAY_SIZE(pmApuChildren),
	.pwrDnLatency = 0,
	.pwrUpLatency = 0,
};

PmPower pmPowerDomainFpd_g = {
	.node = {
		.derived = &pmPowerDomainFpd_g,
		.nodeId = NODE_FPD,
		.typeId = PM_TYPE_PWR_DOMAIN,
		.parent = NULL,
		.currState = PM_PWR_STATE_ON,
		.latencyMarg = MAX_LATENCY,
		.ops = &pmFpdNodeOps,
		.powerInfo = PmDomainPowers,
		.powerInfoCnt = ARRAY_SIZE(PmDomainPowers),
	},
	.children = pmFpdChildren,
	.childCnt = ARRAY_SIZE(pmFpdChildren),
	.pwrDnLatency = PM_POWER_DOMAIN_LATENCY,
	.pwrUpLatency = PM_POWER_DOMAIN_LATENCY,
};

/**
 * PmChildIsInLowestPowerState() - Checked whether the child node is in lowest
 *                                 power state
 * @nodePtr     Pointer to a node whose state should be checked
 */
static bool PmChildIsInLowestPowerState(const PmNode* const nodePtr)
{
	bool status = false;

	if ((true == NODE_IS_PROC(nodePtr->typeId)) ||
	    (true == NODE_IS_POWER(nodePtr->typeId))) {
		if (true == NODE_IS_OFF(nodePtr)) {
			status = true;
		}
	} else {
		/* Node is a slave */
		if (false == PmSlaveRequiresPower((PmSlave*)nodePtr->derived)) {
			status = true;
		}
	}

	return status;
}

/**
 * PmHasAwakeChild() - Check whether power node has awake children
 * @power       Pointer to PmPower object to be checked
 *
 * Used during opportunistic suspend:
 * Function checks whether any child of the power provided as argument stops
 * power from being turned off. In the case of processor or power child, that
 * can be checked by inspecting currState value. For slaves, that is not the
 * case, as slave can be in non-off state just because the off state is entered
 * when power is turned off. This is the case when power parent is common for
 * multiple nodes. Therefore, slave does not block power from turning off if
 * it is unused and not in lowest power state.
 *
 * Latency accounting: determine the lowest latency requirement of any child
 * and pass it up to the power island/domain node.
 *
 * @return      True if it has a child that is not off
 */
static bool PmHasAwakeChild(PmPower* const power)
{
	u32 i;
	u32 minLatencyMargin = MAX_LATENCY;
	bool hasAwakeChild = false;

	for (i = 0U; i < power->childCnt; i++) {
		/* Determine the lowest latency requirement of any child */
		if (power->children[i]->latencyMarg < minLatencyMargin) {
			minLatencyMargin = power->children[i]->latencyMarg;
		}

		if (false == PmChildIsInLowestPowerState(power->children[i])) {
			hasAwakeChild = true;
			PmDbg("%s\n", PmStrNode(power->children[i]->nodeId));
			break;
		}
	}

	/* Pass the lowest latency margin to the power island/domain node */
	power->node.latencyMarg = minLatencyMargin;

	return hasAwakeChild;
}

/**
 * PmOpportunisticSuspend() - After a node goes to sleep, try to power off
 *                            parents
 * @powerParent Pointer to the power node which should try to suspend, as well
 *              its parents.
 */
void PmOpportunisticSuspend(PmPower* const powerParent)
{
	u32 worstCaseLatency;
	PmPower* power = powerParent;

	if (NULL == powerParent) {
		goto done;
	}

	if (true == PmSystemShutdownProcessing()) {
		/*
		 * Opportunistic suspend is not performed when shutting down
		 * or restarting.
		 */
		goto done;
	}

	do {
		PmDbg("Opportunistic suspend attempt for %s\n",
		      PmStrNode(power->node.nodeId));

		worstCaseLatency = power->pwrUpLatency + power->pwrDnLatency;

		if ((false == PmHasAwakeChild(power)) &&
		    (true == NODE_HAS_SLEEP(power->node.ops))) {
			/* Note: latencyMarg field updated by PmHasAwakeChild */
			if (worstCaseLatency < power->node.latencyMarg) {
				/* Call sleep function of this power node */
				power->node.ops->sleep(&power->node);
				power = power->node.parent;
				continue;
			}
		}
		power = NULL;

	} while (NULL != power);

done:
	return;
}

/**
 * PmPowerUpTopParent() - Power up top parent in hierarchy that's currently off
 * @powerChild  Power child whose power parent has to be powered up
 *
 * @return      Status of the power up operation (XST_SUCCESS if all power
 *              parents are already powered on)
 *
 * This function turns on exactly one power parent, starting with the highest
 * level parent that's currently off. If all power parents are on, it will
 * turn on "powerChild", which was passed as an argument.
 *
 * Since MISRA-C doesn't allow recursion, there's an iterative algorithm in
 * PmTriggerPowerUp that calls this function iteratively until all power
 * nodes in the hierarchy are powered up.
 */
static int PmPowerUpTopParent(PmPower* const powerChild)
{
	int status = XST_SUCCESS;
	PmPower* powerParent = powerChild;

	if (NULL == powerParent) {
		status = XST_PM_INTERNAL;
		goto done;
	}

	/*
	 * Powering up needs to happen from the top down, so find the highest
	 * level parent that's currently still off and turn it on.
	 */
	while ((NULL != powerParent->node.parent) &&
	       (true == NODE_IS_OFF(&powerParent->node.parent->node))) {
		powerParent = powerChild->node.parent;
	}

	status = powerParent->node.ops->wake(&powerParent->node);

done:
	return status;
}

/**
 * PmTriggerPowerUp() - Triggered by child node (processor or slave) when it
 *                      needs its power islands/domains to be powered up
 * @power       Power node that needs to be powered up
 *
 * @return      Status of the power up operation.
 */
int PmTriggerPowerUp(PmPower* const power)
{
	int status = XST_SUCCESS;

	if (NULL == power) {
		goto done;
	}

	/*
	 * Multiple hierarchy levels of power islands/domains may need to be
	 * turned on (always top-down).
	 * Use iterative approach for MISRA-C compliance
	 */
	while ((true == NODE_IS_OFF(&power->node)) && (XST_SUCCESS == status)) {
		status = PmPowerUpTopParent(power);
	}

done:
#ifdef DEBUG_PM
	if (XST_SUCCESS != status) {
		PmDbg("ERROR #%d failed to power up\n", status);
	}
#endif

	return status;
}

/**
 * PmGetLowestParent() - Returns the first leaf parent with active children
 * @root    Pointer to a power object of which the leafs should be found
 *
 * In a parent-child tree, a leaf parent is a parent that has one or more
 * children, where none of the children are parents.
 * (Any state != 0 is considered an active state)
 *
 * Iterative algorithm for MISRA-C compliance
 *
 * @return  Pointer to an active leaf or root itself if no leafs exist
 */
static PmPower* PmGetLowestParent(PmPower* const root)
{
	PmPower* prevParent;
	PmPower* currParent = root;

	if (NULL == currParent) {
		goto done;
	}

	do {
		u32 i;
		prevParent = currParent;

		for (i = 0U; i < currParent->childCnt; i++) {
			if ((true != NODE_IS_POWER(currParent->children[i]->typeId)) ||
				(false != NODE_IS_OFF(currParent->children[i]))) {
				continue;
			}

			/* Active power child found */
			currParent = (PmPower*)currParent->children[i]->derived;
			break;
		}
	} while (currParent != prevParent);

done:
	return currParent;
}

/**
 * PmForcePowerDownChildren() - Forces power down for child nodes
 * @parent      pointer to power object whose children are to be turned off
 */
static void PmForcePowerDownChildren(const PmPower* const parent)
{
	u32 i;
	PmNode* child;
	PmProc* proc;

	for (i = 0U; i < parent->childCnt; i++) {
		child = parent->children[i];

		if ((false != PmChildIsInLowestPowerState(child)) ||
		    (true != NODE_HAS_SLEEP(child->ops))) {
			continue;
		}

		PmDbg("Powering OFF child node %s\n", PmStrNode(child->nodeId));

		/* Force the child's state to 0, which is its lowest power state */
		child->ops->sleep(child);
		PmNodeUpdateCurrState(child, 0U);

		/* Special case: node is a processor, release slave-requirements */
		if (PM_TYPE_PROC == child->typeId) {
			proc = (PmProc*)child->derived;

			if ((NULL !=proc) && (true == proc->isPrimary)) {
				/* Notify master so it can release all requirements */
				PmMasterNotify(proc->master, PM_PROC_EVENT_FORCE_PWRDN);
			}
		}
	}

	return;
}

/**
 * PmForceDownTree() - Force power down node and all its children
 * @root        power node (island/domain) to turn off
 *
 * @return      Operation status of power down procedure
 *
 * This is a power island or power domain, power it off from the bottom up:
 * find out if this parent has children which themselves have children.
 * Note: using iterative algorithm for MISRA-C compliance
 * (instead of recursion)
 *
 */
int PmForceDownTree(PmPower* const root)
{
	int status = XST_PM_INTERNAL;
	PmPower* lowestParent;

	if (NULL == root) {
		goto done;
	}

	do {
		lowestParent = PmGetLowestParent(root);
		PmForcePowerDownChildren(lowestParent);
		if (true == NODE_HAS_SLEEP(lowestParent->node.ops)) {
			status = lowestParent->node.ops->sleep(&lowestParent->node);
		}
	} while ((lowestParent != root) && (XST_SUCCESS == status));

done:
	return status;
}
