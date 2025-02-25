/******************************************************************************
* Copyright (c) 2024 -2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xil_util.h"
#include "xpm_pinfunc.h"

#define FUNC_QUERY_NAME_LEN	(FUNC_NAME_SIZE)

/****************************************************************************/
/**
 * @brief  This function returns total number of functions available.
 *
 * @param NumFuncs	Number of functions.
 *
 * @return XST_SUCCESS.
 *
 ****************************************************************************/
XStatus XPmPinFunc_GetNumFuncs(u32 *NumFuncs)
{
	*NumFuncs = (u32)MAX_FUNCTION;
	return XST_SUCCESS;
}

/****************************************************************************/
/**
 * @brief  This function returns function name based on function ID.
 *
 * @param FuncId	Function ID.
 * @param FuncName	Name of the function.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code.
 *
 ****************************************************************************/
XStatus XPmPinFunc_GetFuncName(u32 FuncId, char *FuncName)
{
	XStatus Status = XST_FAILURE;
	const XPm_PinFunc *PinFunc = NULL;
	const u32 CopySize = FUNC_QUERY_NAME_LEN;

	Status = Xil_SMemSet(FuncName, FUNC_QUERY_NAME_LEN, 0, FUNC_QUERY_NAME_LEN);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	PinFunc = XPmPinFunc_GetById(FuncId);
	if (NULL == PinFunc) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = Xil_SMemCpy(FuncName, CopySize, &PinFunc->Name[0], CopySize, CopySize);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function returns number of groups in function based on
 *         function ID.
 *
 * @param FuncId	Function ID.
 * @param NumGroups	Number of groups.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code.
 *
 ****************************************************************************/
XStatus XPmPinFunc_GetNumFuncGroups(u32 FuncId, u32 *NumGroups)
{
	XStatus Status = XST_FAILURE;
	const XPm_PinFunc *PinFunc = NULL;

	PinFunc = XPmPinFunc_GetById(FuncId);
	if (NULL != PinFunc) {
		*NumGroups = PinFunc->NumGroups;
		Status = XST_SUCCESS;
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function returns groups present in function based on
 *         function ID. Index 0 returns the first 6 group IDs, index 6
 *         returns the next 6 group IDs, and so forth.
 *
 * @param FuncId	Function ID.
 * @param Index		Index of next function groups
 * @param Groups	Function groups.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code.
 *
 ****************************************************************************/
XStatus XPmPinFunc_GetFuncGroups(u32 FuncId, u32 Index, u16 *Groups)
{
	XStatus Status = XST_FAILURE;
	u32 i;
	u32 num_read;
	const XPm_PinFunc *PinFunc = NULL;
	u32 Size = MAX_GROUPS_PER_RES * sizeof(u16);

	Status = Xil_SMemSet(Groups, Size, (s32)END_OF_GRP, Size);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	PinFunc = XPmPinFunc_GetById(FuncId);
	if ((NULL == PinFunc) || (Index > PinFunc->NumGroups)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Read up to 6 group IDs from Index */
	if ((PinFunc->NumGroups - Index) > MAX_GROUPS_PER_RES) {
		num_read = MAX_GROUPS_PER_RES;
	} else {
		num_read = PinFunc->NumGroups - Index;
	}

	for (i = 0; i < num_read; i++) {
		Groups[i] = PinFunc->Groups[i + Index];
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

/* TODO: Each function can not be mapped with their corresponding
 *       device. Keeping those devIdx as 0.
 */
static XPm_PinFunc PmPinFuncs[MAX_FUNCTION] = {
	[PIN_FUNC_SD0] = {
		.Id = (u8)PIN_FUNC_SD0,
		.Name = "sd0",
		.DevIdx = (u16)XPM_NODEIDX_DEV_SDIO_0,
		.LmioRegMask = 0x0U,
		.PmioRegMask = 0x4U,
		.NumPins = 10U,
		.NumGroups = 2U,
		.Groups = ((u16 []) {
			PIN_GRP_SD0_0,
			PIN_GRP_SD0_1,
		}),
	},
	[PIN_FUNC_SD1] = {
		.Id = (u8)PIN_FUNC_SD1,
		.Name = "sd1",
		.DevIdx = (u16)XPM_NODEIDX_DEV_SDIO_1,
		.LmioRegMask = 0x0U,
		.PmioRegMask = 0x2U,
		.NumPins = 11U,
		.NumGroups = 2U,
		.Groups = ((u16 []) {
			PIN_GRP_SD1_0,
			PIN_GRP_SD1_1,
		}),
	},
	[PIN_FUNC_UFS] = {
		.Id = (u8)PIN_FUNC_UFS,
		.Name = "ufs",
		.DevIdx = (u16)XPM_NODEIDX_DEV_UFS,
		.LmioRegMask = 0x0U,
		.PmioRegMask = 0x8U,
		.NumPins = 3U,
		.NumGroups = 4U,
		.Groups = ((u16 []) {
			PIN_GRP_UFS_0,
			PIN_GRP_UFS_1,
			PIN_GRP_UFS_2,
			PIN_GRP_UFS_3,
		}),
	},
	[PIN_FUNC_CAN0] = {
		.Id = (u8)PIN_FUNC_CAN0,
		.Name = "can0",
		.DevIdx = (u16)XPM_NODEIDX_DEV_CAN_FD_0,
		.LmioRegMask = 0x180U,
		.PmioRegMask = 0x180U,
		.NumPins = 2U,
		.NumGroups = 10U,
		.Groups = ((u16 []) {
			PIN_GRP_CAN0_0,
			PIN_GRP_CAN0_1,
			PIN_GRP_CAN0_2,
			PIN_GRP_CAN0_3,
			PIN_GRP_CAN0_4,
			PIN_GRP_CAN0_5,
			PIN_GRP_CAN0_6,
			PIN_GRP_CAN0_7,
			PIN_GRP_CAN0_8,
			PIN_GRP_CAN0_9,
		}),
	},
	[PIN_FUNC_CAN1] = {
		.Id = (u8)PIN_FUNC_CAN1,
		.Name = "can1",
		.DevIdx = (u16)XPM_NODEIDX_DEV_CAN_FD_1,
		.LmioRegMask = 0x180U,
		.PmioRegMask = 0x180U,
		.NumPins = 2U,
		.NumGroups = 11U,
		.Groups = ((u16 []) {
			PIN_GRP_CAN1_0,
			PIN_GRP_CAN1_1,
			PIN_GRP_CAN1_2,
			PIN_GRP_CAN1_3,
			PIN_GRP_CAN1_4,
			PIN_GRP_CAN1_5,
			PIN_GRP_CAN1_6,
			PIN_GRP_CAN1_7,
			PIN_GRP_CAN1_8,
			PIN_GRP_CAN1_9,
			PIN_GRP_CAN1_10,
		}),
	},
	[PIN_FUNC_CAN2] = {
		.Id = (u8)PIN_FUNC_CAN2,
		.Name = "can2",
		.DevIdx = 0U,
		.LmioRegMask = 0x180U,
		.PmioRegMask = 0x180U,
		.NumPins = 2U,
		.NumGroups = 8U,
		.Groups = ((u16 []) {
			PIN_GRP_CAN2_0,
			PIN_GRP_CAN2_1,
			PIN_GRP_CAN2_2,
			PIN_GRP_CAN2_3,
			PIN_GRP_CAN2_4,
			PIN_GRP_CAN2_5,
			PIN_GRP_CAN2_6,
			PIN_GRP_CAN2_7,
		}),
	},
	[PIN_FUNC_CAN3] = {
		.Id = (u8)PIN_FUNC_CAN3,
		.Name = "can3",
		.DevIdx = 0U,
		.LmioRegMask = 0x180U,
		.PmioRegMask = 0x180U,
		.NumPins = 2U,
		.NumGroups = 8U,
		.Groups = ((u16 []) {
			PIN_GRP_CAN3_0,
			PIN_GRP_CAN3_1,
			PIN_GRP_CAN3_2,
			PIN_GRP_CAN3_3,
			PIN_GRP_CAN3_4,
			PIN_GRP_CAN3_5,
			PIN_GRP_CAN3_6,
			PIN_GRP_CAN3_7,
		}),
	},
	[PIN_FUNC_GEM0] = {
		.Id = (u8)PIN_FUNC_GEM0,
		.Name = "gem0",
		.DevIdx = (u16)XPM_NODEIDX_DEV_GEM_0,
		.LmioRegMask = 0x4U,
		.PmioRegMask = 0x6U,
		.NumPins = 12U,
		.NumGroups = 2U,
		.Groups = ((u16 []) {
			PIN_GRP_GEM0_0,
			PIN_GRP_GEM0_1,
		}),
	},
	[PIN_FUNC_GEM1] = {
		.Id = (u8)PIN_FUNC_GEM1,
		.Name = "gem1",
		.DevIdx = (u16)XPM_NODEIDX_DEV_GEM_1,
		.LmioRegMask = 0x4U,
		.PmioRegMask = 0x6U,
		.NumPins = 12U,
		.NumGroups = 2U,
		.Groups = ((u16 []) {
			PIN_GRP_GEM1_0,
			PIN_GRP_GEM1_1,
		}),
	},
	[PIN_FUNC_I2C0] = {
		.Id = (u8)PIN_FUNC_I2C0,
		.Name = "i2c0",
		.DevIdx = (u16)XPM_NODEIDX_DEV_I2C_0,
		.LmioRegMask = 0x200U,
		.PmioRegMask = 0x80U,
		.NumPins = 2U,
		.NumGroups = 6U,
		.Groups = ((u16 []) {
			PIN_GRP_I2C0_0,
			PIN_GRP_I2C0_1,
			PIN_GRP_I2C0_2,
			PIN_GRP_I2C0_3,
			PIN_GRP_I2C0_4,
			PIN_GRP_I2C0_5,
		}),
	},
	[PIN_FUNC_I2C1] = {
		.Id = (u8)PIN_FUNC_I2C1,
		.Name = "i2c1",
		.DevIdx = (u16)XPM_NODEIDX_DEV_I2C_1,
		.LmioRegMask = 0x200U,
		.PmioRegMask = 0x80U,
		.NumPins = 2U,
		.NumGroups = 7U,
		.Groups = ((u16 []) {
			PIN_GRP_I2C1_0,
			PIN_GRP_I2C1_1,
			PIN_GRP_I2C1_2,
			PIN_GRP_I2C1_3,
			PIN_GRP_I2C1_4,
			PIN_GRP_I2C1_5,
			PIN_GRP_I2C1_6,
		}),
	},
	[PIN_FUNC_OSPI] = {
		.Id = (u8)PIN_FUNC_OSPI,
		.Name = "ospi",
		.DevIdx = (u16)XPM_NODEIDX_DEV_OSPI,
		.LmioRegMask = 0x0U,
		.PmioRegMask = 0x2U,
		.NumPins = 10U,
		.NumGroups = 1U,
		.Groups = ((u16 []) {
			PIN_GRP_OSPI_0,
		}),
	},
	[PIN_FUNC_PCIE] = {
		.Id = (u8)PIN_FUNC_PCIE,
		.Name = "pcie",
		.DevIdx = 0U,
		.LmioRegMask = 0x100U,
		.PmioRegMask = 0x380U,
		.NumPins = 2U,
		.NumGroups = 3U,
		.Groups = ((u16 []) {
			PIN_GRP_PCIE_0,
			PIN_GRP_PCIE_1,
			PIN_GRP_PCIE_2,
		}),
	},
	[PIN_FUNC_QSPI] = {
		.Id = (u8)PIN_FUNC_QSPI,
		.Name = "qspi",
		.DevIdx = (u16)XPM_NODEIDX_DEV_QSPI,
		.LmioRegMask = 0x0U,
		.PmioRegMask = 0x6U,
		.NumPins = 10U,
		.NumGroups = 1U,
		.Groups = ((u16 []) {
			PIN_GRP_QSPI_0,
		}),
	},
	[PIN_FUNC_SMAP] = {
		.Id = (u8)PIN_FUNC_SMAP,
		.Name = "smap",
		.DevIdx = 0U,
		.LmioRegMask = 0x0U,
		.PmioRegMask = 0x4U,
		.NumPins = 36U,
		.NumGroups = 1U,
		.Groups = ((u16 []) {
			PIN_GRP_SMAP_0,
		}),
	},
	[PIN_FUNC_SPI0] = {
		.Id = (u8)PIN_FUNC_SPI0,
		.Name = "spi0",
		.DevIdx = (u16)XPM_NODEIDX_DEV_SPI_0,
		.LmioRegMask = 0x80U,
		.PmioRegMask = 0x100U,
		.NumPins = 3U,
		.NumGroups = 6U,
		.Groups = ((u16 []) {
			PIN_GRP_SPI0_0,
			PIN_GRP_SPI0_1,
			PIN_GRP_SPI0_2,
			PIN_GRP_SPI0_3,
			PIN_GRP_SPI0_4,
			PIN_GRP_SPI0_5,
		}),
	},
	[PIN_FUNC_SPI1] = {
		.Id = (u8)PIN_FUNC_SPI1,
		.Name = "spi1",
		.DevIdx = (u16)XPM_NODEIDX_DEV_SPI_1,
		.LmioRegMask = 0x80U,
		.PmioRegMask = 0x100U,
		.NumPins = 3U,
		.NumGroups = 6U,
		.Groups = ((u16 []) {
			PIN_GRP_SPI1_0,
			PIN_GRP_SPI1_1,
			PIN_GRP_SPI1_2,
			PIN_GRP_SPI1_3,
			PIN_GRP_SPI1_4,
			PIN_GRP_SPI1_5,
		}),
	},
	[PIN_FUNC_USB0] = {
		.Id = (u8)PIN_FUNC_USB0,
		.Name = "usb0",
		.DevIdx = (u16)XPM_NODEIDX_DEV_USB_0,
		.LmioRegMask = 0x0U,
		.PmioRegMask = 0x6U,
		.NumPins = 13U,
		.NumGroups = 1U,
		.Groups = ((u16 []) {
			PIN_GRP_USB0_0,
		}),
	},
	[PIN_FUNC_USB1] = {
		.Id = (u8)PIN_FUNC_USB1,
		.Name = "usb1",
		.DevIdx = (u16)XPM_NODEIDX_DEV_USB_1,
		.LmioRegMask = 0x0U,
		.PmioRegMask = 0x8U,
		.NumPins = 13U,
		.NumGroups = 1U,
		.Groups = ((u16 []) {
			PIN_GRP_USB1_0,
		}),
	},
	[PIN_FUNC_WWDT] = {
		.Id = (u8)PIN_FUNC_WWDT,
		.Name = "wwdt",
		.DevIdx = 0U,
		.LmioRegMask = 0x0U,
		.PmioRegMask = 0x200U,
		.NumPins = 1U,
		.NumGroups = 5U,
		.Groups = ((u16 []) {
			PIN_GRP_WWDT_0,
			PIN_GRP_WWDT_1,
			PIN_GRP_WWDT_2,
			PIN_GRP_WWDT_3,
			PIN_GRP_WWDT_4,
		}),
	},
	[PIN_FUNC_GPIO0] = {
		.Id = (u8)PIN_FUNC_GPIO0,
		.Name = "gpio0",
		.DevIdx = (u16)XPM_NODEIDX_DEV_GPIO_PMC,
		.LmioRegMask = 0x0U,
		.PmioRegMask = 0x60U,
		.NumPins = 1U,
		.NumGroups = 26U,
		.Groups = ((u16 []) {
			PIN_GRP_GPIO0_0,
			PIN_GRP_GPIO0_1,
			PIN_GRP_GPIO0_2,
			PIN_GRP_GPIO0_3,
			PIN_GRP_GPIO0_4,
			PIN_GRP_GPIO0_5,
			PIN_GRP_GPIO0_6,
			PIN_GRP_GPIO0_7,
			PIN_GRP_GPIO0_8,
			PIN_GRP_GPIO0_9,
			PIN_GRP_GPIO0_10,
			PIN_GRP_GPIO0_11,
			PIN_GRP_GPIO0_12,
			PIN_GRP_GPIO0_13,
			PIN_GRP_GPIO0_14,
			PIN_GRP_GPIO0_15,
			PIN_GRP_GPIO0_16,
			PIN_GRP_GPIO0_17,
			PIN_GRP_GPIO0_18,
			PIN_GRP_GPIO0_19,
			PIN_GRP_GPIO0_20,
			PIN_GRP_GPIO0_21,
			PIN_GRP_GPIO0_22,
			PIN_GRP_GPIO0_23,
			PIN_GRP_GPIO0_24,
			PIN_GRP_GPIO0_25,
		}),
	},
	[PIN_FUNC_GPIO1] = {
		.Id = (u8)PIN_FUNC_GPIO1,
		.Name = "gpio1",
		.DevIdx = (u16)XPM_NODEIDX_DEV_GPIO_PMC,
		.LmioRegMask = 0x0U,
		.PmioRegMask = 0x60U,
		.NumPins = 1U,
		.NumGroups = 26U,
		.Groups = ((u16 []) {
			PIN_GRP_GPIO1_0,
			PIN_GRP_GPIO1_1,
			PIN_GRP_GPIO1_2,
			PIN_GRP_GPIO1_3,
			PIN_GRP_GPIO1_4,
			PIN_GRP_GPIO1_5,
			PIN_GRP_GPIO1_6,
			PIN_GRP_GPIO1_7,
			PIN_GRP_GPIO1_8,
			PIN_GRP_GPIO1_9,
			PIN_GRP_GPIO1_10,
			PIN_GRP_GPIO1_11,
			PIN_GRP_GPIO1_12,
			PIN_GRP_GPIO1_13,
			PIN_GRP_GPIO1_14,
			PIN_GRP_GPIO1_15,
			PIN_GRP_GPIO1_16,
			PIN_GRP_GPIO1_17,
			PIN_GRP_GPIO1_18,
			PIN_GRP_GPIO1_19,
			PIN_GRP_GPIO1_20,
			PIN_GRP_GPIO1_21,
			PIN_GRP_GPIO1_22,
			PIN_GRP_GPIO1_23,
			PIN_GRP_GPIO1_24,
			PIN_GRP_GPIO1_25,
		}),
	},
	[PIN_FUNC_GPIO2] = {
		.Id = (u8)PIN_FUNC_GPIO2,
		.Name = "gpio2",
		.DevIdx = (u16)XPM_NODEIDX_DEV_GPIO,
		.LmioRegMask = 0x40U,
		.PmioRegMask = 0x0U,
		.NumPins = 1U,
		.NumGroups = 26U,
		.Groups = ((u16 []) {
			PIN_GRP_GPIO2_0,
			PIN_GRP_GPIO2_1,
			PIN_GRP_GPIO2_2,
			PIN_GRP_GPIO2_3,
			PIN_GRP_GPIO2_4,
			PIN_GRP_GPIO2_5,
			PIN_GRP_GPIO2_6,
			PIN_GRP_GPIO2_7,
			PIN_GRP_GPIO2_8,
			PIN_GRP_GPIO2_9,
			PIN_GRP_GPIO2_10,
			PIN_GRP_GPIO2_11,
			PIN_GRP_GPIO2_12,
			PIN_GRP_GPIO2_13,
			PIN_GRP_GPIO2_14,
			PIN_GRP_GPIO2_15,
			PIN_GRP_GPIO2_16,
			PIN_GRP_GPIO2_17,
			PIN_GRP_GPIO2_18,
			PIN_GRP_GPIO2_19,
			PIN_GRP_GPIO2_20,
			PIN_GRP_GPIO2_21,
			PIN_GRP_GPIO2_22,
			PIN_GRP_GPIO2_23,
			PIN_GRP_GPIO2_24,
			PIN_GRP_GPIO2_25,
		}),
	},
	[PIN_FUNC_MDIO0] = {
		.Id = (u8)PIN_FUNC_MDIO0,
		.Name = "mdio0",
		.DevIdx = 0U,
		.LmioRegMask = 0x100U,
		.PmioRegMask = 0x100U,
		.NumPins = 2U,
		.NumGroups = 2U,
		.Groups = ((u16 []) {
			PIN_GRP_MDIO0_0,
			PIN_GRP_MDIO0_1,
		}),
	},
	[PIN_FUNC_MDIO1] = {
		.Id = (u8)PIN_FUNC_MDIO1,
		.Name = "mdio1",
		.DevIdx = 0U,
		.LmioRegMask = 0x380U,
		.PmioRegMask = 0x180U,
		.NumPins = 2U,
		.NumGroups = 2U,
		.Groups = ((u16 []) {
			PIN_GRP_MDIO1_0,
			PIN_GRP_MDIO1_1,
		}),
	},
	[PIN_FUNC_MDIO2] = {
		.Id = (u8)PIN_FUNC_MDIO2,
		.Name = "mdio2",
		.DevIdx = 0U,
		.LmioRegMask = 0x280U,
		.PmioRegMask = 0x0U,
		.NumPins = 2U,
		.NumGroups = 4U,
		.Groups = ((u16 []) {
			PIN_GRP_MDIO2_0,
			PIN_GRP_MDIO2_1,
			PIN_GRP_MDIO2_2,
			PIN_GRP_MDIO2_3,
		}),
	},
	[PIN_FUNC_TRACE] = {
		.Id = (u8)PIN_FUNC_TRACE,
		.Name = "trace",
		.DevIdx = 0U,
		.LmioRegMask = 0x8U,
		.PmioRegMask = 0x10U,
		.NumPins = 17U,
		.NumGroups = 3U,
		.Groups = ((u16 []) {
			PIN_GRP_TRACE_0,
			PIN_GRP_TRACE_1,
			PIN_GRP_TRACE_2,
		}),
	},
	[PIN_FUNC_UART0] = {
		.Id = (u8)PIN_FUNC_UART0,
		.Name = "uart0",
		.DevIdx = (u16)XPM_NODEIDX_DEV_UART_0,
		.LmioRegMask = 0x20U,
		.PmioRegMask = 0x40U,
		.NumPins = 4U,
		.NumGroups = 9U,
		.Groups = ((u16 []) {
			PIN_GRP_UART0_0,
			PIN_GRP_UART0_1,
			PIN_GRP_UART0_2,
			PIN_GRP_UART0_3,
			PIN_GRP_UART0_4,
			PIN_GRP_UART0_5,
			PIN_GRP_UART0_6,
			PIN_GRP_UART0_7,
			PIN_GRP_UART0_8,
		}),
	},
	[PIN_FUNC_UART1] = {
		.Id = (u8)PIN_FUNC_UART1,
		.Name = "uart1",
		.DevIdx = (u16)XPM_NODEIDX_DEV_UART_1,
		.LmioRegMask = 0x20U,
		.PmioRegMask = 0x40U,
		.NumPins = 4U,
		.NumGroups = 9U,
		.Groups = ((u16 []) {
			PIN_GRP_UART1_0,
			PIN_GRP_UART1_1,
			PIN_GRP_UART1_2,
			PIN_GRP_UART1_3,
			PIN_GRP_UART1_4,
			PIN_GRP_UART1_5,
			PIN_GRP_UART1_6,
			PIN_GRP_UART1_7,
			PIN_GRP_UART1_8,
		}),
	},
	[PIN_FUNC_SD0_CD] = {
		.Id = (u8)PIN_FUNC_SD0_CD,
		.Name = "sd0_cd",
		.DevIdx = (u16)XPM_NODEIDX_DEV_SDIO_0,
		.LmioRegMask = 0x0U,
		.PmioRegMask = 0x4U,
		.NumPins = 1U,
		.NumGroups = 2U,
		.Groups = ((u16 []) {
			PIN_GRP_SD0_CD_0,
			PIN_GRP_SD0_CD_1,
		}),
	},
	[PIN_FUNC_SD0_PC] = {
		.Id = (u8)PIN_FUNC_SD0_PC,
		.Name = "sd0_pc",
		.DevIdx = (u16)XPM_NODEIDX_DEV_SDIO_0,
		.LmioRegMask = 0x0U,
		.PmioRegMask = 0x4U,
		.NumPins = 1U,
		.NumGroups = 2U,
		.Groups = ((u16 []) {
			PIN_GRP_SD0_PC_0,
			PIN_GRP_SD0_PC_1,
		}),
	},
	[PIN_FUNC_SD0_WP] = {
		.Id = (u8)PIN_FUNC_SD0_WP,
		.Name = "sd0_wp",
		.DevIdx = (u16)XPM_NODEIDX_DEV_SDIO_0,
		.LmioRegMask = 0x0U,
		.PmioRegMask = 0x4U,
		.NumPins = 1U,
		.NumGroups = 2U,
		.Groups = ((u16 []) {
			PIN_GRP_SD0_WP_0,
			PIN_GRP_SD0_WP_1,
		}),
	},
	[PIN_FUNC_SD1_PC] = {
		.Id = (u8)PIN_FUNC_SD1_PC,
		.Name = "sd1_pc",
		.DevIdx = (u16)XPM_NODEIDX_DEV_SDIO_1,
		.LmioRegMask = 0x0U,
		.PmioRegMask = 0x2U,
		.NumPins = 1U,
		.NumGroups = 2U,
		.Groups = ((u16 []) {
			PIN_GRP_SD1_PC_0,
			PIN_GRP_SD1_PC_1,
		}),
	},
	[PIN_FUNC_GEM_TSU] = {
		.Id = (u8)PIN_FUNC_GEM_TSU,
		.Name = "gem_tsu",
		.DevIdx = 0U,
		.LmioRegMask = 0x4U,
		.PmioRegMask = 0x6U,
		.NumPins = 1U,
		.NumGroups = 4U,
		.Groups = ((u16 []) {
			PIN_GRP_GEM_TSU_0,
			PIN_GRP_GEM_TSU_1,
			PIN_GRP_GEM_TSU_2,
			PIN_GRP_GEM_TSU_3,
		}),
	},
	[PIN_FUNC_OSPI_SS] = {
		.Id = (u8)PIN_FUNC_OSPI_SS,
		.Name = "ospi_ss",
		.DevIdx = (u16)XPM_NODEIDX_DEV_OSPI,
		.LmioRegMask = 0x0U,
		.PmioRegMask = 0x2U,
		.NumPins = 2U,
		.NumGroups = 1U,
		.Groups = ((u16 []) {
			PIN_GRP_OSPI_SS_0,
		}),
	},
	[PIN_FUNC_QSPI_SS] = {
		.Id = (u8)PIN_FUNC_QSPI_SS,
		.Name = "qspi_ss",
		.DevIdx = (u16)XPM_NODEIDX_DEV_QSPI,
		.LmioRegMask = 0x0U,
		.PmioRegMask = 0x6U,
		.NumPins = 2U,
		.NumGroups = 1U,
		.Groups = ((u16 []) {
			PIN_GRP_QSPI_SS_0,
		}),
	},
	[PIN_FUNC_SPI0_SS] = {
		.Id = (u8)PIN_FUNC_SPI0_SS,
		.Name = "spi0_ss",
		.DevIdx = (u16)XPM_NODEIDX_DEV_SPI_0,
		.LmioRegMask = 0x80U,
		.PmioRegMask = 0x100U,
		.NumPins = 3U,
		.NumGroups = 6U,
		.Groups = ((u16 []) {
			PIN_GRP_SPI0_SS_0,
			PIN_GRP_SPI0_SS_1,
			PIN_GRP_SPI0_SS_2,
			PIN_GRP_SPI0_SS_3,
			PIN_GRP_SPI0_SS_4,
			PIN_GRP_SPI0_SS_5,
		}),
	},
	[PIN_FUNC_SPI1_SS] = {
		.Id = (u8)PIN_FUNC_SPI1_SS,
		.Name = "spi1_ss",
		.DevIdx = (u16)XPM_NODEIDX_DEV_SPI_1,
		.LmioRegMask = 0x80U,
		.PmioRegMask = 0x100U,
		.NumPins = 3U,
		.NumGroups = 6U,
		.Groups = ((u16 []) {
			PIN_GRP_SPI1_SS_0,
			PIN_GRP_SPI1_SS_1,
			PIN_GRP_SPI1_SS_2,
			PIN_GRP_SPI1_SS_3,
			PIN_GRP_SPI1_SS_4,
			PIN_GRP_SPI1_SS_5,
		}),
	},
	[PIN_FUNC_TEST_CLK] = {
		.Id = (u8)PIN_FUNC_TEST_CLK,
		.Name = "test_clk",
		.DevIdx = 0U,
		.LmioRegMask = 0x0U,
		.PmioRegMask = 0x10U,
		.NumPins = 4U,
		.NumGroups = 1U,
		.Groups = ((u16 []) {
			PIN_GRP_TEST_CLK_0,
		}),
	},
	[PIN_FUNC_TTC0_CLK] = {
		.Id = (u8)PIN_FUNC_TTC0_CLK,
		.Name = "ttc0_clk",
		.DevIdx = (u16)XPM_NODEIDX_DEV_TTC_0,
		.LmioRegMask = 0x280U,
		.PmioRegMask = 0x280U,
		.NumPins = 1U,
		.NumGroups = 8U,
		.Groups = ((u16 []) {
			PIN_GRP_TTC0_CLK_0,
			PIN_GRP_TTC0_CLK_1,
			PIN_GRP_TTC0_CLK_2,
			PIN_GRP_TTC0_CLK_3,
			PIN_GRP_TTC0_CLK_4,
			PIN_GRP_TTC0_CLK_5,
			PIN_GRP_TTC0_CLK_6,
			PIN_GRP_TTC0_CLK_7,
		}),
	},
	[PIN_FUNC_TTC0_WAV] = {
		.Id = (u8)PIN_FUNC_TTC0_WAV,
		.Name = "ttc0_wav",
		.DevIdx = (u16)XPM_NODEIDX_DEV_TTC_0,
		.LmioRegMask = 0x280U,
		.PmioRegMask = 0x280U,
		.NumPins = 1U,
		.NumGroups = 8U,
		.Groups = ((u16 []) {
			PIN_GRP_TTC0_WAV_0,
			PIN_GRP_TTC0_WAV_1,
			PIN_GRP_TTC0_WAV_2,
			PIN_GRP_TTC0_WAV_3,
			PIN_GRP_TTC0_WAV_4,
			PIN_GRP_TTC0_WAV_5,
			PIN_GRP_TTC0_WAV_6,
			PIN_GRP_TTC0_WAV_7,
		}),
	},
	[PIN_FUNC_TTC1_CLK] = {
		.Id = (u8)PIN_FUNC_TTC1_CLK,
		.Name = "ttc1_clk",
		.DevIdx = (u16)XPM_NODEIDX_DEV_TTC_1,
		.LmioRegMask = 0x280U,
		.PmioRegMask = 0x280U,
		.NumPins = 1U,
		.NumGroups = 9U,
		.Groups = ((u16 []) {
			PIN_GRP_TTC1_CLK_0,
			PIN_GRP_TTC1_CLK_1,
			PIN_GRP_TTC1_CLK_2,
			PIN_GRP_TTC1_CLK_3,
			PIN_GRP_TTC1_CLK_4,
			PIN_GRP_TTC1_CLK_5,
			PIN_GRP_TTC1_CLK_6,
			PIN_GRP_TTC1_CLK_7,
			PIN_GRP_TTC1_CLK_8,
		}),
	},
	[PIN_FUNC_TTC1_WAV] = {
		.Id = (u8)PIN_FUNC_TTC1_WAV,
		.Name = "ttc1_wav",
		.DevIdx = (u16)XPM_NODEIDX_DEV_TTC_1,
		.LmioRegMask = 0x280U,
		.PmioRegMask = 0x280U,
		.NumPins = 1U,
		.NumGroups = 9U,
		.Groups = ((u16 []) {
			PIN_GRP_TTC1_WAV_0,
			PIN_GRP_TTC1_WAV_1,
			PIN_GRP_TTC1_WAV_2,
			PIN_GRP_TTC1_WAV_3,
			PIN_GRP_TTC1_WAV_4,
			PIN_GRP_TTC1_WAV_5,
			PIN_GRP_TTC1_WAV_6,
			PIN_GRP_TTC1_WAV_7,
			PIN_GRP_TTC1_WAV_8,
		}),
	},
	[PIN_FUNC_TTC2_CLK] = {
		.Id = (u8)PIN_FUNC_TTC2_CLK,
		.Name = "ttc2_clk",
		.DevIdx = (u16)XPM_NODEIDX_DEV_TTC_2,
		.LmioRegMask = 0x280U,
		.PmioRegMask = 0x280U,
		.NumPins = 1U,
		.NumGroups = 3U,
		.Groups = ((u16 []) {
			PIN_GRP_TTC2_CLK_0,
			PIN_GRP_TTC2_CLK_1,
			PIN_GRP_TTC2_CLK_2,
		}),
	},
	[PIN_FUNC_TTC2_WAV] = {
		.Id = (u8)PIN_FUNC_TTC2_WAV,
		.Name = "ttc2_wav",
		.DevIdx = (u16)XPM_NODEIDX_DEV_TTC_2,
		.LmioRegMask = 0x280U,
		.PmioRegMask = 0x280U,
		.NumPins = 1U,
		.NumGroups = 3U,
		.Groups = ((u16 []) {
			PIN_GRP_TTC2_WAV_0,
			PIN_GRP_TTC2_WAV_1,
			PIN_GRP_TTC2_WAV_2,
		}),
	},
	[PIN_FUNC_TTC3_CLK] = {
		.Id = (u8)PIN_FUNC_TTC3_CLK,
		.Name = "ttc3_clk",
		.DevIdx = (u16)XPM_NODEIDX_DEV_TTC_3,
		.LmioRegMask = 0x0U,
		.PmioRegMask = 0x280U,
		.NumPins = 1U,
		.NumGroups = 2U,
		.Groups = ((u16 []) {
			PIN_GRP_TTC3_CLK_0,
			PIN_GRP_TTC3_CLK_1,
		}),
	},
	[PIN_FUNC_TTC3_WAV] = {
		.Id = (u8)PIN_FUNC_TTC3_WAV,
		.Name = "ttc3_wav",
		.DevIdx = (u16)XPM_NODEIDX_DEV_TTC_3,
		.LmioRegMask = 0x0U,
		.PmioRegMask = 0x280U,
		.NumPins = 1U,
		.NumGroups = 2U,
		.Groups = ((u16 []) {
			PIN_GRP_TTC3_WAV_0,
			PIN_GRP_TTC3_WAV_1,
		}),
	},
	[PIN_FUNC_TEST_SCAN] = {
		.Id = (u8)PIN_FUNC_TEST_SCAN,
		.Name = "test_scan",
		.DevIdx = 0U,
		.LmioRegMask = 0x0U,
		.PmioRegMask = 0x20U,
		.NumPins = 38U,
		.NumGroups = 1U,
		.Groups = ((u16 []) {
			PIN_GRP_TEST_SCAN_0,
		}),
	},
	[PIN_FUNC_TRACE_CLK] = {
		.Id = (u8)PIN_FUNC_TRACE_CLK,
		.Name = "trace_clk",
		.DevIdx = 0U,
		.LmioRegMask = 0x8U,
		.PmioRegMask = 0x10U,
		.NumPins = 1U,
		.NumGroups = 3U,
		.Groups = ((u16 []) {
			PIN_GRP_TRACE_CLK_0,
			PIN_GRP_TRACE_CLK_1,
			PIN_GRP_TRACE_CLK_2,
		}),
	},
	[PIN_FUNC_MMI_DP_HPD] = {
		.Id = (u8)PIN_FUNC_MMI_DP_HPD,
		.Name = "mmi_dp_hpd",
		.DevIdx = 0U,
		.LmioRegMask = 0x200U,
		.PmioRegMask = 0x200U,
		.NumPins = 1U,
		.NumGroups = 4U,
		.Groups = ((u16 []) {
			PIN_GRP_MMI_DP_HPD_0,
			PIN_GRP_MMI_DP_HPD_1,
			PIN_GRP_MMI_DP_HPD_2,
			PIN_GRP_MMI_DP_HPD_3,
		}),
	},
	[PIN_FUNC_OSPI_RST_N] = {
		.Id = (u8)PIN_FUNC_OSPI_RST_N,
		.Name = "ospi_rst_n",
		.DevIdx = (u16)XPM_NODEIDX_DEV_QSPI,
		.LmioRegMask = 0x0U,
		.PmioRegMask = 0x2U,
		.NumPins = 1U,
		.NumGroups = 1U,
		.Groups = ((u16 []) {
			PIN_GRP_OSPI_RST_N_0,
		}),
	},
	[PIN_FUNC_QSPI_FBCLK] = {
		.Id = (u8)PIN_FUNC_QSPI_FBCLK,
		.Name = "qspi_fbclk",
		.DevIdx = (u16)XPM_NODEIDX_DEV_QSPI,
		.LmioRegMask = 0x0U,
		.PmioRegMask = 0x6U,
		.NumPins = 1U,
		.NumGroups = 1U,
		.Groups = ((u16 []) {
			PIN_GRP_QSPI_FBCLK_0,
		}),
	},
	[PIN_FUNC_SYSMON_I2C] = {
		.Id = (u8)PIN_FUNC_SYSMON_I2C,
		.Name = "sysmon_i2c",
		.DevIdx = 0U,
		.LmioRegMask = 0x380U,
		.PmioRegMask = 0x0U,
		.NumPins = 2U,
		.NumGroups = 16U,
		.Groups = ((u16 []) {
			PIN_GRP_SYSMON_I2C_0,
			PIN_GRP_SYSMON_I2C_1,
			PIN_GRP_SYSMON_I2C_2,
			PIN_GRP_SYSMON_I2C_3,
			PIN_GRP_SYSMON_I2C_4,
			PIN_GRP_SYSMON_I2C_5,
			PIN_GRP_SYSMON_I2C_6,
			PIN_GRP_SYSMON_I2C_7,
			PIN_GRP_SYSMON_I2C_8,
			PIN_GRP_SYSMON_I2C_9,
			PIN_GRP_SYSMON_I2C_10,
			PIN_GRP_SYSMON_I2C_11,
			PIN_GRP_SYSMON_I2C_12,
			PIN_GRP_SYSMON_I2C_13,
			PIN_GRP_SYSMON_I2C_14,
			PIN_GRP_SYSMON_I2C_15,
		}),
	},
	[PIN_FUNC_OSPI_ECC_FAIL] = {
		.Id = (u8)PIN_FUNC_OSPI_ECC_FAIL,
		.Name = "ospi_ecc_fail",
		.DevIdx = (u16)XPM_NODEIDX_DEV_OSPI,
		.LmioRegMask = 0x0U,
		.PmioRegMask = 0x2U,
		.NumPins = 1U,
		.NumGroups = 2U,
		.Groups = ((u16 []) {
			PIN_GRP_OSPI_ECC_FAIL_0,
			PIN_GRP_OSPI_ECC_FAIL_1,
		}),
	},
	[PIN_FUNC_EXT_TAMPER_TRIG] = {
		.Id = (u8)PIN_FUNC_EXT_TAMPER_TRIG,
		.Name = "ext_tamper_trig",
		.DevIdx = 0U,
		.LmioRegMask = 0x0U,
		.PmioRegMask = 0x380U,
		.NumPins = 1U,
		.NumGroups = 7U,
		.Groups = ((u16 []) {
			PIN_GRP_EXT_TAMPER_TRIG_0,
			PIN_GRP_EXT_TAMPER_TRIG_1,
			PIN_GRP_EXT_TAMPER_TRIG_2,
			PIN_GRP_EXT_TAMPER_TRIG_3,
			PIN_GRP_EXT_TAMPER_TRIG_4,
			PIN_GRP_EXT_TAMPER_TRIG_5,
			PIN_GRP_EXT_TAMPER_TRIG_6,
		}),
	},
	[PIN_FUNC_SYSMON_I2C_ALERT] = {
		.Id = (u8)PIN_FUNC_SYSMON_I2C_ALERT,
		.Name = "sysmon_i2c_alert",
		.DevIdx = 0U,
		.LmioRegMask = 0x380U,
		.PmioRegMask = 0x0U,
		.NumPins = 1U,
		.NumGroups = 16U,
		.Groups = ((u16 []) {
			PIN_GRP_SYSMON_I2C_ALERT_0,
			PIN_GRP_SYSMON_I2C_ALERT_1,
			PIN_GRP_SYSMON_I2C_ALERT_2,
			PIN_GRP_SYSMON_I2C_ALERT_3,
			PIN_GRP_SYSMON_I2C_ALERT_4,
			PIN_GRP_SYSMON_I2C_ALERT_5,
			PIN_GRP_SYSMON_I2C_ALERT_6,
			PIN_GRP_SYSMON_I2C_ALERT_7,
			PIN_GRP_SYSMON_I2C_ALERT_8,
			PIN_GRP_SYSMON_I2C_ALERT_9,
			PIN_GRP_SYSMON_I2C_ALERT_10,
			PIN_GRP_SYSMON_I2C_ALERT_11,
			PIN_GRP_SYSMON_I2C_ALERT_12,
			PIN_GRP_SYSMON_I2C_ALERT_13,
			PIN_GRP_SYSMON_I2C_ALERT_14,
			PIN_GRP_SYSMON_I2C_ALERT_15,
		}),
	},
};

/****************************************************************************/
/**
 * @brief  This function returns handle to requested XPm_PinFunc struct
 *
 * @param FuncId	Function ID.
 *
 * @return Pointer to XPm_PinFunc if successful, NULL otherwise
 *
 ****************************************************************************/
XPm_PinFunc *XPmPinFunc_GetById(u32 FuncId)
{
	XPm_PinFunc *PinFunc = NULL;

	if ((u32)MAX_FUNCTION > FuncId) {
		PinFunc = &PmPinFuncs[FuncId];
	}

	return PinFunc;
}
