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

#define CDO_ISO_ARG_FORMAT_MASK 	(0X00FFU)
#define CDO_ISO_DEP_COUNT_MASK		(0xFF00U)
#define CDO_ISO_DEP_COUNT_SHIFT		(8U)
#define CDO_ISO_ARG_FORMAT(ARG) (ARG & CDO_ISO_ARG_FORMAT_MASK) /* Extract format field of the at given arg */
#define CDO_ISO_DEP_COUNT(ARG) ((ARG & CDO_ISO_DEP_COUNT_MASK) >> (CDO_ISO_DEP_COUNT_SHIFT))
#define PM_ISO_MAX_NUM_DEPENDENCIES	(2U)

typedef struct XPm_Iso {
	XPm_Node Node; 	/**< Node: Node base class */
	u32 Mask;
	u8 IsPsmLocal;		/* a Non-zero value indicates the node address is belong to PSM local address */
	u8 Polarity;
	u32 NumDependencies;
	u32 Dependencies[PM_ISO_MAX_NUM_DEPENDENCIES];
}XPm_Iso;

/* Polarity */
typedef enum {
	ACTIVE_LOW,
	ACTIVE_HIGH,
}XPm_IsoPolarity;

typedef enum {
	SINGLE_WORD_ACTIVE_LOW,
	SINGLE_WORD_ACTIVE_HIGH,
	PSM_SINGLE_WORD_ACTIVE_LOW,
	PSM_SINGLE_WORD_ACTIVE_HIGH,
	POWER_DOMAIN_DEPENDENCY,
}XPm_IsoCdoArgsFormat;

/* Isolation states */
typedef enum {
	PM_ISOLATION_ON,
	PM_ISOLATION_OFF,
	PM_ISOLATION_REMOVE_PENDING,
}XPm_IsoStates;

#define ISOID(x) \
	NODEID(XPM_NODECLASS_ISOLATION, XPM_NODESUBCL_ISOLATION, XPM_NODETYPE_ISOLATION, x)

XStatus XPmDomainIso_NodeInit(u32 NodeId, u32 BaseAddress, u32 Mask, u8 Psm, \
	u8 Polarity, const u32* Dependencies, u32 NumDependencies);
XStatus XPmDomainIso_Control(u32 IsoIdx, u32 Enable);
XStatus XPmDomainIso_ProcessPending(void);
XStatus XPmDomainIso_GetState(u32 IsoIdx, XPm_IsoStates *State);

#ifdef __cplusplus
}
#endif

#endif /* XPM_DOMAIN_ISO_H */
