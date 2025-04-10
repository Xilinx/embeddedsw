/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xplmi.h"
#include "xpm_defs.h"
#include "xpm_common.h"
#include "xpm_node.h"
#include "xpm_psm.h"
#include "xpm_powerdomain.h"
#include "xpm_pslpdomain.h"
#include "xpm_psfpdomain.h"
#include "xpm_bisr.h"
#include "xpm_device.h"
#include "xpm_regs.h"
#include "xpm_api.h"
#include "xpm_debug.h"
#include "xpm_err.h"
#include "xpm_domain_iso.h"
#include "xpm_pmc.h"

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
XStatus XPmPowerDomain_Init(XPm_PowerDomain *PowerDomain, u32 Id,
			    u32 BaseAddress, XPm_Power *Parent,
			    const struct XPm_PowerDomainOps *Ops)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Status = XPmPower_Init(&PowerDomain->Power, Id, BaseAddress, Parent);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_POWER_DOMAIN_INIT;
		goto done;
	}

	PowerDomain->DomainOps = Ops;
	if (NULL != Parent) {
		PowerDomain->Parents[0] = Parent->Node.Id;
	}

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

	/* TODO: Add Power up sequence for PLD */
	(void)Node;
	Status = XST_SUCCESS;

	return Status;
}

XStatus XPm_PowerDwnPLD(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	(void)Node;

	const XPm_Pmc *Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);
	if (NULL == Pmc) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		Status = XST_FAILURE;
		goto done;
	}

	/* Unset CFG_POR_CNT_SKIP to enable PL_POR counting */
	PmOut32(Pmc->PmcAnalogBaseAddr + PMC_ANLG_CFG_POR_CNT_SKIP_OFFSET,
		0U);
	Status = XST_SUCCESS;

done:
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

XStatus XPm_PowerDwnHnicx(void)
{
	XStatus Status = XST_FAILURE;

	/* TODO: Add Power down sequence for HNICX */
	Status = XST_SUCCESS;

	return Status;
}

XStatus XPm_PowerUpHnicx(void)
{
	XStatus Status = XST_FAILURE;

	/* TODO: Add Power up sequence for HNICX */
	Status = XST_SUCCESS;

	return Status;
}

/****************************************************************************/
/**
 * @brief This function is used if SysMon lower threshold registers are not
 *	  programmed. Hardcoded minimum voltage values or EFUSE are used.
 *
 * @param  Rail: Pointer to power rail node
 * @param  RailVoltage: Current Sysmon voltage reading
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note If the lower threshold registers are programmed the PDI will be device
 *	 dependent. Errors are returned to indicate mismatch in device and boot
 *	 image.
 *****************************************************************************/
static XStatus SysmonVoltageCheck(const XPm_Rail *Rail, u32 RailVoltage)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 NodeIndex;

	/**
	 * Hardcoded voltages used when sysmon lower threshold values are not used.
	 * Second array element is placeholder for when EFUSE is blown.
	 */
	const u32 RailVoltageTable[8][2] = {
		[RAILIDX(XPM_NODEIDX_POWER_VCCINT_PMC)] = {0x2547AU, 0U},   /* 0.66V */
		[RAILIDX(XPM_NODEIDX_POWER_VCCAUX_PMC)] = {0x2b333U, 0U},   /* 1.4V */
		[RAILIDX(XPM_NODEIDX_POWER_VCCINT_PSLP)] = {0x2547AU, 0U},  /* 0.66V */
		[RAILIDX(XPM_NODEIDX_POWER_VCCINT_PSFP)] = {0x2547AU, 0U},  /* 0.66V */
		[RAILIDX(XPM_NODEIDX_POWER_VCCINT_SOC)] = {0x25F5CU, 0U},   /* 0.745V */
		[RAILIDX(XPM_NODEIDX_POWER_VCCINT_RAM)] = {0x25F5CU, 0U},   /* 0.745V */
		[RAILIDX(XPM_NODEIDX_POWER_VCCAUX)] = {0x2b333U, 0U},	    /* 1.4V */
		[RAILIDX(XPM_NODEIDX_POWER_VCCINT_PL)] = {0x2547AU, 0U},    /* 0.66V */
	};

	NodeIndex = NODEINDEX(Rail->Power.Node.Id);

	/**
	 * Check if current rail voltage reading is below the required minimum
	 * voltage for proper operation.
	 */
	if (RailVoltage < RailVoltageTable[RAILIDX(NodeIndex)][0]) {
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
static XStatus XPmPower_SysmonCheckPower(const XPm_Rail *Rail)
{
	XStatus Status = XST_FAILURE;
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
		Status = SysmonVoltageCheck(Rail, RailVoltage);
		if (XST_SUCCESS != Status) {
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
	const struct XPm_PowerDomainOps *Ops = PwrDomain->DomainOps;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	switch (Function) {
	case (u32)FUNC_INIT_START:
		PwrDomain->Power.Node.State = (u8)XPM_POWER_STATE_INITIALIZING;
		Status = XPmPower_UpdateRailStats(PwrDomain,
						  (u8)XPM_POWER_STATE_ON);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_PWR_DOMAIN_RAIL_CONTROL;
			goto done;
		}

		if ((NULL != Ops) && (NULL != Ops->InitStart)) {
			Status = Ops->InitStart(PwrDomain, Args, NumArgs);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_FUNC_INIT_START;
				goto done;
			}
		}

		Status = XST_SUCCESS;
		break;
	case (u32)FUNC_INIT_FINISH:
		if ((u8)XPM_POWER_STATE_INITIALIZING != PwrDomain->Power.Node.State) {
			DbgErr = XPM_INT_ERR_INVALID_PWR_STATE;
			PmWarn("[INIT_FINISH]Skip. PwrDomain 0x%X is in wrong state 0x%X\n\r", \
				PwrDomain->Power.Node.Id, PwrDomain->Power.Node.State);
			Status = XST_SUCCESS;
			goto done;
		}
		if ((NULL != Ops) && (NULL != Ops->InitFinish)) {
			Status = Ops->InitFinish(PwrDomain, Args, NumArgs);
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
		if (NULL != Ops->TrimAms) {
			Status = Ops->TrimAms(PwrDomain, Args, NumArgs);
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

XStatus XPmPower_UpdateRailStats(const XPm_PowerDomain *PwrDomain, u8 State)
{
	XStatus Status = XST_FAILURE;
	u32 i=0, j=0;
	const XPm_PowerDomain *ParentDomain;
	XPm_Rail *ParentRail;

	/* Update rail node usecounts */
	for (i = 0; ((i < MAX_POWERDOMAINS) && (0U != PwrDomain->Parents[i])); i++) {
		/* If power domain is the parent, scan through its rails */
		if ((u32)XPM_NODESUBCL_POWER_DOMAIN == NODESUBCLASS(PwrDomain->Parents[i])) {
			ParentDomain = (XPm_PowerDomain *)XPmPower_GetById(PwrDomain->Parents[i]);
			for (j = 0; ((j < MAX_POWERDOMAINS) && (0U != ParentDomain->Parents[j])); j++) {
				if ((u32)XPM_NODESUBCL_POWER_RAIL == NODESUBCLASS(ParentDomain->Parents[j])) {
					ParentRail = (XPm_Rail *)XPmPower_GetById(ParentDomain->Parents[j]);
					if ((u8)XPM_POWER_STATE_ON == State) {
						ParentRail->Power.UseCount++;
					} else {
						ParentRail->Power.UseCount--;
					}
				}
			}
		} else if ((u32)XPM_NODESUBCL_POWER_RAIL == NODESUBCLASS(PwrDomain->Parents[i])) {
			ParentRail = (XPm_Rail *)XPmPower_GetById(PwrDomain->Parents[i]);
			if ((u8)XPM_POWER_STATE_ON == State) {
				if (PM_POWER_PMC == PwrDomain->Power.Node.Id) {
					ParentRail->Power.Node.State = (u8)XPM_POWER_STATE_ON;
				} else if ((u8)XPM_POWER_STATE_ON != ParentRail->Power.Node.State) {
					PmDbg("Turning %x rail on now\r\n", ParentRail->Power.Node.Id);
					Status = XPmRail_Control(ParentRail,
								(u8)XPM_POWER_STATE_ON, 1U);
					if (XST_SUCCESS != Status) {
						goto done;
					}
				} else {
					/* Required by MISRA */
				}
				ParentRail->Power.UseCount++;
			} else {
				ParentRail->Power.UseCount--;
				if (ParentRail->Power.UseCount <= 0U) {
					PmDbg("Turning %x rail off now\r\n", ParentRail->Power.Node.Id);
					Status = XPmRail_Control(ParentRail,
								(u8)XPM_POWER_STATE_OFF, 0U);
					if (XST_SUCCESS != Status) {
						goto done;
					}
				}
			}
		} else {
			PmDbg("Power parent error.\r\n");
			Status = XST_FAILURE;
			goto done;
		}
	}

	Status = XST_SUCCESS;
done:
	return Status;
}
