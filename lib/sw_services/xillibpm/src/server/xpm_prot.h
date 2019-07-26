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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
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
	u32 StartAddress;
	u32 EndAddress;
}Aperture;

struct XPm_ProtPpu {
	XPm_Prot ProtNode;
	Aperture Aperture_64k;
	Aperture Aperture_1m;
	Aperture Aperture_512m;
};

struct XPm_ProtMpu {
	XPm_Prot ProtNode;
};

/************************** Function Prototypes ******************************/
XStatus XPmProt_Init(XPm_Prot *Node, u32 Id, u32 BaseAddr);
XStatus XPmProt_Configure(XPm_Requirement *Reqm, u32 Enable);
XStatus XPmProt_XppuEnable(u32 NodeId, u32 ApertureInitVal);
XStatus XPmProt_XppuDisable(u32 NodeId);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_PROT_H_ */
