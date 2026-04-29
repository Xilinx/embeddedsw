/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xplmi.h"
#include "xpm_defs.h"
#include "xpm_ioctl.h"
#include "xpm_rpucore.h"
#include "xpm_pmc.h"
#include "xpm_power.h"
#include "xpm_regs.h"
#include "xpm_aie.h"
#include "xpm_api.h"
#include "xpm_debug.h"
#include "xpm_device.h"
#include "xpm_access.h"
#include "xpm_aiedevice.h"
#include "xpm_requirement.h"
#include "xpm_common.h"

/**
 * @brief Clear AIE ISR status.
 *
 * Clears the AIE interrupt status register
 *
 * @param Value ISR bits to be cleared.
 *
 * @return XST_SUCCESS on success, error code otherwise.
 */
XStatus XPm_AieISRClear(u32 Value)
{
	XStatus Status = XST_FAILURE;
	u32 IntrClear = 0x0U;
	u32 IdCode = XPm_GetIdCode();
	u32 PlatformVersion = XPm_GetPlatformVersion();


	XPm_AieDomain *AieDomain = XPmAie_GetDomain();
	if (NULL == AieDomain) {
		Status = XPM_INVALID_PWRDOMAIN;
		goto done;
	}

	/* Only needed for XCVC1902 ES1 devices */
	if ((PLATFORM_VERSION_SILICON == XPm_GetPlatform()) &&
	    (PLATFORM_VERSION_SILICON_ES1 == PlatformVersion) &&
	    (PMC_TAP_IDCODE_DEV_SBFMLY_VC1902 == (IdCode & PMC_TAP_IDCODE_DEV_SBFMLY_MASK))) {
		/* Unlock the AIE PCSR register to allow register writes */
		XPm_UnlockPcsr(AieDomain->AieNpiAddress);

		/* Clear ISR */
		IntrClear = Value & ME_NPI_ME_ISR_MASK;
		XPm_Out32(AieDomain->AieNpiAddress + ME_NPI_ME_ISR_OFFSET, IntrClear);

		/* Re-lock the AIE PCSR register for protection */
		XPm_LockPcsr(AieDomain->AieNpiAddress);
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPm_AieOperation(u32 SubsystemId, u32 Id, pm_ioctl_id IoctlId, u32 Part, u32 Ops, u32 Arg3)
{
	XStatus Status = XST_FAILURE;
	(void)Id;
	(void)SubsystemId;
	(void)Arg3;

	u32 NumArgs = 4U;
	u32 ArgBuf[4U];
	ArgBuf[0] = Id;
	ArgBuf[1] = (u32)IoctlId;
	ArgBuf[2] = Part;
	ArgBuf[3] = Ops;

	/* Forward message event to secondary SLR if required.
	 * Use a longer timeout for AIE operations since zeroization
	 * of large AIE arrays can take significant time.
	 */
	Status = XPm_SsitForwardApiExt(PM_IOCTL, ArgBuf, NumArgs,
				       (u32)NO_HEADER_CMDTYPE, NULL,
				       TIMEOUT_AIE_OPS_COMPL);
	if (XST_DEVICE_NOT_FOUND != Status){
		/* API is forwarded, nothing else to be done */
		goto done;
	}

	/* To-Do: Add Permission Check */

	if (IOCTL_AIE_OPS != IoctlId) {
		Status = XPM_ERR_IOCTL;
		goto done;
	}

	Status = Aie_Operations(Part, Ops);

done:
	return Status;
}

XStatus XPm_GetQos(const u32 DeviceId, pm_ioctl_id IoctlId, u32 *Response)
{
	XStatus Status = XST_FAILURE;

	u32 NumArgs = 2U;
	u32 ArgBuf[2U];
	ArgBuf[0] = DeviceId;
	ArgBuf[1] = (u32)IoctlId;

	/* Forward message event to secondary SLR if required */
	Status = XPm_SsitForwardApi(PM_IOCTL, ArgBuf, NumArgs,
				    (u32)NO_HEADER_CMDTYPE, Response);
	if (XST_DEVICE_NOT_FOUND != Status){
		/* API is forwarded, nothing else to be done */
		goto done;
	}

	if (IS_DEV_AIE(DeviceId)) {
		Status = XPmAieDevice_QueryDivider(Response);
	} else {
		/* Device not supported */
		Status = XST_INVALID_PARAM;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  IOCTL to set AIE clock divider
 *
 * @param  DeviceId: Device Id of the AIE device
 * @param  Divider: Requested divider value
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note none
 *
 ****************************************************************************/
XStatus XPm_SetAieClkDiv(const u32 DeviceId, const u32 Divider)
{
	XStatus Status = XST_FAILURE;

	if (!(IS_DEV_AIE(DeviceId))) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPmAieDevice_UpdateClockDiv(Divider);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  IOCTL read secondary SLR temperature data
 *
 * @param  DeviceId: Device Id (Sysmon node ID + SLR number)
 * @param  IoctlId: IOCTL Id
 * @param  Offset: Local sysmon min/max temperature register offset
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note None
 *
 ****************************************************************************/
XStatus XPm_GetSsitTemp(u32 DeviceId, pm_ioctl_id IoctlId,
			u32 Offset, u32 *const Response)
{
	XStatus Status = XST_FAILURE;
	u32 DataIn = 0U;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	u32 NumArgs = 3U;
	u32 ArgBuf[3];
	ArgBuf[0] = DeviceId;
	ArgBuf[1] = (u32)IoctlId;
	ArgBuf[2] = Offset;

	/* Forward message event to secondary SLR if required */
	Status = XPm_SsitForwardApi(PM_IOCTL, ArgBuf, NumArgs, (u32)NO_HEADER_CMDTYPE, Response);
	if (XST_DEVICE_NOT_FOUND != Status) {
		/* API is forwarded, nothing else to be done */
		goto done;
	}

	/* Check if offset is valid */
	if ((PMC_SYSMON_DEVICE_TEMP_MIN_OFFSET != Offset) &&
	    (PMC_SYSMON_DEVICE_TEMP_MAX_OFFSET != Offset)) {
		DbgErr = XPM_INT_ERR_INVALID_ADDR;
		Status = XST_FAILURE;
		goto done;
	}

	/* Read temperature data */
	DataIn = XPm_In32(PMC_SYSMON_BASEADDR + Offset);
	*Response = DataIn;

	Status = XST_SUCCESS;
done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
