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
#include "xpfw_aib.h"

/* Power states of USB */
#define PM_USB_STATE_UNUSED	0U
#define PM_USB_STATE_OFF	1U
#define PM_USB_STATE_ON		2U

/* Power consumptions for USB defined by its states */
#define DEFAULT_USB_POWER_ON	100U
#define DEFAULT_USB_POWER_OFF	0U

/* USB state transition latency values */
#define PM_USB_UNUSED_TO_ON_LATENCY	152
#define PM_USB_ON_TO_UNUSED_LATENCY	3
#define PM_USB_ON_TO_OFF_LATENCY	3
#define PM_USB_OFF_TO_ON_LATENCY	152

/* USB states */
static const u32 pmUsbStates[] = {
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
static int PmUsbFsmHandler(PmSlave* const slave, const PmStateId nextState)
{
	int status;
	PmSlaveUsb* usb = (PmSlaveUsb*)slave->node.derived;

	switch (slave->node.currState) {
	case PM_USB_STATE_ON:
		if ((PM_USB_STATE_OFF == nextState) ||
		    (PM_USB_STATE_UNUSED == nextState)) {
			/* ON -> OFF*/
			status = usb->PwrDn();
		} else {
			status = XST_NO_FEATURE;
		}
		break;
	case PM_USB_STATE_OFF:
	case PM_USB_STATE_UNUSED:
		if (PM_USB_STATE_ON == nextState) {
			/* OFF -> ON */
			status = usb->PwrUp();
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
		PmDbg(DEBUG_DETAILED,"ERROR: Unknown USB state #%d\r\n",
				slave->node.currState);
		break;
	}

	return status;
}

/* USB FSM */
static const PmSlaveFsm slaveUsbFsm = {
	.states = pmUsbStates,
	.statesCnt = ARRAY_SIZE(pmUsbStates),
	.trans = pmUsbTransitions,
	.transCnt = ARRAY_SIZE(pmUsbTransitions),
	.enterState = PmUsbFsmHandler,
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

static u32 PmUsbPowers[] = {
	[PM_USB_STATE_UNUSED] = DEFAULT_USB_POWER_OFF,
	[PM_USB_STATE_OFF] = DEFAULT_USB_POWER_OFF,
	[PM_USB_STATE_ON] = DEFAULT_USB_POWER_ON,
};

/*
 * PmPowerUpUsb0() - USB0 power up handler, powers up USB0 power island
 * 					 and disables AIBs
 * @return		   - Return value of PMU-ROM handler
 */

static u32 PmPowerUpUsb0(void)
{
	u32 status;

	status = XpbrPwrUpUsb0Handler();

	if (XST_SUCCESS != status) {
		goto done;
	}
	XPfw_AibDisable(XPFW_AIB_LPD_TO_USB0);

done:
	return status;
}

/**
 * PmPowerDownUsb0() - USB0 power down handler, powers down USB0 power island
 * 					   and enables AIBs at USB0 interfaces to respond
 * 					   gracefully to the incoming AXI transactions to
 * 					   powered down USB0 power island.
 * @return			 - Return value of PMU-ROM handler
 */
static u32 PmPowerDownUsb0(void)
{
	if (XPfw_AibEnable(XPFW_AIB_LPD_TO_USB0) != XST_SUCCESS) {
		PmDbg(DEBUG_DETAILED,
				"Warning: Failed to Enable AIB isolation LPD to USB0\r\n");
	} else {
		/* Check if AIB isolation is enabled */
		if (XPfw_AibPollForAck(XPFW_AIB_LPD_TO_USB0, AIB_ACK_TIMEOUT)
				!= XST_SUCCESS) {
			PmDbg(DEBUG_DETAILED, "Warning: Failed to receive acknowledgment "
					"for LPD to USB0 isolation");
		}
	}

	return XpbrPwrDnUsb0Handler();
}

PmSlaveUsb pmSlaveUsb0_g = {
	.slv = {
		.node = {
			.nodeId = NODE_USB_0,
			.class = &pmNodeClassSlave_g,
			.currState = PM_USB_STATE_ON,
			.parent = &pmPowerDomainLpd_g.power,
			.clocks = NULL,
			.derived = &pmSlaveUsb0_g,
			.latencyMarg = MAX_LATENCY,
			.flags = 0U,
			DEFINE_PM_POWER_INFO(PmUsbPowers),
		},
		.class = NULL,
		.reqs = NULL,
		.wake = &pmUsb0Wake.wake,
		.slvFsm = &slaveUsbFsm,
		.flags = 0U,
	},
	.PwrDn = PmPowerDownUsb0,
	.PwrUp = PmPowerUpUsb0,
	.rstId = PM_RESET_USB0_CORERESET,
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

/*
 * PmPowerUpUsb1() - USB1 power up handler, powers up USB1 power island
 * 					 and disables AIBs
 * @return		   - Return value of PMU-ROM handler
 */

static u32 PmPowerUpUsb1(void)
{
	u32 status;

	status = XpbrPwrUpUsb1Handler();

	if (XST_SUCCESS != status) {
		goto done;
	}
	XPfw_AibDisable(XPFW_AIB_LPD_TO_USB1);

done:
	return status;
}

/**
 * PmPowerDownUsb1() - USB1 power down handler, powers down USB1 power island
 * 					   and enables AIBs at USB1 interfaces to respond
 * 					   gracefully to the incoming AXI transactions to
 * 					   powered down USB1 power island.
 * @return			 - Return value of PMU-ROM handler
 */
static u32 PmPowerDownUsb1(void)
{
	if (XPfw_AibEnable(XPFW_AIB_LPD_TO_USB1) != XST_SUCCESS) {
		PmDbg(DEBUG_DETAILED,
				"Warning: Failed to Enable AIB isolation LPD to USB1\r\n");
	} else {
		/* Check if AIB isolation is enabled */
		if (XPfw_AibPollForAck(XPFW_AIB_LPD_TO_USB1, AIB_ACK_TIMEOUT)
				!= XST_SUCCESS) {
			PmDbg(DEBUG_DETAILED, "Warning: Failed to receive acknowledgment "
					"for LPD to USB1 isolation");
		}
	}

	return XpbrPwrDnUsb1Handler();
}

PmSlaveUsb pmSlaveUsb1_g = {
	.slv = {
		.node = {
			.nodeId = NODE_USB_1,
			.class = &pmNodeClassSlave_g,
			.currState = PM_USB_STATE_ON,
			.parent = &pmPowerDomainLpd_g.power,
			.clocks = NULL,
			.derived = &pmSlaveUsb1_g,
			.latencyMarg = MAX_LATENCY,
			.flags = 0U,
			DEFINE_PM_POWER_INFO(PmUsbPowers),
		},
		.class = NULL,
		.reqs = NULL,
		.wake = &pmUsb1Wake.wake,
		.slvFsm = &slaveUsbFsm,
		.flags = 0U,
	},
	.PwrDn = PmPowerDownUsb1,
	.PwrUp = PmPowerUpUsb1,
	.rstId = PM_RESET_USB1_CORERESET,
};

#endif
