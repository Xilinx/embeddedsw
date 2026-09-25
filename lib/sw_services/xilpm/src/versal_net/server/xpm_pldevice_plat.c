/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xpm_pldevice.h"
#include "xpm_device.h"
void XPmPlDevice_ReleaseAieDevice(XPm_PlDevice *PlDevice){
	/* There is no AIE device in current Versal_Net devices family */
	(void)PlDevice;
}

void XPmPlDevice_GetAieParent(const XPm_Device *Device, const XPm_PlDevice **OutParent){
	/* There is no AIE device in current Versal_Net devices family */
	(void)Device;
	(void)OutParent;
}

/****************************************************************************/
/**
 * @brief  Rebuild the DDRMC validation list from the device database
 *
 * @return XST_SUCCESS on success, error code otherwise
 *
 * @note   The list is normally filled in by PldMemCtrlrMap() when the PL
 *         design CDO is processed. An in-place PLM update does not touch the
 *         PL, so that CDO never runs again, and the list starts out empty in
 *         the new PLM. Rebuild it here from the DDRMC nodes, which the update
 *         does restore, so that DDR addresses keep validating after an update.
 *
 *         Every DDRMC node that is present gets added, not just the ones the
 *         PL design CDO described. A node that was never described has no
 *         address regions, so it makes no difference to validation.
 *
 ****************************************************************************/
XStatus XPmPlDevice_RebuildDdrmcValidationList(void)
{
	XStatus Status = XST_FAILURE;
	const XPm_MemCtrlrDevice *MCDev = NULL;
	u32 Idx = (u32)XPM_NODEIDX_DEV_DDRMC_MIN;

	DdrmcValidationCount = 0U;

	while (Idx <= (u32)XPM_NODEIDX_DEV_DDRMC_MAX) {
		/*
		 * Skip the gap between the two DDRMC node index ranges. The
		 * ranges are next to each other today, so this does nothing;
		 * it is kept in step with PldMemCtrlrMap(), which is where new
		 * DDRMC node ranges get added.
		 */
#ifdef XPM_NODEIDX_DEV_DDRMC_MAX_INT_1
		if (((u32)XPM_NODEIDX_DEV_DDRMC_MAX_INT_1 + 1U) == Idx) {
			Idx = (u32)XPM_NODEIDX_DEV_DDRMC_MIN_INT_2;
		}
#endif
		MCDev = (const XPm_MemCtrlrDevice *)XPmDevice_GetById(DDRMC_DEVID(Idx));
		if (NULL != MCDev) {
			/*
			 * PldMemCtrlrMap() checks RegionCount when it fills a
			 * node in, but the update restores the saved data as
			 * is. Check it again here so that a node which would
			 * make XPm_IsMemAddressValid() read past Region[] can
			 * never get into the list.
			 */
			if (MCDev->RegionCount > (u8)ARRAY_SIZE(MCDev->Region)) {
				PmWarn("Skip 0x%x, RegionCount %u\n\r",
					MCDev->Device.Node.Id, MCDev->RegionCount);
			}
			/* Defensive: the walk cannot exceed the list capacity */
			else if (DdrmcValidationCount >= (u8)DDRMC_VALIDATION_LIST_MAX) {
				Status = XST_BUFFER_TOO_SMALL;
				goto done;
			}
			else {
				DdrmcValidationList[DdrmcValidationCount] = MCDev;
				DdrmcValidationCount++;
			}
		}
		Idx++;
	}

	PmDbg("DDRMC entries=%u\n\r", DdrmcValidationCount);

	Status = XST_SUCCESS;

done:
	return Status;
}
