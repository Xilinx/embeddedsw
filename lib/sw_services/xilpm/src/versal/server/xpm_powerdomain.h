/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_POWERDOMAIN_H_
#define XPM_POWERDOMAIN_H_

#include "xpm_power.h"
#include "xpm_reset.h"
#include "xpm_domain_iso.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XPm_PowerDomain XPm_PowerDomain;

/* Extern Variable and Function */
extern u32 SysmonAddresses[];
extern u32 ResetReason;
extern int XLoader_ReloadImage(u32 ImageId);

#define XPM_POLL_TIMEOUT			(0X1000000U)
#define XPM_DOMAIN_INIT_STATUS_REG		PMC_GLOBAL_PERS_GLOB_GEN_STORAGE0

/**
 * The power domain node class.  This is the base class for all the power domain
 * classes.
 */
struct XPm_PowerDomainOps {
	XStatus (*InitStart)(u32 *Args, u32 NumOfArgs);
	XStatus (*InitFinish)(u32 *Args, u32 NumOfArgs);
	XStatus (*ScanClear)(u32 *Args, u32 NumOfArgs);
	XStatus (*Mbist)(u32 *Args, u32 NumOfArgs);
	XStatus (*Lbist)(u32 *Args, u32 NumOfArgs);
	XStatus (*Bisr)(u32 *Args, u32 NumOfArgs);
	XStatus (*PlHouseclean)(u32 *Args, u32 NumOfArgs);
	XStatus (*MemInit)(u32 *Args, u32 NumOfArgs);
	XStatus (*HcComplete)(u32 *Args, u32 NumOfArgs);
	XStatus (*XppuCtrl)(u32 *Args, u32 NumOfArgs);
};

struct XPm_PowerDomain {
	XPm_Power Power; /**< Power: Power node base class */
	XPm_Power *Children; /**< List of children power nodes */
	struct XPm_PowerDomainOps *DomainOps; /**< house cleaning operations */
	u16 InitMask; /**< Mask to indicate house cleaning functions present */
	u16 InitFlag; /**< Flag to indicate house cleaning functions performed */
};

/************************** Function Prototypes ******************************/
XStatus XPmPowerDomain_Init(XPm_PowerDomain *PowerDomain, u32 Id,
			    u32 BaseAddress, XPm_Power *Parent,
			    struct XPm_PowerDomainOps *Ops);
XStatus XPm_PowerUpLPD(XPm_Node *Node);
XStatus XPm_PowerDwnLPD(void);
XStatus XPm_PowerUpFPD(XPm_Node *Node);
XStatus XPm_PowerDwnFPD(XPm_Node *Node);
XStatus XPm_PowerUpPLD(XPm_Node *Node);
XStatus XPm_PowerDwnPLD(void);
XStatus XPm_PowerUpME(XPm_Node *Node);
XStatus XPm_PowerDwnME(void);
XStatus XPm_PowerUpCPM(XPm_Node *Node);
XStatus XPm_PowerDwnCPM(void);
XStatus XPm_PowerUpNoC(XPm_Node *Node);
XStatus XPm_PowerDwnNoC(void);
XStatus XPmPowerDomain_InitDomain(XPm_PowerDomain *PwrDomain, u32 Function,
				  u32 *Args, u32 NumArgs);
XStatus XPmPower_CheckPower(u32 VoltageRailMask);
XStatus XPmPowerDomain_ApplyAmsTrim(u32 DestAddress, u32 PowerDomainId, u32 SateliteIdx);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_POWERDOMAIN_H_ */
