/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
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
	XPm_Prot ProtNode;	/**< Node: Base Class */
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
