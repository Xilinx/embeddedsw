/******************************************************************************
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_REQUIREMENT_PLAT_H_
#define XPM_REQUIREMENT_PLAT_H_

#include "xpm_device.h"
#include "xpm_requirement_info.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XPm_Reqm XPm_Requirement;

/**
 * The requirement class.
 */
struct XPm_Reqm {
SAVE_REGION(
	u16 Flags;	  /** Flags */
	u8 PreallocCaps;  /**< Preallocated capabilities */
	u8 AttrCaps;	/**
			 * Other capabilities like security, coherency and virtualization.
			 * This does not play any role for device state transition so
			 * it is keep as a separate variable.
			 */
	u32 PreallocQoS;  /**< Preallocated QoS value */
	XPm_ReqmInfo Curr; /**< Current requirements */
	XPm_ReqmInfo Next; /**< Pending requirements */
	u8 Allocated; /**< Device has been allocated to the subsystem */
	u8 SetLatReq; /**< Latency has been set from the subsystem */
)
	struct XPm_Subsystem *Subsystem; /**< Subsystem imposing this requirement on the device */
	XPm_Device *Device; /**< Device used by the subsystem */
	XPm_Requirement *NextDevice; /**< Requirement on the next device from this subsystem */
	XPm_Requirement *NextSubsystem; /**< Requirement from the next subsystem on this device */
};

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_REQUIREMENT_PLAT_H_ */
