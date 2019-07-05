/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

#include "sleep.h"
#include "xpm_common.h"
#include "xpm_core.h"
#include "xpm_domain_iso.h"
#include "xpm_npdomain.h"
#include "xpm_pslpdomain.h"
#include "xpm_regs.h"
#include "xpm_clock.h"
#include "xpm_reset.h"
#include "xpm_bisr.h"

#define XPM_NODEIDX_DEV_DDRMC_MIN	XPM_NODEIDX_DEV_DDRMC_0
#define XPM_NODEIDX_DEV_DDRMC_MAX	XPM_NODEIDX_DEV_DDRMC_3

static u32 NpdMemIcAddresses[XPM_NODEIDX_MEMIC_MAX];

static XStatus NpdInitStart(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;

	(void)Args;
	(void)NumOfArgs;

	/* Check vccint_soc first to make sure power is on */
	if (XST_SUCCESS != XPmPower_CheckPower(PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_SOC_MASK)) {
		/* TODO: Request PMC to power up VCCINT_SOC rail and wait for the acknowledgement.*/
		goto done;
	}

	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_PMC_SOC_NPI, FALSE);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Release POR for NoC */
	Status = XPmReset_AssertbyId(POR_RSTID(XPM_NODEIDX_RST_NOC_POR),
				     PM_RESET_ACTION_RELEASE);

done:
	return Status;
}

static void NpdPreBisrReqs()
{
	/* Release NPI Reset */
	XPmReset_AssertbyId(SRST_RSTID(XPM_NODEIDX_RST_NPI),
			PM_RESET_ACTION_RELEASE);

	/* Release NoC Reset */
	XPmReset_AssertbyId(SRST_RSTID(XPM_NODEIDX_RST_NOC),
			PM_RESET_ACTION_RELEASE);

	/* Release Sys Resets */
	XPmReset_AssertbyId(SRST_RSTID(XPM_NODEIDX_RST_SYS_RST_1),
			PM_RESET_ACTION_RELEASE);
	XPmReset_AssertbyId(SRST_RSTID(XPM_NODEIDX_RST_SYS_RST_2),
			PM_RESET_ACTION_RELEASE);
	XPmReset_AssertbyId(SRST_RSTID(XPM_NODEIDX_RST_SYS_RST_3),
			PM_RESET_ACTION_RELEASE);

	return;
}

static XStatus NpdInitFinish(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;
	u32 i=0;
	XPm_Device *Device;
	u32 BaseAddress;

	(void)Args;
	(void)NumOfArgs;

	/* NPD pre bisr requirements - in case if bisr and mbist was skipped */
	NpdPreBisrReqs();

	if (XST_SUCCESS == XPmPower_CheckPower(	PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCAUX_MASK |
						PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_SOC_MASK)) {
		/* Remove vccaux-soc domain isolation */
		Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_VCCAUX_SOC, FALSE);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/* Remove PMC-NoC domain isolation */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_PMC_SOC, FALSE);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Assert ODISABLE NPP for all NMU and NSU*/
	for (i = 0; i < ARRAY_SIZE(NpdMemIcAddresses); i++) {
		PmOut32(NpdMemIcAddresses[i] + NPI_PCSR_LOCK_OFFSET,
			PCSR_UNLOCK_VAL);
		PmOut32(NpdMemIcAddresses[i] + NPI_PCSR_MASK_OFFSET,
			NPI_PCSR_CONTROL_ODISABLE_NPP_MASK)
		PmOut32(NpdMemIcAddresses[i] + NPI_PCSR_CONTROL_OFFSET,
			NPI_PCSR_CONTROL_ODISABLE_NPP_MASK);
		PmOut32(NpdMemIcAddresses[i] + NPI_PCSR_LOCK_OFFSET, 1);
	}

	/* Deassert UB_INITSTATE for DDR blocks */
	for (i = XPM_NODEIDX_DEV_DDRMC_MIN; i <= XPM_NODEIDX_DEV_DDRMC_MAX; i++) {
		Device = XPmDevice_GetById(DDRMC_DEVID(i));
		BaseAddress = Device->Node.BaseAddress;
		PmOut32(BaseAddress + NPI_PCSR_LOCK_OFFSET, PCSR_UNLOCK_VAL);
		PmOut32(BaseAddress + NPI_PCSR_MASK_OFFSET,
			NPI_DDRMC_PSCR_CONTROL_UB_INITSTATE_MASK)
		PmOut32(BaseAddress + NPI_PCSR_CONTROL_OFFSET, 0);
		PmOut32(BaseAddress + NPI_PCSR_LOCK_OFFSET, 1);
		/* Only UB0 for non sillicon platforms */
		if (PLATFORM_VERSION_SILICON != Platform) {
			Status = XST_SUCCESS;
			goto done;
		}
	}
done:
	return Status;
}

static XStatus NpdScanClear(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;
	XPm_Core *Pmc;
	u32 RegValue;
	XPm_OutClockNode *Clk;

	(void)Args;
	(void)NumOfArgs;

	if (PLATFORM_VERSION_SILICON != Platform) {
		Status = XST_SUCCESS;
		goto done;
	}

	Pmc = (XPm_Core *)XPmDevice_GetById(XPM_DEVID_PMC);
	if (NULL == Pmc) {
		Status = XST_FAILURE;
		goto done;
	}

	PmOut32((Pmc->RegAddress[0] + PMC_GLOBAL_ERR1_STATUS_OFFSET),
		(PMC_GLOBAL_ERR1_STATUS_NOC_TYPE_1_NCR_MASK |
		PMC_GLOBAL_ERR1_STATUS_DDRMC_MC_NCR_MASK));

	PmRmw32(PMC_ANALOG_SCAN_CLEAR_TRIGGER,
		PMC_ANALOG_SCAN_CLEAR_TRIGGER_NOC_MASK,
		PMC_ANALOG_SCAN_CLEAR_TRIGGER_NOC_MASK);

	usleep(200);

	/* Enable NPI Clock */
	Clk = (XPm_OutClockNode *)XPmClock_GetByIdx(XPM_NODEIDX_CLK_NPI_REF);
	XPmClock_SetGate((XPm_OutClockNode *)Clk, 1);

	/* Release NPI Reset */
	XPmReset_AssertbyId(SRST_RSTID(XPM_NODEIDX_RST_NPI),
                        PM_RESET_ACTION_RELEASE);

	PmIn32((Pmc->RegAddress[0] + PMC_GLOBAL_ERR1_STATUS_OFFSET),
	       RegValue);
	if (0 != (RegValue & (PMC_GLOBAL_ERR1_STATUS_NOC_TYPE_1_NCR_MASK |
			     PMC_GLOBAL_ERR1_STATUS_DDRMC_MC_NCR_MASK))) {
		Status = XST_FAILURE;
	}

done:
	return Status;
}

static XStatus NpdMbist(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;
	u32 RegValue;
	u32 i;
	XPm_Device *Device;
	u32 DdrMcAddresses[XPM_NODEIDX_DEV_DDRMC_MAX - XPM_NODEIDX_DEV_DDRMC_MIN + 1];

	(void)Args;
	(void)NumOfArgs;

	for (i = 0; i < ARRAY_SIZE(DdrMcAddresses); i++) {
		Device = XPmDevice_GetById(DDRMC_DEVID(XPM_NODEIDX_DEV_DDRMC_MIN + i));
		DdrMcAddresses[i] = Device->Node.BaseAddress;
	}

	/* NPD pre bisr requirements - in case if bisr was skipped */
	NpdPreBisrReqs();

	if (PLATFORM_VERSION_SILICON != Platform) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Deassert PCSR Lock*/
	for (i = 0; i < ARRAY_SIZE(NpdMemIcAddresses); i++) {
		PmOut32(NpdMemIcAddresses[i] + NPI_PCSR_LOCK_OFFSET,
			PCSR_UNLOCK_VAL);
	}

	/* Enable ILA clock for DDR blocks*/
	for (i = 0; i < ARRAY_SIZE(DdrMcAddresses); i++) {
		PmOut32(DdrMcAddresses[i] + NPI_PCSR_LOCK_OFFSET, PCSR_UNLOCK_VAL);
		PmRmw32(DdrMcAddresses[i] + NOC_DDRMC_UB_CLK_GATE_OFFSET,
			NOC_DDRMC_UB_CLK_GATE_ILA_EN_MASK,
			NOC_DDRMC_UB_CLK_GATE_ILA_EN_MASK);
	}

	/* Trigger Mem clear */
	for (i = 0; i < ARRAY_SIZE(NpdMemIcAddresses); i++) {
		PmOut32(NpdMemIcAddresses[i] + NPI_PCSR_MASK_OFFSET,
			NPI_PCSR_CONTROL_MEM_CLEAR_TRIGGER_MASK)
		PmOut32(NpdMemIcAddresses[i] + NPI_PCSR_CONTROL_OFFSET,
			NPI_PCSR_CONTROL_MEM_CLEAR_TRIGGER_MASK);
	}
	for (i = 0; i < ARRAY_SIZE(DdrMcAddresses); i++) {
		PmOut32(DdrMcAddresses[i] + NPI_PCSR_MASK_OFFSET,
			NPI_PCSR_CONTROL_MEM_CLEAR_TRIGGER_MASK);
		PmOut32(DdrMcAddresses[i] + NPI_PCSR_CONTROL_OFFSET,
			NPI_PCSR_CONTROL_MEM_CLEAR_TRIGGER_MASK);
	}

	/* Check for Mem clear done */
	for (i = 0; i < ARRAY_SIZE(NpdMemIcAddresses); i++) {
		Status = XPm_PollForMask(NpdMemIcAddresses[i] +
					 NPI_PCSR_STATUS_OFFSET,
					 NPI_PCSR_STATUS_MEM_CLEAR_DONE_MASK,
					 XPM_POLL_TIMEOUT);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}
	for (i = 0; i < ARRAY_SIZE(DdrMcAddresses); i++) {
		Status = XPm_PollForMask(DdrMcAddresses[i] + NPI_PCSR_STATUS_OFFSET,
				 NPI_PCSR_STATUS_MEM_CLEAR_DONE_MASK,
				 XPM_POLL_TIMEOUT);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/* Check for Mem clear Pass/Fail */
	for (i = 0; i < ARRAY_SIZE(NpdMemIcAddresses); i++) {
		PmIn32(NpdMemIcAddresses[i] + NPI_PCSR_STATUS_OFFSET, RegValue);
		if (NPI_PCSR_STATUS_MEM_CLEAR_PASS_MASK !=
		    (RegValue & NPI_PCSR_STATUS_MEM_CLEAR_PASS_MASK)) {
			Status = XST_FAILURE;
			goto done;
		}
	}
	for (i = 0; i < ARRAY_SIZE(DdrMcAddresses); i++) {
		PmIn32(DdrMcAddresses[i] + NPI_PCSR_STATUS_OFFSET, RegValue);
		if (NPI_PCSR_STATUS_MEM_CLEAR_PASS_MASK !=
		    (RegValue & NPI_PCSR_STATUS_MEM_CLEAR_PASS_MASK)) {
			Status = XST_FAILURE;
			goto done;
		}
	}

	/* Disable ILA clock for DDR blocks*/
	for (i = 0; i < ARRAY_SIZE(DdrMcAddresses); i++) {
		PmRmw32(DdrMcAddresses[i] + NOC_DDRMC_UB_CLK_GATE_OFFSET,
			NOC_DDRMC_UB_CLK_GATE_ILA_EN_MASK, 0);
		PmOut32(DdrMcAddresses[i] + NPI_PCSR_LOCK_OFFSET, 1);
	}

	/* Assert PCSR Lock*/
	for (i = 0; i < ARRAY_SIZE(NpdMemIcAddresses); i++) {
		PmOut32(NpdMemIcAddresses[i] + NPI_PCSR_LOCK_OFFSET, 1);
	}
done:
	return Status;
}

static XStatus NpdBisr(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;
	u32 i = 0;
	XPm_Device *Device;
	u32 DdrMcAddresses[XPM_NODEIDX_DEV_DDRMC_MAX - XPM_NODEIDX_DEV_DDRMC_MIN + 1];

	(void)Args;
	(void)NumOfArgs;

	for (i = 0; i < ARRAY_SIZE(DdrMcAddresses); i++) {
		Device = XPmDevice_GetById(DDRMC_DEVID(XPM_NODEIDX_DEV_DDRMC_MIN + i));
		DdrMcAddresses[i] = Device->Node.BaseAddress;
	}

	/* NPD pre bisr requirements */
	NpdPreBisrReqs();


	if (PLATFORM_VERSION_SILICON != Platform) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Enable Bisr clock */
	for (i = 0; i < ARRAY_SIZE(DdrMcAddresses); i++) {
		/* Unlock writes */
		PmOut32(DdrMcAddresses[i] + NPI_PCSR_LOCK_OFFSET, PCSR_UNLOCK_VAL);
		PmRmw32(DdrMcAddresses[i] + NOC_DDRMC_UB_CLK_GATE_OFFSET,
			NOC_DDRMC_UB_CLK_GATE_BISR_EN_MASK,
			NOC_DDRMC_UB_CLK_GATE_BISR_EN_MASK);
	}

	/* Run BISR */
	Status = XPmBisr_Repair(DDRMC_TAG_ID);

	/* Disable Bisr clock */
	for (i = 0; i < ARRAY_SIZE(DdrMcAddresses); i++) {
		PmRmw32(DdrMcAddresses[i] + NOC_DDRMC_UB_CLK_GATE_OFFSET,
			NOC_DDRMC_UB_CLK_GATE_BISR_EN_MASK, 0);
		/* Lock writes */
		PmOut32(DdrMcAddresses[i] + NPI_PCSR_LOCK_OFFSET, 1);
	}

done:
	return Status;
}

struct XPm_PowerDomainOps NpdOps = {
	.InitStart = NpdInitStart,
	.InitFinish = NpdInitFinish,
	.ScanClear = NpdScanClear,
	.Mbist = NpdMbist,
	.Bisr = NpdBisr,
};

XStatus XPmNpDomain_Init(XPm_NpDomain *Npd, u32 Id, u32 BaseAddress,
			 XPm_Power *Parent)
{
	XPmPowerDomain_Init(&Npd->Domain, Id, BaseAddress, Parent, &NpdOps);

	return XST_SUCCESS;
}

XStatus XPmNpDomain_MemIcInit(u32 DeviceId, u32 BaseAddr)
{
	XStatus Status = XST_SUCCESS;
	u32 Idx = NODEINDEX(DeviceId);
	u32 Type = NODETYPE(DeviceId);

	if (((XPM_NODETYPE_MEMIC_SLAVE != Type) &&
	    (XPM_NODETYPE_MEMIC_MASTER != Type)) ||
	    (XPM_NODEIDX_MEMIC_MAX <= Idx)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	NpdMemIcAddresses[Idx] = BaseAddr;

done:
	return Status;
}
