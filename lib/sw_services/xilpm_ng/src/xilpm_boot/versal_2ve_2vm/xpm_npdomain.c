/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "sleep.h"
#include "xpm_common.h"
#include "xpm_npdomain.h"
#include "xpm_pslpdomain.h"
#include "xpm_regs.h"
#include "xpm_debug.h"
#include "xpm_device.h"
#include "xpm_rail.h"
#include "xpm_ams_trim.h"

#define XPM_NODEIDX_MONITOR_SYSMON_NPD_MIN	XPM_NODEIDX_MONITOR_SYSMON_NPD_0

XStatus NpdInitStart(XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;

	const XPm_Rail *VccSocRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCINT_SOC);

	/* Check vccint_soc first to make sure power is on */
	Status = XPmPower_CheckPower(VccSocRail, PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_SOC_MASK);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_POWER_SUPPLY;
		goto done;
	}
done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}


XStatus NpdInitFinish(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	XStatus SocRailPwrSts = XST_FAILURE;
	XStatus AuxRailPwrSts = XST_FAILURE;
	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;

	const XPm_Rail *VccSocRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCINT_SOC);
	const XPm_Rail *VccauxRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCAUX);

	SocRailPwrSts = XPmPower_CheckPower(VccSocRail,
				PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_SOC_MASK);
	AuxRailPwrSts = XPmPower_CheckPower(VccauxRail,
				PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCAUX_MASK);
	if ((XST_SUCCESS != SocRailPwrSts) || (XST_SUCCESS != AuxRailPwrSts)) {
		DbgErr = XPM_INT_ERR_POWER_SUPPLY;
		goto done;
	}
	Status = XST_SUCCESS;


done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/****************************************************************************/
/**
 * @brief Apply cached AMS trim data to the NPD system monitors.
 *
 * Iterates over all NPD system-monitor node indices. Existing monitors receive
 * the trim data associated with the NOC power domain; absent monitors are
 * skipped. Processing stops at the first trim-application error.
 *
 * @param PwrDomain Unused power-domain callback argument.
 * @param Args Unused callback argument array.
 * @param NumOfArgs Unused number of callback arguments.
 *
 * @return XST_SUCCESS when the last attempted trim succeeds;
 *         XST_FAILURE if no trim operation establishes success; or the error
 *         returned by XPm_ApplyAmsTrim() when applying trim data fails.
 ****************************************************************************/
XStatus NpdAmsTrim(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs) {
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 SysmonAddr = 0;
	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;

	/* Visit each NPD system monitor that may hold trim data. */
	/* Copy sysmon data */
	for (u32 i = (u32)XPM_NODEIDX_MONITOR_SYSMON_NPD_MIN; i < (u32)XPM_NODEIDX_MONITOR_SYSMON_NPD_MAX; i++) {
		/* Copy_trim< AMS_SAT_N> */
		SysmonAddr = XPm_GetSysmonByIndex(i);
		if (0U != SysmonAddr) {
			Status = XPm_ApplyAmsTrim(SysmonAddr, PM_POWER_NOC, i-(u32)XPM_NODEIDX_MONITOR_SYSMON_NPD_MIN);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_AMS_TRIM;
				goto done;
			}
		}
	}
done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}



XStatus XPmNpDomain_Init(XPm_NpDomain *Npd, u32 Id, u32 BaseAddress,
			 XPm_Power *Parent)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Status = XPmPowerDomain_Init(&Npd->Domain, Id, BaseAddress, Parent);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_POWER_DOMAIN_INIT;
	}

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
