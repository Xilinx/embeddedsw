/******************************************************************************
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef XPM_ACCESS_H_
#define XPM_ACCESS_H_

#include "xpm_node.h"
#include "xpm_power.h"
#include "xpm_regnode.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XPm_NodeAccess XPm_NodeAccess;
typedef struct XPm_NodeAper XPm_NodeAper;

/* Size of bit fields for XPm_NodeAper struct */
#define NODE_APER_OFFSET_BIT_FIELD_SIZE		(20U)
#define NODE_APER_SIZE_BIT_FIELD_SIZE		(8U)
#define NODE_APER_ACCESS_BIT_FIELD_SIZE		(4U)

/* SET_NODE_ACCESS Masks/Shifts */
#define NODE_APER_OFFSET_SHIFT			(0U)
#define NODE_APER_OFFSET_MASK			(0xFFFFFU)

#define NODE_APER_SIZE_SHIFT			(24U)
#define NODE_APER_SIZE_MASK			(0xFFU)

#define NODE_APER_ACCESS_SHIFT			(0U)
#define NODE_APER_ACCESS_MASK			(0xFU)

#define NODE_APER_OFFSET(Ap)			(((Ap) >> NODE_APER_OFFSET_SHIFT) & NODE_APER_OFFSET_MASK)
#define NODE_APER_SIZE(Ap)			(((Ap) >> NODE_APER_SIZE_SHIFT) & NODE_APER_SIZE_MASK)
#define NODE_APER_ACCESS(Ap)			(((Ap) >> NODE_APER_ACCESS_SHIFT) & NODE_APER_ACCESS_MASK)

/**
 * Node access types.
 */
typedef enum {
	ACCESS_RESERVED = 0U,		/**< 0x0 */
	ACCESS_ANY_RO,			/**< 0x1 */
	ACCESS_ANY_RW,			/**< 0x2 */
	ACCESS_SEC_RO,			/**< 0x3 */
	ACCESS_SEC_RW,			/**< 0x4 */
	ACCESS_SEC_NS_SUBSYS_RO,	/**< 0x5 */
	ACCESS_SEC_NS_SUBSYS_RW,	/**< 0x6 */
	ACCESS_SEC_SUBSYS_RO,		/**< 0x7 */
	ACCESS_SEC_SUBSYS_RW,		/**< 0x8 */

	/* Always keep this enum last */
	ACCESS_TYPE_MAX,		/**< 0x9 */
} XPm_NodeAccessTypes;

/**
 * Node aperture access attributes for a node.
 */
struct XPm_NodeAper {
	u32 Offset : NODE_APER_OFFSET_BIT_FIELD_SIZE;	/**< Aperture offset */
	u32 Size : NODE_APER_SIZE_BIT_FIELD_SIZE;	/**< Aperture size */
	u32 Access : NODE_APER_ACCESS_BIT_FIELD_SIZE;	/**< Aperture access policy */
	XPm_NodeAper *NextAper;		/**< Next aperture of the same node */
};

/**
 * Node access table.
 */
struct XPm_NodeAccess {
	u32 Id;				/**< Node Id for given node */
	XPm_NodeAper *Aperture;		/**< Node aperture attributes */
	XPm_NodeAccess *NextNode;	/**< Link to next entry in the table */
};

/************************** Function Prototypes ******************************/
/* Add regnodes to pm database */
void XPmAccess_RegnodeInit(XPm_RegNode *RegNode,
			   u32 NodeId, u32 BaseAddress, XPm_Power *Power);

/* Add requirements on a regnode from different subsystems */
XStatus XPmAccess_AddRegnodeRequirement(u32 SubsystemId, u32 RegnodeId);

/* Set node access handler */
XStatus XPmAccess_UpdateTable(XPm_NodeAccess *NodeEntry,
			      const u32 *Args, u32 NumArgs);

/* Debug only function meant for printing regnodes and "Node Access Table" */
void XPmAccess_PrintTable(void);

/* IOCTL handlers */
XStatus XPmAccess_ReadReg(u32 SubsystemId,
			  u32 DeviceId,
			  pm_ioctl_id IoctlId,
			  u32 Offset, u32 Count,
			  u32 *const Response, u32 CmdType);

XStatus XPmAccess_MaskWriteReg(u32 SubsystemId,
			       u32 DeviceId,
			       pm_ioctl_id IoctlId,
			       u32 Offset, u32 Mask, u32 Value,
			       u32 CmdType);

#ifdef __cplusplus
}
#endif

#endif /* XPM_ACCESS_H_ */
