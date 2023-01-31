/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_POWERDOMAIN_H_
#define XPM_POWERDOMAIN_H_

#include "xpm_power.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XPm_PowerDomain XPm_PowerDomain;

#define XPM_POLL_TIMEOUT			(0X1000000U)
#define XPM_DOMAIN_INIT_STATUS_REG		PMC_GLOBAL_PERS_GLOB_GEN_STORAGE0
#define MAX_POWERDOMAINS			6U

/**
 * The power domain node class.  This is the base class for all the power domain
 * classes.
 */
struct XPm_PowerDomainOps {
	XStatus (*InitStart)(XPm_PowerDomain *PwrDomain, const u32 *Args, u32 NumOfArgs);
	XStatus (*InitFinish)(const XPm_PowerDomain *PwrDomain, const u32 *Args, u32 NumOfArgs);
};

struct XPm_PowerDomain {
	XPm_Power Power; /**< Power: Power node base class */
	const struct XPm_PowerDomainOps *DomainOps; /**< house cleaning operations */
	u32 Parents[MAX_POWERDOMAINS]; /**< List of Parent Rail Ids */
	u32 Children[MAX_POWERDOMAINS]; /**< List of depedent children Ids */
	u16 InitFlag; /**< Flag to indicate which Ops are performed */
	u32 HcDisableMask; /**< Mask for skipping housecleaning operations */
};

/************************** Function Prototypes ******************************/
XStatus XPmPowerDomain_Init(XPm_PowerDomain *PowerDomain, u32 Id,
			    u32 BaseAddress, XPm_Power *Parent,
			    struct XPm_PowerDomainOps const *Ops);
XStatus XPmPowerDomain_AddParent(u32 Id, const u32 *ParentNodes, u32 NumParents);
XStatus XPm_PowerUpLPD(const XPm_Node *Node);
XStatus XPm_PowerDwnLPD(void);
XStatus XPm_PowerUpFPD(const XPm_Node *Node);
XStatus XPm_PowerDwnFPD(const XPm_Node *Node);
XStatus XPm_PowerUpPLD(XPm_Node *Node);
XStatus XPm_PowerDwnPLD(const XPm_Node *Node);
XStatus XPm_PowerUpCPM5N(const XPm_Node *Node);
XStatus XPm_PowerDwnCPM5N(const XPm_Node *Node);
XStatus XPm_PowerUpNoC(XPm_Node *Node);
XStatus XPm_PowerDwnNoC(void);
XStatus XPm_PowerUpHnicx(void);
XStatus XPm_PowerDwnHnicx(void);
XStatus XPmPowerDomain_InitDomain(XPm_PowerDomain *PwrDomain, u32 Function,
				  const u32 *Args, u32 NumArgs);
#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_POWERDOMAIN_H_ */
