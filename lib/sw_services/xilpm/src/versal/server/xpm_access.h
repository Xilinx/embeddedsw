/******************************************************************************
 * Copyright (c) 2021 - 2021 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef XPM_ACCESS_H_
#define XPM_ACCESS_H_

#include "xpm_node.h"
#include "xpm_power.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XPm_RegNode XPm_RegNode;
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

#define NODE_APER_OFFSET(Ap)	(u32)(((Ap) >> NODE_APER_OFFSET_SHIFT) & NODE_APER_OFFSET_MASK)
#define NODE_APER_SIZE(Ap)	(u32)(((Ap) >> NODE_APER_SIZE_SHIFT) & NODE_APER_SIZE_MASK)
#define NODE_APER_ACCESS(Ap)	(u32)(((Ap) >> NODE_APER_ACCESS_SHIFT) & NODE_APER_ACCESS_MASK)

/**
 * Regnode class.
 */
struct XPm_RegNode {
	u32 Id;				/**< Node Id for regnode */
	u32 BaseAddress;		/**< Base address of given node */
	XPm_Power *Power;		/**< Parent power node */
	XPm_RegNode *NextRegnode;	/**< Link to next regnode */
};

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
void XPmAccess_RegnodeInit(XPm_RegNode *RegNode,
			   u32 NodeId, u32 BaseAddress, XPm_Power *Power);

XStatus XPmAccess_UpdateTable(XPm_NodeAccess *NodeEntry,
			      const u32 *Args, u32 NumArgs);

#ifdef __cplusplus
}
#endif

#endif /* XPM_ACCESS_H_ */
