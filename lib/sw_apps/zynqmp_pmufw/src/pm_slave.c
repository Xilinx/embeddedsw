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
#include "pm_clock.h"

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
	&pmSlaveGpu_g,
	&pmSlavePcie_g,
	&pmSlavePcap_g,
	&pmSlaveRtc_g,
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
	PmRequirement* req = slave->reqs;
	bool hasRequests = false;
	bool requiresPower = false;

	while (NULL != req) {
		if ((0U != (PM_MASTER_USING_SLAVE_MASK & req->info)) &&
		    (0U != req->currReq)) {
			/* Slave is used by this master and has current request for caps */
			hasRequests = true;
			break;
		}
		req = req->nextMaster;
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
	PmRequirement* req = slave->reqs;
	u32 maxCaps = 0U;

	while (NULL != req) {
		if (0U != (PM_MASTER_USING_SLAVE_MASK & req->info)) {
			maxCaps |= req->currReq;
		}
		req = req->nextMaster;
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
int PmCheckCapabilities(const PmSlave* const slave, const u32 capabilities)
{
	PmStateId i;
	int status = XST_NO_FEATURE;

	for (i = 0U; i < slave->slvFsm->statesCnt; i++) {
		/* Find the first state that contains all capabilities */
		if ((capabilities & slave->slvFsm->states[i]) == capabilities) {
			status = XST_SUCCESS;
			break;
		}
	}

	return status;
}

/**
 * PmSlaveHasWakeUpCap() - Check if the slave has a wake-up capability
 * @slv		Slave to be checked
 *
 * @return	XST_SUCCESS if the slave has the wake-up capability
 *		XST_NO_FEATURE if the slave doesn't have the wake-up capability
 */
int PmSlaveHasWakeUpCap(const PmSlave* const slv)
{
	int status;

	/* Check is the slave's pointer to the GIC Proxy wake initialized */
	if (NULL == slv->wake) {
		status = XST_NO_FEATURE;
		goto done;
	}

	/* Check whether the slave has a state with wake-up capability */
	status = PmCheckCapabilities(slv, PM_CAP_WAKEUP);

done:
	return status;
}

/**
 * PmSlavePrepareState() - Prepare for entering a state
 * @slv		Slave that would enter next state
 * @next	Next state the slave would enter
 *
 * @return	Status fo preparing for the transition (XST_SUCCESS or an error
 *		code)
 */
static int PmSlavePrepareState(const PmSlave* const slv, const PmStateId next)
{
	int status = XST_SUCCESS;
	const PmStateId curr = slv->node.currState;

	/* If slave has power parent make sure the parent is in proper state */
	if (NULL != slv->node.parent) {

		if ((0U == (slv->slvFsm->states[curr] & PM_CAP_POWER)) &&
		    (0U != (slv->slvFsm->states[next] & PM_CAP_POWER))) {

			/* Slave will need power parent to be on */
			if (true == NODE_IS_OFF(&slv->node.parent->node)) {
				status = PmTriggerPowerUp(slv->node.parent);
				if (XST_SUCCESS != status) {
					goto done;
				}
			}
		}
	}

	/* Check if slave requires clocks in the next state */
	if (NULL != slv->node.clocks) {
		if ((0U == (slv->slvFsm->states[curr] & PM_CAP_CLOCK)) &&
		    (0U != (slv->slvFsm->states[next] & PM_CAP_CLOCK))) {
			status = PmClockRequest(&slv->node);
		}
	}

done:
	return status;
}

/**
 * PmSlaveClearAfterState() - Clean after exiting a state
 * @slv		Slave that exited the prev state
 * @prev	Previous state the slave was in
 */
static void PmSlaveClearAfterState(const PmSlave* const slv, const PmStateId prev)
{
	const PmStateId curr = slv->node.currState;

	/* Check if slave doesn't use clocks in the new state */
	if (NULL != slv->node.clocks) {
		if ((0U != (slv->slvFsm->states[prev] & PM_CAP_CLOCK)) &&
		    (0U == (slv->slvFsm->states[curr] & PM_CAP_CLOCK))) {
			PmClockRelease(&slv->node);
		}
	}
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

	/* Check what needs to be done prior to performing the transition */
	status = PmSlavePrepareState(slave, state);
	if (XST_SUCCESS != status) {
		goto done;
	}

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
		if (NULL != slave->slvFsm->enterState) {
			/* Execute transition action of slave's FSM */
			status = slave->slvFsm->enterState(slave, state);
		} else {
			status = XST_SUCCESS;
		}
		break;
	}

done:
	if ((oldState != state) && (XST_SUCCESS == status)) {
		PmNodeUpdateCurrState(&slave->node, state);
		PmSlaveClearAfterState(slave, oldState);
	}
#ifdef DEBUG_PM
	if (XST_SUCCESS == status) {
		PmDbg("%s %d->%d\r\n", PmStrNode(slave->node.nodeId), oldState,
		      slave->node.currState);
	} else {
		PmDbg("%s ERROR #%d\r\n", PmStrNode(slave->node.nodeId), status);
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

	for (i = 0U; i < slave->slvFsm->statesCnt; i++) {
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
	PmRequirement* req = slave->reqs;
	u32 minLatency = MAX_LATENCY;

	while (NULL != req) {
		if (0U != (PM_MASTER_USING_SLAVE_MASK & req->info)) {
			if (minLatency > req->latencyReq) {
				minLatency = req->latencyReq;
			}
		}
		req = req->nextMaster;
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
 * PmSlaveGetUsersMask() - Gets all masters' mask currently using the slave
 * @slave       Slave in question
 *
 * @return      Each master has unique ipiMask which identifies it (one hot
 *              encoding). Return value represents ORed masks of all masters
 *              which are currently using the slave.
 */
u32 PmSlaveGetUsersMask(const PmSlave* const slave)
{
	PmRequirement* req = slave->reqs;
	u32 usage = 0U;

	while (NULL != req) {
		if (0U != (PM_MASTER_USING_SLAVE_MASK & req->info)) {
			/* Found master which is using slave */
			usage |= req->master->ipiMask;
		}
		req = req->nextMaster;
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

/**
 * PmSlaveVerifyRequest() - Check whether PM framework can grant the request
 * @slave       Slave node that is requested
 *
 * @return      XST_SUCCESS if the following condition is satisfied : (slave
 *              is shareable) OR (it is exclusively used AND no other master
 *              currently uses the slave)
 *              XST_PM_NODE_USED otherwise
 */
int PmSlaveVerifyRequest(const PmSlave* const slave)
{
	int status = XST_SUCCESS;
	u32 usage;

	/* If slave is shareable the request is ok */
	if (0U != (PM_SLAVE_FLAG_IS_SHAREABLE & slave->flags)) {
		goto done;
	}

	usage = PmSlaveGetUsersMask(slave);
	/* Slave is not shareable, if it is unused the request is ok */
	if (0U == usage) {
		goto done;
	}

	/* Slave request cannot be granted, node is non-shareable and used */
	status = XST_PM_NODE_USED;

done:
	return status;
}
