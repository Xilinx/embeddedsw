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
#include "xpfw_config.h"
#ifdef ENABLE_PM

#include "pm_requirement.h"
#include "pm_master.h"
#include "pm_slave.h"
#include "pm_sram.h"
#include "pm_usb.h"
#include "pm_pll.h"
#include "pm_periph.h"
#include "pm_ddr.h"
#include "pm_clock.h"

/*
 * The array below is used as the heap, because dynamic memory allocation is
 * not allowed by MISRA. When a requirement structure needs to be allocated
 * we get the first empty structure from the array.
 */
static PmRequirement pmReqData[PM_REQUIREMENT_MAX];

/* Top of the heap = index of the first free entry in pmReqData array (heap) */
static u32 pmReqTop;

/**
 * PmRequirementLink() - Link requirement struct into master's and slave's lists
 * @req	Pointer to the requirement structure to be linked in lists
 */
static void PmRequirementLink(PmRequirement* const req)
{
	if (NULL != req->master) {
		/* The req structure is becoming master's head of requirements list */
		req->nextSlave = req->master->reqs;
		req->master->reqs = req;
	}

	/* The req is becoming the head of slave's requirements list as well */
	req->nextMaster = req->slave->reqs;
	req->slave->reqs = req;
}

/**
 * PmRequirementMalloc() - Allocate a PmRequirement structure
 *
 * @return      Pointer to the allocated structure or NULL if there is no free
 *              memory
 */
static PmRequirement* PmRequirementMalloc(void)
{
	PmRequirement* newReq = NULL;

	if (pmReqTop < ARRAY_SIZE(pmReqData)) {
		newReq = &pmReqData[pmReqTop];
		pmReqTop++;
	} else {
		PmAlert("out of memory!\r\n");
	}

	return newReq;
}

/**
 * PmRequirementFreeAll() - Clear all data on requirements heap
 */
void PmRequirementFreeAll(void)
{
	/* Clear the used content of pmReqData */
	(void)memset(pmReqData, 0U, pmReqTop * sizeof(PmRequirement));

	/* Reset top of the heap */
	pmReqTop = 0U;
}

/**
 * PmRequirementAdd() - Add a requirement structure for master/slave pair
 * @master      Master that can use the slave
 * @slave       Slave that can be used by the master
 *
 * @return      Pointer to the requirement if added, NULL if there is no
 *              free memory to add new requirement
 */
PmRequirement* PmRequirementAdd(PmMaster* const master, PmSlave* const slave)
{
	PmRequirement* req = PmRequirementMalloc();

	if (NULL == req) {
		goto done;
	}

	req->master = master;
	req->slave = slave;
	PmRequirementLink(req);

done:
	return req;
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
 * PmRequirementRequest() - Process request for new requirements
 * @req		Requirements determining master/slave pair
 * @caps	Requested capabilities
 *
 * @return	Status of processing the request
 */
int PmRequirementRequest(PmRequirement* const req, const u32 caps)
{
	int status;

	req->info |= PM_MASTER_REQUESTED_SLAVE_MASK;
	status = PmRequirementUpdate(req, caps);

	return status;
}

/**
 * PmRequirementRelease() - Process release of requirements
 * @first	Pointer to the requirement structure
 * @scope	Scope of the release
 *
 * @return	Status of processing the release
 */
int PmRequirementRelease(PmRequirement* const first, const PmReleaseScope scope)
{
	int status = XST_FAILURE;
	PmRequirement* req = first;

	if (RELEASE_ONE == scope) {
		PmRequirementClear(req);
		status = PmUpdateSlave(req->slave);
		goto done;
	}

	while (NULL != req) {
		if ((RELEASE_ALL == scope) ||
		   ((RELEASE_UNREQUESTED == scope) && !MASTER_REQUESTED_SLAVE(req))) {
			PmRequirementClear(req);
			status = PmUpdateSlave(req->slave);
			if (XST_SUCCESS != status) {
				break;
			}
		}
		req = req->nextSlave;
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
				PmErr("updating %s\r\n", req->slave->node.name);
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
			req->nextReq = req->currReq;
		}
		req = req->nextSlave;
	}
}

/**
 * PmRequirementPreRequest() - Request requirements for master which it will not
 *			request for itself
 * @master	Master whose requirements are requested
 *
 * When waking up from forced power down, master may have some requirements to
 * be configured before it enters active state (example TCM for RPU). Loop
 * through all slaves, find such requirements and update next requirements data
 * in master/slave requirement structure that will be configured.
 */
void PmRequirementPreRequest(const PmMaster* const master)
{
	PmRequirement* req = master->reqs;

	while (NULL != req) {
		if (0U != req->preReq) {
			/* Set flag to state that master is using slave */
			req->info |= PM_MASTER_REQUESTED_SLAVE_MASK;
			req->nextReq = req->preReq;
		}
		req = req->nextSlave;
	}
}

/**
 * PmRequirementClockRestore() - Restore clock configuration for the master's
 *				preallocated requirements
 * @master	Master for whom clock restoration is done
 *
 * When waking up from forced power down, clocks for the preallocated
 * requirements must be restored. Loop through all slaves, find such
 * requirements and restore their clock configuration.
 */
void PmRequirementClockRestore(const PmMaster* const master)
{
	PmRequirement* req = master->reqs;

	while (NULL != req) {
		if (0U != req->preReq) {
			PmClockRestore(&req->slave->node);
		}
		req = req->nextSlave;
	}
}

/**
 * PmRequirementClear() - Clear requirements
 * @req         Requirements to clear
 *
 * @note        The function marks slave as unused
 */
void PmRequirementClear(PmRequirement* const req)
{
	/* Clear flag - master is not using slave anymore */
	req->info &= ~PM_MASTER_REQUESTED_SLAVE_MASK;

	/* Release current and next requirements */
	req->currReq = 0U;
	req->nextReq = 0U;
}

/**
 * PmRequirementGet() - Get requirement for master/slave pair
 * @master  Master whose request structure should be found
 * @slave   Slave in question
 *
 * @return  Pointer to the requirement associated with the master/slave pair.
 *          NULL if such structure is not found.
 */
PmRequirement* PmRequirementGet(const PmMaster* const master,
				const PmSlave* const slave)
{
	PmRequirement* req = master->reqs;

	while (NULL != req) {
		if (slave == req->slave) {
			break;
		}
		req = req->nextSlave;
	}

	return req;
}

/**
 * PmRequirementGetNoMaster() - Get system requirement for a slave
 * @slave	Slave in question
 *
 * @return	Pointer to the requirement or NULL if not found.
 */
PmRequirement* PmRequirementGetNoMaster(const PmSlave* const slave)
{
	PmRequirement* req = slave->reqs;

	while (NULL != req) {
		if (NULL == req->master) {
			break;
		}
		req = req->nextMaster;
	}

	return req;
}

/**
 * PmRequirementSetConfig() - Set requirement configuration
 * @req         Requirement structure to configure
 * @flags       Flags to configure (is the slave currently used by the master)
 * @currReq     Current requirements of the master
 * @defaultReq  Default requirement of the master
 *
 * @return      XST_SUCCESS if requirements are configured properly,
 *              XST_FAILURE otherwise
 */
int PmRequirementSetConfig(PmRequirement* const req, const u32 flags,
			   const u32 currReq, const u32 defaultReq)
{
	int status;

	status = PmCheckCapabilities(req->slave, currReq);
	if (XST_SUCCESS != status) {
		status = XST_FAILURE;
		goto error;
	}

	status = PmCheckCapabilities(req->slave, defaultReq);
	if (XST_SUCCESS != status) {
		status = XST_FAILURE;
		goto error;
	}

	if (0U != (PM_MASTER_REQUESTED_SLAVE_MASK & flags)) {
		req->info |= PM_MASTER_REQUESTED_SLAVE_MASK;
		req->currReq = currReq;
		req->nextReq = currReq;
		PmClockSave(&req->slave->node);
	}
	req->preReq = currReq;
	req->defaultReq = defaultReq;
	req->latencyReq = MAX_LATENCY;
	goto done;

error:
	PmErr("%s has no state with caps 0x%lx\r\n",
	      req->slave->node.name, currReq);

done:
	return status;
}

#endif
