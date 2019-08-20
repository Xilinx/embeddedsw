/******************************************************************************
*
* Copyright (C) 2018-2019 Xilinx, Inc.  All rights reserved.
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

#ifndef XPM_DOMAIN_ISO_H
#define XPM_DOMAIN_ISO_H

#include "xpm_node.h"
#include "xpm_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FALSE_IMMEDIATE		2

typedef struct XPm_Iso {
	XPm_Node Node; /**< Node: Node base class */
	u8 Offset;
	u8 Polarity;
	u32 DependencyNodeHandles[2];
}XPm_Iso;

/* Polarity */
typedef enum {
        PM_ACTIVE_LOW,
        PM_ACTIVE_HIGH,
}XPm_IsoPolarity;

/* Isoaltion states */
typedef enum {
	PM_ISOLATION_ON,
	PM_ISOLATION_OFF,
	PM_ISOLATION_REMOVE_PENDING,
}XPm_IsoStates;

#define ISOID(x) \
	NODEID(XPM_NODECLASS_ISOLATION, XPM_NODESUBCL_ISOLATION, XPM_NODETYPE_ISOLATION, x)

#define PMC_NODEID NODEID(XPM_NODECLASS_POWER, \
	XPM_NODESUBCL_POWER_DOMAIN, XPM_NODETYPE_POWER_DOMAIN_PMC, XPM_NODEIDX_POWER_PMC)
#define NPD_NODEID NODEID(XPM_NODECLASS_POWER, \
	XPM_NODESUBCL_POWER_DOMAIN, XPM_NODETYPE_POWER_DOMAIN_NOC, XPM_NODEIDX_POWER_NOC)
#define FPD_NODEID NODEID(XPM_NODECLASS_POWER, \
	XPM_NODESUBCL_POWER_DOMAIN, XPM_NODETYPE_POWER_DOMAIN_PS_FULL, XPM_NODEIDX_POWER_FPD)
#define LPD_NODEID NODEID(XPM_NODECLASS_POWER, \
	XPM_NODESUBCL_POWER_DOMAIN, XPM_NODETYPE_POWER_DOMAIN_PS_LOW, XPM_NODEIDX_POWER_LPD)
#define CPD_NODEID NODEID(XPM_NODECLASS_POWER, \
	XPM_NODESUBCL_POWER_DOMAIN, XPM_NODETYPE_POWER_DOMAIN_CPM, XPM_NODEIDX_POWER_CPM)
#define ME_NODEID NODEID(XPM_NODECLASS_POWER, \
	XPM_NODESUBCL_POWER_DOMAIN, XPM_NODETYPE_POWER_DOMAIN_ME, XPM_NODEIDX_POWER_ME)

XStatus XPmDomainIso_Control(u32 IsoIdx, u32 Enable);
XStatus XPmDomainIso_ProcessPending(u32 PowerDomainId);

#ifdef __cplusplus
}
#endif

#endif /* XPM_DOMAIN_ISO_H */
