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

#include "pm_core.h"
#include "pm_system.h"
#include "pm_common.h"
#include "crl_apb.h"
#include "pm_callbacks.h"
#include "xpfw_resets.h"

/*********************************************************************
 * Structure definitions
 ********************************************************************/
/**
 * PmSystem - System level information
 * @masters             ORed ipi masks of masters available in the system (to be
 *                      updated based on specific configuration)
 * @permissions         ORed ipi masks of masters which are allowed to request
 *                      system shutdown/restart
 */
typedef struct {
	u32 masters;
	u32 permissions;
} PmSystem;

/*********************************************************************
 * Data initialization
 ********************************************************************/

/* By default all masters are allowed to request shutdown. */
PmSystem pmSystem = {
	.masters = IPI_PMU_0_IER_APU_MASK |
		   IPI_PMU_0_IER_RPU_0_MASK,
	.permissions = IPI_PMU_0_IER_APU_MASK |
		       IPI_PMU_0_IER_RPU_0_MASK,
};

/*********************************************************************
 * Function definitions
 ********************************************************************/
/**
 * PmSystemProcessShutdown() - Process shutdown by initiating suspend requests
 * @master      Master which requested system shutdown
 * @type        Shutdown type
 * @subtype     Shutdown subtype
 */
void PmSystemProcessShutdown(const PmMaster *master, u32 type, u32 subtype)
{
	PmDbg("\r\n");

	if (PMF_SHUTDOWN_TYPE_SHUTDOWN == type) {
		u32 dummy;

		if (PMF_SHUTDOWN_SUBTYPE_SYSTEM == subtype) {
			PmForcePowerDownInt(NODE_PL, &dummy);
		}

		PmForcePowerDownInt(NODE_FPD, &dummy);
		PmForcePowerDownInt(NODE_LPD, &dummy);
	}

	if (PMF_SHUTDOWN_TYPE_RESET == type) {
		if (PMF_SHUTDOWN_SUBTYPE_SYSTEM == subtype) {
			/* assert soft reset */
			XPfw_RMW32(CRL_APB_RESET_CTRL,
				   CRL_APB_RESET_CTRL_SOFT_RESET_MASK,
				   CRL_APB_RESET_CTRL_SOFT_RESET_MASK);
		}

		if (PMF_SHUTDOWN_SUBTYPE_PS_ONLY == subtype) {
			XPfw_ResetPsOnly();
		}
	}

	while (true) {
		mb_sleep();
	}
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
