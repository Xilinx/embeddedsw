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
 * Contains system-level PM functions and state
 ********************************************************************/

#include "pm_system.h"
#include "pm_common.h"
#include "crl_apb.h"
#include "pm_callbacks.h"

/*********************************************************************
 * Enum definitions
 ********************************************************************/
typedef enum {
	PM_SYSTEM_STATE_NORMAL,
	PM_SYSTEM_STATE_SHUTDOWN,
	PM_SYSTEM_STATE_RESTART,
} PmSystemState;

/*********************************************************************
 * Structure definitions
 ********************************************************************/
/**
 * PmSystem - System level information
 * @state               Current system state representing system level operation
 *                      in progress (normal, shutting down, restarting)
 * @masters             ORed ipi masks of masters available in the system (to be
 *                      updated based on specific configuration)
 * @permissions         ORed ipi masks of masters which are allowed to request
 *                      system shutdown/restart
 * @shuttingMasters     ORed ipi masks of masters whose next sleep of a
 *                      processor is its shutdown suspend
 * @doneMasters         Masters which are done with suspending or were not even
 *                      involved in the shutdown procedure because they are
 *                      forced to power down.
 */
typedef struct {
	PmSystemState state;
	u32 masters;
	u32 permissions;
	u32 shuttingMasters;
	u32 doneMasters;
} PmSystem;

/*********************************************************************
 * Data initialization
 ********************************************************************/

/* By default all masters are allowed to request shutdown. */
PmSystem pmSystem = {
	.state = PM_SYSTEM_STATE_NORMAL,
	.masters = IPI_PMU_0_IER_APU_MASK |
		   IPI_PMU_0_IER_RPU_0_MASK,
	.permissions = IPI_PMU_0_IER_APU_MASK |
		       IPI_PMU_0_IER_RPU_0_MASK,
	.shuttingMasters = 0U,
	.doneMasters = 0U,
};

/*********************************************************************
 * Function definitions
 ********************************************************************/

/**
 * PmShutdownFinalize() - Shut down or restart entire system, final step
 *
 * @note	Turns off power to the power rails or asserts system reset
 */
static void PmShutdownFinalize(void)
{
	PmDbg("\n");

	if (PM_SYSTEM_STATE_SHUTDOWN == pmSystem.state) {
		/*
		 * Communicate with PMIC to turn off power rails - request for
		 * LPD off must be the last!
		 */
	}

	if (PM_SYSTEM_STATE_RESTART == pmSystem.state) {
		/* assert soft reset */
		XPfw_RMW32(CRL_APB_RESET_CTRL,
			   CRL_APB_RESET_CTRL_SOFT_RESET_MASK,
			   CRL_APB_RESET_CTRL_SOFT_RESET_MASK);
	}
}

/**
 * PmSystemInitShutdownMaster() - Init suspend due to shutdown of a master
 * @mst Master to be initiated to suspend
 */
static void PmSystemInitShutdownMaster(const PmMaster* const mst)
{
	/* Request master to shutdown */
	PmInitSuspendCb(mst, SUSPEND_REASON_SYS_SHUTDOWN, MAX_LATENCY, 0U, 0U);

	/*
	 * If master is suspending it first has to finish its ongoing suspend,
	 * than it will be woken up to perform suspend due to shutdown (it will
	 * then be labelled as shutting down)
	 */
	if (false == PmMasterIsSuspending(mst)) {
		pmSystem.shuttingMasters |= mst->ipiMask;
	}
}

/**
 * PmSystemProcessShutdown() - Process shutdown by initiating suspend requests
 * @master      Master which requested system shutdown
 * @restart     Argument of the system shutdown (shutdown or restart)
 *
 * @return      Status of initiating shutdown which should be returned to the
 *              master which requested shutdown
 */
int PmSystemProcessShutdown(const PmMaster* const master, const u32 restart)
{
	int status = XST_SUCCESS;
	u32 i;

	if (PM_SHUTDOWN == restart) {
		pmSystem.state = PM_SYSTEM_STATE_SHUTDOWN;
	} else {
		pmSystem.state = PM_SYSTEM_STATE_RESTART;
	}

	for (i = 0U; i < ARRAY_SIZE(pmAllMasters); i++) {
		/*
		 * Master requesting shutdown will suspend on its own, no need
		 * to send init suspend callback.
		 */
		if (master == pmAllMasters[i]) {
			pmSystem.shuttingMasters |= master->ipiMask;
			continue;
		}

		/* Check if the master is even possible/used in the system */
		if (0U == (pmAllMasters[i]->ipiMask & pmSystem.masters)) {
			continue;
		}

		if (true == PmMasterIsSuspended(pmAllMasters[i])) {
			/* Wake up master to prepare itself for shutdown */
			status = PmMasterWake(pmAllMasters[i]);
			if (XST_SUCCESS != status) {
				goto done;
			}
		}

		/* If master is killed it is not involved in shutdown */
		if (true == PmMasterIsKilled(pmAllMasters[i])) {
			pmSystem.doneMasters |= pmAllMasters[i]->ipiMask;
			continue;
		}

		PmSystemInitShutdownMaster(pmAllMasters[i]);
	}

done:
	return status;
}

/**
 * PmSystemCaptureSleep() - Called when shutting down to capture a sleep
 * @master      Master whose last awake processor has just went to sleep
 */
void PmSystemCaptureSleep(const PmMaster* const master)
{
	if (PM_SYSTEM_STATE_NORMAL == pmSystem.state) {
		goto done;
	}

	if (0U == (pmSystem.shuttingMasters & master->ipiMask)) {
		/*
		 * This sleep is not for shutdown. This master will suspend
		 * again due to shutdown, mark it as shutting.
		 */
		pmSystem.shuttingMasters |= master->ipiMask;
		goto done;
	}

	pmSystem.doneMasters |= master->ipiMask;

	if (pmSystem.doneMasters == pmSystem.masters) {
		PmShutdownFinalize();
	}

done:
	return;
}

/**
 * PmSystemShutdownProcessing() - Check whether system shutdown is in progress
 */
inline bool PmSystemShutdownProcessing(void)
{
	return (PM_SYSTEM_STATE_SHUTDOWN == pmSystem.state) ||
	       (PM_SYSTEM_STATE_RESTART == pmSystem.state);
}

/**
 * PmSystemRequestNotAllowed() - Check whether the master is allowed to request
 *                               system level transition
 * @master      Pointer to the master whose permissions are to be checked
 */
inline bool PmSystemRequestNotAllowed(const PmMaster* const master)
{
	return 0U == (master->ipiMask & pmSystem.permissions);
}
