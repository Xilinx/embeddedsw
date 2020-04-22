/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_REQUIREMENT_H_
#define XPM_REQUIREMENT_H_

#include "xpm_device.h"
#include "xpm_subsystem.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XPm_ReqmInfo XPm_ReqmInfo;
typedef struct XPm_Reqm XPm_Requirement;

/**
 * Specific requirement information.
 */
struct XPm_ReqmInfo {
	u32 Capabilities:4; /**< Device capabilities (1-hot) */
	u32 Latency:21; /**< Maximum device latency */
	u32 QoS:7; /**< QoS requirement */
};

typedef enum {
        RELEASE_ONE,
        RELEASE_ALL,
        RELEASE_UNREQUESTED,
        RELEASE_DEVICE,
} XPm_ReleaseScope;

enum XPm_ReqUsageFlags{
        REQ_NO_RESTRICTION,
        REQ_SHARED,
        REQ_NONSHARED,
        REQ_TIME_SHARED,
};

enum XPm_ReqSecurityFlags{
        REQ_ACCESS_SECURE,
	REQ_ACCESS_SECURE_NONSECURE,
};

#define MAX_REQ_PARAMS 		1U
#define REG_FLAGS_USAGE_MASK	0x3U
#define REG_FLAGS_SECURITY_MASK	0x4U
#define REG_FLAGS_SECURITY_OFFSET 0x2U

/**
 * The requirement class.
 */
struct XPm_Reqm {
	struct XPm_Subsystem *Subsystem; /**< Subsystem imposing this requirement on the device */
	XPm_Device *Device; /**< Device used by the subsystem */
	XPm_Requirement *NextDevice; /**< Requirement on the next device from this subsystem */
	XPm_Requirement *NextSubsystem; /**< Requirement from the next subsystem on this device */
	u8 Allocated; /**< Device has been allocated to the subsystem */
	u8 SetLatReq; /**< Latency has been set from the subsystem */
	u8 Flags;	  /** Flags */
	u8 NumParams; /**< Params count */
	u32 Params[MAX_REQ_PARAMS]; /**< Params */
	XPm_ReqmInfo Curr; /**< Current requirements */
	XPm_ReqmInfo Next; /**< Pending requirements */
};

/************************** Function Prototypes ******************************/

XStatus XPmRequirement_Add(XPm_Subsystem *Subsystem, XPm_Device *Device, u32 Flags, u32 *Params, u32 NumParams);
void XPm_RequiremntUpdate(XPm_Requirement *Reqm);
XStatus XPmRequirement_Release(XPm_Requirement *Reqm, XPm_ReleaseScope Scope);
void XPmRequirement_Clear(XPm_Requirement* Reqm);
XStatus XPmRequirement_UpdateScheduled(XPm_Subsystem *Subsystem, u32 Swap);
XStatus XPmRequirement_IsExclusive(XPm_Requirement *Reqm);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_REQUIREMENT_H_ */
