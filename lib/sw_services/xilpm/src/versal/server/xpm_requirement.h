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

/**
 * Read/Write access control policy.
 * NOTE: only applicable for memory regions
 */
enum XPm_ReqRdWrFlags {
	REQ_TRANS_NOT_ALLOWED,	/**< Transaction allowed */
	REQ_TRANS_ALLOWED,	/**< Transaction not Allowed */
};

/**
 * Non-secure memory region check type policy.
 * Secure masters may or may not be allowed to access Non-Secure (NS) memory regions.
 */
enum XPm_ReqNsRegionCheckType {
	REQ_ACCESS_RELAXED_CHECKING,	/**< Secure requests may access a non-secure (NS) region */
	REQ_ACCESS_STRICT_CHECKING,	/**< Secure requests may only access a secure region */
};

#define MAX_REQ_PARAMS			(1U)
#define REG_FLAGS_USAGE_MASK		(0x3U)
#define REG_FLAGS_SECURITY_MASK		(0x4U)
#define REG_FLAGS_SECURITY_OFFSET	(0x2U)
/**
 * Following bits are only applicable for Memory Region nodes
 */
#define REG_FLAGS_RD_POLICY_MASK	(0x8U)
#define REG_FLAGS_RD_POLICY_OFFSET	(0x3U)
#define REG_FLAGS_WR_POLICY_MASK	(0x10U)
#define REG_FLAGS_WR_POLICY_OFFSET	(0x4U)
#define REG_FLAGS_NSREGN_CHECK_MASK	(0x20U)
#define REG_FLAGS_NSREGN_CHECK_OFFSET	(0x5U)

#define REG_FLAGS_CAPABILITY_OFFSET	(0x8U)
#define REG_FLAGS_CAPABILITY_MASK	(0x7F00U)
#define REG_FLAGS_PREALLOC_OFFSET	(0xFU)
#define REG_FLAGS_PREALLOC_MASK		(0x8000U)
/**
 * Combined mask for requirement flags
 */
#define REG_FLAGS_MASK		(REG_FLAGS_USAGE_MASK |\
				 REG_FLAGS_SECURITY_MASK |\
				 REG_FLAGS_RD_POLICY_MASK |\
				 REG_FLAGS_WR_POLICY_MASK |\
				 REG_FLAGS_NSREGN_CHECK_MASK |\
				 REG_FLAGS_CAPABILITY_MASK |\
				 REG_FLAGS_PREALLOC_MASK)

/**
 * Make Requirement Flags from individual attributes
 * NOTE:
 *   Following policies are ONLY applicable to Memory Region nodes;
 *   for all the other device nodes these are ignored:
 *     - Region checking policy
 *     - Read Policy
 *     - Write Policy
 */
#define REQUIREMENT_FLAGS(Prealloc, Capability, RegnCheck, Wr, Rd, Security, Usage) \
				(((u32)(Prealloc) << (REG_FLAGS_PREALLOC_OFFSET)) | \
				 ((u32)(Capability) << (REG_FLAGS_CAPABILITY_OFFSET)) | \
				 ((u32)(RegnCheck) << (REG_FLAGS_NSREGN_CHECK_OFFSET)) | \
				 ((u32)(Wr) << (REG_FLAGS_WR_POLICY_OFFSET)) | \
				 ((u32)(Rd) << (REG_FLAGS_RD_POLICY_OFFSET)) | \
				 ((u32)(Security) << (REG_FLAGS_SECURITY_OFFSET)) | \
				 ((u32)(Usage) & (REG_FLAGS_USAGE_MASK)))

/**
 * Macros for extracting attributes from Flags
 */
#define USAGE_POLICY(Flags)	((Flags) & REG_FLAGS_USAGE_MASK)
#define SECURITY_POLICY(Flags)	(((Flags) & REG_FLAGS_SECURITY_MASK) >> REG_FLAGS_SECURITY_OFFSET)
#define RD_POLICY(Flags)	(((Flags) & REG_FLAGS_RD_POLICY_MASK) >> REG_FLAGS_RD_POLICY_OFFSET)
#define WR_POLICY(Flags)	(((Flags) & REG_FLAGS_WR_POLICY_MASK) >> REG_FLAGS_WR_POLICY_OFFSET)
#define REGN_CHECK_POLICY(Flags)(((Flags) & REG_FLAGS_NSREGN_CHECK_MASK) >> REG_FLAGS_NSREGN_CHECK_OFFSET)
#define CAPABILITY(Flags)	(((Flags) & REG_FLAGS_CAPABILITY_MASK) >> REG_FLAGS_CAPABILITY_OFFSET)
#define ISPREALLOCREQUIRED(Flags)(((Flags) & REG_FLAGS_PREALLOC_MASK) >> REG_FLAGS_PREALLOC_OFFSET)

/**
 * The requirement class.
 */
struct XPm_Reqm {
	struct XPm_Subsystem *Subsystem; /**< Subsystem imposing this requirement on the device */
	XPm_Device *Device; /**< Device used by the subsystem */
	XPm_Requirement *NextDevice; /**< Requirement on the next device from this subsystem */
	XPm_Requirement *NextSubsystem; /**< Requirement from the next subsystem on this device */
	u32 Params[MAX_REQ_PARAMS]; /**< Params */
	XPm_ReqmInfo Curr; /**< Current requirements */
	XPm_ReqmInfo Next; /**< Pending requirements */
	u16 Flags;	  /** Flags */
	u8 Allocated; /**< Device has been allocated to the subsystem */
	u8 SetLatReq; /**< Latency has been set from the subsystem */
	u8 NumParams; /**< Params count */
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
