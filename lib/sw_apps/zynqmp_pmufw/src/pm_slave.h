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

/* Forward declarations */
typedef struct PmMaster PmMaster;
typedef struct PmRequirement PmRequirement;
typedef struct PmSlave PmSlave;
typedef struct PmSlaveClass PmSlaveClass;
typedef struct PmWakeEvent PmWakeEvent;

/*********************************************************************
 * Macros
 ********************************************************************/

/* Mask definitions for slave's flags */
#define PM_SLAVE_FLAG_IS_SHAREABLE	0x1U

#define DEFINE_SLAVE_STATES(s)	.states = (s), \
				.statesCnt = ARRAY_SIZE(s)

#define DEFINE_SLAVE_TRANS(t)	.trans = (t), \
				.transCnt = ARRAY_SIZE(t)

/*********************************************************************
 * Structure definitions
 ********************************************************************/
/**
 * PmWakeEventClass - Class of the wake event
 * @set		Set event as the wake source (must be defined by each class)
 * @config	Configure the propagation of wake event (master requested)
 */
typedef struct PmWakeEventClass {
	void (*const set)(PmWakeEvent* const wake, const u32 ipi, const u32 en);
	void (*const config)(PmWakeEvent* const wake, const u32 ipi, const u32 en);
} PmWakeEventClass;

/**
 * PmWakeEvent - Structure to model wake event that can be triggered by slave
 * @derived	Pointer to the derived structure
 * @class	Pointer to the class specific to the derived structure
 */
typedef struct PmWakeEvent {
	void* const derived;
	PmWakeEventClass* const class;
} PmWakeEvent;

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
 * @states      Pointer to states array. Index in array is a state id.
 * @enterState  Pointer to a function that executes FSM actions to enter a state
 * @trans       Pointer to array of transitions of the FSM
 * @transCnt    Number of elements in transition array
 * @statesCnt   Number of states in state array
 */
typedef struct {
	const u32* const states;
	int (*const enterState)(PmSlave* const slave, const PmStateId nextState);
	const PmStateTran* const trans;
	const u8 statesCnt;
	const u8 transCnt;
} PmSlaveFsm;

/**
 * PmSlave - Slave structure used for managing slave's states
 * @node        Pointer to the node structure of this slave
 * @class       Slave class (NULL if derived slave has no specific methods to be
 *              called in addition to methods of PmNodeClass)
 * @reqs        Pointer to the list of masters' requirements for the slave
 * @wake        Wake event this slave can generate
 * @slvFsm      Slave finite state machine
 * @flags       Slave's flags (bit 0: whether the slave is shareable (1) or
 *              exclusive (0) resource)
 */
typedef struct PmSlave {
	PmNode node;
	PmSlaveClass* const class;
	PmRequirement* reqs;
	PmWakeEvent* const wake;
	const PmSlaveFsm* slvFsm;
	u8 flags;
} PmSlave;

/**
 * PmSlaveClass - Slave class to model properties of PmSlave derived objects
 * @init	Initialize the slave
 * @forceDown	Force down specific to the slave
 */
typedef struct PmSlaveClass {
	int (*const init)(PmSlave* const slave);
	int (*const forceDown)(PmSlave* const slave);
} PmSlaveClass;

/*********************************************************************
 * Global data declarations
 ********************************************************************/

extern PmNodeClass pmNodeClassSlave_g;

/*********************************************************************
 * Function declarations
 ********************************************************************/
int PmUpdateSlave(PmSlave* const slave);
int PmCheckCapabilities(const PmSlave* const slave, const u32 capabilities);
int PmSlaveHasWakeUpCap(const PmSlave* const slv);
int PmSlaveSetConfig(PmSlave* const slave, const u32 policy, const u32 perms);

int PmSlaveVerifyRequest(const PmSlave* const slave);

u32 PmSlaveGetUsersMask(const PmSlave* const slave);

u32 PmSlaveGetUsageStatus(const PmSlave* const slave,
			  const PmMaster* const master);
u32 PmSlaveGetRequirements(const PmSlave* const slave,
			   const PmMaster* const master);
void PmResetSlaveStates(void);

#endif
