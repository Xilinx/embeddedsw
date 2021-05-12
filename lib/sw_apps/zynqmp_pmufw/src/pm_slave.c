/*
* Copyright (c) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */

#include "xpfw_config.h"
#ifdef ENABLE_PM

/*************************************************************************
 * PM slave structures definitions and code for handling states of slaves.
 ************************************************************************/

#include "pm_slave.h"
#include "pm_requirement.h"
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
#include <unistd.h>
#include "pm_gpp.h"
#include "pm_extern.h"
#include "pm_system.h"

#define HAS_CAPABILITIES(slavePtr, state, caps)	\
	((caps) == ((caps) & (slavePtr)->slvFsm->states[state]))

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
		maxCaps |= req->currReq;
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
 * @return  Status whether slave has a state with given capabilities
 *          - XST_SUCCESS if slave has state with given capabilities
 *          - XST_NO_FEATURE if slave does not have such state
 */
s32 PmCheckCapabilities(const PmSlave* const slave, const u32 capabilities)
{
	PmStateId i;
	s32 status = XST_NO_FEATURE;

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
s32 PmSlaveHasWakeUpCap(const PmSlave* const slv)
{
	s32 status;

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
static s32 PmSlavePrepareState(PmSlave* const slv, const PmStateId next)
{
	s32 status = XST_SUCCESS;
	const PmStateId curr = slv->node.currState;

	/* If slave has power parent make sure the parent is in proper state */
	if (NULL != slv->node.parent) {

		if ((0U == (slv->slvFsm->states[curr] & PM_CAP_POWER)) &&
		    (0U != (slv->slvFsm->states[next] & PM_CAP_POWER))) {
			status = PmPowerRequestParent(&slv->node);
			if (XST_SUCCESS != status) {
				goto done;
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
static void PmSlaveClearAfterState(PmSlave* const slv, const PmStateId prev)
{
	const PmStateId curr = slv->node.currState;

	/* Check if slave doesn't use clocks in the new state */
	if (NULL != slv->node.clocks) {
		if ((0U != (slv->slvFsm->states[prev] & PM_CAP_CLOCK)) &&
		    (0U == (slv->slvFsm->states[curr] & PM_CAP_CLOCK))) {
			PmClockRelease(&slv->node);
		}
	}

	/* Check if slave doesn't need power in the new state */
	if (NULL != slv->node.parent) {
		if ((0U != (slv->slvFsm->states[prev] & PM_CAP_POWER)) &&
		    (0U == (slv->slvFsm->states[curr] & PM_CAP_POWER))) {
			PmPowerReleaseParent(&slv->node);
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
static s32 PmSlaveChangeState(PmSlave* const slave, const PmStateId state)
{
	u32 t;
	s32 status;
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
	if (XST_SUCCESS == status) {
		PmInfo("%s %d->%d\r\n", slave->node.name, oldState,
		       slave->node.currState);
	} else {
		PmErr("#%d %s state#%u\r\n", status, slave->node.name,
		      oldState);
	}

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
static s32 PmGetStateWithCaps(const PmSlave* const slave, const u32 caps,
				  PmStateId* const state)
{
	PmStateId i;
	s32 status = XST_PM_CONFLICT;

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
		if (0U != (PM_MASTER_SET_LATENCY_REQ & req->info)) {
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
static u32 PmGetLatencyFromState(const PmSlave* const slave,
			  const PmStateId state)
{
	u32 i, latency = 0U;
	PmStateId highestState = (PmStateId)slave->slvFsm->statesCnt - 1U;

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
static s32 PmConstrainStateByLatency(const PmSlave* const slave,
				     PmStateId* const state,
				     const u32 capsToSet,
				     const u32 minLatency)
{
	s32 status = XST_PM_CONFLICT;
	PmStateId startState = *state;
	u32 wkupLat, i;

	for (i = startState; i < slave->slvFsm->statesCnt; i++) {
		if ((capsToSet & slave->slvFsm->states[i]) != capsToSet) {
			/* State candidate has no required capabilities */
			continue;
		}
		wkupLat = PmGetLatencyFromState(slave, (PmStateId)i);
		if (wkupLat > minLatency) {
			/* State does not satisfy latency requirement */
			continue;
		}

		status = XST_SUCCESS;
		*state = (PmStateId)i;
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
 *
 * @note	A slave may not have state with zero capabilities. If that is
 * the case and no capabilities are requested, it is put in lowest power state
 * (state ID 0).
 * When non-zero capabilities are requested and a selected state which has the
 * requested capabilities doesn't satisfy the wake-up latency requirements, the
 * first higher power state which satisfies latency requirement and has the
 * requested capabilities is configured (in the worst case it's the highest
 * power state).
 */
s32 PmUpdateSlave(PmSlave* const slave)
{
	PmStateId state = 0U;
	s32 status = XST_SUCCESS;
	u32 wkupLat, minLat;
	u32 caps = PmGetMaxCapabilities(slave);

	if (0U != caps) {
		/* Find which state has the requested capabilities */
		status = PmGetStateWithCaps(slave, caps, &state);
		if (XST_SUCCESS != status) {
			goto done;
		}
	}

	minLat = PmGetMinRequestedLatency(slave);
	wkupLat = PmGetLatencyFromState(slave, state);
	if (wkupLat > minLat) {
		/* State does not satisfy latency requirement, find another */
		status = PmConstrainStateByLatency(slave, &state, caps, minLat);
		if (XST_SUCCESS != status) {
			goto done;
		}
		wkupLat = PmGetLatencyFromState(slave, state);
	}

	slave->node.latencyMarg = minLat - wkupLat;
	if (state != slave->node.currState) {
		status = PmSlaveChangeState(slave, state);
		if (XST_SUCCESS != status) {
			goto done;
		}
	} else {
		if (!HAS_CAPABILITIES(slave, state, PM_CAP_POWER) &&
		    (NULL != slave->node.parent)) {
			/* Notify power parent (changed latency requirement) */
			status = PmPowerUpdateLatencyReq(&slave->node);
		}
	}

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
		if (MASTER_REQUESTED_SLAVE(req)) {
			/* Found master which is using slave */
			usage |= req->master->ipiMask;
		}
		req = req->nextMaster;
	}

	return usage;
}

/**
 * PmSlaveGetUsageStatus() - get current usage status for a slave node
 * @slave      Slave node for which the usage status is requested
 * @master     Master that's requesting the current usage status
 *
 * @return  Usage status:
 *	    - 0: No master is currently using the node
 *	    - 1: Only requesting master is currently using the node
 *	    - 2: Only other masters (1 or more) are currently using the node
 *	    - 3: Both the current and at least one other master is currently
 *               using the node
 */
u32 PmSlaveGetUsageStatus(const PmSlave* const slave,
			  const PmMaster* const master)
{
	u32 usageStatus = 0U;
	const PmRequirement* req = slave->reqs;

	while (NULL != req) {

		if (MASTER_REQUESTED_SLAVE(req)) {
			/* This master is currently using this slave */
			if (master == req->master) {
				usageStatus |= PM_USAGE_CURRENT_MASTER;
			} else {
				usageStatus |= PM_USAGE_OTHER_MASTER;
			}
		}
		req = req->nextMaster;
	}

	return usageStatus;
}

/**
 * PmSlaveGetRequirements() - get current requirements for a slave node
 * @slave      Slave node for which the current requirements are requested
 * @master     Master that's making the request
 *
 * @return  Current requirements of the requesting master on the node
 */
u32 PmSlaveGetRequirements(const PmSlave* const slave,
			   const PmMaster* const master)
{
	u32 currReq = 0U;
	PmRequirement* masterReq = PmRequirementGet(master, slave);

	if (NULL == masterReq) {
		/* This master has no access to this slave */
		goto done;
	}

	if (!MASTER_REQUESTED_SLAVE(masterReq)) {
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
s32 PmSlaveVerifyRequest(const PmSlave* const slave)
{
	s32 status = XST_SUCCESS;
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

/**
 * PmSlaveSetConfig() - Set the configuration for the slave
 * @slave       Slave to configure
 * @policy      Usage policy for the slave to configure
 * @perms       Permissions to use the slave (ORed IPI masks of permissible
 *              masters)
 * @return      XST_SUCCESS if configuration is set, XST_FAILURE otherwise
 *
 * @note        For each master whose IPI is encoded in the 'perms', the
 *              requirements structure is automatically allocated and added in
 *              master's/slave's lists of requirements.
 */
s32 PmSlaveSetConfig(PmSlave* const slave, const u32 policy, const u32 perms)
{
	s32 status = XST_SUCCESS;
	u32 masterIpiMasks = perms;
	u32 caps = slave->slvFsm->states[slave->slvFsm->statesCnt - 1U];

	if (0U != (policy & PM_SLAVE_FLAG_IS_SHAREABLE)) {
		slave->flags |= PM_SLAVE_FLAG_IS_SHAREABLE;
	}

	/* Extract and process one by one master from the encoded perms */
	while (0U != masterIpiMasks) {
		PmMaster* master = PmMasterGetNextFromIpiMask(&masterIpiMasks);
		PmRequirement* req;

		if (NULL == master) {
			status = XST_FAILURE;
			goto done;
		}

		req = PmRequirementAdd(master, slave);
		if (NULL == req) {
			status = XST_FAILURE;
			goto done;
		}
		req->currReq = (u8)caps;
	}

done:
	return status;
}

/**
 * PmSlaveClearConfig() - Clear configuration of the slave node
 * @slaveNode	Slave node to clear
 */
static void PmSlaveClearConfig(PmNode* const slaveNode)
{
	PmSlave* const slave = (PmSlave*)slaveNode->derived;

	slave->reqs = NULL;
	slave->flags = 0U;
}

/**
 * PmSlaveGetWakeUpLatency() - Get wake-up latency of the slave node
 * @node	Slave node
 * @lat		Pointer to the location where the latency value should be stored
 *
 * @return	XST_SUCCESS if latency value is stored in *lat, XST_NO_FEATURE
 *		if the latency depends on power parent which has no method
 *		(getWakeUpLatency) to provide latency information
 */
static s32 PmSlaveGetWakeUpLatency(const PmNode* const node, u32* const lat)
{
	PmSlave* const slave = (PmSlave*)node->derived;
	PmNode* powerNode;
	s32 status = XST_SUCCESS;
	u32 latency = 0U;

	*lat = PmGetLatencyFromState(slave, slave->node.currState);

	if (NULL == node->parent) {
		status = XST_NO_FEATURE;
		goto done;
	}

	powerNode = &node->parent->node;
	if (NULL == powerNode->class->getWakeUpLatency) {
		status = XST_NO_FEATURE;
		goto done;
	}

	status = powerNode->class->getWakeUpLatency(powerNode, &latency);
	if (XST_SUCCESS == status) {
		*lat += latency;
	}

done:
	return status;

}

 /**
 * PmSlaveForceDown() - Force down the slave node
 * @node	Slave node to force down
 *
 * @return	Status of performing force down operation
 */
static s32 PmSlaveForceDown(PmNode* const node)
{
	s32 status = XST_SUCCESS;
	PmSlave* const slave = (PmSlave*)node->derived;
	PmRequirement* req = slave->reqs;

	while (NULL != req) {
		if (MASTER_REQUESTED_SLAVE(req)) {
			PmRequirementClear(req);
		}
		req = req->nextMaster;
	}
	status = PmUpdateSlave(slave);

	if ((NULL != slave->class) && (NULL != slave->class->forceDown)) {
		status = slave->class->forceDown(slave);
	}

	return status;
}

/**
 * PmSlaveInit() - Initialize the slave node
 * @node	Node to initialize
 *
 * @return	Status of initializing the node
 */
static s32 PmSlaveInit(PmNode* const node)
{
	PmSlave* const slave = (PmSlave*)node->derived;
	s32 status = XST_SUCCESS;

	if (NULL != node->parent) {
		if (HAS_CAPABILITIES(slave, node->currState, PM_CAP_POWER)) {
			status = PmPowerRequestParent(node);
			if (XST_SUCCESS != status) {
				goto done;
			}
		}
	}

	if (NULL != node->clocks) {
		if (HAS_CAPABILITIES(slave, node->currState, PM_CAP_CLOCK)) {
			status = PmClockRequest(node);
		}
	}

	if ((NULL != slave->class) && (NULL != slave->class->init)) {
		status = slave->class->init(slave);
	}

done:
	return status;
}

/**
 * PmSlaveIsUsable() - Check if slave is usable according to the configuration
 * @node	Slave node to check
 *
 * @return	True if slave can be used, false otherwise
 */
static bool PmSlaveIsUsable(PmNode* const node)
{
	bool usable = true;
	PmSlave* const slave = (PmSlave*)node->derived;

	/* Slave is not usable if it has no allocated requirements */
	if (NULL == slave->reqs) {
		usable = false;
	}

	return usable;
}

/**
 * PmSlaveGetPerms() - Get permissions of masters to control slave's clocks
 * @node	Slave node
 *
 * @return	ORed masks of permissible masters' IPI masks
 *
 * @note	Only masters that have requested the slave are accounted to have
 *		permissions
 */
static u32 PmSlaveGetPerms(const PmNode* const node)
{
	PmSlave* slave = (PmSlave*)node->derived;
	PmRequirement* req = slave->reqs;
	u32 perms = 0U;

	while (NULL != req) {
		/* Check if system requirement (used by PMU) */
		if (NULL == req->master) {
			if (SYSTEM_USING_SLAVE(req)) {
				perms |= IPI_PMU_0_IER_PMU_0_MASK;
			}
		} else {
			if (MASTER_REQUESTED_SLAVE(req)) {
				perms |= req->master->ipiMask;
			}
		}
		req = req->nextMaster;
	}

	return perms;
}

/* Collection of slave nodes */
static PmNode* pmNodeSlaveBucket[] = {
	&pmSlaveL2_g.slv.node,
	&pmSlaveOcm0_g.slv.node,
	&pmSlaveOcm1_g.slv.node,
	&pmSlaveOcm2_g.slv.node,
	&pmSlaveOcm3_g.slv.node,
	&pmSlaveTcm0A_g.sram.slv.node,
	&pmSlaveTcm0B_g.sram.slv.node,
	&pmSlaveTcm1A_g.sram.slv.node,
	&pmSlaveTcm1B_g.sram.slv.node,
	&pmSlaveUsb0_g.slv.node,
	&pmSlaveUsb1_g.slv.node,
	&pmSlaveTtc0_g.node,
	&pmSlaveTtc1_g.node,
	&pmSlaveTtc2_g.node,
	&pmSlaveTtc3_g.node,
	&pmSlaveSata_g.node,
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
	&pmSlaveDdr_g.node,
	&pmSlaveIpiApu_g.node,
	&pmSlaveIpiRpu0_g.node,
	&pmSlaveIpiRpu1_g.node,
	&pmSlaveIpiPl0_g.node,
	&pmSlaveIpiPl1_g.node,
	&pmSlaveIpiPl2_g.node,
	&pmSlaveIpiPl3_g.node,
	&pmSlaveGpu_g.node,
	&pmSlavePcie_g.node,
	&pmSlavePcap_g.node,
	&pmSlaveRtc_g.node,
	&pmSlaveVcu_g.slv.node,
	&pmSlaveExternDevice_g.node,
	&pmSlavePl_g.node,
	&pmSlaveFpdWdt_g.node,
};

PmNodeClass pmNodeClassSlave_g = {
	DEFINE_NODE_BUCKET(pmNodeSlaveBucket),
	.id = NODE_CLASS_SLAVE,
	.clearConfig = PmSlaveClearConfig,
	.construct = NULL,
	.getWakeUpLatency = PmSlaveGetWakeUpLatency,
	.getPowerData = PmNodeGetPowerInfo,
	.forceDown = PmSlaveForceDown,
	.init = PmSlaveInit,
	.isUsable = PmSlaveIsUsable,
	.getPerms = PmSlaveGetPerms,
};

void PmResetSlaveStates(void)
{
	PmSlave* slave;
	u32 i;

	for (i = 0U; i < ARRAY_SIZE(pmNodeSlaveBucket); i++) {
		slave = (PmSlave*)pmNodeSlaveBucket[i]->derived;
		if (XST_SUCCESS != PmSlaveChangeState(slave,
						slave->slvFsm->statesCnt - 1U)) {
			PmWarn("Error in change state for %s\r\n", slave->node.name);
		}
	}
}

#endif
