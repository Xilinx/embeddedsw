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
 * All functions, data and definitions needed for
 * managing PM slaves' states.
 *********************************************************************/

#ifndef PM_SLAVE_H_
#define PM_SLAVE_H_

#include "pm_defs.h"
#include "pm_common.h"
#include "pm_node.h"
#include "pm_gic_proxy.h"

/* Forward declarations */
typedef struct PmMaster PmMaster;
typedef struct PmRequirement PmRequirement;
typedef struct PmSlave PmSlave;
typedef struct PmSlaveUsb PmSlaveUsb;

typedef int (*const PmSlaveFsmHandler)(PmSlave* const slave,
					   const PmStateId nextState);

/*********************************************************************
 * Macros
 ********************************************************************/

/* Mask definitions for slave's flags */
#define PM_SLAVE_FLAG_IS_SHAREABLE	0x1U

/*********************************************************************
 * Structure definitions
 ********************************************************************/
/**
 * PmStateTran - Transition for a state in finite state machine
 * @latency     Transition latency in microseconds
 * @fromState   From which state the transition is taken
 * @toState     To which state the transition is taken
 */
typedef struct {
	const u32 latency;
	PmStateId fromState;
	PmStateId toState;
} PmStateTran;

/**
 * PmSlaveFsm - Finite state machine data for slaves
 * @state       Pointer to states array. Index in array is a state id, elements
 *              of array are power values in that state. For power island values
 *              are 0 and 1, for power domains values are in mV
 * @enterState  Pointer to a function that executes FSM actions to enter a state
 * @trans       Pointer to array of transitions of the FSM
 * @transCnt    Number of elements in transition array
 * @statesCnt   Number of states in state array
 */
typedef struct {
	const u32* const states;
	PmSlaveFsmHandler enterState;
	const PmStateTran* const trans;
	const u8 statesCnt;
	const u8 transCnt;
} PmSlaveFsm;

/**
 * PmSlave - Slave structure used for managing slave's states
 * @node        Pointer to the node structure of this slave
 * @reqs        Pointer to the list of masters' requirements for the slave
 * @wake        Wake event this slave can generate
 * @slvFsm      Slave finite state machine
 * @flags       Slave's flags (bit 0: whether the slave is shareable (1) or
 *              exclusive (0) resource)
 */
typedef struct PmSlave {
	PmNode node;
	PmRequirement* reqs;
	const PmGicProxyWake* const wake;
	const PmSlaveFsm* slvFsm;
	u8 flags;
} PmSlave;

/*********************************************************************
 * Function declarations
 ********************************************************************/
int PmUpdateSlave(PmSlave* const slave);
int PmCheckCapabilities(const PmSlave* const slave, const u32 capabilities);
int PmSlaveHasWakeUpCap(const PmSlave* const slv);

bool PmSlaveRequiresPower(const PmSlave* const slave);

int PmSlaveVerifyRequest(const PmSlave* const slave);

u32 PmGetLatencyFromState(const PmSlave* const slave, const PmStateId state);
u32 PmSlaveGetUsersMask(const PmSlave* const slave);

u32 PmSlaveGetUsageStatus(const u32 slavenode, const PmMaster *const master);
u32 PmSlaveGetRequirements(const u32 slavenode, const PmMaster *const master);

#endif
