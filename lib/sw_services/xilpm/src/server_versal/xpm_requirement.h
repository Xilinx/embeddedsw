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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

#ifndef XPM_REQUIREMENT_H_
#define XPM_REQUIREMENT_H_

#include "xpm_device.h"
#include "xpm_subsystem.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XPm_ReqmInfo XPm_ReqmInfo;

/**
 * Specific requirement information.
 */
struct XPm_ReqmInfo {
	u32 Capabilities:4; /**< Device capabilities (1-hot) */
	u32 Latency:21; /**< Maximum device latency */
	u32 QoS:7; /**< QoS requirement */
};

typedef struct XPm_Device XPm_Device;
typedef struct XPm_Requirement XPm_Requirement;
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

#define MAX_REQ_PARAMS 1
#define REG_FLAGS_USAGE_MASK	0x3U
#define REG_FLAGS_SECURITY_MASK	0x4U
#define REG_FLAGS_SECURITY_OFFSET 0x2U

/**
 * The requirement class.
 */
struct XPm_Requirement {
	XPm_Subsystem *Subsystem; /**< Subsystem imposing this requirement on the device */
	XPm_Device *Device; /**< Device used by the subsystem */
	XPm_Requirement *NextDevice; /**< Requirement on the next device from this subsystem */
	XPm_Requirement *NextSubsystem; /**< Requirement from the next subsystem on this device */
	u8 Allocated; /**< Device has been allocated to the subsystem */
	u8 SetLatReq; /**< Latency has been set from the subsystem */
	u8 Flags;	  /** Flags */
	u32 Params[MAX_REQ_PARAMS];
	XPm_ReqmInfo Curr; /**< Current requirements */
	XPm_ReqmInfo Next; /**< Pending requirements */
};

/************************** Function Prototypes ******************************/

XStatus XPmRequirement_Add(XPm_Subsystem *Subsystem, XPm_Device *Device, u32 Flags, u32 *Params, u32 NumParams);
void XPm_RequiremntUpdate(XPm_Requirement *Reqm);
XStatus XPmRequirement_Release(XPm_Requirement *Reqm, XPm_ReleaseScope Scope);
void XPmRequirement_Clear(XPm_Requirement* Reqm);
XStatus XPmRequirement_UpdateScheduled(XPm_Subsystem *Subsystem, u32 Swap);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_REQUIREMENT_H_ */
