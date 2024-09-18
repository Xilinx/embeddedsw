/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_POWERDOMAIN_H_
#define XPM_POWERDOMAIN_H_

#include "xpm_power.h"
#include "xpm_reset.h"
#include "xpm_domain_iso.h"
#include "xpm_rail.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XPm_PowerDomain XPm_PowerDomain;

#ifdef CPPUTEST
#define XPM_POLL_TIMEOUT			(0X100U)
#else
#define XPM_POLL_TIMEOUT			(0X1000000U)
#endif
#define XPM_SLD_POLL_TIMEOUT		(5000U)	/* Secure lock down poll time out */
#define XPM_DOMAIN_INIT_STATUS_REG		PMC_GLOBAL_PERS_GLOB_GEN_STORAGE0
#define MAX_POWERDOMAINS			6U
#define MAX_DOMAIN_CONTROL_MODES	2U

/*Return success to null function*/
#define XPM_HOUSECLEAN(PwrDomainNode, Func, Args, Num_Args, OutStatus) {\
	if (PwrDomainNode->DomainOps->Func != NULL) { \
		OutStatus = PwrDomainNode->DomainOps->Func(PwrDomainNode, Args, Num_Args);\
	} else {\
		OutStatus = XST_SUCCESS;\
	}\
}

#define IS_SECLOCKDOWN(SecLockDownInfo) ((SecLockDownInfo) & 0x1U)

maybe_unused static inline u32 GetPollTimeOut(u32 SecLockDownInfo,
	u32 DefaultTimeOut) {
	u32 TimeOut = XPM_SLD_POLL_TIMEOUT;

	if (0U == (SecLockDownInfo & 0x1U)) {
		TimeOut = DefaultTimeOut;
	}

	return TimeOut;
}

maybe_unused static inline u32 GetSecLockDownInfoFromArgs(const u32* Args, u32 NumOfArgs){
	u32 val;

	if ((NULL == Args) || (0U == NumOfArgs)) {
		val = 0U;
		goto done;
	}
	val = Args[0U];

done:
	return val;
}

/**
 * The power domain node class.  This is the base class for all the power domain
 * classes.
 */
struct XPm_PowerDomainOps {
	XStatus (*InitStart)(XPm_PowerDomain *PwrDomain, const u32 *Args, u32 NumOfArgs);
	XStatus (*InitFinish)(const XPm_PowerDomain *PwrDomain, const u32 *Args, u32 NumOfArgs);
	XStatus (*ScanClear)(const XPm_PowerDomain *PwrDomain, const u32 *Args, u32 NumOfArgs);
	XStatus (*Mbist)(const XPm_PowerDomain *PwrDomain, const u32 *Args, u32 NumOfArgs);
	XStatus (*Lbist)(const XPm_PowerDomain *PwrDomain, const u32 *Args, u32 NumOfArgs);
	XStatus (*Bisr)(const XPm_PowerDomain *PwrDomain, const u32 *Args, u32 NumOfArgs);
	XStatus (*PlHouseclean)(XPm_PowerDomain *PwrDomain, const u32 *Args, u32 NumOfArgs);
	XStatus (*MemInit)(const XPm_PowerDomain *PwrDomain, const u32 *Args, u32 NumOfArgs);
	XStatus (*HcComplete)(const XPm_PowerDomain *PwrDomain, const u32 *Args, u32 NumOfArgs);
	XStatus (*MioFlush)(const XPm_PowerDomain *PwrDomain, const u32 *Args, u32 NumOfArgs);
	const u16 InitMask;	/**< Mask to indicate which Ops are present */
};

struct XPm_PowerDomain {
	XPm_Power Power; /**< Power: Power node base class */
	const struct XPm_PowerDomainOps *DomainOps; /**< house cleaning operations */
	u32 Parents[MAX_POWERDOMAINS]; /**< List of Parent Rail Ids */
	u32 Children[MAX_POWERDOMAINS]; /**< List of depedent children Ids */
	u16 InitFlag; /**< Flag to indicate which Ops are performed */
	u32 HcDisableMask; /**< Mask for skipping housecleaning operations */
};

#ifdef VERSAL_ENABLE_DOMAIN_CONTROL_GPIO
typedef struct {
	u16 Offset;			/* GPIO Register address offset */
	u32 Mask;			/* GPIO pin mask */
	u32 Value;			/* GPIO pin value */
} XPmDomainCtrl_GPIO;

typedef struct {
	XPm_Power Power;
	u32 ParentId;		/* Parent GPIO ID */
	XPmDomainCtrl_GPIO GpioCtrl[MAX_DOMAIN_CONTROL_MODES];
} XPm_DomainCtrl;
#endif

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
XStatus XPm_PowerUpME(const XPm_Node *Node);
XStatus XPm_PowerDwnME(const XPm_Node *Node);
XStatus XPm_PowerUpCPM(const XPm_Node *Node);
XStatus XPm_PowerDwnCPM(const XPm_Node *Node);
XStatus XPm_PowerUpCPM5(const XPm_Node *Node);
XStatus XPm_PowerDwnCPM5(const XPm_Node *Node);
XStatus XPm_PowerUpNoC(XPm_Node *Node);
XStatus XPm_PowerDwnNoC(void);
XStatus XPmPowerDomain_InitDomain(XPm_PowerDomain *PwrDomain, u32 Function,
				  const u32 *Args, u32 NumArgs);
XStatus XPmPower_CheckPower(const XPm_Rail *Rail, u32 VoltageRailMask);
XStatus XPmPower_UpdateRailStats(const XPm_PowerDomain *PwrDomain, u8 State);
XStatus XPmPowerDomain_SecureEfuseTransfer(const u32 NodeId);

#ifdef VERSAL_ENABLE_DOMAIN_CONTROL_GPIO
XStatus XPmDomainCtrl_Init(XPm_DomainCtrl *DomainCtrl, u32 DomainCtrlId, const u32 *Args, u32 NumArgs);
#endif

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_POWERDOMAIN_H_ */
