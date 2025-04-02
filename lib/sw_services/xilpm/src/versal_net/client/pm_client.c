/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
/* Max number of APU masters */
#define MAX_SUPPORTED_PROC		(16U)
#define ISB				__asm__ ("isb")
#define PM_APU_CORE_COUNT_PER_CLUSTER	(4U)
#define CORE_PWRDN_EN_BIT_MASK		(0x1U)
#define MPIDR_AFF1_SHIFT		(8U)
#define MPIDR_AFF2_SHIFT		(16U)
#define APU_PCIL_BASEADDR		(0xECB10000U)
#define CORE_0_PWRDWN_OFFSET		(0x0U)
#define CORE_1_PWRDWN_OFFSET		(0x30U)
#define CORE_2_PWRDWN_OFFSET		(0x60U)
#define CORE_3_PWRDWN_OFFSET		(0x90U)
#define CORE_4_PWRDWN_OFFSET		(0xC0U)
#define CORE_5_PWRDWN_OFFSET		(0xF0U)
#define CORE_6_PWRDWN_OFFSET		(0x120U)
#define CORE_7_PWRDWN_OFFSET		(0x150U)
#define CORE_8_PWRDWN_OFFSET		(0x180U)
#define CORE_9_PWRDWN_OFFSET		(0x1B0U)
#define CORE_10_PWRDWN_OFFSET		(0x1E0U)
#define CORE_11_PWRDWN_OFFSET		(0x210U)
#define CORE_12_PWRDWN_OFFSET		(0x240U)
#define CORE_13_PWRDWN_OFFSET		(0x270U)
#define CORE_14_PWRDWN_OFFSET		(0x2A0U)
#define CORE_15_PWRDWN_OFFSET		(0x2D0U)
#define APU_PWRDWN_EN_MASK		(0x00000001U)
#define CORE_ISR_POWER_OFFSET		(0x10U)
#define CORE_IEN_POWER_OFFSET		(0x18U)
#define CORE_ISR_WAKE_OFFSET		(0x20U)
#define CORE_IEN_WAKE_OFFSET		(0x28U)
#define CORE_IDS_POWER_OFFSET		(0x1CU)
#define CORE_IDS_WAKE_OFFSET		(0x2CU)

static struct XPm_Proc Proc_APU0_0 = {
	.DevId = PM_DEV_ACPU_0_0,
	.PwrCtrl = APU_PCIL_BASEADDR + CORE_0_PWRDWN_OFFSET,
	.PwrDwnMask = APU_PWRDWN_EN_MASK,
	.Ipi = NULL,
};

static struct XPm_Proc Proc_APU0_1 = {
	.DevId = PM_DEV_ACPU_0_1,
	.PwrCtrl = APU_PCIL_BASEADDR + CORE_1_PWRDWN_OFFSET,
	.PwrDwnMask = APU_PWRDWN_EN_MASK,
	.Ipi = NULL,
};

static struct XPm_Proc Proc_APU0_2 = {
	.DevId = PM_DEV_ACPU_0_2,
	.PwrCtrl = APU_PCIL_BASEADDR + CORE_2_PWRDWN_OFFSET,
	.PwrDwnMask = APU_PWRDWN_EN_MASK,
	.Ipi = NULL,
};

static struct XPm_Proc Proc_APU0_3 = {
	.DevId = PM_DEV_ACPU_0_3,
	.PwrCtrl = APU_PCIL_BASEADDR + CORE_3_PWRDWN_OFFSET,
	.PwrDwnMask = APU_PWRDWN_EN_MASK,
	.Ipi = NULL,
};

static struct XPm_Proc Proc_APU1_0 = {
	.DevId = PM_DEV_ACPU_1_0,
	.PwrCtrl = APU_PCIL_BASEADDR + CORE_4_PWRDWN_OFFSET,
	.PwrDwnMask = APU_PWRDWN_EN_MASK,
	.Ipi = NULL,
};

static struct XPm_Proc Proc_APU1_1 = {
	.DevId = PM_DEV_ACPU_1_1,
	.PwrCtrl = APU_PCIL_BASEADDR + CORE_5_PWRDWN_OFFSET,
	.PwrDwnMask = APU_PWRDWN_EN_MASK,
	.Ipi = NULL,
};

static struct XPm_Proc Proc_APU1_2 = {
	.DevId = PM_DEV_ACPU_1_2,
	.PwrCtrl = APU_PCIL_BASEADDR + CORE_6_PWRDWN_OFFSET,
	.PwrDwnMask = APU_PWRDWN_EN_MASK,
	.Ipi = NULL,
};

static struct XPm_Proc Proc_APU1_3 = {
	.DevId = PM_DEV_ACPU_1_3,
	.PwrCtrl = APU_PCIL_BASEADDR + CORE_7_PWRDWN_OFFSET,
	.PwrDwnMask = APU_PWRDWN_EN_MASK,
	.Ipi = NULL,
};

static struct XPm_Proc Proc_APU2_0 = {
	.DevId = PM_DEV_ACPU_2_0,
	.PwrCtrl = APU_PCIL_BASEADDR + CORE_8_PWRDWN_OFFSET,
	.PwrDwnMask = APU_PWRDWN_EN_MASK,
	.Ipi = NULL,
};

static struct XPm_Proc Proc_APU2_1 = {
	.DevId = PM_DEV_ACPU_2_1,
	.PwrCtrl = APU_PCIL_BASEADDR + CORE_9_PWRDWN_OFFSET,
	.PwrDwnMask = APU_PWRDWN_EN_MASK,
	.Ipi = NULL,
};

static struct XPm_Proc Proc_APU2_2 = {
	.DevId = PM_DEV_ACPU_2_2,
	.PwrCtrl = APU_PCIL_BASEADDR + CORE_10_PWRDWN_OFFSET,
	.PwrDwnMask = APU_PWRDWN_EN_MASK,
	.Ipi = NULL,
};

static struct XPm_Proc Proc_APU2_3 = {
	.DevId = PM_DEV_ACPU_2_3,
	.PwrCtrl = APU_PCIL_BASEADDR + CORE_11_PWRDWN_OFFSET,
	.PwrDwnMask = APU_PWRDWN_EN_MASK,
	.Ipi = NULL,
};

static struct XPm_Proc Proc_APU3_0 = {
	.DevId = PM_DEV_ACPU_3_0,
	.PwrCtrl = APU_PCIL_BASEADDR + CORE_12_PWRDWN_OFFSET,
	.PwrDwnMask = APU_PWRDWN_EN_MASK,
	.Ipi = NULL,
};

static struct XPm_Proc Proc_APU3_1 = {
	.DevId = PM_DEV_ACPU_3_1,
	.PwrCtrl = APU_PCIL_BASEADDR + CORE_13_PWRDWN_OFFSET,
	.PwrDwnMask = APU_PWRDWN_EN_MASK,
	.Ipi = NULL,
};

static struct XPm_Proc Proc_APU3_2 = {
	.DevId = PM_DEV_ACPU_3_2,
	.PwrCtrl = APU_PCIL_BASEADDR + CORE_14_PWRDWN_OFFSET,
	.PwrDwnMask = APU_PWRDWN_EN_MASK,
	.Ipi = NULL,
};

static struct XPm_Proc Proc_APU3_3 = {
	.DevId = PM_DEV_ACPU_3_3,
	.PwrCtrl = APU_PCIL_BASEADDR + CORE_15_PWRDWN_OFFSET,
	.PwrDwnMask = APU_PWRDWN_EN_MASK,
	.Ipi = NULL,
};

static struct XPm_Proc *const ProcList[] = {
	&Proc_APU0_0,
	&Proc_APU0_1,
	&Proc_APU0_2,
	&Proc_APU0_3,
	&Proc_APU1_0,
	&Proc_APU1_1,
	&Proc_APU1_2,
	&Proc_APU1_3,
	&Proc_APU2_0,
	&Proc_APU2_1,
	&Proc_APU2_2,
	&Proc_APU2_3,
	&Proc_APU3_0,
	&Proc_APU3_1,
	&Proc_APU3_2,
	&Proc_APU3_3,
};

struct XPm_Proc *PrimaryProc = &Proc_APU0_0;
#elif defined (__arm__)
/* Max number of RPU masters */
#define MAX_SUPPORTED_PROC		(4U)
#define PM_RPU_CORE_COUNT_PER_CLUSTER	(2U)
#define PSX_RPU_CLUSTER_A_BASEADDR	(0xEB580000U)
#define PSX_RPU_CLUSTER_B_BASEADDR	(0xEB590000U)
#define LPX_SLCR_RPU_PCIL_A0_PWRDWN (0xEB4200C0U)
#define LPX_SLCR_RPU_PCIL_A1_PWRDWN (0xEB4201C0U)
#define LPX_SLCR_RPU_PCIL_B0_PWRDWN (0xEB4210C0U)
#define LPX_SLCR_RPU_PCIL_B1_PWRDWN (0xEB4211C0U)
#define CLUSTER_CFG_OFFSET		(0x0U)
#define RPU_PWRDWN_EN_MASK		(0x1U)
#define MPIDR_AFF0_SHIFT		(0U)
#define MPIDR_AFF1_SHIFT		(8U)
#define RPU_GLBL_CNTL_SLSPLIT_MASK	(0x1U)
/*This is negative offset from PwrCtrl properties which is
LPX_SLCR_RPU_PCIL_*_PWRDWN*/
#define LPX_SLCR_RPU_PCIL_CORE_IEN_OFFSET		(-0xB8U)
#define LPX_SLCR_RPU_PCIL_CORE_IDS_OFFSET		(-0xB4U)

static struct XPm_Proc Proc_RPU_A_0 = {
	.DevId = PM_DEV_RPU_A_0,
	.PwrCtrl = LPX_SLCR_RPU_PCIL_A0_PWRDWN,
	.PwrDwnMask = RPU_PWRDWN_EN_MASK,
	.Ipi = NULL,
};

static struct XPm_Proc Proc_RPU_A_1 = {
	.DevId = PM_DEV_RPU_A_1,
	.PwrCtrl = LPX_SLCR_RPU_PCIL_A1_PWRDWN,
	.PwrDwnMask = RPU_PWRDWN_EN_MASK,
	.Ipi = NULL,
};

static struct XPm_Proc Proc_RPU_B_0 = {
	.DevId = PM_DEV_RPU_B_0,
	.PwrCtrl = LPX_SLCR_RPU_PCIL_B0_PWRDWN,
	.PwrDwnMask = RPU_PWRDWN_EN_MASK,
	.Ipi = NULL,
};

static struct XPm_Proc Proc_RPU_B_1 = {
	.DevId = PM_DEV_RPU_B_1,
	.PwrCtrl = LPX_SLCR_RPU_PCIL_B1_PWRDWN,
	.PwrDwnMask = RPU_PWRDWN_EN_MASK,
	.Ipi = NULL,
};

static struct XPm_Proc *const ProcList[] = {
	&Proc_RPU_A_0,
	&Proc_RPU_A_1,
	&Proc_RPU_B_0,
	&Proc_RPU_B_1,
};

struct XPm_Proc *PrimaryProc = &Proc_RPU_A_0;
char ProcName[7] = "RPU";
static char RPU_LS_A[] = "RPU_A";
static char RPU_LS_B[] = "RPU_B";
static char RPU_A0[] = "RPU_A0";
static char RPU_A1[] = "RPU_A1";
static char RPU_B0[] = "RPU_B0";
static char RPU_B1[] = "RPU_B1";
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
	CpuId = (mfcp(XREG_CP15_MULTI_PROC_AFFINITY) >> MPIDR_AFF0_SHIFT) & MPIDR_AFFLVL_MASK;
	ClusterId = (mfcp(XREG_CP15_MULTI_PROC_AFFINITY) >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK;
	ProcId = (((u32)ClusterId * PM_RPU_CORE_COUNT_PER_CLUSTER) + (u32)CpuId);
	if (PM_RPU_CORE_COUNT_PER_CLUSTER > ProcId) {
		if (0U == (XPm_Read(PSX_RPU_CLUSTER_A_BASEADDR + CLUSTER_CFG_OFFSET) & RPU_GLBL_CNTL_SLSPLIT_MASK)) {
			ProcId = 0U;
			Status = Xil_SMemCpy(ProcName, sizeof(RPU_LS_A), RPU_LS_A, sizeof(RPU_LS_A), sizeof(RPU_LS_A));
			if (XST_SUCCESS != Status) {
				goto done;
			}
			XPm_Dbg("Running in lock-step mode\r\n");
		} else {
			if (0U == ProcId) {
				Status = Xil_SMemCpy(ProcName, sizeof(RPU_A0), RPU_A0, sizeof(RPU_A0), sizeof(RPU_A0));
				if (XST_SUCCESS != Status) {
					goto done;
				}
			} else {
				Status = Xil_SMemCpy(ProcName, sizeof(RPU_A1), RPU_A1, sizeof(RPU_A1), sizeof(RPU_A1));
				if (XST_SUCCESS != Status) {
					goto done;
				}
			}
		}
	} else if ((PM_RPU_CORE_COUNT_PER_CLUSTER * 2U) > ProcId) {
		if (0U == (XPm_Read(PSX_RPU_CLUSTER_B_BASEADDR + CLUSTER_CFG_OFFSET) & RPU_GLBL_CNTL_SLSPLIT_MASK)) {
			ProcId = 2U;
			Status = Xil_SMemCpy(ProcName, sizeof(RPU_LS_B), RPU_LS_B, sizeof(RPU_LS_B), sizeof(RPU_LS_B));
			if (XST_SUCCESS != Status) {
				goto done;
			}
			XPm_Dbg("Running in lock-step mode\r\n");
		} else {
			if (2U == ProcId) {
				Status = Xil_SMemCpy(ProcName, sizeof(RPU_B0), RPU_B0, sizeof(RPU_B0), sizeof(RPU_B0));
				if (XST_SUCCESS != Status) {
					goto done;
				}
			} else {
				Status = Xil_SMemCpy(ProcName, sizeof(RPU_B1), RPU_B1, sizeof(RPU_B1), sizeof(RPU_B1));
				if (XST_SUCCESS != Status) {
					goto done;
				}
			}
		}
	} else {
		/* Required for MISRA */
	}
#endif

	Status = XST_SUCCESS;

	if (ProcId < MAX_SUPPORTED_PROC) {
		PrimaryProc = ProcList[ProcId];
	}

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
