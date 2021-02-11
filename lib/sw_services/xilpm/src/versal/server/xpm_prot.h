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
enum XPm_ProtState {
	XPM_PROT_DISABLED,
	XPM_PROT_ENABLED,
};

typedef struct XPm_Node XPm_Prot;
typedef struct XPm_ProtMpu XPm_ProtMpu;
typedef struct XPm_ProtPpu XPm_ProtPpu;
typedef struct XPm_PpuAperture XPm_PpuAperture;

/* Aperture information of an XPPU instance */
struct XPm_PpuAperture {
	u32 Total;		/**< Number of Supported Apertures */
	u32 Start;		/**< Aperture Base Address */
	u32 End;		/**< Aperture End Address */
};

/* Protection - XPPU Class */
struct XPm_ProtPpu {
	XPm_Prot Node;		/**< Node: Base Class */
	XPm_PpuAperture A64k;	/**< Aperture 64k */
	XPm_PpuAperture A1m;	/**< Aperture 1m */
	XPm_PpuAperture A512m;	/**< Aperture 512m */
	u32 AperPermInitMask;	/**< Default aperture permission mask */
	u8 MIDParityEn;		/**< Parity error checking status for Master IDs */
	u8 AperParityEn;	/**< Parity error checking status for Aperture entries */
};

/* Protection - XMPU Class */
struct XPm_ProtMpu {
	XPm_Prot Node;		/**< Node: Base Class */
	u8 AlignCfg;		/**< Region alignment: 4k or 1m aligned */
};

/************************** Function Prototypes ******************************/
XStatus XPmProtPpu_Init(XPm_ProtPpu *PpuNode, u32 Id, u32 BaseAddr);
XStatus XPmProtMpu_Init(XPm_ProtMpu *MpuNode, u32 Id, u32 BaseAddr);
XStatus XPmProt_Configure(XPm_Requirement *Reqm, u32 Enable);
XStatus XPmProt_CommonXppuCtrl(u32 *Args, u32 NumOfArgs);
XStatus XPmProt_CommonXmpuCtrl(u32 *Args, u32 NumOfArgs);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_PROT_H_ */
