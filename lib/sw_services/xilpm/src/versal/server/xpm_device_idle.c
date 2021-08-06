/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_device_idle.h"
#include "xpm_common.h"

#define XPM_MAX_TIMEOUT 		(0x1FFFFFFF)

static XPmDevice_SoftResetInfo DeviceRstData[] = {
#ifdef XILPM_USB_0
	{
		.DeviceId = PM_DEV_USB_0,
		.IdleHook = NodeUsbIdle,
		.IdleHookArgs = XILPM_USB_0,
	},
#endif
#ifdef XILPM_ETH_0
	{
		.DeviceId = PM_DEV_GEM_0,
		.IdleHook = NodeGemIdle,
		.IdleHookArgs = XILPM_ETH_0,
	},
#endif
#ifdef XILPM_ETH_1
	{
		.DeviceId = PM_DEV_GEM_1,
		.IdleHook = NodeGemIdle,
		.IdleHookArgs = XILPM_ETH_1,
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

	Status = XOspiPsv_CfgInitialize(&OspiInst, ConfigPtr);
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

	Status = XSdPs_CfgInitialize(&SdioInst, ConfigPtr, BaseAddress);
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

	Status = XUsbPsu_CfgInitialize(&UsbInst, ConfigPtr, BaseAddress);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	XUsbPsu_Idle(&UsbInst);
done:
	return Status;
}
#endif

#if defined(XILPM_ETH_0) || defined(XILPM_ETH_1)
/**
 * NodeGemIdle() - Custom code to idle the GEM
 *
 * @DeviceId:	 Device ID of GEM node
 * @BaseAddress: GEM base address
 */
XStatus NodeGemIdle(u16 DeviceId, u32 BaseAddress)
{
	XStatus Status = XST_FAILURE;
	u32 Reg;
	u32 Timeout = XPM_MAX_TIMEOUT;

	/* Warning Fix */
	(void)(DeviceId);

	/* Make sure MDIO is in IDLE state */
	do {
		Reg = XEmacPs_ReadReg(BaseAddress, XEMACPS_NWSR_OFFSET);
		Timeout--;
	} while ((0U == (Reg & XEMACPS_NWSR_MDIOIDLE_MASK)) && (Timeout > 0U));

	if (0U == (Reg & XEMACPS_NWSR_MDIOIDLE_MASK)) {
		PmWarn("gem not idle\r\n");
		goto done;
	}

	/* Stop all transactions of the Ethernet and disable all interrupts */
	XEmacPs_WriteReg(BaseAddress, XEMACPS_IDR_OFFSET, XEMACPS_IXR_ALL_MASK);

	/* Disable the receiver & transmitter */
	Reg = XEmacPs_ReadReg(BaseAddress, XEMACPS_NWCTRL_OFFSET);
	Reg &= ~((u32)XEMACPS_NWCTRL_RXEN_MASK);
	Reg &= ~((u32)XEMACPS_NWCTRL_TXEN_MASK);
	XEmacPs_WriteReg(BaseAddress, XEMACPS_NWCTRL_OFFSET, Reg);

	Status = XST_SUCCESS;

done:
	return Status;
}
#endif

#if (defined(XILPM_ZDMA_0) || defined(XILPM_ZDMA_1) || defined(XILPM_ZDMA_2) || \
	defined(XILPM_ZDMA_3) || defined(XILPM_ZDMA_4) || defined(XILPM_ZDMA_5) || \
	defined(XILPM_ZDMA_6) || defined(XILPM_ZDMA_7))

#define XZDMA_CH_OFFSET		(0x10000U)	/* Channel offset per DMA */
#define XZDMA_NUM_CHANNEL	(8U)		/* Number of Channels */
/**
 * NodeZdmaIdle() - Custom code to idle the ZDMA (GDMA and ADMA)
 *
 * @DeviceId:	 Device ID of ZDMA node
 * @BaseAddr:    ZDMA base address of the first channel
 */
XStatus NodeZdmaIdle(u16 DeviceId, u32 BaseAddr)
{
	XStatus Status = XST_FAILURE;
	u8 Channel = 0U;
	u32 RegVal = 0U, LocalTimeout;
	u32 BaseAddress = BaseAddr;

	/* Warning Fix */
	(void)(DeviceId);

	/* Idle each of the 8 Channels */
	for (Channel = 0; Channel < XZDMA_NUM_CHANNEL; Channel++) {
		/* Disable/stop the Channel */
		XZDma_WriteReg(BaseAddress, (XZDMA_CH_CTRL2_OFFSET),
			       (XZDMA_CH_CTRL2_DIS_MASK));

		/* Wait till transfers are not completed or halted */
		/* TODO: not right to use max timeout. do calibrate*/
		LocalTimeout = XPM_MAX_TIMEOUT;
		do {
			RegVal = XZDma_ReadReg(BaseAddress, XZDMA_CH_STS_OFFSET) & XZDMA_STS_BUSY_MASK;
			LocalTimeout--;
		} while ((0U != RegVal) && (LocalTimeout > 0U));

		if (0U != RegVal) {
			goto done;
		}

		/* Disable and clear all interrupts */
		XZDma_WriteReg(BaseAddress, XZDMA_CH_IDS_OFFSET, XZDMA_IXR_ALL_INTR_MASK);

		RegVal = XZDma_ReadReg(BaseAddress, XZDMA_CH_ISR_OFFSET);
		XZDma_WriteReg(BaseAddress, XZDMA_CH_ISR_OFFSET,
			       (RegVal & XZDMA_IXR_ALL_INTR_MASK));

		/* Reset all the configurations */
		XZDma_WriteReg(BaseAddress, XZDMA_CH_CTRL0_OFFSET, XZDMA_CTRL0_RESET_VALUE);
		XZDma_WriteReg(BaseAddress, XZDMA_CH_CTRL1_OFFSET, XZDMA_CTRL1_RESET_VALUE);
		XZDma_WriteReg(BaseAddress, XZDMA_CH_DATA_ATTR_OFFSET, XZDMA_DATA_ATTR_RESET_VALUE);
		XZDma_WriteReg(BaseAddress, XZDMA_CH_DSCR_ATTR_OFFSET, XZDMA_DSCR_ATTR_RESET_VALUE);

		/* Clears total byte transferred */
		RegVal = XZDma_ReadReg(BaseAddress, XZDMA_CH_TOTAL_BYTE_OFFSET);
		XZDma_WriteReg(BaseAddress, XZDMA_CH_TOTAL_BYTE_OFFSET, RegVal);

		/* Read interrupt counts to clear it on both source and destination Channels*/
		(void)XZDma_ReadReg(BaseAddress, XZDMA_CH_IRQ_SRC_ACCT_OFFSET);
		(void)XZDma_ReadReg(BaseAddress, XZDMA_CH_IRQ_DST_ACCT_OFFSET);

		/* Reset the channel's coherent attributes. */
		XZDma_WriteReg(BaseAddress, XZDMA_CH_DSCR_ATTR_OFFSET, 0x0);
		XZDma_WriteReg(BaseAddress, XZDMA_CH_SRC_DSCR_WORD3_OFFSET, 0x0);
		XZDma_WriteReg(BaseAddress, XZDMA_CH_DST_DSCR_WORD3_OFFSET, 0x0);

		BaseAddress += XZDMA_CH_OFFSET;
	}

	Status = XST_SUCCESS;

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
