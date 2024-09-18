/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_REQUIREMENT_H_
#define XPM_REQUIREMENT_H_

#include "xpm_device.h"
#include "xpm_subsystem.h"
#include "xpm_requirement_plat.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
	RELEASE_ONE,
	RELEASE_ALL,
	RELEASE_UNREQUESTED,
	RELEASE_DEVICE,
} XPm_ReleaseScope;

/**
 * Device usage policies.
 */
enum XPm_ReqUsageFlags {
	REQ_NO_RESTRICTION,	/**< device accessible from all subsystems */
	REQ_SHARED,		/**< device simultaneously shared between two or more subsystems */
	REQ_NONSHARED,		/**< device exclusively reserved by one subsystem, always */
	REQ_TIME_SHARED,	/**< device is time shared between two or more subsystems */
};

/**
 * Device/Memory region security status requirement per TrustZone.
 */
enum XPm_ReqSecurityFlags {
	REQ_ACCESS_SECURE,	/**< Device/Memory region only allows access from secure masters */
	REQ_ACCESS_SECURE_NONSECURE, /**< Device/Memory region allow both secure or non-secure masters */
};

#define REG_FLAGS_USAGE_MASK		(0x3U)
#define REG_FLAGS_SECURITY_MASK		(0x4U)
#define REG_FLAGS_SECURITY_OFFSET	(2U)
/* NOTE: Some bits are reserved in between */
#define REG_FLAGS_PREALLOC_MASK		(0x40U)
#define REG_FLAGS_PREALLOC_OFFSET	(6U)

/**
 * Combined mask for requirement flags
 */
#define REG_FLAGS_MASK		(REG_FLAGS_USAGE_MASK | REG_FLAGS_SECURITY_MASK | REG_FLAGS_PREALLOC_MASK)

/**
 * Make Requirement Flags from individual attributes
 */
#define REQUIREMENT_FLAGS(Prealloc, Security, Usage) \
				(((u32)(Prealloc) << (REG_FLAGS_PREALLOC_OFFSET)) | \
				 ((u32)(Security) << (REG_FLAGS_SECURITY_OFFSET)) | \
				 ((u32)(Usage) & (REG_FLAGS_USAGE_MASK)))

/**
 * Macros for extracting attributes from Flags
 */
#define USAGE_POLICY(Flags)	((Flags) & REG_FLAGS_USAGE_MASK)
#define SECURITY_POLICY(Flags)	(((Flags) & REG_FLAGS_SECURITY_MASK) >> REG_FLAGS_SECURITY_OFFSET)
#define PREALLOC(Flags)		(((Flags) & REG_FLAGS_PREALLOC_MASK) >> REG_FLAGS_PREALLOC_OFFSET)

/************************** Function Prototypes ******************************/

XStatus XPmRequirement_Add(XPm_Subsystem *Subsystem, XPm_Device *Device,
			   u32 Flags, u32 PreallocCaps, u32 PreallocQoS);
void XPm_RequiremntUpdate(XPm_Requirement *Reqm);
XStatus XPmRequirement_Release(XPm_Requirement *Reqm, XPm_ReleaseScope Scope);
void XPmRequirement_Clear(XPm_Requirement* Reqm);
XStatus XPmRequirement_UpdateScheduled(const XPm_Subsystem *Subsystem, u32 Swap);
XStatus XPmRequirement_IsExclusive(const XPm_Requirement *Reqm);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_REQUIREMENT_H_ */
