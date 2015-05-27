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
#include "pm_power.h"

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
	&pmSlaveSata_g.slv,
};

/**
 * PmSlaveHasCapRequests() - Check whether slave has any request for any
 *                           capability
 * @slave   Pointer to a slave whose requests are to be checked
 *
 * @note    Check all master requests and return :
 *          - true if there is at least one master requesting a capability
 *          - false if no master is requesting anything from this slave
 */
bool PmSlaveHasCapRequests(const PmSlave* const slave)
{
	u32 i;
	bool hasReq = false;

	for (i = 0U; i < slave->reqsCnt; i++) {
		if ((0U != (PM_MASTER_USING_SLAVE_MASK & slave->reqs[i]->info)) &&
			(0U != slave->reqs[i]->currReq)) {
			/* Slave is used by this master and has current request for caps */
			hasReq = true;
			break;
		}
	}

	return hasReq;
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
 * PmCheckCapabilities() - Check whether the slave has given capabilities
 * @slave   Slave pointer whose capabilities/states should be checked
 * @cap     Check for these capabilities
 *
 * @return  Status does slave have a state with given capabilities.
 *          - PM_RET_SUCCESS - slave has state with given capabilities
 *          - PM_RET_ERROR_NOTSUPPORTED - slave does not have such state
 */
u32 PmCheckCapabilities(PmSlave* const slave, const u32 cap)
{
	PmStateId i;
	u32 status = PM_RET_ERROR_NOTSUPPORTED;

	for (i = 0; i < slave->slvFsm->statesCnt; i++) {
		/* Find the first state that contains all capabilities */
		if ((cap & slave->slvFsm->states[i]) == cap) {
			return PM_RET_SUCCESS;
		}
	}

	return status;
}

/**
 * PmSlaveChangeState() - Change state of a slave
 * @slave       Slave pointer whose state should be changed
 * @state       New state
 *
 * @return      PM_RET_SUCCESS if transition was performed successfully
 *              error otherwise
 */
u32 PmSlaveChangeState(PmSlave* const slave, const PmStateId state)
{
	u32 t;
	u32 status = PM_RET_ERROR_FAILURE;
	const PmSlaveFsm* fsm = slave->slvFsm;

	for (t = 0U; t < fsm->transCnt; t++) {
		/* Find transition from current state to state to be set */
		if ((fsm->trans[t].fromState != slave->node.currState) ||
			(fsm->trans[t].toState != state)) {
			continue;
		}

		if (NULL != fsm->actions) {
			/* Execute transition action */
			u32 ret = fsm->actions[(slave->instId * fsm->transCnt) + t]();

			/* Check the status of transition handler */
			if (ret == XST_SUCCESS) {
				slave->node.currState = state;
				status = PM_RET_SUCCESS;
			} else {
				status = PM_RET_ERROR_FAILURE;
			}
		} else {
			/*
			 * Slave's FSM has no actions, because it has no private
			 * properties to be controlled here.
			 */
			status = PM_RET_SUCCESS;
		}

		break;
	}
	return status;
}

/**
 * PmUpdateSlave() - Update the slave's state according to the current
 *                   requirements from all masters
 * @slave       Slave whose state is about to be updated
 *
 * @return      Status of operation
 */
u32 PmUpdateSlave(PmSlave* const slave)
{
	PmStateId s;
	const PmSlaveFsm* fsm = slave->slvFsm;
	u32 status = PM_RET_ERROR_NOTSUPPORTED;
	u32 capsToSet = PmGetMaxCapabilities(slave);

	if (capsToSet == slave->slvFsm->states[slave->node.currState]) {
		/* Slave is already in right state */
		status = PM_RET_SUCCESS;
		goto done;
	}

	if (0U != capsToSet) {
		for (s = 0U; s < fsm->statesCnt; s++) {
			/* Find the first state with all required capabilities */
			if ((capsToSet & fsm->states[s]) == capsToSet) {
				status = PmSlaveChangeState(slave, s);
				break;
			}
		}
	} else {
		/*
		 * Set the lowest power state, no capabilities are required. This
		 * check has to exist because some slaves have no state with 0
		 * capabilities. Therefore, they are always placed in first, lowest
		 * power state when their capabilities are not required.
		 */
		status = PmSlaveChangeState(slave, 0U);
	}

done:
	return status;
}

/**
 * PmSlaveWakeMasters() - Called when slave has to wake-up it's masters
 * @slave   Pointer to a slave whose masters has to be woken-up (if master has
 *          requested this slave as wake-up source before going to sleep)
 *
 * @note:   Wake event of this slave is disabled together with all other slaves
 *          as part of the wake-up sequence.
 */
static void PmSlaveWakeMasters(PmSlave* const slave)
{
	PmMasterId i;

	for (i = 0U; i < slave->reqsCnt; i++) {
		if (slave->reqs[i]->info & PM_MASTER_WAKEUP_REQ_MASK) {
			slave->reqs[i]->info &= ~PM_MASTER_WAKEUP_REQ_MASK;
			PmDbg("%s->%s\n", PmStrNode(slave->node.nodeId),
				  PmStrNode(slave->reqs[i]->requestor->procs->node.nodeId));
			PmProcFsm(slave->reqs[i]->requestor->procs, PM_PROC_EVENT_WAKE);
		}
	}

	PmSlaveWakeDisable(slave);
}

/**
 * PmSlaveProcessWake() - Slave has generated wake-up interrupt, find both slave
 *              source and master targets to and trigger wake-up.
 * @wakeMask    Mask read from GPI1 register, based on which slave source that
 *              generated interrupt will be determined. Master targets are
 *              determined based on requirements for slave's wake-up capability.
 *
 * @note        If multiple slaves has simultaneously generated interrupts (wake
 *              events), they will be all processed in this function). For FPD
 *              GIC Proxy this is a must because reading 32-bit status register
 *              clears the interrupt, meaning that there could be up to 31 irqs
 *              that would be lost if not handled immediately.
 */
void PmSlaveProcessWake(const u32 wakeMask)
{
	u32 g;

	if (!(PMU_LOCAL_GPI1_ENABLE_FPD_WAKE_GIC_PROX_MASK & wakeMask)) {
		goto done;
	}

	for (g = 0U; g < ARRAY_SIZE(gicProxyGroups_g); g++) {
		/* Reading status register clears interrupts */
		u32 s;
		u32 irqStatus = XPfw_Read32(gicProxyGroups_g[g].baseAddr +
									FPD_GICP_STATUS_OFFSET);

		for (s = 0U; (0U != irqStatus) && (s < ARRAY_SIZE(pmSlaves)); s++) {
			if ((NULL != pmSlaves[s]->wake) &&
				(pmSlaves[s]->wake->proxyIrqMask & irqStatus)) {
				PmSlaveWakeMasters(pmSlaves[s]);
				irqStatus &= ~pmSlaves[s]->wake->proxyIrqMask;
			}
		}
	}

done:
	return;
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
	PmDbg("%s\n", PmStrNode(slave->node.nodeId));

	if (NULL == slave->wake) {
		goto done;
	}

	/* Enable GIC Proxy IRQ */
	XPfw_Write32(slave->wake->proxyGroup->baseAddr + FPD_GICP_IRQ_ENABLE_OFFSET,
		     slave->wake->proxyIrqMask);
	/* Enable GIC Proxy group */
	XPfw_Write32(FPD_GICP_PMU_IRQ_ENABLE, slave->wake->proxyGroup->pmuIrqBit);

	/* Enable GPI1 FPD GIC Proxy wake event */
	ENABLE_WAKE(PMU_LOCAL_GPI1_ENABLE_FPD_WAKE_GIC_PROX_MASK);

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
		XPfw_Write32(FPD_GICP_PMU_IRQ_DISABLE, slave->wake->proxyGroup->pmuIrqBit);
		if (false == PmWaitingForGicProxyWake()) {
			/* Disable FPD GPI1 wake event */
			DISABLE_WAKE(PMU_LOCAL_GPI1_ENABLE_FPD_WAKE_GIC_PROX_MASK);
		}
	}

done:
	return;
}
