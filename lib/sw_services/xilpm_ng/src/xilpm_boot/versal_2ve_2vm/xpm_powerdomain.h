/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_POWERDOMAIN_H_
#define XPM_POWERDOMAIN_H_

#include "xpm_power.h"
// #include "xpm_device.h"
#include "xpm_rail.h"

#if defined(XILPM_NG_DOMAIN_CONTROL_GPIO)
#include "xpm_domainctrl.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XPm_PowerDomain XPm_PowerDomain;

#define XPM_POLL_TIMEOUT			(0X1000000U)
#define XPM_NPI_CSR_POLL_TIMEOUT		(0x3U)
#define XPM_DOMAIN_INIT_STATUS_REG		PMC_GLOBAL_PERS_GLOB_GEN_STORAGE0
#define MAX_POWERDOMAINS			6U /**< Maximum number of power domains */

struct XPm_PowerDomain {
	XPm_Power Power; /**< Power: Power node base class */
	u32 Parents[MAX_POWERDOMAINS]; /**< List of Parent Rail Ids */
	u32 Children[MAX_POWERDOMAINS]; /**< List of dependent children Ids */
	u16 InitFlag; /**< Flag to indicate which Ops are performed */
	u32 HcDisableMask; /**< Mask for skipping housecleaning operations */
#if defined(XILPM_NG_DOMAIN_CONTROL_GPIO)
	XPm_DomainCtrl *DomainCtrl; 	/**< DomainCtrl: GPIO based domain control class */
#endif
};

/************************** Function Prototypes ******************************/
XStatus XPmPowerDomain_Init(XPm_PowerDomain *PowerDomain, u32 Id,
			    u32 BaseAddress, XPm_Power *Parent);
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
XStatus XPmPower_UpdateDomainPower(const XPm_PowerDomain *PwrDomain, u32 State);
XStatus XPmPowerDomain_InitDomain(XPm_PowerDomain *PwrDomain, u32 Function,
				  const u32 *Args, u32 NumArgs);
XStatus XPmPower_CheckPower(const XPm_Rail *Rail, u32 VoltageRailMask);
XStatus XPmPower_SysmonCheckPower(const XPm_Rail *Rail);
#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_POWERDOMAIN_H_ */
