/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xplmi.h"
#include "xpm_defs.h"
#include "xpm_common.h"
#include "xpm_node.h"
#include "xpm_powerdomain.h"
#include "xpm_pslpdomain.h"
#include "xpm_psfpdomain.h"
#include "xpm_npdomain.h"
#include "xpm_pldomain.h"
#include "xpm_aie.h"
#include "xpm_device.h"

#include "xpm_regs.h"
#include "xpm_api.h"
#include "xpm_debug.h"
#include "xpm_err.h"
#include "xpm_domain_iso.h"
#include "xpm_pmc.h"
#ifdef PLM_ENABLE_STL
#include "xstl_plminterface.h"
#endif

#define SYSMON_CHECK_POWER_TIMEOUT	2000000U

/* If Sysmon low threshold registers are not used they will be programmed to 0.
 * SysMon data uses Modified Floating point Data format. Bits [18:17] indicate
 * the exponenent offset used to calculate the actual voltage from the register
 * reading and is always programmed to 01. We must account for this when
 * reading the sysmon registers even when they are programmed to zero.
 */
#define SYSMON_THRESH_NOT_PROGRAMMED	0x20000U

/*
 * Power rail index map
 */
#define RAILIDX(Idx) \
		((u32)(Idx) - (u32)XPM_NODEIDX_POWER_VCCINT_PMC)

/* Encoded voltage register values used for SysMon threshold comparisons */
#define VOLT_CODE_0_66V   0x2547AU
#define VOLT_CODE_0_745V  0x25F5CU
#define VOLT_CODE_1_40V   0x2B333U

/* Forward declarations for domain-specific operations */

/* Function pointer typedefs for dispatch mechanism */
typedef XStatus (*XPm_DomainInitStartOp)(XPm_PowerDomain *PwrDomain, const u32 *Args, u32 NumOfArgs);
typedef XStatus (*XPm_DomainInitFinishOp)(const XPm_PowerDomain *PwrDomain, const u32 *Args, u32 NumOfArgs);
typedef XStatus (*XPm_DomainAmsTrimOp)(const XPm_PowerDomain *PwrDomain, const u32 *Args, u32 NumOfArgs);

/* Dispatch functions */
static XPm_DomainInitStartOp XPmPowerDomain_GetInitStartOp(u32 DomainId)
{
	switch (DomainId) {
	case PM_POWER_LPD:
		return LpdInitStart;
	case PM_POWER_FPD:
		return FpdInitStart;
	case PM_POWER_PLD:
		return PldInitStart;
	case PM_POWER_NOC:
		return NpdInitStart;
	case PM_POWER_ME:
	case PM_POWER_ME2:
		return Aie2InitStart;
	default:
		return NULL;
	}
}

static XPm_DomainInitFinishOp XPmPowerDomain_GetInitFinishOp(u32 DomainId)
{
	switch (DomainId) {
	case PM_POWER_LPD:
		return LpdInitFinish;
	case PM_POWER_FPD:
		return FpdInitFinish;
	case PM_POWER_PLD:
		return PldInitFinish;
	case PM_POWER_NOC:
		return NpdInitFinish;
	case PM_POWER_ME:
	case PM_POWER_ME2:
		return Aie2InitFinish;
	default:
		return NULL;
	}
}

static XPm_DomainAmsTrimOp XPmPowerDomain_GetAmsTrimOp(u32 DomainId)
{
	switch (DomainId) {
	case PM_POWER_LPD:
		return LpdAmsTrim;
	case PM_POWER_FPD:
		return FpdAmsTrim;
	case PM_POWER_NOC:
		return NpdAmsTrim;
	case PM_POWER_PLD:
	case PM_POWER_ME:
	case PM_POWER_ME2:
		return NULL; /* These domains don't have AMS trim operations */
	default:
		return NULL;
	}
}

/**
 * @brief 	Helper function to perform PL domain reset actions (assert/de-assert)
 *
 * @param RstNodeId 	Node Id to assert reset for PL Power Domain
 * @param Action	Reset action to be performed (assert/de-assert)
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 */
static inline XStatus PLRstAction(u32 RstNodeId, u32 Action) {
	XStatus Status = XST_FAILURE;
	XPm_ResetNode *Rst = XPmReset_GetById(RstNodeId);
	u32 ControlReg = 0x0UL;
	u32 Mask = 0x0UL;

	if (NULL == Rst) {
		goto done;
	}

	if (XPM_RST_STATE_ASSERTED == Action) {
		/* extract base address and register mask */
		ControlReg = Rst->Node.BaseAddress;
		Mask = BITNMASK(Rst->Shift, Rst->Width);

		/* assert by writing to hw register */
		XPm_RMW32(ControlReg, Mask, Mask);
		Rst->Node.State = XPM_RST_STATE_ASSERTED;
	}
	else {
		Rst->Node.State = XPM_RST_STATE_DEASSERTED;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmPowerDomain_Init(XPm_PowerDomain *PowerDomain, u32 Id,
			    u32 BaseAddress, XPm_Power *Parent)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Status = XPmPower_Init(&PowerDomain->Power, Id, BaseAddress, Parent);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_POWER_DOMAIN_INIT;
		goto done;
	}

	if (NULL != Parent) {
		PowerDomain->Parents[0] = Parent->Node.Id;
	}

#if defined(XILPM_NG_DOMAIN_CONTROL_GPIO)
	/**
	 * Initialize the DomainCtrl pointer to NULL. This will be updated later (via board topology data)
	*/
	PowerDomain->DomainCtrl = NULL;
#endif

	/*TBD: Set houseclean disable mask to default */

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPmPowerDomain_AddParent(u32 Id, const u32 *ParentNodes, u32 NumParents)
{
	XStatus Status = XST_FAILURE;
	XPm_PowerDomain *PowerD;
	XPm_PowerDomain *ParentPowerD;
	u32 i, j;

	PowerD = (XPm_PowerDomain *)XPmPower_GetById(Id);

	if (NULL == PowerD) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	if ((MAX_POWERDOMAINS - 1U) < NumParents) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	for (i = 0; i < NumParents; i++) {
		PowerD->Parents[i + 1U] = ParentNodes[i];

		/* Add Id as child of each parent node */
		ParentPowerD = (XPm_PowerDomain *)XPmPower_GetById(ParentNodes[i]);
		if (NULL == ParentPowerD) {
			Status = XPM_PM_INVALID_NODE;
			goto done;
		}

		for (j = 0U; j < MAX_POWERDOMAINS; j++) {
			if (ParentPowerD->Children[j] != 0U) {
				continue;
			}

			ParentPowerD->Children[j] = Id;
			break;
		}

		if (MAX_POWERDOMAINS == j) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPm_PowerUpLPD(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;

	/* TODO: Add Power up sequence for LPD */
	(void)Node;
	Status = XST_SUCCESS;

	return Status;
}

XStatus XPm_PowerDwnLPD(void)
{
	XStatus Status = XST_FAILURE;

	/* TODO: Add Power down sequence for LPD */
	Status = XST_SUCCESS;

	return Status;
}

XStatus XPm_PowerUpFPD(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;

	/* TODO: Add Power up sequence for FPD */
	(void)Node;
	Status = XST_SUCCESS;

	return Status;
}

XStatus XPm_PowerDwnFPD(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;

	/* TODO: Add Power down sequence for FPD */
	(void)Node;
	Status = XST_SUCCESS;

	return Status;
}

XStatus XPm_PowerUpPLD(XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_PlDomain *PldDomain = NULL;

	/**
	 * Perform the PLD power up sequence
	 *
	 * - CDO takes care of the sequence,
	 * 	only perform HW PowerUp
	*/

	if (NULL == Node) {
		DbgErr = XPM_INT_ERR_INVALID_NODE;
		Status = XST_FAILURE;
		goto done;
	}

	PldDomain = (XPm_PlDomain *)XPmPower_GetById(Node->Id);
	if (NULL == PldDomain) {
		Status = XPM_INVALID_PWRDOMAIN;
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		goto done;
	}

	Status = XPmPower_UpdateDomainPower(&PldDomain->Domain, XPM_POWER_STATE_ON);

	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PLD_RAIL_CONTROL;
		goto done;
	}

	/**
	 * change Reset Node change to de-asserted for software sync
	 * ( actual reset de-assertion is done by CDO )
	*/
	Status = PLRstAction(PM_RST_PL_POR, XPM_RST_STATE_DEASSERTED);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_RST_RELEASE;
		goto done;
	}
	Status = PLRstAction(PM_RST_PL_SRST, XPM_RST_STATE_DEASSERTED);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_RST_RELEASE;
		goto done;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPm_PowerDwnPLD(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_PlDomain *PldDomain = NULL;
	const XPm_Pmc *Pmc = NULL;

	if (NULL == Node) {
		DbgErr = XPM_INT_ERR_INVALID_NODE;
		Status = XST_FAILURE;
		goto done;
	}

	PldDomain = (XPm_PlDomain *)XPmPower_GetById(Node->Id);
	Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);

	if (NULL == Pmc) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (NULL == PldDomain) {
		Status = XST_INVALID_PARAM;
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		goto done;
	}

	/* disable Write Protections */
	PmOut32(CRP_BASEADDR + CRP_WPROT_OFFSET, 0U);

	/* Unset CFG_POR_CNT_SKIP to enable PL_POR counting */
	PmOut32(Pmc->PmcAnalogBaseAddr + PMC_ANLG_CFG_POR_CNT_SKIP_OFFSET, 0U);

	/* Isolate: SOC-PL */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PL_SOC, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PL_SOC_ISO;
		goto done;
	}
	/* Isolate: FPD-PL */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_FPD_PL, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_FPD_PL_ISO;
		goto done;
	}
	/* Isolate: FPD_TEST-PL */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_FPD_PL_TEST, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_FPD_PL_TEST_ISO;
		goto done;
	}
	/* Isolate: LPD-PL */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_LPD_PL, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_LPD_PL_ISO;
		goto done;
	}
	/* Isolate: LPD_TEST-PL */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_LPD_PL_TEST, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_LPD_PL_TEST_ISO;
		goto done;
	}
	/* Isolate: PMC-PL */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_PL, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PMC_PL_ISO;
		goto done;
	}
	/* Isolate: PMC_TEST-PL */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_PL_TEST, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PMC_PL_TEST_ISO;
		goto done;
	}
	/* Isolate: PMC_CFRAME-PL */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_PL_CFRAME, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PMC_PL_CFRAME_ISO;
		goto done;
	}
	/* Isolate: VCCAUX-PL */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_VCCAUX_VCCRAM, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_VCCAUX_VCCRAM_ISO;
		goto done;
	}
	/* Isolate: VCCSOC-PL */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_VCCRAM_SOC, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_VCCRAM_SOC_ISO;
		goto done;
	}

	/* Extract needed logic from XPmReset_AssertbyId() to boot module */
	/* Assert POR PL */
	Status = PLRstAction(PM_RST_PL_POR, XPM_RST_STATE_ASSERTED);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_RST_ASSERT;
		goto done;
	}
	/* Assert SRST PL */
	Status = PLRstAction(PM_RST_PL_SRST, XPM_RST_STATE_ASSERTED);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_RST_ASSERT;
		goto done;
	}

	/* Power down PLD power rail */
	Status = XPmPower_UpdateDomainPower(&PldDomain->Domain, XPM_POWER_STATE_OFF);

	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PLD_RAIL_CONTROL;
		goto done;
	}

done:
	/* enable Write Protections */
	PmOut32(CRP_BASEADDR + CRP_WPROT_OFFSET, 1U);

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPm_PowerUpCPM5N(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;

	/* TODO: Add Power up sequence for CPM5N */
	(void)Node;
	Status = XST_SUCCESS;

	return Status;
}

XStatus XPm_PowerDwnCPM5N(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;

	/* TODO: Add Power down sequence for CPM5N */
	(void)Node;
	Status = XST_SUCCESS;

	return Status;
}

XStatus XPm_PowerUpNoC(XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;

	/* TODO: Add Power up sequence for NOC */
	(void)Node;
	Status = XST_SUCCESS;

        return Status;
}

XStatus XPm_PowerDwnNoC(void)
{
	XStatus Status = XST_FAILURE;

	/* TODO: Add Power down sequence for NOC */
	Status = XST_SUCCESS;

	return Status;
}


/****************************************************************************/
/**
 * @brief This function is used if SysMon lower threshold registers are not
 *        programmed. Hardcoded minimum voltage values or EFUSE are used.
 *
 * @param  Rail: Pointer to power rail node
 * @param  RailVoltage: Current Sysmon voltage reading
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note If the lower threshold registers are programmed the PDI will be device
 *       dependent. Errors are returned to indicate mismatch in device and boot
 *       image.
 *****************************************************************************/
static XStatus SysmonVoltageCheck(const XPm_Rail *Rail, u32 RailVoltage)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 NodeIndex, RailVoltageTableIndex;
	u32 LowThreshVal;

	/**
	 * Hardcoded voltages used when sysmon lower threshold values are not used.
	 * Second array element is placeholder for when EFUSE is blown.
	 */
	const u32 RailVoltageTable[8][2] = {
		[RAILIDX(XPM_NODEIDX_POWER_VCCINT_PMC)] = {VOLT_CODE_0_66V,  0U}, /* 0.66V */
		[RAILIDX(XPM_NODEIDX_POWER_VCCAUX_PMC)] = {VOLT_CODE_1_40V,  0U}, /* 1.40V */
		[RAILIDX(XPM_NODEIDX_POWER_VCCINT_PSLP)] = {VOLT_CODE_0_66V, 0U},  /* 0.66V */
		[RAILIDX(XPM_NODEIDX_POWER_VCCINT_PSFP)] = {VOLT_CODE_0_66V, 0U},  /* 0.66V */
		[RAILIDX(XPM_NODEIDX_POWER_VCCINT_SOC)] = {VOLT_CODE_0_745V, 0U},  /* 0.745V */
		[RAILIDX(XPM_NODEIDX_POWER_VCCINT_RAM)] = {VOLT_CODE_0_745V, 0U},  /* 0.745V */
		[RAILIDX(XPM_NODEIDX_POWER_VCCAUX)]      = {VOLT_CODE_1_40V,  0U}, /* 1.40V */
		[RAILIDX(XPM_NODEIDX_POWER_VCCINT_PL)]   = {VOLT_CODE_0_66V,  0U}, /* 0.66V */
	};

	NodeIndex = NODEINDEX(Rail->Power.Node.Id);
	RailVoltageTableIndex = RAILIDX(NodeIndex);
	if (RailVoltageTableIndex < ARRAY_SIZE(RailVoltageTable)) {
		LowThreshVal = RailVoltageTable[RailVoltageTableIndex][0];
	} else if (NODEINDEX(XPM_NODEIDX_POWER_VCCINT_ME) == NodeIndex) {
		LowThreshVal = VOLT_CODE_0_66V; /* 0.66V */
	} else {
		DbgErr = XPM_INT_ERR_DEVICE_NOT_SUPPORTED;
		Status = XPM_ERR_RAIL_VOLTAGE;
		goto done;
	}

	/**
	 * Check if current rail voltage reading is below the required minimum
	 * voltage for proper operation.
	 */
	if (RailVoltage < LowThreshVal) {
		DbgErr = XPM_INT_ERR_POWER_SUPPLY;
		Status = XPM_ERR_RAIL_VOLTAGE;
		goto done;
	}

	Status = XST_SUCCESS;

done:
	if (XST_SUCCESS != Status) {
		PmDbg("0x%x\r\n", DbgErr);
	}

	return Status;
}


/****************************************************************************/
/**
 * @brief  Check power rail if minimum operational voltage has been reached
 *		   using Sysmon
 *
 * @param  Rail: Pointer to power rail node
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 ****************************************************************************/
XStatus XPmPower_SysmonCheckPower(const XPm_Rail *Rail)
{
	volatile XStatus Status = XST_FAILURE;
	volatile XStatus StatusTmp = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 RailVoltage = 0;
	u32 LowThreshVal = 0;

	/**
	 * Index is stored as the BaseAddress and is used to calculate the SysMon
	 * SUPPLYn and NewDataFlag registers
	 */
	u32 Index = Rail->Power.Node.BaseAddress;
	u32 SysmonSupplyReg = (u32)PMC_SYSMON_SUPPLY0 + (Index * 4U);
	u32 SysmonLowThReg = (u32)PMC_SYSMON_SUPPLY0_TH_LOWER + (Index * 4U);
	u32 NewDataFlagReg = (u32)PMC_SYSMON_NEW_DATA_FLAG0 + ((Index/32U) * 4U);
	u32 Offset = Index % 32U;

	/* Wait for New Data Flag */
	Status = XPm_PollForMask(NewDataFlagReg, BIT32(Offset),
								SYSMON_CHECK_POWER_TIMEOUT);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_NEW_DATA_FLAG_TIMEOUT;
		Status = XPM_ERR_NEW_DATA_FLAG_TIMEOUT;
		goto done;
	}

	PmIn32(SysmonLowThReg, LowThreshVal);
	PmIn32(SysmonSupplyReg, RailVoltage);

	/* If lower threshold values are not programmed, use hardcoded voltages */
	if (SYSMON_THRESH_NOT_PROGRAMMED == LowThreshVal) {
		PmDbg("Using hardcoded voltages for power rail checks\r\n");
		XSECURE_REDUNDANT_CALL(Status, StatusTmp, SysmonVoltageCheck, Rail, RailVoltage);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			DbgErr = XPM_INT_ERR_POWER_SUPPLY;
			goto done;
		}
	} else {
		PmDbg("Using Sysmon lower threshold voltages for power rail checks\r\n");
		if (RailVoltage < LowThreshVal) {
			DbgErr = XPM_INT_ERR_POWER_SUPPLY;
			Status = XPM_ERR_RAIL_VOLTAGE;
		}
	}

	/* Unlock Root SysMon registers */
	XPm_UnlockPcsr(PMC_SYSMON_BASEADDR);

	/* Clear New Data Flag */
	PmOut32(NewDataFlagReg, BIT32(Offset));

	/* Lock Root SysMon registers */
	XPm_LockPcsr(PMC_SYSMON_BASEADDR);

done:
	if (XST_SUCCESS != Status) {
		PmDbg("0x%x\r\n", DbgErr);
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief  Check power rail power using power detectors. This check uses the
 *		   PMC_GLOBAL_PWR_SUPPLY_STATUS registers
 *
 * @param  VoltageRailMask: Mask of PMC_GLOBAL_PWR_SUPPLY_STATUS registers for
 *		   rails to be checked
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note This function uses power detectors which are not very reliable. It
 *		 should only be used when no other methods are available i.e. sysmon
 *
 ****************************************************************************/
static XStatus XPmPower_DetectorCheckPower(u32 VoltageRailMask)
{
	XStatus Status = XST_FAILURE;
	u32 RegVal;
	const XPm_Pmc *Pmc;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);
	if (NULL == Pmc) {
		Status = XST_SUCCESS;
		goto done;
	}

	PmIn32(Pmc->PmcGlobalBaseAddr + PWR_SUPPLY_STATUS_OFFSET, RegVal);
	if((RegVal & VoltageRailMask) != VoltageRailMask) {
		DbgErr = XPM_INT_ERR_POWER_SUPPLY;
		goto done;
	}

	Status = XST_SUCCESS;

done:
	if (XST_SUCCESS != Status) {
		PmDbg("0x%x\r\n", DbgErr);
	}

	return Status;

}


XStatus XPmPower_CheckPower(const XPm_Rail *Rail, u32 VoltageRailMask)
{
	XStatus Status = XST_FAILURE;
	u32 Source;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 Platform = XPm_GetPlatform();

	if ((NULL == Rail) || (XPM_PGOOD_SYSMON != Rail->Source) ||
	    (PLATFORM_VERSION_QEMU == Platform) || (PLATFORM_VERSION_COSIM == Platform)) {
		Status = XPmPower_DetectorCheckPower(VoltageRailMask);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_POWER_SUPPLY;
		}
		goto done;
	}

	Source = (u32)Rail->Source;
	if ((u32)XPM_PGOOD_SYSMON == Source) {
		Status = XPmPower_SysmonCheckPower(Rail);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_POWER_SUPPLY;
		}
	} else {
		DbgErr = XPM_INT_ERR_RAIL_SOURCE;
	}

done:
	if (XST_SUCCESS != Status) {
		PmDbg("0x%x\r\n", DbgErr);
	}

	return Status;
}

XStatus XPmPowerDomain_InitDomain(XPm_PowerDomain *PwrDomain, u32 Function,
				const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 DomainId = PwrDomain->Power.Node.Id;

	switch (Function) {
	case (u32)FUNC_INIT_START:
		PwrDomain->Power.Node.State = (u8)XPM_POWER_STATE_INITIALIZING;
		Status = XPmPower_UpdateDomainPower(PwrDomain, XPM_POWER_STATE_ON);

		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_PWR_DOMAIN_RAIL_CONTROL;
			goto done;
		}

		XPm_DomainInitStartOp InitStartOp = XPmPowerDomain_GetInitStartOp(DomainId);
		if (NULL != InitStartOp) {
			Status = InitStartOp(PwrDomain, Args, NumArgs);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_FUNC_INIT_START;
				goto done;
			}
		}

		Status = XST_SUCCESS;
		break;
	case (u32)FUNC_INIT_FINISH:
#ifdef PLM_ENABLE_STL
		(void)XStl_PlmStartupPreCdoTask(PwrDomain->Power.Node.Id);
#endif
		if ((u8)XPM_POWER_STATE_INITIALIZING != PwrDomain->Power.Node.State) {
			DbgErr = XPM_INT_ERR_INVALID_PWR_STATE;
			PmWarn("[INIT_FINISH]Skip. PwrDomain 0x%X is in wrong state 0x%X\n\r", \
				PwrDomain->Power.Node.Id, PwrDomain->Power.Node.State);
			Status = XST_SUCCESS;
			goto done;
		}
		XPm_DomainInitFinishOp InitFinishOp = XPmPowerDomain_GetInitFinishOp(DomainId);
		if (NULL != InitFinishOp) {
			Status = InitFinishOp(PwrDomain, Args, NumArgs);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_FUNC_INIT_FINISH;
				goto done;
			}
			PwrDomain->Power.Node.State = (u8)XPM_POWER_STATE_ON;
		}

		Status = XPmDomainIso_ProcessPending();
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_DOMAIN_ISO;
			goto done;
		}
		Status = XST_SUCCESS;
		break;
	case (u32)FUNC_AMS_TRIM:
		XPm_DomainAmsTrimOp AmsTrimOp = XPmPowerDomain_GetAmsTrimOp(DomainId);
		if (NULL != AmsTrimOp) {
			Status = AmsTrimOp(PwrDomain, Args, NumArgs);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_FUNC_AMS_TRIM;
			}
		} else {
			Status = XST_SUCCESS;
		}
		break;
	default:
		DbgErr = XPM_INT_ERR_INVALID_FUNC;
		Status = XST_INVALID_PARAM;
		break;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/**
 * @brief 	Turn ON/OFF Power Domain using GPIO Domain Control
 *
 * @param PwrDomain 	Pointer to the power domain to be updated
 * @param State 	Desired power state of the domain (ON/OFF)
 *
 * @return XST_SUCCESS if the power domain was successfully updated, otherwise an error code
*/
XStatus XPmPower_UpdateDomainPower(const XPm_PowerDomain *PwrDomain, u32 State) {
	XStatus Status = XST_FAILURE;

	(void)State;

	if (NULL == PwrDomain) {
		Status = XPM_INVALID_PWRDOMAIN;
		goto done;
	}

	/* Node must be a power domain */
	if ((u32)XPM_NODESUBCL_POWER_DOMAIN != NODESUBCLASS(PwrDomain->Power.Node.Id)) {
		PmErr("Invalid Power Node: 0x%x\r\n", PwrDomain->Power.Node.Id);
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/**
	 * Turn ON/OFF the given Power Domain on the following conditions:
	 * - Power Down if State == XPM_POWER_STATE_OFF
	 * - Power Up if State == XPM_POWER_STATE_ON
	 *
	 * Note: Powering Up and Powering Down of a Power Domain is done using GPIO Control (external HW sequencer)
	*/

#if defined(XILPM_NG_DOMAIN_CONTROL_GPIO)
	/* check whether the domain control parent node exists or not */
	if (NULL != PwrDomain->DomainCtrl) {
		Status = XPmDomainCtrl_Control(PwrDomain, State);
		goto done;
	}
	else {
		/* Domain control parent node does not exist, skip GPIO control and return success */
		Status = XST_SUCCESS;
	}
#else
	Status = XST_SUCCESS;
#endif

done:
	return Status;
}
