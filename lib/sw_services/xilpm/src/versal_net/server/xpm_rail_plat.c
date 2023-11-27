/******************************************************************************
* Copyright (c) 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_debug.h"
#include "xpm_device.h"
#include "xpm_rail.h"

/*
 * eFuse values for VID power rails
 */
u8 VCCINT_VIDTable[] = { \
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10 \
};

u8 VCC_FPD_VIDTable[] = { \
	48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63 \
};

u8 VCC_CPM5N_VIDTable[] = { \
	48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63 \
};

XStatus XPmRail_AdjustVID(XPm_Rail *Rail)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_Device *EfuseCache;
	u32 VID;
	u8 ValidOverride = 0;
	u8 VIDIndex, Index;
	u8 Found = 0;

	/* if this rail has already been VID adjusted, then nothing more to do */
	if (1U == Rail->VIDAdjusted) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* A non-zero value in RTCA location indicates an override value */
	VID = XPm_In32(XPLMI_RTCFG_VID_OVERRIDE);
	if (0U != VID) {
		if ((0U == ((VID >> VID_CNTRL_OFFSET) & VID_CNTRL_MASK))) {
			Status = XST_SUCCESS;
			goto done;
		}

		/* Override control value should be valid for plan 1 */
		if (1U == ((VID >> VID_CNTRL_OFFSET) & VID_CNTRL_MASK)) {
			ValidOverride = 1;
		} else {
			DbgErr = XPM_INT_ERR_INVALID_VID;
			goto done;
		}
	}

	/* No override value is present so read the eFuse location */
	if (0U == ValidOverride) {
		EfuseCache = XPmDevice_GetById(PM_DEV_EFUSE_CACHE);
		if (NULL == EfuseCache) {
			DbgErr = XPM_INT_ERR_INVALID_DEVICE;
			goto done;
		}

		VID = XPm_In32(EfuseCache->Node.BaseAddress + EFUSE_CACHE_VID);
	}

	switch (Rail->Power.Node.Id) {
	case PM_POWER_VCCINT_PL:
		VIDIndex = ((VID >> VID_VCCINT_VCC_HNICX_OFFSET) &
			    VID_VCCINT_VCC_HNICX_MASK);

		/* No adjustment from default voltage if VID value is 0 */
		if (0U == VIDIndex) {
			Status = XST_SUCCESS;
			goto done;
		}

		for (Index = 0; Index < sizeof(VCCINT_VIDTable); Index++) {
			if (VCCINT_VIDTable[Index] == VIDIndex) {
				Found = 1;
				break;
			}
		}

		if (!Found) {
			DbgErr = XPM_INT_ERR_INVALID_VID;
			goto done;
		}

		break;
	case PM_POWER_VCCINT_PSFP:
		VIDIndex = ((VID >> VID_VCC_FPD_OFFSET) & VID_VCC_FPD_MASK);

		/* No adjustment from default voltage if VID value is 0 */
		if (0U == VIDIndex) {
			Status = XST_SUCCESS;
			goto done;
		}

		for (Index = 0; Index < sizeof(VCC_FPD_VIDTable); Index++) {
			if (VCC_FPD_VIDTable[Index] == VIDIndex) {
				Found = 1;
				break;
			}
		}

		if (!Found) {
			DbgErr = XPM_INT_ERR_INVALID_VID;
			goto done;
		}

		break;
	case PM_POWER_VCCINT_CPM5N:
		VIDIndex = ((VID >> VID_VCC_CPM5N_OFFSET) & VID_VCC_CPM5N_MASK);

		/* No adjustment from default voltage if VID value is 0 */
		if (0U == VIDIndex) {
			Status = XST_SUCCESS;
			goto done;
		}

		for (Index = 0; Index < sizeof(VCC_CPM5N_VIDTable); Index++) {
			if (VCC_CPM5N_VIDTable[Index] == VIDIndex) {
				Found = 1;
				break;
			}
		}

		if (!Found) {
			DbgErr = XPM_INT_ERR_INVALID_VID;
			goto done;
		}

		break;
	default:
		Status = XST_SUCCESS;
		goto done;
	}

	Status = XPmRail_Control(Rail, (u8)XPM_POWER_STATE_ON, (Index + 2));
	if (XST_SUCCESS == Status) {
		Rail->VIDAdjusted = 1;
	} else {
		DbgErr = XPM_INT_ERR_RAIL_VID;
		goto done;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
