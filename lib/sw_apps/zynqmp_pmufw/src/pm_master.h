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
 * This file contains PM master related data structures and
 * functions for accessing them.
 *********************************************************************/

#ifndef PM_MASTER_H_
#define PM_MASTER_H_

#include "pm_slave.h"
#include "pm_common.h"
#include "pm_proc.h"
#include "pm_node.h"
#include "pm_power.h"
#include "xil_types.h"
#include "pm_gic_proxy.h"

typedef struct PmMaster PmMaster;
typedef struct PmRequirement PmRequirement;
/*********************************************************************
 * Enum definitions
 ********************************************************************/

typedef enum {
	PM_MASTER_EVENT_WAKE,
	PM_MASTER_EVENT_SLEEP,
	PM_MASTER_EVENT_SELF_SUSPEND,
	PM_MASTER_EVENT_ABORT_SUSPEND,
	PM_MASTER_EVENT_FORCED_PROC,
	PM_MASTER_EVENT_FORCE_DOWN,
} PmMasterEvent;

/*********************************************************************
 * Macros
 ********************************************************************/

/* Master state definitions */

/* Master is active if at least one of its processors is in active state */
#define PM_MASTER_STATE_ACTIVE      1U

/* Master is suspending if the last awake processor is in suspending state */
#define PM_MASTER_STATE_SUSPENDING  2U

/* Master is suspended if the last standing processor was properly suspended */
#define PM_MASTER_STATE_SUSPENDED   3U

/*
 * Master is killed if the last standing processor or the power parent was
 * forced to power down
 */
#define PM_MASTER_STATE_KILLED      4U

/* Master has not sent notification that it has initialized PM */
#define PM_MASTER_STATE_UNINITIALIZED	5U

/*********************************************************************
 * Structure definitions
 ********************************************************************/

/**
 * PmSuspendRequest() - For tracking information about request suspend being
 *                      processed at the moment
 * @initiator   Master which has requested suspend
 * @acknowledge Acknowledge argument provided with the request suspend call
 */
typedef struct {
	const PmMaster* initiator;
	u32 acknowledge;
} PmSuspendRequest;

/**
 * PmMaster - contains PM master related informations
 * @procs       Pointer to the array of processors within the master
 * @wakeProc    Processor to wake-up (pointer to the processor that has been
 *              suspended the last)
 * @reqs        Pointer to the master's list of requirements for slaves'
 *              capabilities. For every slave that the master can use there has
 *              to be a dedicated requirements structure
 * @nextMaster  Pointer to the next used master in the system
 * @gic         If the master has its own GIC which is controlled by the PMU,
 *              this is a pointer to it.
 * @evalState   Function to be called when a state specified by the master
 *              needs to be evaluated (implicit scheduling of requirements)
 * @remapAddr   Remap address (used when master's and PMU's memory map differ)
 * @memories    Pointer to the array of memories used by the master
 * @ipiMask     Mask dedicated to the master in IPI registers
 * @nid         Placeholder nodeId - used to encode request suspend for group of
 *              processors sharing the same communication channel. When PM
 *              receives this nid as request suspend argument, it initiates
 *              init suspend to the master. At the PU, in init_suspend_cb
 *              implementation, the request is mappend to actual suspend of all
 *              processors in the PU. In RPU case, this data could be
 *              initialized from PCW, based on RPU configuration.
 * @procsCnt    Number of processors within the master
 * @wakePerms   ORed ipi masks of masters which can wake-up this master
 * @suspendPerms ORed ipi masks of masters which can request this master to
 *              suspend
 * @suspendRequest Captures info about the ongoing suspend request (this master
 *              is the target which suppose to suspend). At any moment only
 *              one suspend request can be active for one target/master
 * @suspendTimeout Timeout which specifies how much time the master has to
 *              complete suspend, otherwise it is considered to be unresponsive
 * @state       State of the master which is a combination of the states of its
 *              processors and also depends on the order in which processors
 *              enter their states.
 * @name	Master name
 */
typedef struct PmMaster {
	PmSuspendRequest suspendRequest;
	PmProc** const procs;
	PmProc* wakeProc;
	PmRequirement* reqs;
	PmMaster* nextMaster;
	const PmGicProxy* const gic;
	int (*const evalState)(const u32 state);
	u32 (*const remapAddr)(const u32 address);
	const PmSlave** const memories;
	const char* const name;
	u32 ipiMask;
	u32 wakePerms;
	u32 suspendPerms;
	u32 suspendTimeout;
	PmNodeId nid;
	const u8 procsCnt;
	u8 state;
} PmMaster;

/**
 * PmMasterConfig - Structure to store master configuration data
 * @ipiMask             IPI mask assigned to the master
 * @suspendTimeout      Master's suspend timeout
 * @suspendPerms        Permissions to request suspend of other masters
 * @wakePerms           Permissions to request wake of other masters
 */
typedef struct PmMasterConfig {
	u32 ipiMask;
	u32 suspendTimeout;
	u32 suspendPerms;
	u32 wakePerms;
} PmMasterConfig;

/*********************************************************************
 * Global data declarations
 ********************************************************************/
extern PmMaster pmMasterApu_g;
extern PmMaster pmMasterRpu_g;
extern PmMaster pmMasterRpu0_g;
extern PmMaster pmMasterRpu1_g;

/*********************************************************************
 * Function declarations
 ********************************************************************/
/* Get functions */
PmMaster* PmGetMasterByIpiMask(const u32 mask);
PmMaster* PmMasterGetNextFromIpiMask(u32* const mask);

PmProc* PmGetProcByWfiStatus(const u32 mask);
PmProc* PmGetProcOfThisMaster(const PmMaster* const master,
			      const PmNodeId nodeId);
PmProc* PmGetProcOfOtherMaster(const PmMaster* const master,
			       const PmNodeId nodeId);

int PmMasterWakeProc(PmProc* const proc);
int PmMasterFsm(PmMaster* const master, const PmMasterEvent event);
int PmMasterRestart(PmMaster* const master);
int PmMasterInitFinalize(PmMaster* const master);

void PmMasterDefaultConfig(void);
void PmMasterSetConfig(PmMaster* const mst, const PmMasterConfig* const cfg);
void PmMasterClearConfig(void);
#ifdef IDLE_PERIPHERALS
void PmMasterIdleSystem(void);
#endif

bool PmCanRequestSuspend(const PmMaster* const reqMaster,
			 const PmMaster* const respMaster);
bool PmIsRequestedToSuspend(const PmMaster* const master);

int PmMasterSuspendAck(PmMaster* const mst, const int response);

PmMaster* PmMasterGetPlaceholder(const PmNodeId nodeId);

int PmMasterWake(const PmMaster* const mst);
int PmWakeMasterBySlave(const PmSlave * const slave);

/* Inline functions for checking the state of the master */

static inline bool PmMasterIsSuspending(const PmMaster* const master)
{
	return PM_MASTER_STATE_SUSPENDING == master->state;
}

static inline bool PmMasterIsSuspended(const PmMaster* const master)
{
	return PM_MASTER_STATE_SUSPENDED == master->state;
}

static inline bool PmMasterIsKilled(const PmMaster* const master)
{
	return PM_MASTER_STATE_KILLED == master->state;
}

static inline bool PmMasterIsActive(const PmMaster* const master)
{
	return PM_MASTER_STATE_ACTIVE == master->state;
}

static inline bool PmMasterCanReceiveCb(const PmMaster* const master)
{
	return (PM_MASTER_STATE_KILLED != master->state) &&
		(PM_MASTER_STATE_UNINITIALIZED != master->state);
}

/**
 * PmMasterCanRequestWake() - Check if master has permissions to request wake
 * @requestor   Master which requested wake
 * @target      Target master to wake
 *
 * @return      True if master has permission to request wake, false otherwise
 */
static inline bool PmMasterCanRequestWake(const PmMaster* const requestor,
					  const PmMaster* const target)
{
	return 0U != (requestor->ipiMask & target->wakePerms);
}

bool PmMasterCanForceDown(const PmMaster* const master,
			  const PmPower* const power);
bool PmMasterIsLastSuspending(const PmMaster* const master);
bool PmMasterIsUniqueWakeup(const PmSlave* const slave);
int PmMasterReleaseAll(void);

#endif
