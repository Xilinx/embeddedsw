/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_device_idle.h"
#include "xpm_common.h"

static XPmDevice_SoftResetInfo DeviceRstData[] = {
#ifdef XILPM_USB_0
	{
		.DeviceId = PM_DEV_USB_0,
		.IdleHook = NodeUsbIdle,
		.IdleHookArgs = XILPM_USB_0,
	},
#endif
#ifdef XILPM_OSPI_0
	{
		.DeviceId = PM_DEV_OSPI,
		.IdleHook = NodeOspiIdle,
		.IdleHookArgs = XILPM_OSPI_0,
	},
#endif
#ifdef XILPM_QSPI_0
	{
		.DeviceId = PM_DEV_QSPI,
		.IdleHook = NodeQspiIdle,
		.IdleHookArgs = XILPM_QSPI_0,
	},
#endif
#ifdef XILPM_SD_0
	{
		.DeviceId = PM_DEV_SDIO_0,
		.IdleHook = NodeSdioIdle,
		.IdleHookArgs = XILPM_SD_0,
	},
#endif
#ifdef XILPM_SD_1
	{
		.DeviceId = PM_DEV_SDIO_1,
		.IdleHook = NodeSdioIdle,
		.IdleHookArgs = XILPM_SD_1,
	},
#endif
};

#if defined(XILPM_QSPI_0)
/**
 * NodeQspiIdle() - Idle the QSPI node
 *
 * @DeviceId:	 Device ID of QSPI node
 * @BaseAddress: QSPI base address
 */
XStatus NodeQspiIdle(u16 DeviceId, u32 BaseAddress)
{
	XStatus Status = XST_FAILURE;
	const XQspiPsu_Config *ConfigPtr;
	XQspiPsu QspiInst = {0};

	ConfigPtr = XQspiPsu_LookupConfig(DeviceId);
	if (NULL == ConfigPtr) {
		goto done;
	}

	Status = XQspiPsu_CfgInitialize(&QspiInst, ConfigPtr, BaseAddress);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	XQspiPsu_Idle(&QspiInst);

done:
	return Status;
}
#endif

#if defined(XILPM_OSPI_0)
/**
 * NodeQspiIdle() - Idle the OSPI node
 *
 * @DeviceId:	 Device ID of OSPI node
 * @BaseAddress: OSPI base address
 */
XStatus NodeOspiIdle(u16 DeviceId, u32 BaseAddress)
{
	XStatus Status = XST_FAILURE;
	XOspiPsv_Config *ConfigPtr;
	XOspiPsv OspiInst = {0};

	/* Warning Fix */
	(void)(BaseAddress);

	ConfigPtr = XOspiPsv_LookupConfig(DeviceId);
	if (NULL == ConfigPtr) {
		goto done;
	}

	Status = (XStatus)XOspiPsv_CfgInitialize(&OspiInst, ConfigPtr);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	XOspiPsv_Idle(&OspiInst);

done:
	return Status;
}
#endif

#if defined(XILPM_SD_0) || defined(XILPM_SD_1)
/**
 * NodeSdioIdle() - Idle the SDIO node
 *
 * @DeviceId:	 Device ID of SDIO node
 * @BaseAddress: SDIO base address
 */
XStatus NodeSdioIdle(u16 DeviceId, u32 BaseAddress)
{
	XStatus Status = XST_FAILURE;
	XSdPs_Config *ConfigPtr;
	XSdPs SdioInst = {0};

	ConfigPtr = XSdPs_LookupConfig(DeviceId);
	if (NULL == ConfigPtr) {
		goto done;
	}

	Status = (XStatus)XSdPs_CfgInitialize(&SdioInst, ConfigPtr, BaseAddress);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XSdPs_Idle(&SdioInst);

done:
	return Status;
}
#endif

#if defined(XILPM_USB_0)
/**
 * NodeUsbIdle() - Idle the USB node
 *
 * @DeviceId:	 Device ID of USB node
 * @BaseAddress: USB base address
 */
XStatus NodeUsbIdle(u16 DeviceId, u32 BaseAddress)
{
	XStatus Status = XST_FAILURE;
	XUsbPsu_Config *ConfigPtr;
	static struct XUsbPsu UsbInst;

	ConfigPtr = XUsbPsu_LookupConfig(DeviceId);
	if (NULL == ConfigPtr) {
		goto done;
	}

	Status = (XStatus)XUsbPsu_CfgInitialize(&UsbInst, ConfigPtr, BaseAddress);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	XUsbPsu_Idle(&UsbInst);
done:
	return Status;
}
#endif

XStatus XPmDevice_SoftResetIdle(const XPm_Device *Device, const u32 IdleReq)
{
	XStatus Status = XST_FAILURE;
	u32 Idx;
	u32 DevRstDataSize = ARRAY_SIZE(DeviceRstData);
	const XPmDevice_SoftResetInfo *RstInfo = NULL;

	if (0U != DevRstDataSize) {
		for (Idx = 0; Idx < DevRstDataSize; Idx++) {
			if (Device->Node.Id == DeviceRstData[Idx].DeviceId) {
				RstInfo = &DeviceRstData[Idx];
				break;
			}
		}
	}

	if (NULL == RstInfo) {
		Status = XST_SUCCESS;
		goto done;
	}

	if (DEVICE_IDLE_REQ == IdleReq) {
		if (NULL != RstInfo->IdleHook) {
			Status = RstInfo->IdleHook(RstInfo->IdleHookArgs,
						   Device->Node.BaseAddress);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
	}

	/* TODO: Perform the device reset using its reset lines and its reset actions */

	Status = XST_SUCCESS;

done:
	return Status;
}
