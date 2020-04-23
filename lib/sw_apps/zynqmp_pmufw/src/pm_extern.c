/*
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */

#include "xpfw_config.h"
#ifdef ENABLE_PM

#include "pm_extern.h"
#include "pm_slave.h"
#include "pm_master.h"

/**
 * PmWakeEventExtern - External wake event, derived from PmWakeEvent
 * @wake	Basic PmWakeEvent structure
 * @set		ORed IPI masks of masters that have set wake enabled
 * @enabled	ORed IPI masks of masters that are waiting to be woken up
 */
typedef struct PmWakeEventExtern {
	PmWakeEvent wake;
	u32 set;
	u32 enabled;
} PmWakeEventExtern;

/**
 * PmWakeEventExternSet() - Set external wake event as the wake source
 * @wake	Wake event
 * @ipiMask	IPI mask of the master which sets the wake source
 * @enable	Flag: for enable non-zero value, for disable value zero
 */
static void PmWakeEventExternSet(PmWakeEvent* const wake, const u32 ipiMask,
				 const u32 enable)
{
	PmWakeEventExtern* ext = (PmWakeEventExtern*)wake->derived;

	if (0U != enable) {
		ext->set |= ipiMask;
	} else {
		ext->set &= ~ipiMask;
	}
}

/**
 * PmWakeEventExternEnable() - Enable external wake event for a master
 * @ext		External wake event
 * @ipiMask	IPI mask of the master which enables the wake source
 */
static void PmWakeEventExternEnable(PmWakeEventExtern* const ext,
				    const u32 ipiMask)
{
	/* If the propagation of wake event is already enabled we're done */
	if (0U != ext->enabled) {
		goto done;
	}

	ENABLE_WAKE(PMU_IOMODULE_GPI1_MIO_WAKE_0_MASK);

done:
	ext->enabled |= ipiMask;
	return;
}

/**
 * PmWakeEventExternDisable() - Disable the propagation of external wake event
 * @ext		External wake event
 * @ipiMask	IPI mask of the master which disables the wake source
 */
static void PmWakeEventExternDisable(PmWakeEventExtern* const ext,
				     const u32 ipiMask)
{
	ext->set &= ~ipiMask;

	/* If the propagation of wake event is not enabled we're done */
	if (0U == ext->enabled) {
		goto done;
	}
	ext->enabled &= ~ipiMask;

	/* If there are still masters waiting for this wake we're done */
	if (0U != ext->enabled) {
		goto done;
	}

	DISABLE_WAKE(PMU_IOMODULE_GPI1_MIO_WAKE_0_MASK);

done:
	return;
}

/**
 * PmWakeEventExternConfig() - Configure propagation of external wake event
 * @wake	Wake event
 * @ipiMask	IPI mask of the master which configures the wake
 * @enable	Flag: for enable non-zero value, for disable value zero
 */
static void PmWakeEventExternConfig(PmWakeEvent* const wake, const u32 ipiMask,
				    const u32 enable)
{
	PmWakeEventExtern* ext = (PmWakeEventExtern*)wake->derived;

	/* If wake event was not set by the master there is nothing to do */
	if (0U == (ipiMask & ext->set)) {
		goto done;
	}

	if (0U != enable) {
		PmWakeEventExternEnable(ext, ipiMask);
	} else {
		PmWakeEventExternDisable(ext, ipiMask);
	}

done:
	return;
}

static PmWakeEventClass pmWakeEventClassExtern = {
	.set = PmWakeEventExternSet,
	.config = PmWakeEventExternConfig,
};

static PmWakeEventExtern pmExternWake = {
	.wake = {
		.derived = &pmExternWake,
		.class = &pmWakeEventClassExtern,
	},
	.set = 0U,
	.enabled = 0U,
};

static const u8 pmExternDeviceFsmStates[] = {
	PM_CAP_WAKEUP,
};

static const PmSlaveFsm pmExternDeviceFsm = {
	DEFINE_SLAVE_STATES(pmExternDeviceFsmStates),
	.trans = NULL,
	.transCnt = 0U,
	.enterState = NULL,
};

PmSlave pmSlaveExternDevice_g = {
	.node = {
		.derived = &pmSlaveExternDevice_g,
		.nodeId = NODE_EXTERN,
		.class = &pmNodeClassSlave_g,
		.parent = NULL,
		.clocks = NULL,
		.currState = 0U,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		.powerInfo = NULL,
		.powerInfoCnt = 0U,
		DEFINE_NODE_NAME("extern"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = &pmExternWake.wake,
	.slvFsm = &pmExternDeviceFsm,
	.flags = 0U,
};

/**
 * PmExternWakeMasters() - Wake masters that enabled external wake source
 * @return	Status of performing wake-up
 */
s32 PmExternWakeMasters(void)
{
	u32 masters = pmExternWake.enabled;
	s32 totalStatus = XST_SUCCESS;
	s32 status;

	while (0U != masters) {
		PmMaster* master;

		master = PmMasterGetNextFromIpiMask(&masters);
		if (NULL == master) {
			totalStatus = XST_FAILURE;
			continue;
		}

		status = PmMasterWake(master);
		if (XST_SUCCESS != status) {
			totalStatus = XST_FAILURE;
			continue;
		}
	}

	return totalStatus;
}

#endif
