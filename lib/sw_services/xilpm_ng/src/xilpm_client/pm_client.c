/******************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "pm_client.h"
#include <xil_cache.h>
#include "xil_util.h"
#include "xpm_nodeid.h"
#if defined (__aarch64__)
#include <xreg_cortexa53.h>
#elif defined (__arm__)
#include <xreg_cortexr5.h>
#endif

#ifndef __microblaze__
/** @cond xilpm_internal */
#define XPM_ARRAY_SIZE(x)		(sizeof(x) / sizeof(x[0]))

#define MPIDR_AFFLVL_MASK		(0xFFU)

#define WFI				__asm__ ("wfi")
#endif

#if defined (__aarch64__)
#define ISB				__asm__ ("isb")
#define PM_APU_CORE_COUNT_PER_CLUSTER	(2U)
#define CORE_PWRDN_EN_BIT_MASK		(0x1U)
#define MPIDR_AFF1_SHIFT		(8U)
#define MPIDR_AFF2_SHIFT		(16U)
#define APU_PCIL_BASEADDR		(0xECB10000U)
#define CORE_0_0_PWRDWN_OFFSET		(0x0U)
#define CORE_0_1_PWRDWN_OFFSET		(0x30U)
#define CORE_0_2_PWRDWN_OFFSET		(0x60U)
#define CORE_0_3_PWRDWN_OFFSET		(0x90U)
#define CORE_1_0_PWRDWN_OFFSET		(0xC0U)
#define CORE_1_1_PWRDWN_OFFSET		(0xF0U)
#define CORE_1_2_PWRDWN_OFFSET		(0x120U)
#define CORE_1_3_PWRDWN_OFFSET		(0x150U)
#define CORE_2_0_PWRDWN_OFFSET		(0x180U)
#define CORE_2_1_PWRDWN_OFFSET		(0x1B0U)
#define CORE_2_2_PWRDWN_OFFSET		(0x1E0U)
#define CORE_2_3_PWRDWN_OFFSET		(0x210U)
#define CORE_3_0_PWRDWN_OFFSET		(0x240U)
#define CORE_3_1_PWRDWN_OFFSET		(0x270U)
#define CORE_3_2_PWRDWN_OFFSET		(0x2A0U)
#define CORE_3_3_PWRDWN_OFFSET		(0x2D0U)
#define APU_PWRDWN_EN_MASK		(0x00000001U)
#define CORE_ISR_POWER_OFFSET		(0x10U)
#define CORE_IEN_POWER_OFFSET		(0x18U)
#define CORE_ISR_WAKE_OFFSET		(0x20U)
#define CORE_IEN_WAKE_OFFSET		(0x28U)
#define CORE_IDS_POWER_OFFSET		(0x1CU)
#define CORE_IDS_WAKE_OFFSET		(0x2CU)

#define DEFINE_APU_PROC(cluster_num, core_num) \
	static struct XPm_Proc Proc_APU ## cluster_num ## _ ## core_num = { \
		.DevId = PM_DEV_ACPU_ ## cluster_num ## _ ## core_num, \
		.PwrCtrl = APU_PCIL_BASEADDR + CORE_ ## cluster_num ## _ ##core_num ## _PWRDWN_OFFSET, \
		.PwrDwnMask = APU_PWRDWN_EN_MASK, \
		.Ipi = NULL, \
	};

#define DEFINE_APU_CLUSTER(cluster_num) \
	DEFINE_APU_PROC(cluster_num, 0) \
	DEFINE_APU_PROC(cluster_num, 1)

DEFINE_APU_CLUSTER(0)
DEFINE_APU_CLUSTER(1)
DEFINE_APU_CLUSTER(2)
DEFINE_APU_CLUSTER(3)

static struct XPm_Proc *const ProcList[] = {
	&Proc_APU0_0,
	&Proc_APU0_1,
	&Proc_APU1_0,
	&Proc_APU1_1,
	&Proc_APU2_0,
	&Proc_APU2_1,
	&Proc_APU3_0,
	&Proc_APU3_1,
};

struct XPm_Proc *PrimaryProc = &Proc_APU0_0;

#elif defined (__arm__)
#define PM_RPU_CORE_COUNT_PER_CLUSTER	(2U)
#define PSX_RPU_CLUSTER_A_BASEADDR	(0xEB580000U)
#define PSX_RPU_CLUSTER_B_BASEADDR	(0xEB590000U)
#define PSX_RPU_CLUSTER_C_BASEADDR	(0xEB5A0000U)
#define PSX_RPU_CLUSTER_D_BASEADDR	(0xEB5B0000U)
#define PSX_RPU_CLUSTER_E_BASEADDR	(0xEB5C0000U)
#define LPX_SLCR_RPU_PCIL_A0_PWRDWN	(0xEB4200C0U)
#define LPX_SLCR_RPU_PCIL_A1_PWRDWN	(0xEB4201C0U)
#define LPX_SLCR_RPU_PCIL_B0_PWRDWN	(0xEB4210C0U)
#define LPX_SLCR_RPU_PCIL_B1_PWRDWN	(0xEB4211C0U)
#define LPX_SLCR_RPU_PCIL_C0_PWRDWN	(0xEB4211E0U)
#define LPX_SLCR_RPU_PCIL_C1_PWRDWN	(0xEB421200U)
#define LPX_SLCR_RPU_PCIL_D0_PWRDWN	(0xEB421220U)
#define LPX_SLCR_RPU_PCIL_D1_PWRDWN	(0xEB421240U)
#define LPX_SLCR_RPU_PCIL_E0_PWRDWN	(0xEB421260U)
#define LPX_SLCR_RPU_PCIL_E1_PWRDWN	(0xEB421280U)
#define CLUSTER_CFG_OFFSET		(0x0U)
#define RPU_PWRDWN_EN_MASK		(0x1U)
#define MPIDR_AFF0_SHIFT		(0U)
#define MPIDR_AFF1_SHIFT		(8U)
#define RPU_GLBL_CNTL_SLSPLIT_MASK	(0x1U)
/*This is negative offset from PwrCtrl properties which is
LPX_SLCR_RPU_PCIL_*_PWRDWN*/
#define LPX_SLCR_RPU_PCIL_CORE_IEN_OFFSET		(-0xB8U)
#define LPX_SLCR_RPU_PCIL_CORE_IDS_OFFSET		(-0xB4U)

#define DEFINE_RPU_PROC(cluster_name, core_num) \
	static struct XPm_Proc Proc_RPU_ ## cluster_name ## _ ## core_num = { \
		.DevId = PM_DEV_RPU_ ## cluster_name ## _ ## core_num, \
		.PwrCtrl = LPX_SLCR_RPU_PCIL_## cluster_name ## core_num ##_PWRDWN, \
		.PwrDwnMask = RPU_PWRDWN_EN_MASK, \
		.Ipi = NULL, \
	}; \
	static char RPU_## cluster_name ## _ ## core_num[] = "RPU_"#cluster_name"_"#core_num;

#define DEFINE_RPU_CLUSTER(cluster_name) \
	DEFINE_RPU_PROC(cluster_name, 0) \
	DEFINE_RPU_PROC(cluster_name, 1) \
	static char RPU_LS_ ## cluster_name[] = "RPU_"#cluster_name;

DEFINE_RPU_CLUSTER(A)
DEFINE_RPU_CLUSTER(B)
DEFINE_RPU_CLUSTER(C)
DEFINE_RPU_CLUSTER(D)
DEFINE_RPU_CLUSTER(E)

static struct XPm_Proc *const ProcList[] = {
	&Proc_RPU_A_0,
	&Proc_RPU_A_1,
	&Proc_RPU_B_0,
	&Proc_RPU_B_1,
	&Proc_RPU_C_0,
	&Proc_RPU_C_1,
	&Proc_RPU_D_0,
	&Proc_RPU_D_1,
	&Proc_RPU_E_0,
	&Proc_RPU_E_1,
};

struct XPm_Proc *PrimaryProc = &Proc_RPU_A_0;

char ProcName[7] = "RPU";
#endif

void XPm_RMW32(u32 RegAddress, u32 Mask, u32 Value)
{
	u32 l_Val;

	l_Val = XPm_Read(RegAddress);
	l_Val = (l_Val & (~Mask)) | (Mask & Value);

	XPm_Write(RegAddress, l_Val);
}

/**
 *  XPm_SetPrimaryProc() - Set primary processor based on processor ID
 */
#ifndef __microblaze__
XStatus XPm_SetPrimaryProc(void)
{
	u32 ProcId;
	XStatus Status = (s32)XST_FAILURE;
	u64 CpuId;
	u64 ClusterId;

#if defined (__aarch64__)
	CpuId = (mfcp(MPIDR_EL1) >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK;
	ClusterId = (mfcp(MPIDR_EL1) >> MPIDR_AFF2_SHIFT) & MPIDR_AFFLVL_MASK;
	ProcId = (((u32)ClusterId * PM_APU_CORE_COUNT_PER_CLUSTER) + (u32)CpuId);
#elif defined (__arm__)
#define RPU_NAME_DECISION_CASE(cluster_name) \
	if (0U == (XPm_Read(PSX_RPU_CLUSTER_##cluster_name##_BASEADDR + CLUSTER_CFG_OFFSET) & RPU_GLBL_CNTL_SLSPLIT_MASK)) { \
		ProcId = 0U; \
		TempProcName = RPU_LS_##cluster_name; \
		XPm_Dbg("Running in lock-step mode\r\n"); \
	} else { \
		if(0U == ProcId) { \
			TempProcName = RPU_##cluster_name##_0; \
		} else { \
			TempProcName = RPU_##cluster_name##_1; \
		} \
	}
	char *TempProcName;
	CpuId = (mfcp(XREG_CP15_MULTI_PROC_AFFINITY) >> MPIDR_AFF0_SHIFT) & MPIDR_AFFLVL_MASK;
	ClusterId = (mfcp(XREG_CP15_MULTI_PROC_AFFINITY) >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK;
	ProcId = (((u32)ClusterId * PM_RPU_CORE_COUNT_PER_CLUSTER) + (u32)CpuId);
	switch (ClusterId) {
		case 0:
			RPU_NAME_DECISION_CASE(A)
			break;
		case 1:
			RPU_NAME_DECISION_CASE(B)
			break;
		case 2:
			RPU_NAME_DECISION_CASE(C)
			break;
		case 3:
			RPU_NAME_DECISION_CASE(D)
			break;
		case 4:
			RPU_NAME_DECISION_CASE(E)
			break;
		default:
			/* Required for MISRA */
	}
	Status = Xil_SMemCpy(ProcName, sizeof(ProcName), TempProcName, sizeof(ProcName), sizeof(ProcName));
	if (XST_SUCCESS != Status) {
		goto done;
	}

#endif

	Status = XST_SUCCESS;

	PrimaryProc = ProcList[ProcId];

#if defined (__arm__)
done:
#endif
	return Status;
}
#else
XStatus XPm_SetPrimaryProc(void)
{
	PrimaryProc = &Proc_MB_PL_0;
	return XST_SUCCESS;
}
#endif

#ifndef __microblaze__
struct XPm_Proc *XPm_GetProcByDeviceId(u32 DeviceId)
{
	struct XPm_Proc *Proc = NULL;
	u8 Idx;

	for (Idx = 0; Idx < XPM_ARRAY_SIZE(ProcList); Idx++) {
		if (DeviceId == ProcList[Idx]->DevId) {
			Proc = ProcList[Idx];
			break;
		}
	}

	return Proc;
}
#endif

void XPm_ClientSuspend(const struct XPm_Proc *const Proc)
{
	/* Disable interrupts at processor level */
	XpmDisableInterrupts();

#if defined (__arm__)
	u32 PwrDwnReg;
	/* Set powerdown request */
	PwrDwnReg = XPm_Read(Proc->PwrCtrl);
	PwrDwnReg |= Proc->PwrDwnMask;
	XPm_Write(Proc->PwrCtrl, PwrDwnReg);
#else
	u64 Val;
	Val = mfcp(S3_0_C15_C2_7);
	Val |= CORE_PWRDN_EN_BIT_MASK;
	mtcp(S3_0_C15_C2_7, Val);
	ISB;
	XPm_RMW32(Proc->PwrCtrl, Proc->PwrDwnMask, Proc->PwrDwnMask);
	XPm_RMW32(Proc->PwrCtrl + CORE_ISR_POWER_OFFSET, Proc->PwrDwnMask, Proc->PwrDwnMask);
	XPm_RMW32(Proc->PwrCtrl + CORE_IEN_POWER_OFFSET, Proc->PwrDwnMask, Proc->PwrDwnMask);
	XPm_RMW32(Proc->PwrCtrl + CORE_ISR_WAKE_OFFSET, Proc->PwrDwnMask, Proc->PwrDwnMask);
	XPm_RMW32(Proc->PwrCtrl + CORE_IEN_WAKE_OFFSET, Proc->PwrDwnMask, Proc->PwrDwnMask);
#endif
}

void XPm_ClientWakeUp(const struct XPm_Proc *const Proc)
{
#if defined (__arm__)
	if (NULL != Proc) {
		u32 Val;

		Val = XPm_Read(Proc->PwrCtrl);
		Val &= ~Proc->PwrDwnMask;
		XPm_Write(Proc->PwrCtrl, Val);
	}
#else
	u64 Val;
	Val = mfcp(S3_0_C15_C2_7);
	Val &= ~CORE_PWRDN_EN_BIT_MASK;
	mtcp(S3_0_C15_C2_7, Val);
	ISB;
	XPm_RMW32(Proc->PwrCtrl, Proc->PwrDwnMask, ~Proc->PwrDwnMask);
	XPm_RMW32(Proc->PwrCtrl + CORE_IDS_POWER_OFFSET, Proc->PwrDwnMask, Proc->PwrDwnMask);
	XPm_RMW32(Proc->PwrCtrl + CORE_IDS_WAKE_OFFSET, Proc->PwrDwnMask, Proc->PwrDwnMask);
#endif
}

#ifndef __microblaze__
void XPm_ClientSuspendFinalize(void)
{
	u32 CtrlReg;

	/* Flush the data cache only if it is enabled */
#ifdef __aarch64__
	CtrlReg = (u32)mfcp(SCTLR_EL3);
	if (0U != (XREG_CONTROL_DCACHE_BIT & CtrlReg)) {
		Xil_DCacheFlush();
	}
#else
	CtrlReg = mfcp(XREG_CP15_SYS_CONTROL);
	if (0U != (XREG_CP15_CONTROL_C_BIT & CtrlReg)) {
		Xil_DCacheFlush();
	}
#endif

	XPm_Dbg("Going to WFI...\n");
	WFI;
	XPm_Dbg("WFI exit...\n");
}
#endif

void XPm_ClientAbortSuspend(void)
{
#if defined (__arm__)
	u32 PwrDwnReg;

	/* Clear powerdown request */
	PwrDwnReg = XPm_Read(PrimaryProc->PwrCtrl);
	PwrDwnReg &= ~PrimaryProc->PwrDwnMask;
	XPm_Write(PrimaryProc->PwrCtrl, PwrDwnReg);
	XPm_RMW32(PrimaryProc->PwrCtrl + LPX_SLCR_RPU_PCIL_CORE_IDS_OFFSET, PrimaryProc->PwrDwnMask, PrimaryProc->PwrDwnMask);
#else
	u64 Val;
	Val = mfcp(S3_0_C15_C2_7);
	Val &= ~CORE_PWRDN_EN_BIT_MASK;
	mtcp(S3_0_C15_C2_7, Val);
	ISB;
	XPm_RMW32(PrimaryProc->PwrCtrl, PrimaryProc->PwrDwnMask, ~PrimaryProc->PwrDwnMask);
	XPm_RMW32(PrimaryProc->PwrCtrl + CORE_IDS_POWER_OFFSET, PrimaryProc->PwrDwnMask,
		  PrimaryProc->PwrDwnMask);
	XPm_RMW32(PrimaryProc->PwrCtrl + CORE_IDS_WAKE_OFFSET, PrimaryProc->PwrDwnMask,
		  PrimaryProc->PwrDwnMask);
#endif

	/* Enable interrupts at processor level */
	XpmEnableInterrupts();
}

/** @endcond */
