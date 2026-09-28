/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xplmi.h"
#include "xplmi_err.h"
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
#ifdef XILPM_RUNTIME
#include "xpm_gic_proxy.h"
#endif
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

/******************************************************************/
/**
 * @brief  CMN Flushing feature is not supported in BOOT
 *
 * @return Status of the operation: XPM_NO_FEATURE
 *
 * @note  Weak Symbol - Stub implementation for BOOT-only builds. The
 *        parameter is left undocumented here on purpose; the runtime
 *        implementation in xpm_runtime_mem_subsys.c documents it, and
 *        Doxygen merges both into one symbol.
 *
 ******************************************************************/
XStatus __attribute__((weak, noinline)) XPmSubsystem_CmnFlushForCore(u32 PowerDomainId) {
	/* The strong memory-subsystem implementation replaces this weak boot default. */
	XStatus Status = XPM_FAILURE;
	(void)PowerDomainId;

	Status = XPM_NO_FEATURE;
	return Status;
}

/* Dispatch functions */
/****************************************************************************/
/**
 * @brief  Return the init-start operation handler for a power domain.
 *
 * @param  DomainId  Power domain node ID to look up
 *
 * @return Pointer to the matching init-start operation, or NULL when the
 *         domain is unsupported
 *
 ****************************************************************************/
static XPm_DomainInitStartOp XPmPowerDomain_GetInitStartOp(u32 DomainId)
{
	XPm_DomainInitStartOp InitStartOp = NULL;

	/* Select the platform initialization entry point for this domain */
	switch (DomainId) {
	case PM_POWER_LPD:
		InitStartOp = &LpdInitStart;
		break;
	case PM_POWER_FPD:
		InitStartOp = &FpdInitStart;
		break;
	case PM_POWER_PLD:
		InitStartOp = &PldInitStart;
		break;
	case PM_POWER_NOC:
		InitStartOp = &NpdInitStart;
		break;
	case PM_POWER_ME:
	case PM_POWER_ME2:
		InitStartOp = &Aie2InitStart;
		break;
	default:
		/* No operation for unsupported domains */
		break;
	}

	return InitStartOp;
}

/****************************************************************************/
/**
 * @brief  Return the init-finish operation handler for the given power domain.
 *
 * @param  DomainId: Power domain node ID to look up
 *
 * @return Pointer to the matching init-finish operation, or NULL if the
 *         domain is not supported
 *
 ****************************************************************************/
static XPm_DomainInitFinishOp XPmPowerDomain_GetInitFinishOp(u32 DomainId)
{
	XPm_DomainInitFinishOp InitFinishOp = NULL;

	/* Select the platform completion entry point for this domain */
	switch (DomainId) {
	case PM_POWER_LPD:
		InitFinishOp = &LpdInitFinish;
		break;
	case PM_POWER_FPD:
		InitFinishOp = &FpdInitFinish;
		break;
	case PM_POWER_PLD:
		InitFinishOp = &PldInitFinish;
		break;
	case PM_POWER_NOC:
		InitFinishOp = &NpdInitFinish;
		break;
	case PM_POWER_ME:
	case PM_POWER_ME2:
		InitFinishOp = &Aie2InitFinish;
		break;
	default:
		/* No operation for unsupported domains */
		break;
	}

	return InitFinishOp;
}

/****************************************************************************/
/**
 * @brief  Return the AMS trim operation handler for a given power domain.
 *
 * @param  DomainId: Node Id of the power domain
 *
 * @return Pointer to the domain's AMS trim operation, or NULL if the domain
 *         has no AMS trim operation
 *
 ****************************************************************************/
static XPm_DomainAmsTrimOp XPmPowerDomain_GetAmsTrimOp(u32 DomainId)
{
	XPm_DomainAmsTrimOp AmsTrimOp = NULL;

	switch (DomainId) {
	case PM_POWER_LPD:
		AmsTrimOp = &LpdAmsTrim;
		break;
	case PM_POWER_FPD:
		AmsTrimOp = &FpdAmsTrim;
		break;
	case PM_POWER_NOC:
		AmsTrimOp = &NpdAmsTrim;
		break;
	case PM_POWER_PLD:
	case PM_POWER_ME:
	case PM_POWER_ME2:
		/* These domains don't have AMS trim operations */
		break;
	default:
		/* No operation for unsupported domains */
		break;
	}

	return AmsTrimOp;
}

/**
 * @brief 	Helper function to perform power domain reset actions (assert/de-assert)
 *
 * @param RstNodeId 	Node Id to assert reset for Power Domain
 * @param Action	Reset action to be performed (assert/de-assert)
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 */
static inline XStatus PwrDomainRstAction(u32 RstNodeId, u32 Action) {
	XStatus Status = XST_FAILURE;
	XPm_ResetNode *Rst = XPmReset_GetById(RstNodeId);
	u32 ControlReg = 0x0UL;
	u32 Mask = 0x0UL;

	if (NULL == Rst) {
		/*
		 * Reset node not present in this device topology (e.g. the FPD
		 * POR/SRST nodes prior to their addition in versal-topology).
		 * Treat as a no-op so the power transition is not blocked.
		 */
		Status = XST_SUCCESS;
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
	volatile XStatus Status = XST_FAILURE;
	XPm_PowerDomain *PowerD;
	XPm_PowerDomain *ParentPowerD;
	XPm_Power *Parent;
	u32 i, j;

	if ((u32)XPM_NODESUBCL_POWER_DOMAIN != NODESUBCLASS(Id)) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	PowerD = (XPm_PowerDomain *)XPmPower_GetById(Id);

	if (NULL == PowerD) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	if (((MAX_POWERDOMAINS - 1U) < NumParents) ||
	    ((0U != NumParents) && (NULL == ParentNodes))) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	for (i = 0; i < NumParents; i++) {
		Parent = XPmPower_GetById(ParentNodes[i]);
		if (NULL == Parent) {
			Status = XPM_PM_INVALID_NODE;
			goto done;
		}

		PowerD->Parents[i + 1U] = ParentNodes[i];
		if ((u32)XPM_NODESUBCL_POWER_DOMAIN !=
		    NODESUBCLASS(Parent->Node.Id)) {
			continue;
		}

		/* Only power domains have a list of dependent children. */
		ParentPowerD = (XPm_PowerDomain *)Parent;
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

/****************************************************************************/
/**
 * @brief  Perform the hardware power up sequence for the FPD power domain.
 *
 * @param  Node: Pointer to the FPD domain node
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note   The CDO drives the overall FPD resume sequence; this only performs
 *         the hardware power up.
 *
 ****************************************************************************/
XStatus XPm_PowerUpFPD(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_PsFpDomain *FpDomain = NULL;

	/**
	 * Perform the FPD power up sequence
	 *
	 * - CDO takes care of the sequence,
	 * 	only perform HW PowerUp
	*/

	/* Validate the caller-provided node before using its domain identifier */
	if (NULL == Node) {
		DbgErr = XPM_INT_ERR_INVALID_NODE;
		Status = XST_FAILURE;
		goto done;
	}

	/* Resolve the FPD object that owns the rail-control state */
	FpDomain = (XPm_PsFpDomain *)XPmPower_GetById(Node->Id);
	if (NULL == FpDomain) {
		Status = XPM_INVALID_PWRDOMAIN;
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		goto done;
	}

	/* Request the domain rail transition after all lookups have succeeded */
	Status = XPmPower_UpdateDomainPower(&FpDomain->Domain, XPM_POWER_STATE_ON);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_FPD_RAIL_CONTROL;
		goto done;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/**
 * @brief	Arm the GIC proxy so an APU wake interrupt can still be latched
 *		while the FPD power domain is off
 *
 * @return XST_SUCCESS if the proxy was armed or the core is not suspending,
 *	   else XST_FAILURE or error code
 *
 * @note	Bit 0 of ResumeAddr is the valid flag and is only set once the
 *		core has self-suspended. Without it the FPD power down is a
 *		shutdown and there is no wake path to arm.
 */
static XStatus XPm_ApuArmGicProxyWake(void)
{
	XStatus Status = XST_FAILURE;
	const XPm_Core *ApuCore = (XPm_Core *)XPmDevice_GetById(PM_DEV_ACPU_0_0);

	if (NULL == ApuCore) {
		/* No APU in topology: nothing to arm, power-down may still proceed */
		PmWarn("APU core 0x%x not in topology, GIC proxy not armed\r\n",
		       PM_DEV_ACPU_0_0);
		Status = XST_SUCCESS;
		goto done;
	}

	if (0U == (u32)(ApuCore->ResumeAddr & 1ULL)) {
		/* No resume address: treat as shutdown, not suspend-with-wake */
		PmInfo("No APU resume address set, treating FPD off as shutdown\r\n");
		Status = XST_SUCCESS;
		goto done;
	}

#ifdef XILPM_RUNTIME
	/* Core is suspending: arm GIC proxy before FPD rail drops */
	Status = XPmGicProxy_EnableWakeup();
	if (XST_SUCCESS != Status) {
		PmErr("GIC proxy wakeup enable failed, Status=0x%x\r\n", Status);
	}
#else
	/* No GIC proxy in boot-only builds: nothing to arm */
	Status = XST_SUCCESS;
#endif

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Perform the hardware power down sequence for the FPD power domain.
 *
 * @param  Node: Pointer to the FPD domain node
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 ****************************************************************************/
XStatus XPm_PowerDwnFPD(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;
	u32 DbgErr = (u32)XPM_INT_ERR_UNDEFINED;
	u32 IsoVal = 0U;
	const u32 MmiIsoMask = PSXC_LPX_SLCR_DOMAIN_ISO_CNTRL_FPD_MMI_MASK |
			       PSXC_LPX_SLCR_DOMAIN_ISO_CNTRL_FPD_MMI_DFX_MASK;
	const XPm_Device *AmsRoot = NULL;
	const XPm_PsFpDomain *FpDomain = NULL;

	if (NULL == Node) {
		DbgErr = (u32)XPM_INT_ERR_INVALID_NODE;
		Status = XST_FAILURE;
		goto done;
	}

	/* Flush subsystem state before FPD power-off */
	Status = XPmSubsystem_CmnFlushForCore(Node->Id);
	if (XST_SUCCESS != Status) {
		DbgErr = (u32)XPM_INT_ERR_FPD_CMN_FLUSH;
		Status = XST_FAILURE;
		goto done;
	}

	AmsRoot = XPmDevice_GetById(PM_DEV_AMS_ROOT);
	if (NULL == AmsRoot) {
		PmErr("AMS root device not found\r\n");
		DbgErr = (u32)XPM_INT_ERR_INVALID_DEVICE;
		Status = XST_FAILURE;
		goto done;
	}

	/* Bypass FPD at the AMS root before the FPD rail drops */
	XPm_UnlockPcsr(AmsRoot->Node.BaseAddress);
	PmRmw32(AmsRoot->Node.BaseAddress + AMS_ROOT_TOKEN_MNGR_OFFSET,
		AMS_ROOT_TOKEN_MNGR_BYPASS_FPD_MASK,
		AMS_ROOT_TOKEN_MNGR_BYPASS_FPD_MASK);
	(void)XPm_LockPcsr(AmsRoot->Node.BaseAddress);

	/* disable CRL Write Protections */
	PmOut32(PSX_CRL_BASEADDR + PSX_CRL_WPROT_OFFSET, 0U);
	/* disable LPD SLCR Write Protections */
	PmOut32(LPD_SLCR_BASEADDR + LPD_SLCR_WPROT0_OFFSET, 0U);
	/* disable CRP Write Protections */
	PmOut32(CRP_BASEADDR + CRP_WPROT_OFFSET, 0U);

	/**
	 * Mask APLL lock-loss on PMC3 before cutting FPD rail.
	 * Unmasked in FpdInitFinish once the FPD CDO has relocked the APLLs.
	 */
	(void)XPlmi_EmDisableLpdSlcrErrors(EAM_PMC3_ERR0_MASK_REG,
					   EAM_APLL_LOCK_LOSS_MASK);

	/* Isolate: FPD-SOC */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_FPD_SOC, XPM_ISO_ENABLE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_FPD_SOC_ISO;
		goto done;
	}

	/* Isolate: FPD-LPD */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_LPD_FPD, XPM_ISO_ENABLE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_FPD_LPD_ISO;
		goto done;
	}

	/* Isolate: FPD-LPD_DFX */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_LPD_FPD_DFX, XPM_ISO_ENABLE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_FPD_LPD_DFX_ISO;
		goto done;
	}

	/* Isolate: FPD-PL_TEST */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_FPD_PL_TEST, XPM_ISO_ENABLE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_FPD_PL_TEST_ISO;
		goto done;
	}

	/* Isolate: FPD-PL */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_FPD_PL, XPM_ISO_ENABLE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_FPD_PL_ISO;
		goto done;
	}

	/* Isolate: FPD-MMI and FPD-MMI_DFX */
	PmRmw32(PSXC_LPX_SLCR_DOMAIN_ISO_CNTRL, MmiIsoMask, MmiIsoMask);
	PmIn32(PSXC_LPX_SLCR_DOMAIN_ISO_CNTRL, IsoVal);
	if (MmiIsoMask != (IsoVal & MmiIsoMask)) {
		PmErr("FPD-MMI isolation failed, DOMAIN_ISO_CNTRL=0x%x\r\n", IsoVal);
		DbgErr = (0U == (IsoVal & PSXC_LPX_SLCR_DOMAIN_ISO_CNTRL_FPD_MMI_MASK)) ?
			 XPM_INT_ERR_FPD_MMI_ISO : XPM_INT_ERR_FPD_MMI_DFX_ISO;
		Status = XST_FAILURE;
		goto done;
	}

	/* Assert FPD SRST before POR */
	Status = PwrDomainRstAction(PM_RST_FPD_SRST, XPM_RST_STATE_ASSERTED);
	if (XST_SUCCESS != Status) {
		PmErr("FPD SRST assert failed, Status=0x%x\r\n", Status);
		DbgErr = XPM_INT_ERR_RST_ASSERT;
		goto done;
	}
	Status = PwrDomainRstAction(PM_RST_FPD_POR, XPM_RST_STATE_ASSERTED);
	if (XST_SUCCESS != Status) {
		PmErr("FPD POR assert failed, Status=0x%x\r\n", Status);
		DbgErr = XPM_INT_ERR_RST_ASSERT;
		goto done;
	}

	/* set reset value for LPD SLCR Scan Clear Trigger */
	PmOut32(PSXC_LPX_SLCR_SCAN_CLEAR_TRIGGER, 0U);

	/* set reset value for LPD SCLR Mem Clear Trigger */
	PmOut32(PSXC_LPX_SLCR_MEM_CLEAR_TRIGGER, 0U);

	/* set reset value for CRP XDP Control */
	/* TODO: FIXME: MACROs missing */
	PmRmw32(CRP_BASEADDR + CRP_PPLL_TO_XPD_CTRL_OFFSET, 0x3FF00U, 0x200U);

	/*
	 * The GIC sits inside FPD and is shut off with the domain, so the GIC
	 * proxy has to carry the wake path while FPD is down. Arm it before the
	 * rail drops so a wake cannot land with the GIC already gone.
	 */
	Status = XPm_ApuArmGicProxyWake();
	if (XST_SUCCESS != Status) {
		DbgErr = (u32)XPM_INT_ERR_GIC_PROXY_WAKEUP;
		goto done;
	}

	FpDomain = (XPm_PsFpDomain *)XPmPower_GetById(Node->Id);
	if (NULL == FpDomain) {
		Status = XPM_INVALID_PWRDOMAIN;
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		goto done;
	}

	/* Rail Update Status - must be the last step */
	Status = XPmPower_UpdateDomainPower(&FpDomain->Domain, XPM_POWER_STATE_OFF);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_FPD_RAIL_CONTROL;
		goto done;
	}

done:
	/* enable CRL Write Protections */
	PmOut32(PSX_CRL_BASEADDR + PSX_CRL_WPROT_OFFSET, 1U);
	/* enable LPD SLCR Write Protections */
	PmOut32(LPD_SLCR_BASEADDR + LPD_SLCR_WPROT0_OFFSET, 1U);
	/* enable CRP Write Protections */
	PmOut32(CRP_BASEADDR + CRP_WPROT_OFFSET, 1U);

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/****************************************************************************/
/**
 * @brief  Perform the hardware power up sequence for the PL power domain.
 *
 * @param  Node: Pointer to the PL domain node
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 ****************************************************************************/
XStatus XPm_PowerUpPLD(const XPm_Node *Node)
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

	Status = (XStatus)(XPmPower_UpdateDomainPower(&PldDomain->Domain, (u32)XPM_POWER_STATE_ON));

	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PLD_RAIL_CONTROL;
		goto done;
	}

	/**
	 * change Reset Node change to de-asserted for software sync
	 * ( actual reset de-assertion is done by CDO )
	*/
	Status = PwrDomainRstAction(PM_RST_PL_POR, XPM_RST_STATE_DEASSERTED);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_RST_RELEASE;
		goto done;
	}
	Status = PwrDomainRstAction(PM_RST_PL_SRST, XPM_RST_STATE_DEASSERTED);
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
	Status = (XStatus)(XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PL_SOC, XPM_ISO_ENABLE));
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PL_SOC_ISO;
		goto done;
	}
	/* Isolate: FPD-PL */
	Status = (XStatus)(XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_FPD_PL, XPM_ISO_ENABLE));
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_FPD_PL_ISO;
		goto done;
	}
	/* Isolate: FPD_TEST-PL */
	Status = (XStatus)(XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_FPD_PL_TEST, XPM_ISO_ENABLE));
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_FPD_PL_TEST_ISO;
		goto done;
	}
	/* Isolate: LPD-PL */
	Status = (XStatus)(XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_LPD_PL, XPM_ISO_ENABLE));
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_LPD_PL_ISO;
		goto done;
	}
	/* Isolate: LPD_TEST-PL */
	Status = (XStatus)(XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_LPD_PL_TEST, XPM_ISO_ENABLE));
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_LPD_PL_TEST_ISO;
		goto done;
	}
	/* Isolate: PMC-PL */
	Status = (XStatus)(XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_PL, XPM_ISO_ENABLE));
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PMC_PL_ISO;
		goto done;
	}
	/* Isolate: PMC_TEST-PL */
	Status = (XStatus)(XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_PL_TEST, XPM_ISO_ENABLE));
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PMC_PL_TEST_ISO;
		goto done;
	}
	/* Isolate: PMC_CFRAME-PL */
	Status = (XStatus)(XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_PL_CFRAME, XPM_ISO_ENABLE));
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PMC_PL_CFRAME_ISO;
		goto done;
	}
	/* Isolate: VCCAUX-PL */
	Status = (XStatus)(XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_VCCAUX_VCCRAM, XPM_ISO_ENABLE));
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_VCCAUX_VCCRAM_ISO;
		goto done;
	}
	/* Isolate: VCCSOC-PL */
	Status = (XStatus)(XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_VCCRAM_SOC, XPM_ISO_ENABLE));
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_VCCRAM_SOC_ISO;
		goto done;
	}

	/* Extract needed logic from XPmReset_AssertbyId() to boot module */
	/* Assert POR PL */
	Status = PwrDomainRstAction(PM_RST_PL_POR, XPM_RST_STATE_ASSERTED);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_RST_ASSERT;
		goto done;
	}
	/* Assert SRST PL */
	Status = PwrDomainRstAction(PM_RST_PL_SRST, XPM_RST_STATE_ASSERTED);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_RST_ASSERT;
		goto done;
	}

	/* Power down PLD power rail */
	Status = (XStatus)(XPmPower_UpdateDomainPower(&PldDomain->Domain, (u32)XPM_POWER_STATE_OFF));

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

maybe_unused static XStatus XPm_PowerUpCPM5N(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;

	/* TODO: Add Power up sequence for CPM5N */
	(void)Node;
	Status = XST_SUCCESS;

	return Status;
}

maybe_unused static XStatus XPm_PowerDwnCPM5N(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;

	/* TODO: Add Power down sequence for CPM5N */
	(void)Node;
	Status = XST_SUCCESS;

	return Status;
}

/****************************************************************************/
/**
 * @brief  Perform the power up sequence for the NoC power domain.
 *
 * @param  Node: Pointer to the NoC domain node
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 ****************************************************************************/
XStatus XPm_PowerUpNoC(const XPm_Node *Node)
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
	} else if (NODEINDEX((u32)XPM_NODEIDX_POWER_VCCINT_ME) == NodeIndex) {
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
	(void)XPm_LockPcsr(PMC_SYSMON_BASEADDR);

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
	{
		PwrDomain->Power.Node.State = (u8)XPM_POWER_STATE_INITIALIZING;
		Status = (XStatus)(XPmPower_UpdateDomainPower(PwrDomain, (u32)XPM_POWER_STATE_ON));

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
	}
	case (u32)FUNC_INIT_FINISH:
	{
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
	}
	case (u32)FUNC_AMS_TRIM:
	{
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
	}
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
