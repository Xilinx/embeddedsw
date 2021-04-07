/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_PROT_H_
#define XPM_PROT_H_

#include "xpm_requirement.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Protection node states
 */
enum XPm_ProtState {
	XPM_PROT_DISABLED,
	XPM_PROT_ENABLED,
};

/**
 *  XPPU InitNode functions
 */
enum XPm_XppuInitFunctions {
	FUNC_XPPU_DISABLE,
	FUNC_XPPU_ENABLE,
	FUNC_XPPU_RECONFIG,
};

/**
 *  XMPU InitNode functions
 */
enum XPm_XmpuInitFunctions {
	FUNC_XMPU_DISABLE,
	FUNC_XMPU_ENABLE,
};

typedef struct XPm_Node XPm_Prot;
typedef struct XPm_ProtMpu XPm_ProtMpu;
typedef struct XPm_ProtPpu XPm_ProtPpu;
typedef struct XPm_PpuAperture XPm_PpuAperture;

/* Protection Init Node Ops */
typedef XStatus (*XPm_ProtPpuOps)
	(XPm_ProtPpu *Ppu, u32 Func, const u32 *Args, u32 NumArgs);
typedef XStatus (*XPm_ProtMpuOps)
	(XPm_ProtMpu *Mpu, u32 Func, const u32 *Args, u32 NumArgs);

/* Aperture information of an XPPU instance */
struct XPm_PpuAperture {
	u32 Total;		/**< Number of Supported Apertures */
	u32 Start;		/**< Aperture Base Address */
	u32 End;		/**< Aperture End Address */
};

/* Protection - XPPU Class */
struct XPm_ProtPpu {
	XPm_Prot Node;		/**< Node: Base Class */
	XPm_Power *Power;	/**< Parent: Parent power node */
	XPm_ProtPpuOps Ops;	/**< Node initialization operations */
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
	XPm_Power *Power;	/**< Parent: Parent power node */
	XPm_ProtMpuOps Ops;	/**< Node initialization operations */
	u8 AlignCfg;		/**< Region alignment: 4k or 1m aligned */
};

/************************** Function Prototypes ******************************/
XStatus XPmProt_PpuInit(XPm_ProtPpu *Ppu,
			u32 Id, u32 BaseAddr, XPm_Power *Power);
XStatus XPmProt_MpuInit(XPm_ProtMpu *Mpu,
			u32 Id, u32 BaseAddr, XPm_Power *Power);

XStatus XPmProt_PpuControl(const XPm_Requirement *Reqm,
			   const u32 AddrToProt,
			   const u32 Enable);
XStatus XPmProt_MpuControl(const XPm_Requirement *Reqm,
			   u32 Enable);

XPm_Prot *XPmProt_GetById(const u32 Id);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_PROT_H_ */
