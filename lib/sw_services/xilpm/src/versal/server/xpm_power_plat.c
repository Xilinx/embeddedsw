/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_bisr.h"
#include "xpm_power.h"
#include "xpm_regs.h"
#include "xpm_powerdomain.h"
#include "xpm_pslpdomain.h"
#include "xpm_psm.h"
#include "sleep.h"
#include "xpm_debug.h"
#include "xpm_rpucore.h"

static XStatus PowerUpXram(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;
	const XPm_Device *Device;
	u32 XramSlcrAddress, PwrCtlAddress, PwrStatusAddress, RegVal;
	u32 BitMask;

	Device = XPmDevice_GetByIndex((u32)XPM_NODEIDX_DEV_XRAM_0);
	if (NULL == Device) {
		goto done;
	}
	XramSlcrAddress = Device->Node.BaseAddress;
	BitMask = Node->BaseAddress;

	switch (NODEINDEX(Node->Id)) {
	case (u32)XPM_NODEIDX_POWER_XRAM_0:
	case (u32)XPM_NODEIDX_POWER_XRAM_1:
	case (u32)XPM_NODEIDX_POWER_XRAM_2:
	case (u32)XPM_NODEIDX_POWER_XRAM_3:
		PwrCtlAddress = XramSlcrAddress + XRAM_SLCR_PWR_UP_BANK0_OFFSET;
		PwrStatusAddress = XramSlcrAddress + XRAM_SLCR_PWR_STATUS_BANK0_OFFSET;
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODEIDX_POWER_XRAM_4:
	case (u32)XPM_NODEIDX_POWER_XRAM_5:
	case (u32)XPM_NODEIDX_POWER_XRAM_6:
	case (u32)XPM_NODEIDX_POWER_XRAM_7:
		PwrCtlAddress = XramSlcrAddress + XRAM_SLCR_PWR_UP_BANK1_OFFSET;
		PwrStatusAddress = XramSlcrAddress + XRAM_SLCR_PWR_STATUS_BANK1_OFFSET;
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODEIDX_POWER_XRAM_8:
	case (u32)XPM_NODEIDX_POWER_XRAM_9:
	case (u32)XPM_NODEIDX_POWER_XRAM_10:
	case (u32)XPM_NODEIDX_POWER_XRAM_11:
		PwrCtlAddress = XramSlcrAddress + XRAM_SLCR_PWR_UP_BANK2_OFFSET;
		PwrStatusAddress = XramSlcrAddress + XRAM_SLCR_PWR_STATUS_BANK2_OFFSET;
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODEIDX_POWER_XRAM_12:
	case (u32)XPM_NODEIDX_POWER_XRAM_13:
	case (u32)XPM_NODEIDX_POWER_XRAM_14:
	case (u32)XPM_NODEIDX_POWER_XRAM_15:
		PwrCtlAddress = XramSlcrAddress + XRAM_SLCR_PWR_UP_BANK3_OFFSET;
		PwrStatusAddress = XramSlcrAddress + XRAM_SLCR_PWR_STATUS_BANK3_OFFSET;
		Status = XST_SUCCESS;
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Check if already power up */
	RegVal = XPm_In32(PwrStatusAddress);
	if ((RegVal & BitMask) == BitMask) {
		goto done;
	}

	/* Enable power state for selected bank */
	PmRmw32(PwrCtlAddress, BitMask, BitMask);

	/* Poll for power status to set */
	Status = XPm_PollForMask(PwrStatusAddress, BitMask, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* TODO: Wait for power to ramp up */
	usleep(1);

	/* TODO: Set chip enable bit */

done:
	return Status;
}

static XStatus PowerDwnXram(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;
	const XPm_Device *Device;
	u32 XramSlcrAddress, PwrCtlAddress, PwrStatusAddress, RegVal;
	u32 BitMask;

	Device = XPmDevice_GetByIndex((u32)XPM_NODEIDX_DEV_XRAM_0);
	if (NULL == Device) {
		goto done;
	}
	XramSlcrAddress = Device->Node.BaseAddress;
	BitMask = Node->BaseAddress;

	switch (NODEINDEX(Node->Id)) {
	case (u32)XPM_NODEIDX_POWER_XRAM_0:
	case (u32)XPM_NODEIDX_POWER_XRAM_1:
	case (u32)XPM_NODEIDX_POWER_XRAM_2:
	case (u32)XPM_NODEIDX_POWER_XRAM_3:
		PwrCtlAddress = XramSlcrAddress + XRAM_SLCR_PWR_DWN_BANK0_OFFSET;
		PwrStatusAddress = XramSlcrAddress + XRAM_SLCR_PWR_STATUS_BANK0_OFFSET;
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODEIDX_POWER_XRAM_4:
	case (u32)XPM_NODEIDX_POWER_XRAM_5:
	case (u32)XPM_NODEIDX_POWER_XRAM_6:
	case (u32)XPM_NODEIDX_POWER_XRAM_7:
		PwrCtlAddress = XramSlcrAddress + XRAM_SLCR_PWR_DWN_BANK1_OFFSET;
		PwrStatusAddress = XramSlcrAddress + XRAM_SLCR_PWR_STATUS_BANK1_OFFSET;
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODEIDX_POWER_XRAM_8:
	case (u32)XPM_NODEIDX_POWER_XRAM_9:
	case (u32)XPM_NODEIDX_POWER_XRAM_10:
	case (u32)XPM_NODEIDX_POWER_XRAM_11:
		PwrCtlAddress = XramSlcrAddress + XRAM_SLCR_PWR_DWN_BANK2_OFFSET;
		PwrStatusAddress = XramSlcrAddress + XRAM_SLCR_PWR_STATUS_BANK2_OFFSET;
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODEIDX_POWER_XRAM_12:
	case (u32)XPM_NODEIDX_POWER_XRAM_13:
	case (u32)XPM_NODEIDX_POWER_XRAM_14:
	case (u32)XPM_NODEIDX_POWER_XRAM_15:
		PwrCtlAddress = XramSlcrAddress + XRAM_SLCR_PWR_DWN_BANK3_OFFSET;
		PwrStatusAddress = XramSlcrAddress + XRAM_SLCR_PWR_STATUS_BANK3_OFFSET;
		Status = XST_SUCCESS;
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Check if already power up */
	RegVal = XPm_In32(PwrStatusAddress);
	if ((RegVal & BitMask) == 0U) {
		goto done;
	}

	/* TODO: Clear chip enable bit */

	/* Disable power state for selected bank */
	PmRmw32(PwrCtlAddress, BitMask, BitMask);

	/* Poll for power status to clear */
	Status = XPm_PollForZero(PwrStatusAddress, BitMask, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}


done:
	return Status;
}

XStatus XPmPower_SendIslandPowerUpReq(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;
	const XPm_PsLpDomain *LpDomain = (XPm_PsLpDomain *)XPmPower_GetById(PM_POWER_LPD);
	u32 Platform;

	if (NULL == LpDomain) {
		Status = XST_FAILURE;
		goto done;
	}

	if ((u32)XPM_NODETYPE_POWER_ISLAND_XRAM == NODETYPE(Node->Id)) {
		Status = PowerUpXram(Node);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	} else {
		Status = XPmPsm_SendPowerUpReq(Node->BaseAddress);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Platform = XPm_GetPlatform();
	if ((PLATFORM_VERSION_SILICON_ES1 == XPm_GetPlatformVersion()) &&
	    ((u32)PLATFORM_VERSION_SILICON == Platform) && ((u32)XPM_NODEIDX_POWER_RPU0_0 ==
	    NODEINDEX(Node->Id)) && (0U == (LPD_BISR_DONE & LpDomain->LpdBisrFlags))) {
		if (0U != (LPD_BISR_DATA_COPIED & LpDomain->LpdBisrFlags)) {
			Status = XPmBisr_TriggerLpd();
		} else {
			Status = XPmBisr_Repair(LPD_TAG_ID);
		}
	}

done:
	return Status;
}

XStatus XPmPower_SendIslandPowerDwnReq(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;
	XPm_PsLpDomain *LpDomain = (XPm_PsLpDomain *)XPmPower_GetById(PM_POWER_LPD);

	if (NULL == LpDomain) {
		Status = XST_FAILURE;
		goto done;
	}

	if ((u32)XPM_NODETYPE_POWER_ISLAND_XRAM == NODETYPE(Node->Id)) {
		Status = PowerDwnXram(Node);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	} else {
		Status = XPmPsm_SendPowerDownReq(Node->BaseAddress);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}


	/*
	 * For XCVC1902 ES1, there is a bug in LPD which requires the
	 * LPD_INT and RPU power domain signals needs to be
	 * asserted, to prevent repair vector corruption(EDT-993543).
	 * To fix this bug rerun LPD BISR whenever the RPU power
	 * island is powered down and brought up again.
	 */
	if ((u32)XPM_NODEIDX_POWER_RPU0_0 == NODEINDEX(Node->Id)) {
		LpDomain->LpdBisrFlags &= (u8)(~(LPD_BISR_DONE));
		/*
		 * Clear and Disable RPU internal reg comparators to prevent
		 * PSM_GLOBAL_REG.PSM_ERR1_STATUS.rpu_ls from erroring out
		 */
		Status = XPm_RpuRstComparators(PM_DEV_RPU0_0);

		if (Status != XST_SUCCESS) {
			PmErr("Unable to reset RPU Comparators\n\r");
			goto done;
		}
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmPower_PlatSendPowerUpReq(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;

	switch (NODEINDEX(Node->Id)) {
	case (u32)XPM_NODEIDX_POWER_ME:
	case (u32)XPM_NODEIDX_POWER_ME2:
		Status = XPm_PowerUpME(Node);
		break;
	case (u32)XPM_NODEIDX_POWER_CPM:
		Status = XPm_PowerUpCPM(Node);
		break;
	case (u32)XPM_NODEIDX_POWER_CPM5:
		Status = XPm_PowerUpCPM5(Node);
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

	return Status;
}

XStatus XPmPower_PlatSendPowerDownReq(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;

	switch (NODEINDEX(Node->Id)) {
	case (u32)XPM_NODEIDX_POWER_ME:
	case (u32)XPM_NODEIDX_POWER_ME2:
		Status = XPm_PowerDwnME(Node);
		break;
	case (u32)XPM_NODEIDX_POWER_CPM:
		Status = XPm_PowerDwnCPM(Node);
		break;
	case (u32)XPM_NODEIDX_POWER_CPM5:
		Status = XPm_PowerDwnCPM5(Node);
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

	return Status;
}
