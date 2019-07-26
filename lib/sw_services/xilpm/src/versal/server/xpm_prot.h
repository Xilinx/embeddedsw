/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_PROT_H_
#define XPM_PROT_H_

#include "xpm_requirement.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Protection node states */
typedef enum {
	XPM_PROT_DISABLED,
	XPM_PROT_ENABLED,
} XPm_ProtState;

typedef struct XPm_Prot XPm_Prot;
typedef struct XPm_ProtMpu XPm_ProtMpu;
typedef struct XPm_ProtPpu XPm_ProtPpu;

/**
 * The processor core class.  This is the base class for all processor cores.
 */
struct XPm_Prot {
	XPm_Node Node; /**< Node: Base class */
};

typedef struct Aperture {
	u32 NumSupported;	/**< Number of Supported Apertures */
	u32 StartAddress;	/**< Aperture Base Address */
	u32 EndAddress;		/**< Aperture End Address */
}Aperture;

struct XPm_ProtPpu {
	XPm_Prot ProtNode;	/**< Node: Base Class */
	Aperture Aperture_64k;	/**< Aperture 64k */
	Aperture Aperture_1m;	/**< Aperture 1m */
	Aperture Aperture_512m;	/**< Aperture 512m */
	u8 MIDParityEn;		/**< Parity error checking status for Master IDs */
	u8 AperParityEn;	/**< Parity error checking status for Aperture entries */
};

struct XPm_ProtMpu {
	XPm_Prot ProtNode;
};

/************************** Function Prototypes ******************************/
XStatus XPmProtPpu_Init(XPm_ProtPpu *PpuNode, u32 Id, u32 BaseAddr);
XStatus XPmProtMpu_Init(XPm_ProtMpu *MpuNode, u32 Id, u32 BaseAddr);
XStatus XPmProt_Configure(XPm_Requirement *Reqm, u32 Enable);
XStatus XPmProt_XppuEnable(u32 NodeId, u32 ApertureInitVal);
XStatus XPmProt_XppuDisable(u32 NodeId);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_PROT_H_ */
