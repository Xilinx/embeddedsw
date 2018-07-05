/*
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */

#include "xpfw_config.h"
#ifdef ENABLE_PM

/*********************************************************************
 * Definitions of PM slave USB structures and state transitions.
 *********************************************************************/

#include "pm_usb.h"
#include "pm_common.h"
#include "pm_master.h"
#include "pm_reset.h"
#include "xpfw_rom_interface.h"
#include "lpd_slcr.h"

/* Power states of USB */
#define PM_USB_STATE_UNUSED	0U
#define PM_USB_STATE_OFF	1U
#define PM_USB_STATE_ON		2U
#define PM_USB_MAX_STATE	3U /* This macro show that the USB support
				      how many types of power state, here  it's 3U */

/* Power consumptions for USB defined by its states */
#define DEFAULT_USB_POWER_ON	100U
#define DEFAULT_USB_POWER_OFF	0U

/* USB state transition latency values */
#define PM_USB_UNUSED_TO_ON_LATENCY	152U
#define PM_USB_ON_TO_UNUSED_LATENCY	3U
#define PM_USB_ON_TO_OFF_LATENCY	3U
#define PM_USB_OFF_TO_ON_LATENCY	152U

/* USB states */
static const u8 pmUsbStates[PM_USB_MAX_STATE] = {
	[PM_USB_STATE_UNUSED] = 0U,
	[PM_USB_STATE_OFF] = PM_CAP_WAKEUP | PM_CAP_POWER,
	[PM_USB_STATE_ON] = PM_CAP_WAKEUP | PM_CAP_ACCESS | PM_CAP_CONTEXT |
				PM_CAP_CLOCK | PM_CAP_POWER,
};

/* USB transition table (from which to which state USB can transit) */
static const PmStateTran pmUsbTransitions[] = {
	{
		.fromState = PM_USB_STATE_OFF,
		.toState = PM_USB_STATE_ON,
		.latency = PM_USB_OFF_TO_ON_LATENCY,
	}, {
		.fromState = PM_USB_STATE_UNUSED,
		.toState = PM_USB_STATE_ON,
		.latency = PM_USB_UNUSED_TO_ON_LATENCY,
	}, {
		.fromState = PM_USB_STATE_ON,
		.toState = PM_USB_STATE_OFF,
		.latency = PM_USB_ON_TO_OFF_LATENCY,
	}, {
		.fromState = PM_USB_STATE_UNUSED,
		.toState = PM_USB_STATE_OFF,
		.latency = 0U,
	}, {
		.fromState = PM_USB_STATE_ON,
		.toState = PM_USB_STATE_UNUSED,
		.latency = PM_USB_ON_TO_UNUSED_LATENCY,

	}, {
		.fromState = PM_USB_STATE_OFF,
		.toState = PM_USB_STATE_UNUSED,
		.latency = 0U,
	},
};

/**
 * PmUsbFsmHandler() - Usb FSM handler, performs transition actions
 * @slave       Slave whose state should be changed
 * @nextState   State the slave should enter
 *
 * @return      Status of performing transition action
 */
static s32 PmUsbFsmHandler(PmSlave* const slave, const PmStateId nextState)
{
	s32 status;
	PmSlaveUsb* usb = (PmSlaveUsb*)slave->node.derived;

	switch (slave->node.currState) {
	case PM_USB_STATE_ON:
		if ((PM_USB_STATE_OFF == nextState) ||
		    (PM_USB_STATE_UNUSED == nextState)) {
			/* ON -> OFF*/
			XPfw_AibEnable(usb->aibId);
			status = (s32)usb->PwrDn();
		} else {
			status = XST_NO_FEATURE;
		}
		break;
	case PM_USB_STATE_OFF:
	case PM_USB_STATE_UNUSED:
		if (PM_USB_STATE_ON == nextState) {
			/* OFF -> ON */
			status = (s32)usb->PwrUp();
			XPfw_AibDisable(usb->aibId);
			if (XST_SUCCESS == status) {
				status = PmResetAssertInt(usb->rstId,
						PM_RESET_ACTION_PULSE);
			}
		} else {
			status = XST_NO_FEATURE;
		}
		break;
	default:
		status = XST_PM_INTERNAL;
		PmNodeLogUnknownState(&slave->node, slave->node.currState);
		break;
	}

	return status;
}

/* USB FSM */
static const PmSlaveFsm slaveUsbFsm = {
	.states = pmUsbStates,
	.enterState = PmUsbFsmHandler,
	.trans = pmUsbTransitions,
	.statesCnt = ARRAY_SIZE(pmUsbStates),
	.transCnt = ARRAY_SIZE(pmUsbTransitions),

};

static PmWakeEventGicProxy pmUsb0Wake = {
	.wake = {
		.derived = &pmUsb0Wake,
		.class = &pmWakeEventClassGicProxy_g,
	},
	.mask = LPD_SLCR_GICP2_IRQ_MASK_SRC11_MASK |
		LPD_SLCR_GICP2_IRQ_MASK_SRC5_MASK |
		LPD_SLCR_GICP2_IRQ_MASK_SRC4_MASK |
		LPD_SLCR_GICP2_IRQ_MASK_SRC3_MASK |
		LPD_SLCR_GICP2_IRQ_MASK_SRC2_MASK |
		LPD_SLCR_GICP2_IRQ_MASK_SRC1_MASK,
	.group = 2U,
};

static u8 PmUsbPowers[PM_USB_MAX_STATE] = {
	[PM_USB_STATE_UNUSED] = DEFAULT_USB_POWER_OFF,
	[PM_USB_STATE_OFF] = DEFAULT_USB_POWER_OFF,
	[PM_USB_STATE_ON] = DEFAULT_USB_POWER_ON,
};

PmSlaveUsb pmSlaveUsb0_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveUsb0_g,
			.class = &pmNodeClassSlave_g,
			.parent = &pmPowerDomainLpd_g.power,
			.clocks = NULL,
			DEFINE_PM_POWER_INFO(PmUsbPowers),
			.latencyMarg = MAX_LATENCY,
			DEFINE_NODE_NAME("usb0"),
			.nodeId = NODE_USB_0,
			.currState = PM_USB_STATE_ON,
			.flags = 0U,
		},
		.class = NULL,
		.reqs = NULL,
		.wake = &pmUsb0Wake.wake,
		.slvFsm = &slaveUsbFsm,
		.flags = 0U,
	},
	.PwrDn = XpbrPwrDnUsb0Handler,
	.PwrUp = XpbrPwrUpUsb0Handler,
	.rstId = PM_RESET_USB0_CORERESET,
	.aibId = XPFW_AIB_LPD_TO_USB0,
};

static PmWakeEventGicProxy pmUsb1Wake = {
	.wake = {
		.derived = &pmUsb1Wake,
		.class = &pmWakeEventClassGicProxy_g,
	},
	.mask = LPD_SLCR_GICP2_IRQ_MASK_SRC12_MASK |
		LPD_SLCR_GICP2_IRQ_MASK_SRC10_MASK |
		LPD_SLCR_GICP2_IRQ_MASK_SRC9_MASK |
		LPD_SLCR_GICP2_IRQ_MASK_SRC8_MASK |
		LPD_SLCR_GICP2_IRQ_MASK_SRC7_MASK |
		LPD_SLCR_GICP2_IRQ_MASK_SRC6_MASK,
	.group = 2U,
};

PmSlaveUsb pmSlaveUsb1_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveUsb1_g,
			.class = &pmNodeClassSlave_g,
			.parent = &pmPowerDomainLpd_g.power,
			.clocks = NULL,
			DEFINE_PM_POWER_INFO(PmUsbPowers),
			.latencyMarg = MAX_LATENCY,
			DEFINE_NODE_NAME("usb1"),
			.nodeId = NODE_USB_1,
			.currState = PM_USB_STATE_ON,
			.flags = 0U,
		},
		.class = NULL,
		.reqs = NULL,
		.wake = &pmUsb1Wake.wake,
		.slvFsm = &slaveUsbFsm,
		.flags = 0U,
	},
	.PwrDn = XpbrPwrDnUsb1Handler,
	.PwrUp = XpbrPwrUpUsb1Handler,
	.rstId = PM_RESET_USB1_CORERESET,
	.aibId = XPFW_AIB_LPD_TO_USB1,
};

#endif
