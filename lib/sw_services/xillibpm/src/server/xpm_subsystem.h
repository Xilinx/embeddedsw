/******************************************************************************
*
* Copyright (C) 2018-2019 Xilinx, Inc.  All rights reserved.
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
*
******************************************************************************/

#ifndef XPM_SUBSYSTEM_H_
#define XPM_SUBSYSTEM_H_

#include "xpm_requirement.h"

#define	INVALID_SUBSYSID	-1U
#define SUBSYSTEMCLASS_MASK \
	((XPM_NODECLASS_SUBSYSTEM & NODE_CLASS_MASK_BITS) << NODE_CLASS_SHIFT)
#define ISVALIDSUBSYSTEM(id) \
	( (((id & SUBSYSTEMCLASS_MASK) == SUBSYSTEMCLASS_MASK) && \
	((id & NODE_INDEX_MASK_BITS) < XPM_NODEIDX_SUBSYS_MAX)) ? 1 : 0)

#define XPM_SUBSYSID_DEFAULT NODEID(XPM_NODECLASS_SUBSYSTEM, XPM_NODESUBCL_SUBSYSTEM, XPM_NODETYPE_SUBSYSTEM, XPM_NODEIDX_SUBSYS_DEFAULT)
#define XPM_SUBSYSID_PMC 	NODEID(XPM_NODECLASS_SUBSYSTEM, XPM_NODESUBCL_SUBSYSTEM, XPM_NODETYPE_SUBSYSTEM, XPM_NODEIDX_SUBSYS_PMC)
#define XPM_SUBSYSID_PSM	 NODEID(XPM_NODECLASS_SUBSYSTEM, XPM_NODESUBCL_SUBSYSTEM, XPM_NODETYPE_SUBSYSTEM, XPM_NODEIDX_SUBSYS_PSM)
#define XPM_SUBSYSID_APU 	NODEID(XPM_NODECLASS_SUBSYSTEM, XPM_NODESUBCL_SUBSYSTEM, XPM_NODETYPE_SUBSYSTEM, XPM_NODEIDX_SUBSYS_APU)
#define XPM_SUBSYSID_RPU0_LOCK NODEID(XPM_NODECLASS_SUBSYSTEM, XPM_NODESUBCL_SUBSYSTEM, XPM_NODETYPE_SUBSYSTEM, XPM_NODEIDX_SUBSYS_RPU0_LOCK)
#define XPM_SUBSYSID_RPU0_0 NODEID(XPM_NODECLASS_SUBSYSTEM, XPM_NODESUBCL_SUBSYSTEM, XPM_NODETYPE_SUBSYSTEM, XPM_NODEIDX_SUBSYS_RPU0_0)
#define XPM_SUBSYSID_RPU0_1 NODEID(XPM_NODECLASS_SUBSYSTEM, XPM_NODESUBCL_SUBSYSTEM, XPM_NODETYPE_SUBSYSTEM, XPM_NODEIDX_SUBSYS_RPU0_1)
#define XPM_SUBSYSID_DDR0 	NODEID(XPM_NODECLASS_SUBSYSTEM, XPM_NODESUBCL_SUBSYSTEM, XPM_NODETYPE_SUBSYSTEM, XPM_NODEIDX_SUBSYS_DDR0)
#define XPM_SUBSYSID_ME	 	NODEID(XPM_NODECLASS_SUBSYSTEM, XPM_NODESUBCL_SUBSYSTEM, XPM_NODETYPE_SUBSYSTEM, XPM_NODEIDX_SUBSYS_ME)
#define XPM_SUBSYSID_PL 	NODEID(XPM_NODECLASS_SUBSYSTEM, XPM_NODESUBCL_SUBSYSTEM, XPM_NODETYPE_SUBSYSTEM, XPM_NODEIDX_SUBSYS_PL)

/**
 * Subsystem creation states.
 */
typedef enum {
	OFFLINE,
	RESERVED,
	ONLINE,
	SUSPENDING,
	MAX_STATE
} XPm_SubsysState;

typedef struct XPm_Requirement XPm_Requirement;
typedef struct XPm_Subsystem XPm_Subsystem;

/**
 * The subsystem class.
 */
struct XPm_Subsystem {
	u32 Id; /**< Subsystem ID */
	XPm_SubsysState State; /**< Subsystem state */
	u32 IpiMask;
	XPm_Requirement *Requirements;
		/**< Head of the requirement list for all devices. */
	void (*NotifyCb)(u32 SubsystemId, const u32 EventId);
};

extern u32 ReservedSubsystemId;
extern XPm_Subsystem PmSubsystems[XPM_NODEIDX_SUBSYS_MAX];
extern u32 XPm_SubsystemIpiMask[XPM_NODEIDX_SUBSYS_MAX];

/************************** Function Prototypes ******************************/

u32 XPmSubsystem_GetIPIMask(u32 SubsystemId);
u32 XPmSubsystem_GetSubSysIdByIpiMask(u32 IpiMask);
XStatus XPm_IsWakeAllowed(u32 SubsystemId, u32 NodeId);
XStatus XPm_IsAccessAllowed(u32 SubsystemId, u32 NodeId);
XStatus XPmSubsystem_SetState(const u32 SubsystemId, const u32 State);
XStatus XPmSubsystem_Add(u32 SubsystemId);
XStatus XPmSubsystem_Destroy(u32 SubsystemId);
XStatus XPmSubsystem_IsAllProcDwn(u32 SubsystemId);
XStatus XPm_IsForcePowerDownAllowed(u32 SubsystemId, u32 NodeId);
XStatus XPmSubsystem_ForceDownCleanup(u32 SubsystemId);
int XPmSubsystem_Idle(u32 SubsystemId);
XPm_Subsystem * XPmSubsystem_GetById(u32 SubsystemId);
XStatus XPmSubsystem_SetCurrent(u32 SubsystemId);
u32 XPmSubsystem_GetCurrent(void);
XStatus XPmSubsystem_Restart(u32 SubsystemId);

#endif /* XPM_SUBSYSTEM_H_ */
