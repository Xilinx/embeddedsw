/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_common.h"
#include "xpm_cpmdomain.h"
#include "xpm_regs.h"
#include "xpm_bisr.h"
#include "xpm_power.h"
#include "sleep.h"
#include "xpm_device.h"
#include "xpm_debug.h"
#include "xpm_pldomain.h"
#include "xpm_psm_api.h"
#include "xpm_rail.h"
#include "xpm_api.h"
#include "xpm_ams_trim.h"

static XStatus Cpm5nInitStart(XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XPM_ERR_INIT_START;
	XStatus PslpRailPwrSts = XST_FAILURE;
	XStatus IntRailPwrSts = XST_FAILURE;
	XStatus AuxRailPwrSts = XST_FAILURE;
	XStatus RamRailPwrSts = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;

	const XPm_Rail *VccintPslpRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCINT_PSLP);
	const XPm_Rail *VccintRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCINT_PL);
	const XPm_Rail *VccauxRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCAUX);
	const XPm_Rail *VccRamRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCINT_RAM);
	const XPm_Rail *VccCpm5nRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCINT_CPM5N);

	/* Check LPD and PL power rails first to make sure power is on */
	PslpRailPwrSts = XPmPower_CheckPower(VccintPslpRail,
			PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_LPD_MASK);
	IntRailPwrSts = XPmPower_CheckPower(VccintRail,
			PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_PL_MASK);
	AuxRailPwrSts =  XPmPower_CheckPower(VccauxRail,
			PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCAUX_MASK);
	RamRailPwrSts =  XPmPower_CheckPower(VccRamRail,
			PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_RAM_MASK);

	if ((XST_SUCCESS != PslpRailPwrSts) || (XST_SUCCESS != IntRailPwrSts) ||
	    (XST_SUCCESS != AuxRailPwrSts) || (XST_SUCCESS != RamRailPwrSts)) {
		DbgErr = XPM_INT_ERR_INVALID_PWR_STATE;
		goto done;
	}

	/* Perform VID adjustment */
	Status = XPmRail_AdjustVID((XPm_Rail *)VccintRail);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_VID_ADJUST;
		goto done;
	}

	Status = XPmRail_AdjustVID((XPm_Rail *)VccCpm5nRail);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_VID_ADJUST;
		goto done;
	}

	Status = XST_SUCCESS;
done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus Cpm5nInitFinish(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;
	Status = XST_SUCCESS;
	return Status;
}

static XStatus CpmAmsTrim(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 SysmonAddr;

	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;

	/* Copy CPM5N sysmon data */
	SysmonAddr = XPm_GetSysmonByIndex(XPM_NODEIDX_MONITOR_SYSMON_CPM5N);
	if (0U == SysmonAddr) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		PmWarn("Warning Device XPM_NODEIDX_MONITOR_SYSMON_CPM5N doest not exist");
		Status = XST_SUCCESS;
		goto done;
	}
	Status = XPm_ApplyAmsTrim(SysmonAddr, PM_POWER_CPM5N, 0);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_AMS_TRIM;
		goto done;
	}
done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static const struct XPm_PowerDomainOps Cpm5nOps = {
		.InitStart = Cpm5nInitStart,
		.InitFinish = Cpm5nInitFinish,
		.TrimAms = CpmAmsTrim
};

XStatus XPmCpmDomain_Init(XPm_CpmDomain *CpmDomain, u32 Id, u32 BaseAddress,
			  XPm_Power *Parent, const u32 *OtherBaseAddresses,
			  u32 OtherBaseAddressesCnt)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Status = XPmPowerDomain_Init(&CpmDomain->Domain, Id, BaseAddress, Parent, &Cpm5nOps);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_POWER_DOMAIN_INIT;
		goto done;
	}

	/* Make sure enough base addresses are being passed */
	if (4U <= OtherBaseAddressesCnt) {
		CpmDomain->CpmSlcrBaseAddr = OtherBaseAddresses[0];
		CpmDomain->CpmSlcrSecureBaseAddr = OtherBaseAddresses[1];
		CpmDomain->CpmPcsrBaseAddr = OtherBaseAddresses[2];
		CpmDomain->CpmCrCpmBaseAddr = OtherBaseAddresses[3];
		Status = XST_SUCCESS;
	} else {
		DbgErr = XPM_INT_ERR_INVALID_BASEADDR;
		Status = XST_FAILURE;
	}

	/*TBD: Clear CPM section of PMC RAM register reserved for houseclean disable */

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
