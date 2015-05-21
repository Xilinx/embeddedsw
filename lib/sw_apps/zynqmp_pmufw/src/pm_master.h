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
 * This file contains PM master related data structures and
 * functions for accessing them. Also, each PM masters have defined
 * data structures used for tracking it's requests for each slave's
 * capabilities/states.
 *********************************************************************/

#ifndef PM_MASTER_H_
#define PM_MASTER_H_

#include "pm_slave.h"
#include "pm_common.h"
#include "pm_proc.h"
#include "pm_node.h"
#include "pm_power.h"
#include "xil_types.h"

typedef u8 PmMasterId;
typedef u8 PmRequirementId;
typedef u8 PmMasterEvent;

typedef struct PmMaster PmMaster;

/*********************************************************************
 * Macros
 ********************************************************************/
/* Apu slaves */
#define PM_MASTER_APU_SLAVE_OCM0    0U
#define PM_MASTER_APU_SLAVE_OCM1    1U
#define PM_MASTER_APU_SLAVE_OCM2    2U
#define PM_MASTER_APU_SLAVE_OCM3    3U
#define PM_MASTER_APU_SLAVE_L2      4U
#define PM_MASTER_APU_SLAVE_USB0    5U
#define PM_MASTER_APU_SLAVE_USB1    6U
#define PM_MASTER_APU_SLAVE_TTC0    7U
#define PM_MASTER_APU_SLAVE_SATA    8U
#define PM_MASTER_APU_SLAVE_MAX     9U

/* Rpu0 slaves */
#define PM_MASTER_RPU_0_SLAVE_TCM0A 0U
#define PM_MASTER_RPU_0_SLAVE_TCM0B 1U
#define PM_MASTER_RPU_0_SLAVE_TCM1A 2U
#define PM_MASTER_RPU_0_SLAVE_TCM1B 3U
#define PM_MASTER_RPU_0_SLAVE_OCM0  4U
#define PM_MASTER_RPU_0_SLAVE_OCM1  5U
#define PM_MASTER_RPU_0_SLAVE_OCM2  6U
#define PM_MASTER_RPU_0_SLAVE_OCM3  7U
#define PM_MASTER_RPU_0_SLAVE_USB0  8U
#define PM_MASTER_RPU_0_SLAVE_USB1  9U
#define PM_MASTER_RPU_0_SLAVE_TTC0  10U
#define PM_MASTER_RPU_0_SLAVE_SATA  11U
#define PM_MASTER_RPU_0_SLAVE_MAX   12U

/* Pm Master request info masks */
#define PM_MASTER_WAKEUP_REQ_MASK   0x1U
#define PM_MASTER_USING_SLAVE_MASK  0x2U

/* Suspend request related */
#define PM_REQUESTED_SUSPEND        0x1U

/*********************************************************************
 * Structure definitions
 ********************************************************************/
/**
 * PmRequirement - structure for tracking requirements of a master for the slave
 *              setting. One structure should be statically assigned for each
 *              possible combination of master/slave, because dynamic memory
 *              allocation cannot be used.
 * @slave       Pointer to the slave structure
 * @requestor   Pointer to the master structure. Can be removed if need to
 *              optimize for space instead performance
 * @info        Contains information about master's request - a bit for
 *              encoding has master requested or released node, and a bit to
 *              encode has master requested a wake-up of this slave.
 * @defaultReq  Default requirements of a master - requirements for slave
 *              capabilities without which master's primary processor cannot
 *              switch to active state.
 * @currReq     Currently holding requirements of a master for this slave
 * @nextReq     Requirements of a master to be configured when it changes the
 *              state (after it goes to sleep or before it gets awake)
 */
typedef struct PmRequirement {
	PmSlave* const slave;
	PmMaster* const requestor;
	u8 info;
	const u32 defaultReq;
	u32 currReq;
	u32 nextReq;
} PmRequirement;

/**
 * PmSuspendRequest() - For tracking information about which master can/has
 *                      requested which master to suspend.
 * @reqMst      Master which is priviledged to request suspend of respMst
 * @flags       Flags storing information about the actual request
 * @ackReq      Acknowledge argument provided with the request suspend call
 */
typedef struct {
	const PmMaster* reqMst;
	u8 flags;
	u32 ackReq;
} PmSuspendRequest;

/**
 * PmMaster - contains PM master related informations
 * @procs       Pointer to the array of processors within the master
 * @procsCnt    Number of processors within the master
 * @ipiMask     Mask dedicated to the master in IPI registers
 * @pmuBuffer   IPI buffer address into which PMU can write (PMU's buffer)
 * @buffer      IPI buffer address into which this master can write
 *              (master's buffer)
 * @reqs        Pointer to the masters array of requirements for slave
 *              capabilities. For every slave the master can use, there has to
 *              be a statically initialized structure for that master/slave pair
 * @reqsCnt     Number of requirement elements (= worst case for number of
 *              used slaves)
 */
typedef struct PmMaster {
	PmProc* const procs;
	const u8 procsCnt;
	const u32 ipiMask;
	const u32 pmuBuffer;
	const u32 buffer;
	PmRequirement* const reqs;
	PmSuspendRequest pmSuspRequests;
	const PmRequirementId reqsCnt;
} PmMaster;

/*********************************************************************
 * Global data declarations
 ********************************************************************/
extern PmMaster pmMasterApu_g;
extern PmMaster pmMasterRpu0_g;
extern PmMaster pmMasterRpu1_g;

extern PmRequirement pmApuReq_g[PM_MASTER_APU_SLAVE_MAX];
extern PmRequirement pmRpu0Req_g[PM_MASTER_RPU_0_SLAVE_MAX];

/*********************************************************************
 * Function declarations
 ********************************************************************/
/* Get functions */
const PmMaster* PmGetMasterByIpiMask(const u32 mask);

PmProc* PmGetProcByWfiStatus(const u32 mask);
PmProc* PmGetProcByWakeStatus(const u32 mask);
PmProc* PmGetProcByNodeId(const PmNodeId nodeId);
PmProc* PmGetProcOfThisMaster(const PmMaster* const master,
			      const PmNodeId nodeId);
PmProc* PmGetProcOfOtherMaster(const PmMaster* const master,
			       const PmNodeId nodeId);
PmRequirement* PmGetRequirementForSlave(const PmMaster* const master,
			      const PmNodeId nodeId);

/* Requirements related functions */
int PmRequirementSchedule(PmRequirement* const masterReq, const u32 caps);
int PmRequirementUpdate(PmRequirement* const masterReq, const u32 caps);
/* Notify master by a primary core when changing state */
int PmMasterNotify(PmMaster* const master, const PmProcEvent event);

/* Call at initialization to enable all masters' IPI interrupts */
void PmEnableAllMasterIpis(void);

/* Call when FPD goes down to enable GIC Proxy interrupts */
void PmEnableProxyWake(PmMaster* const master);

bool PmCanRequestSuspend(const PmMaster* const reqMaster,
			 const PmMaster* const respMaster);
bool PmIsRequestedToSuspend(const PmMaster* const master);

int PmRememberSuspendRequest(const PmMaster* const reqMaster,
				 PmMaster* const respMaster,
				 const u32 ack);

int PmMasterSuspendAck(PmMaster* const respMaster,
			   const int response);


#endif
