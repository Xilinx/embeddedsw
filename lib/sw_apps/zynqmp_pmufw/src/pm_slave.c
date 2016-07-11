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

/*************************************************************************
 * PM slave structures definitions and code for handling states of slaves.
 ************************************************************************/

#include "pm_slave.h"
#include "pm_master.h"
#include "pm_defs.h"
#include "pm_common.h"
#include "pm_node.h"
#include "pm_sram.h"
#include "pm_usb.h"
#include "pm_periph.h"
#include "pm_pll.h"
#include "pm_power.h"
#include "lpd_slcr.h"
#include "pm_ddr.h"

/* Used for tracking number of enabled interrupts in each GIC Proxy group */
PmGicProxyProperties gicProxyGroups_g[FPD_GICP_GROUP_MAX] = {
	[FPD_GICP_GROUP0] = {
		.baseAddr = FPD_GICP_GROUP0_BASE_ADDR,
		.pmuIrqBit = FPD_GICP_PMU_IRQ_GROUP0,
	},
	[FPD_GICP_GROUP1] = {
		.baseAddr = FPD_GICP_GROUP1_BASE_ADDR,
		.pmuIrqBit = FPD_GICP_PMU_IRQ_GROUP1,
	},
	[FPD_GICP_GROUP2] = {
		.baseAddr = FPD_GICP_GROUP2_BASE_ADDR,
		.pmuIrqBit = FPD_GICP_PMU_IRQ_GROUP2,
	},
	[FPD_GICP_GROUP3] = {
		.baseAddr = FPD_GICP_GROUP3_BASE_ADDR,
		.pmuIrqBit = FPD_GICP_PMU_IRQ_GROUP3,
	},
	[FPD_GICP_GROUP4] = {
		.baseAddr = FPD_GICP_GROUP4_BASE_ADDR,
		.pmuIrqBit = FPD_GICP_PMU_IRQ_GROUP4,
	},
};

/* All slaves array */
static PmSlave* const pmSlaves[] = {
	&pmSlaveL2_g.slv,
	&pmSlaveOcm0_g.slv,
	&pmSlaveOcm1_g.slv,
	&pmSlaveOcm2_g.slv,
	&pmSlaveOcm3_g.slv,
	&pmSlaveTcm0A_g.slv,
	&pmSlaveTcm0B_g.slv,
	&pmSlaveTcm1A_g.slv,
	&pmSlaveTcm1B_g.slv,
	&pmSlaveUsb0_g.slv,
	&pmSlaveUsb1_g.slv,
	&pmSlaveTtc0_g.slv,
	&pmSlaveTtc1_g.slv,
	&pmSlaveTtc2_g.slv,
	&pmSlaveTtc3_g.slv,
	&pmSlaveSata_g.slv,
	&pmSlaveApll_g.slv,
	&pmSlaveVpll_g.slv,
	&pmSlaveDpll_g.slv,
	&pmSlaveRpll_g.slv,
	&pmSlaveIOpll_g.slv,
	&pmSlaveGpuPP0_g.slv,
	&pmSlaveGpuPP1_g.slv,
	&pmSlaveUart0_g,
	&pmSlaveUart1_g,
	&pmSlaveSpi0_g,
	&pmSlaveSpi1_g,
	&pmSlaveI2C0_g,
	&pmSlaveI2C1_g,
	&pmSlaveSD0_g,
	&pmSlaveSD1_g,
	&pmSlaveCan0_g,
	&pmSlaveCan1_g,
	&pmSlaveEth0_g,
	&pmSlaveEth1_g,
	&pmSlaveEth2_g,
	&pmSlaveEth3_g,
	&pmSlaveAdma_g,
	&pmSlaveGdma_g,
	&pmSlaveDP_g,
	&pmSlaveNand_g,
	&pmSlaveQSpi_g,
	&pmSlaveGpio_g,
	&pmSlaveAFI_g,
	&pmSlaveDdr_g,
	&pmSlaveIpiApu_g,
	&pmSlaveIpiRpu0_g,
};

/**
 * PmSlaveRequiresPower() - Check whether slave has any request for any
 *                          capability and if current state requires power
 * @slave   Pointer to a slave whose requests are to be checked
 *
 * @return  Based on checking all masters' requests function returns :
 *          - true if there is at least one master requesting a capability and
 *            current state requires power
 *          - false if no master is requesting anything from this slave
 */
bool PmSlaveRequiresPower(const PmSlave* const slave)
{
	u32 i;
	bool hasRequests = false;
	bool requiresPower = false;

	for (i = 0U; i < slave->reqsCnt; i++) {
		if ((0U != (PM_MASTER_USING_SLAVE_MASK & slave->reqs[i]->info)) &&
		    (0U != slave->reqs[i]->currReq)) {
			/* Slave is used by this master and has current request for caps */
			hasRequests = true;
			break;
		}
	}

	if (true == hasRequests) {
		if (0U != (PM_CAP_POWER &
			   slave->slvFsm->states[slave->node.currState])) {
			requiresPower = true;
		}
	}

	return requiresPower;
}

/**
 * PmGetMaxCapabilities()- Get maximum of all requested capabilities of slave
 * @slave   Slave whose maximum required capabilities should be determined
 *
 * @return  32bit value encoding the capabilities
 */
static u32 PmGetMaxCapabilities(const PmSlave* const slave)
{
	u32 i;
	u32 maxCaps = 0U;

	for (i = 0U; i < slave->reqsCnt; i++) {
		if (0U != (PM_MASTER_USING_SLAVE_MASK & slave->reqs[i]->info)) {
			maxCaps |= slave->reqs[i]->currReq;
		}
	}

	return maxCaps;
}

/**
 * PmCheckCapabilities() - Check whether the slave has state with specified
 *                         capabilities
 * @slave   Slave pointer whose capabilities/states should be checked
 * @cap     Check for these capabilities
 *
 * @return  Status wheter slave has a state with given capabilities
 *          - XST_SUCCESS if slave has state with given capabilities
 *          - XST_NO_FEATURE if slave does not have such state
 */
int PmCheckCapabilities(PmSlave* const slave, const u32 cap)
{
	PmStateId i;
	int status = XST_NO_FEATURE;

	for (i = 0; i < slave->slvFsm->statesCnt; i++) {
		/* Find the first state that contains all capabilities */
		if ((cap & slave->slvFsm->states[i]) == cap) {
			status = XST_SUCCESS;
			break;
		}
	}

	return status;
}

/**
 * PmSlaveChangeState() - Change state of a slave
 * @slave       Slave pointer whose state should be changed
 * @state       New state
 *
 * @return      XST_SUCCESS if transition was performed successfully.
 *              Error otherwise.
 */
static int PmSlaveChangeState(PmSlave* const slave, const PmStateId state)
{
	u32 t;
	int status;
	const PmSlaveFsm* fsm = slave->slvFsm;
	PmStateId oldState = slave->node.currState;

	if (0U == fsm->transCnt) {
		/* Slave's FSM has no transitions when it has only one state */
		status = XST_SUCCESS;
	} else {
		/*
		 * Slave has transitions to change the state. Assume the failure
		 * and change status if state is changed correctly.
		 */
		status = XST_FAILURE;
	}

	for (t = 0U; t < fsm->transCnt; t++) {
		/* Find transition from current state to state to be set */
		if ((fsm->trans[t].fromState != slave->node.currState) ||
			(fsm->trans[t].toState != state)) {
			continue;
		}
		if ((0U != (slave->slvFsm->states[state] & PM_CAP_POWER)) &&
		    (NULL != slave->node.parent) &&
		    (true == NODE_IS_OFF(&slave->node.parent->node))) {
			/* Next state requires powering up power parent */
			status = PmTriggerPowerUp(slave->node.parent);
			if (XST_SUCCESS != status) {
				goto done;
			}
		}
		if (NULL != slave->slvFsm->enterState) {
			/* Execute transition action of slave's FSM */
			status = slave->slvFsm->enterState(slave, state);
		} else {
			/*
			 * Slave's FSM has no actions, because it has no private
			 * properties to be controlled here.
			 */
			slave->node.currState = state;
			status = XST_SUCCESS;
		}
		break;
	}

	if ((oldState == slave->node.currState) || (XST_SUCCESS != status)) {
		goto done;
	}

done:
#ifdef DEBUG_PM
	if (XST_SUCCESS == status) {
		PmDbg("%s %d->%d\n", PmStrNode(slave->node.nodeId), oldState,
		      slave->node.currState);
	} else {
		PmDbg("%s ERROR #%d\n", PmStrNode(slave->node.nodeId), status);
	}
#endif
	return status;
}

/**
 * PmGetStateWithCaps() - Get id of the state with provided capabilities
 * @slave       Slave whose states are searched
 * @caps        Capabilities the state must have
 * @state       Pointer to a PmStateId variable where the result is put if
 *              state is found
 *
 * @return      Status of the operation
 *              - XST_SUCCESS if state is found
 *              - XST_NO_FEATURE if state with required capabilities does not
 *                exist
 *
 * This function is to be called when state of a slave should be updated,
 * to find the slave's state with required capabilities.
 * Argument caps has included capabilities requested by all masters which
 * currently use the slave. Although these separate capabilities are validated
 * at the moment request is made, it could happen that there is no state that
 * has capabilities requested by all masters. This conflict has to be resolved
 * between the masters, so PM returns an error.
 */
static int PmGetStateWithCaps(const PmSlave* const slave, const u32 caps,
				  PmStateId* const state)
{
	PmStateId i;
	int status = XST_PM_CONFLICT;

	for (i = 0; i < slave->slvFsm->statesCnt; i++) {
		/* Find the first state that contains all capabilities */
		if ((caps & slave->slvFsm->states[i]) == caps) {
			status = XST_SUCCESS;
			if (NULL != state) {
				*state = i;
			}
			break;
		}
	}

	return status;
}

/**
 * PmGetMinRequestedLatency() - Find minimum of all latency requirements
 * @slave       Slave whose min required latency should be found
 *
 * @return      Latency in microseconds
 */
static u32 PmGetMinRequestedLatency(const PmSlave* const slave)
{
	u32 i, minLatency = MAX_LATENCY;

	for (i = 0U; i < slave->reqsCnt; i++) {
		if (0U != (PM_MASTER_USING_SLAVE_MASK & slave->reqs[i]->info)) {
			if (minLatency > slave->reqs[i]->latencyReq) {
				minLatency = slave->reqs[i]->latencyReq;
			}
		}
	}

	return minLatency;
}

/**
 * PmGetLatencyFromToState() - Get latency from given state to the highest state
 * @slave       Pointer to the slave whose states are in question
 * @state       State from which the latency is calculated
 *
 * @return      Return value for the found latency
 */
u32 PmGetLatencyFromState(const PmSlave* const slave,
			  const PmStateId state)
{
	u32 i, latency = 0U;
	PmStateId highestState = slave->slvFsm->statesCnt - 1;

	for (i = 0U; i < slave->slvFsm->transCnt; i++) {
		if ((state == slave->slvFsm->trans[i].fromState) &&
		    (highestState == slave->slvFsm->trans[i].toState)) {
			latency = slave->slvFsm->trans[i].latency;
			break;
		}
	}

	return latency;
}

/**
 * PmConstrainStateByLatency() - Find a higher power state which satisfies
 *                               latency requirements
 * @slave       Slave whose state may be constrained
 * @state       Chosen state which does not satisfy latency requirements
 * @capsToSet   Capabilities that the state must have
 * @minLatency  Latency requirements to be satisfied
 *
 * @return      Status showing whether the higher power state is found or not.
 *              State may not be found if multiple masters have contradicting
 *              requirements, then XST_PM_CONFLICT is returned. Otherwise,
 *              function returns success.
 */
static int PmConstrainStateByLatency(const PmSlave* const slave,
				     PmStateId* const state,
				     const u32 capsToSet,
				     const u32 minLatency)
{
	int status = XST_PM_CONFLICT;
	PmStateId startState = *state;
	u32 wkupLat, i;

	for (i = startState; i < slave->slvFsm->statesCnt; i++) {
		if ((capsToSet & slave->slvFsm->states[i]) != capsToSet) {
			/* State candidate has no required capabilities */
			continue;
		}
		wkupLat = PmGetLatencyFromState(slave, i);
		if (wkupLat > minLatency) {
			/* State does not satisfy latency requirement */
			continue;
		}

		status = XST_SUCCESS;
		*state = i;
		break;
	}

	return status;
}

/**
 * PmUpdateSlave() - Update the slave's state according to the current
 *                   requirements from all masters
 * @slave       Slave whose state is about to be updated
 *
 * @return      Status of operation of updating slave's state.
 */
int PmUpdateSlave(PmSlave* const slave)
{
	PmStateId state = 0U;
	int status = XST_SUCCESS;
	u32 wkupLat, minLat, latencyMargin;
	u32 capsToSet = PmGetMaxCapabilities(slave);
	PmPower* parent;

	if (0U != capsToSet) {
		/*
		 * This check has to exist because some slaves have no state
		 * with 0 capabilities. Therefore, they are always placed in
		 * first, lowest power state when their caps are not required.
		 */

		/* Get state that has all required capabilities */
		status = PmGetStateWithCaps(slave, capsToSet, &state);
	}

	if (XST_SUCCESS != status) {
		goto done;
	}

	minLat = PmGetMinRequestedLatency(slave);
	wkupLat = PmGetLatencyFromState(slave, state);
	if (wkupLat > minLat) {
		/*
		 * State does not satisfy wake-up latency requirements. Find the
		 * first higher power state which does (in the worst case the
		 * highest power state may be chosen)
		 */
		status = PmConstrainStateByLatency(slave, &state, capsToSet,
						   minLat);

		if (XST_SUCCESS != status) {
			goto done;
		}
	}

	if (state != slave->node.currState) {
		/*
		 * Change state of a slave if state with required capabilities
		 * exists and slave is not already in that state.
		 */
		status = PmSlaveChangeState(slave, state);
	} else {
		/* Ensure that parents meet latency requirement as well */
		parent = slave->node.parent;
		latencyMargin = minLat;

		while ((NULL != parent) && (true == NODE_IS_OFF(&parent->node))) {
			/* Calculate remaining latency budget */
			latencyMargin -= wkupLat;
			wkupLat = parent->pwrUpLatency;
			if (latencyMargin < wkupLat) {
				/* Power up parents from this level up */
				status = PmTriggerPowerUp(parent);
				if (XST_SUCCESS != status) {
					goto done;
				}
				break;
			}
			parent = parent->node.parent;
		}
	}

	/* determine the new latency margin for the parent nodes */
	latencyMargin = slave->node.latencyMarg;
	/* remember the remaining latency margin for upper levels to use */
	slave->node.latencyMarg = minLat - PmGetLatencyFromState(slave, state);

	if ((0U == (slave->slvFsm->states[state] & PM_CAP_POWER)) &&
	    (NULL != slave->node.parent)) {
		PmOpportunisticSuspend(slave->node.parent);
	}

	/* remember the remaining latency margin for upper levels to use */
	slave->node.latencyMarg = minLat - PmGetLatencyFromState(slave, state);

done:
	return status;
}

/**
 * PmSlaveWakeMasters() - Called when slave has to wake-up it's masters
 * @slave   Pointer to a slave whose masters has to be woken-up (if master has
 *          requested this slave as wake-up source before going to sleep)
 *
 * @return  Return status of waking up processors
 *
 * @note:   Wake event of this slave is disabled together with all other slaves
 *          as part of the wake-up sequence.
 */
static int PmSlaveWakeMasters(PmSlave* const slave)
{
	unsigned int i;
	int status;
	int totalSt = XST_SUCCESS;

	for (i = 0U; i < slave->reqsCnt; i++) {
		if (slave->reqs[i]->info & PM_MASTER_WAKEUP_REQ_MASK) {
			PmDbg("%s->%s\n", PmStrNode(slave->node.nodeId),
			      PmStrNode(slave->reqs[i]->requestor->procs->node.nodeId));

			slave->reqs[i]->info &= ~PM_MASTER_WAKEUP_REQ_MASK;
			status = PmProcFsm(slave->reqs[i]->requestor->procs,
					   PM_PROC_EVENT_WAKE);
			if (XST_SUCCESS != status) {
				/*
				 * Failed waking up processor, remember
				 * failure and try to wake-up others
				 */
				totalSt = status;
			}
		}
	}
	PmSlaveWakeDisable(slave);

	return totalSt;
}

/**
 * PmSlaveProcessWake() - Slave has generated wake-up interrupt, find both slave
 *              source and master targets to and trigger wake-up.
 * @wakeMask    Mask read from GPI1 register, based on which slave source that
 *              generated interrupt will be determined. Master targets are
 *              determined based on requirements for slave's wake-up capability.
 *
 * @return      Status of performing wake-up.
 *
 * @note        If multiple slaves has simultaneously generated interrupts (wake
 *              events), they will be all processed in this function). For FPD
 *              GIC Proxy this is a must because reading 32-bit status register
 *              clears the interrupt, meaning that there could be up to 31 irqs
 *              that would be lost if not handled immediately.
 */
int PmSlaveProcessWake(const u32 wakeMask)
{
	int status = XST_SUCCESS;
	u32 g, s, irq;

	if (!(PMU_LOCAL_GPI1_ENABLE_FPD_WAKE_GIC_PROX_MASK & wakeMask)) {
		goto done;
	}

	for (g = 0U; g < ARRAY_SIZE(gicProxyGroups_g); g++) {
		/* Reading status register clears interrupts */
		irq = XPfw_Read32(gicProxyGroups_g[g].baseAddr +
					FPD_GICP_STATUS_OFFSET);
		if (0U == irq) {
			/* No wake in this GIC proxy group, try next group */
			continue;
		}

		for (s = 0U; s < ARRAY_SIZE(pmSlaves); s++) {
			if (NULL == pmSlaves[s]->wake) {
				/* This slave has no wake */
				continue;
			}
			if (pmSlaves[s]->wake->proxyIrqMask & irq) {
				status = PmSlaveWakeMasters(pmSlaves[s]);
				irq &= ~pmSlaves[s]->wake->proxyIrqMask;
				if (0U == irq) {
					break;
				}
			}
		}
	}

done:
	return status;
}

/**
 * PmWaitingForGicProxyWake() - Check is any Fpd wake is unmasked
 * @return  True if there are some wake events unmasked, false otherwise
 */
static bool PmWaitingForGicProxyWake(void)
{
	u32 i;
	bool waitingForWake = false;

	for (i = 0; i < ARRAY_SIZE(gicProxyGroups_g); i++) {
		u32 reg = XPfw_Read32(gicProxyGroups_g[i].baseAddr + FPD_GICP_MASK_OFFSET);

		if (FPD_GICP_ALL_IRQ_MASKED_IN_GROUP != reg) {
			waitingForWake = true;
			break;
		}
	}

	return waitingForWake;
}

/**
 * PmSlaveWakeEnable() - Enable wake interrupt of this slave
 * @slave       Slave whose wake should be enabled
 */
void PmSlaveWakeEnable(PmSlave* const slave)
{
	u32 grMask = slave->wake->proxyGroup->pmuIrqBit;

	PmDbg("%s\n", PmStrNode(slave->node.nodeId));

	if (NULL == slave->wake) {
		goto done;
	}

	/* Check whether a group interrupt is already enabled */
	if (grMask == (XPfw_Read32(LPD_SLCR_GICP_PMU_IRQ_MASK) & grMask)) {
		/* Clear GIC Proxy group */
		XPfw_Write32(LPD_SLCR_GICP_PMU_IRQ_STATUS, grMask);
		/* Enable GIC Proxy group */
		XPfw_Write32(LPD_SLCR_GICP_PMU_IRQ_ENABLE, grMask);
		/* Enable GPI1 FPD GIC Proxy wake event */
		ENABLE_WAKE(PMU_LOCAL_GPI1_ENABLE_FPD_WAKE_GIC_PROX_MASK);
	}

	/* Enable GIC Proxy IRQ */
	XPfw_Write32(slave->wake->proxyGroup->baseAddr +
		     FPD_GICP_IRQ_ENABLE_OFFSET, slave->wake->proxyIrqMask);

done:
	return;
}

/**
 * PmSlaveWakeDisable() - Disable wake interrupt of this slave
 * @slave       Slave whose wake should be disabled
 */
void PmSlaveWakeDisable(PmSlave* const slave)
{
	PmDbg("%s\n", PmStrNode(slave->node.nodeId));

	if (NULL == slave->wake) {
		goto done;
	}

	XPfw_Write32(slave->wake->proxyGroup->baseAddr + FPD_GICP_IRQ_DISABLE_OFFSET,
		     slave->wake->proxyIrqMask);
	if (FPD_GICP_ALL_IRQ_MASKED_IN_GROUP ==
	    XPfw_Read32(slave->wake->proxyGroup->baseAddr + FPD_GICP_MASK_OFFSET)) {
		/* Disable group */
		XPfw_Write32(LPD_SLCR_GICP_PMU_IRQ_DISABLE, slave->wake->proxyGroup->pmuIrqBit);
		if (false == PmWaitingForGicProxyWake()) {
			/* Disable FPD GPI1 wake event */
			DISABLE_WAKE(PMU_LOCAL_GPI1_ENABLE_FPD_WAKE_GIC_PROX_MASK);
		}
	}

done:
	return;
}

/**
 * PmSlaveGetUsersMask() - Gets all masters' mask currently using the slave
 * @slave       Slave in question
 *
 * @return      Each master has unique ipiMask which identifies it (one hot
 *              encoding). Return value represents ORed masks of all masters
 *              which are currently using the slave.
 */
u32 PmSlaveGetUsersMask(const PmSlave* const slave)
{
	u32 i, usage = 0U;

	for (i = 0U; i < slave->reqsCnt; i++) {
		if (0U != (PM_MASTER_USING_SLAVE_MASK & slave->reqs[i]->info)) {
			/* Found master which is using slave */
			usage |= slave->reqs[i]->requestor->ipiMask;
		}
	}

	return usage;
}

/**
 * PmSlaveGetUsageStatus() - get current usage status for a slave node
 * @slavenode  Slave node for which the usage status is requested
 * @master     Master that's requesting the current usage status
 *
 * @return  Usage status:
 *	    - 0: No master is currently using the node
 *	    - 1: Only requesting master is currently using the node
 *	    - 2: Only other masters (1 or more) are currently using the node
 *	    - 3: Both the current and at least one other master is currently
 *               using the node
 */
u32 PmSlaveGetUsageStatus(const u32 slavenode, const PmMaster *const master)
{
	u32 i;
	u32 usageStatus = 0;
	PmMaster* currMaster;
	PmRequirement* masterReq;

	for (i = 0U; i < PM_MASTER_MAX; i++) {
		currMaster = pmAllMasters[i];

		masterReq = PmGetRequirementForSlave(currMaster, slavenode);

		if (NULL == masterReq) {
			/* This master has no access to this slave */
			continue;
		}

		if (0U == (masterReq->info & PM_MASTER_USING_SLAVE_MASK)) {
			/* This master is currently not using this slave */
			continue;
		}

		/* This master is currently using this slave */
		if (currMaster == master) {
			usageStatus |= PM_USAGE_CURRENT_MASTER;
		} else {
			usageStatus |= PM_USAGE_OTHER_MASTER;
		}
	}
	return usageStatus;
}

/**
 * PmSlaveGetRequirements() - get current requirements for a slave node
 * @slavenode  Slave node for which the current requirements are requested
 * @master     Master that's making the request
 *
 * @return  Current requirements of the requesting master on the node
 */
u32 PmSlaveGetRequirements(const u32 slavenode, const PmMaster *const master)
{
	u32 currReq = 0;
	PmRequirement* masterReq = PmGetRequirementForSlave(master, slavenode);

	if (NULL == masterReq) {
		/* This master has no access to this slave */
		goto done;
	}

	if (0U == (masterReq->info & PM_MASTER_USING_SLAVE_MASK)) {
		/* This master is currently not using this slave */
		goto done;
	}

	/* This master is currently using this slave */
	currReq = masterReq->currReq;

done:
	return currReq;
}
