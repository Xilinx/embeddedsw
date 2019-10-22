/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_device_idle.h"
#include "xpm_common.h"

#define XPM_MAX_TIMEOUT 		(0x1FFFFFFF)

static XPmDevice_SoftResetInfo DeviceRstData[] = {
#ifdef XPAR_XUSBPSU_0_DEVICE_ID
	{
		.DeviceId = PM_DEV_USB_0,
		.SoftRst = NULL,
		.IdleHook = NodeUsbIdle,
		.IdleHookArgs = XPAR_XUSBPSU_0_DEVICE_ID,
	},
#endif
#ifdef XPAR_PSV_ETHERNET_0_DEVICE_ID
	{
		.DeviceId = PM_DEV_GEM_0,
		.SoftRst = NULL,
		.IdleHook = NodeGemIdle,
		.IdleHookArgs = XPAR_PSV_ETHERNET_0_DEVICE_ID,
	},
#endif
#ifdef XPAR_PSV_ETHERNET_1_DEVICE_ID
	{
		.DeviceId = PM_DEV_GEM_1,
		.SoftRst = NULL,
		.IdleHook = NodeGemIdle,
		.IdleHookArgs = XPAR_PSV_ETHERNET_1_DEVICE_ID,
	},
#endif
#ifdef XPAR_PSV_OSPI_0_DEVICE_ID
	{
		.DeviceId = PM_DEV_OSPI,
		.SoftRst = NULL,
		.IdleHook = NodeOspiIdle,
		.IdleHookArgs = XPAR_PSV_OSPI_0_DEVICE_ID,
	},
#endif
#ifdef XPAR_PSV_QSPI_0_DEVICE_ID
	{
		.DeviceId = PM_DEV_QSPI,
		.SoftRst = NULL,
		.IdleHook = NodeQspiIdle,
		.IdleHookArgs = XPAR_PSV_QSPI_0_DEVICE_ID,
	},
#endif
#ifdef XPAR_PSV_SD_0_DEVICE_ID
	{
		.DeviceId = PM_DEV_SDIO_0,
		.SoftRst = NULL,
		.IdleHook = NodeSdioIdle,
		.IdleHookArgs = XPAR_PSV_SD_0_DEVICE_ID,
	},
#endif
#ifdef XPAR_PSV_SD_1_DEVICE_ID
	{
		.DeviceId = PM_DEV_SDIO_1,
		.SoftRst = NULL,
		.IdleHook = NodeSdioIdle,
		.IdleHookArgs = XPAR_PSV_SD_1_DEVICE_ID,
	},
#endif
};

#if defined(XPAR_PSV_QSPI_0_DEVICE_ID)
/**
 * NodeQspiIdle() - Idle the QSPI node
 *
 * @DeviceId:	 Device ID of QSPI node
 * @BaseAddress: QSPI base address
 */
void NodeQspiIdle(u16 DeviceId, u32 BaseAddress)
{
	int Status = XST_FAILURE;
	XQspiPsu_Config *ConfigPtr;
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
	return;
}
#endif

#if defined(XPAR_PSV_OSPI_0_DEVICE_ID)
/**
 * NodeQspiIdle() - Idle the OSPI node
 *
 * @DeviceId:	 Device ID of OSPI node
 * @BaseAddress: OSPI base address
 */
void NodeOspiIdle(u16 DeviceId, u32 BaseAddress)
{
	int Status = XST_FAILURE;
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
	return;
}
#endif

#if defined(XPAR_PSV_SD_0_DEVICE_ID) || defined(XPAR_PSV_SD_1_DEVICE_ID)
/**
 * NodeSdioIdle() - Idle the SDIO node
 *
 * @DeviceId:	 Device ID of SDIO node
 * @BaseAddress: SDIO base address
 */
void NodeSdioIdle(u16 DeviceId, u32 BaseAddress)
{
	int Status = XST_FAILURE;
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

	XSdPs_Idle(&SdioInst);

done:
	return;
}
#endif

#if defined(XPAR_XUSBPSU_0_DEVICE_ID)
/**
 * NodeUsbIdle() - Idle the USB node
 *
 * @DeviceId:	 Device ID of USB node
 * @BaseAddress: USB base address
 */
void NodeUsbIdle(u16 DeviceId, u32 BaseAddress)
{
	int Status = XST_FAILURE;
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
	return;
}
#endif

#if defined(XPAR_PSV_ETHERNET_0_DEVICE_ID) || defined(XPAR_PSV_ETHERNET_1_DEVICE_ID)
/**
 * NodeGemIdle() - Custom code to idle the GEM
 *
 * @DeviceId:	 Device ID of GEM node
 * @BaseAddress: GEM base address
 */
void NodeGemIdle(u16 DeviceId, u32 BaseAddress)
{
	u32 Reg;
	u32 Timeout = XPM_MAX_TIMEOUT;

	/* Warning Fix */
	(void)(DeviceId);

	/* Make sure MDIO is in IDLE state */
	do {
		Reg = XEmacPs_ReadReg(BaseAddress, XEMACPS_NWSR_OFFSET);
		Timeout--;
	} while ((0U == (Reg & XEMACPS_NWSR_MDIOIDLE_MASK)) && (Timeout > 0U));

	if (0U == Timeout) {
		PmWarn("gem not idle\r\n");
	}

	/* Stop all transactions of the Ethernet and disable all interrupts */
	XEmacPs_WriteReg(BaseAddress, XEMACPS_IDR_OFFSET, XEMACPS_IXR_ALL_MASK);

	/* Disable the receiver & transmitter */
	Reg = XEmacPs_ReadReg(BaseAddress, XEMACPS_NWCTRL_OFFSET);
	Reg &= ~((u32)XEMACPS_NWCTRL_RXEN_MASK);
	Reg &= ~((u32)XEMACPS_NWCTRL_TXEN_MASK);
	XEmacPs_WriteReg(BaseAddress, XEMACPS_NWCTRL_OFFSET, Reg);
}
#endif

#if defined(XPAR_PSV_GDMA_0_DEVICE_ID) || defined(XPAR_PSV_ADMA_0_DEVICE_ID)

#define XZDMA_CH_OFFSET		(0x10000U)	/* Channel offset per DMA */
#define XZDMA_NUM_CHANNEL	(8U)		/* Number of Channels */
/**
 * NodeZdmaIdle() - Custom code to idle the ZDMA (GDMA and ADMA)
 *
 * @DeviceId:	 Device ID of ZDMA node
 * @BaseAddress: ZDMA base address of the first channel
 */
void NodeZdmaIdle(u16 DeviceId, u32 BaseAddress)
{
	u8 Channel = 0U;
	u32 RegVal = 0U, LocalTimeout;

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
}
#endif

void XPmDevice_SoftResetIdle(XPm_Device *Device, const u32 IdleReq)
{
	u32 Idx;
	u32 DevRstDataSize = ARRAY_SIZE(DeviceRstData);
	XPmDevice_SoftResetInfo *RstInfo = NULL;

	if (0U != DevRstDataSize) {
		for (Idx = 0; Idx < DevRstDataSize; Idx++) {
			if (Device->Node.Id == DeviceRstData[Idx].DeviceId) {
				RstInfo = &DeviceRstData[Idx];
				break;
			}
		}
	}

	if (NULL == RstInfo) {
		return;
	}

	if (DEVICE_IDLE_REQ == IdleReq) {
		if (NULL != RstInfo->IdleHook) {
			RstInfo->IdleHook(RstInfo->IdleHookArgs,
					  Device->Node.BaseAddress);
		}

		if (NULL != RstInfo->SoftRst) {
			RstInfo->SoftRst(Device->Node.BaseAddress);
		}
	}

	/* Perform the device reset using its reset lines and its reset actions */
}
