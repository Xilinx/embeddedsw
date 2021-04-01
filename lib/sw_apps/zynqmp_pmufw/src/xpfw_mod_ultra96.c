/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpfw_default.h"
#include "xpfw_config.h"
#include "xpfw_core.h"
#include "xpfw_module.h"

#include "xpfw_mod_ultra96.h"

#ifdef ENABLE_MOD_ULTRA96

#ifndef ENABLE_SCHEDULER
#error "ERROR: Ultra96 module requires scheduler to be enabled! Define ENABLE_SCHEDULER"
#endif


#ifndef ENABLE_PM
#error "ERROR: Ultra96 module requires PM module to be enabled! Define ENABLE_PM"
#endif

#ifdef ULTRA96_VERSION
	#if ULTRA96_VERSION == 1
		#define PWR_BTN_POLL_MASK 0U
	#elif ULTRA96_VERSION == 2
		#define PWR_BTN_POLL_MASK PMU_IOMODULE_GPI1_MIO_WAKE_0_MASK
	#else
		#error "Unsupported ULTRA96_VERSION specified"
	#endif
#else
	#define PWR_BTN_POLL_MASK 0U
#endif


#include "pm_core.h"
#include "pm_master.h"
#include "pm_callbacks.h"
#include "pm_config.h"
#include "rpu.h"

/*
 * Define the interval at which the power button input is polled
 * This period should be a minimum of 10ms and multiples thereof, as per the scheduler configuration
 */
#define ULTRA96_PWR_BTN_POLL_PERIOD_MS 10U

const XPfw_Module_t *Ultra96ModPtr;

static void Ultra96PowerButtonHandler(void)
{
	/*
	 * Don't check for the pin state if PM config is not yet loaded
	 * This also means that FSBL is not yet running, MIO is not configured and we don't have the IPI info
	 */
	if(!PmConfigObjectIsLoaded()) {
		return;
	}
	/* Check if Power Button pin is Active*/
	if((XPfw_Read32(PMU_IOMODULE_GPI1) & PMU_IOMODULE_GPI1_MIO_WAKE_0_MASK) == PWR_BTN_POLL_MASK) {

		/* Do a second check on the pin to mitigate sub-microsecond glitches, if any */
		if ((XPfw_Read32(PMU_IOMODULE_GPI1) & PMU_IOMODULE_GPI1_MIO_WAKE_0_MASK) != PWR_BTN_POLL_MASK) {
			/* If the pin is still not active, don't initiate a power down. Just return. */
			return;
		}

		/* Start Board Power Down Sequence. First Remove the pin polling task from scheduler
		 * to prevent re-triggering when shutdown is in progress
		 */
		XPfw_CoreRemoveTask(Ultra96ModPtr, ULTRA96_PWR_BTN_POLL_PERIOD_MS, Ultra96PowerButtonHandler);

	#ifdef ENABLE_DIRECT_POWEROFF_ULTRA96
		/* Turn-Off the board directly by toggling MIO */
		PmKillBoardPower();
	#else
		/* Initiate Shutdown for all masters in the system */
		/* TODO: All the PM related calls below should be wrapped into a single API
		 *       like PmIntiateSystemShutdown in PM Module and this module needs to call it.
		 */
		u32 rpu_mode = XPfw_Read32(RPU_RPU_GLBL_CNTL);
		/* APU */
		if (PM_MASTER_STATE_ACTIVE == PmMasterIsActive(&pmMasterApu_g)) {
			PmInitSuspendCb(&pmMasterApu_g,
					SUSPEND_REASON_SYS_SHUTDOWN, 1, 0, 0);
		}
		/* RPU Split Mode */
		if (0U == (rpu_mode & RPU_RPU_GLBL_CNTL_SLSPLIT_MASK)) {
			if (PM_MASTER_STATE_ACTIVE == PmMasterIsActive(&pmMasterRpu0_g)) {
				PmInitSuspendCb(&pmMasterRpu0_g,
						SUSPEND_REASON_SYS_SHUTDOWN, 1, 0, 0);
			}
			if (PM_MASTER_STATE_ACTIVE == PmMasterIsActive(&pmMasterRpu1_g)) {
				PmInitSuspendCb(&pmMasterRpu1_g,
						SUSPEND_REASON_SYS_SHUTDOWN, 1, 0, 0);
			}
		} else {
		/* RPU Lockstep Mode */
			if (PM_MASTER_STATE_ACTIVE == PmMasterIsActive(&pmMasterRpu_g)) {
				PmInitSuspendCb(&pmMasterRpu_g,
						SUSPEND_REASON_SYS_SHUTDOWN, 1, 0, 0);
			}
		}
	#endif /* ENABLE_DIRECT_POWEROFF_ULTRA96 */
	}
}


static void Ultra96CfgInit(const XPfw_Module_t *ModPtr, const u32 *CfgData, u32 Len)
{
	s32 Status;

	Status = XPfw_CoreScheduleTask(ModPtr, ULTRA96_PWR_BTN_POLL_PERIOD_MS, Ultra96PowerButtonHandler);
	if (XST_FAILURE == Status) {
		XPfw_Printf(DEBUG_ERROR,"Ultra96 (MOD-%d):Scheduling MIO Poll task failed.",
				ModPtr->ModId);
	}
}


void ModUltra96Init(void)
{
	Ultra96ModPtr = XPfw_CoreCreateMod();
	if (XST_SUCCESS != XPfw_CoreSetCfgHandler(Ultra96ModPtr,Ultra96CfgInit)) {
		XPfw_Printf(DEBUG_DETAILED,"Ultra96: Set Cfg handler failed\r\n");
	}
}
#else /* ENABLE_MOD_ULTRA96 */
void ModUltra96Init(void) { }
#endif /* ENABLE_MOD_ULTRA96 */
