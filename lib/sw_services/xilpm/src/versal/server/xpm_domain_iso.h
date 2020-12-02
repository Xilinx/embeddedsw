/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_DOMAIN_ISO_H
#define XPM_DOMAIN_ISO_H

#include "xpm_node.h"
#include "xpm_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FALSE_VALUE		(0U)
#define TRUE_VALUE		(1U)
#define FALSE_IMMEDIATE		(2U)	/* Remove isolation immediately */
#define TRUE_PENDING_REMOVE	(3U)	/* Set isolation, but pending removal */

typedef struct XPm_Iso {
	XPm_Node Node; /**< Node: Node base class */
	u32 Mask;
	u8 Polarity;
	u32 DependencyNodeHandles[2];
}XPm_Iso;

/* Polarity */
typedef enum {
        PM_ACTIVE_LOW,
        PM_ACTIVE_HIGH,
}XPm_IsoPolarity;

/* Isolation states */
typedef enum {
	PM_ISOLATION_ON,
	PM_ISOLATION_OFF,
	PM_ISOLATION_REMOVE_PENDING,
}XPm_IsoStates;

#define ISOID(x) \
	NODEID(XPM_NODECLASS_ISOLATION, XPM_NODESUBCL_ISOLATION, XPM_NODETYPE_ISOLATION, x)

XStatus XPmDomainIso_Control(u32 IsoIdx, u32 Enable);
XStatus XPmDomainIso_ProcessPending(u32 PowerDomainId);
XStatus XPmDomainIso_GetState(u32 IsoIdx, XPm_IsoStates *State);

#ifdef __cplusplus
}
#endif

#endif /* XPM_DOMAIN_ISO_H */
