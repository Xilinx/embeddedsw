/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_pinfunc.h"

#define FUNC_QUERY_NAME_LEN	(FUNC_NAME_SIZE)

/* TODO: Each function can not be mapped with their corresponding
 *       device. Keeping those devIdx as 0.
 */
static XPm_PinFunc PmPinFuncs[MAX_FUNCTION] = {
	[PIN_FUNC_SPI0] = {
		.Id = (u8)PIN_FUNC_SPI0,
		.Name = "spi0",
		.DevIdx = (u16)XPM_NODEIDX_DEV_SPI_0,
		.LmioRegMask = 0x80,
		.PmioRegMask = 0x100,
		.NumPins = 3,
		.NumGroups = 6,
		.Groups = ((u16 []) {
			PIN_GRP_SPI0_0,
			PIN_GRP_SPI0_1,
			PIN_GRP_SPI0_2,
			PIN_GRP_SPI0_3,
			PIN_GRP_SPI0_4,
			PIN_GRP_SPI0_5,
		}),
	},
	[PIN_FUNC_SPI0_SS] = {
		.Id = (u8)PIN_FUNC_SPI0_SS,
		.Name = "spi0_ss",
		.DevIdx = (u16)XPM_NODEIDX_DEV_SPI_0,
		.LmioRegMask = 0x80,
		.PmioRegMask = 0x100,
		.NumPins = 1,
		.NumGroups = 18,
		.Groups = ((u16 []) {
			PIN_GRP_SPI0_0_SS0,
			PIN_GRP_SPI0_0_SS1,
			PIN_GRP_SPI0_0_SS2,
			PIN_GRP_SPI0_1_SS0,
			PIN_GRP_SPI0_1_SS1,
			PIN_GRP_SPI0_1_SS2,
			PIN_GRP_SPI0_2_SS0,
			PIN_GRP_SPI0_2_SS1,
			PIN_GRP_SPI0_2_SS2,
			PIN_GRP_SPI0_3_SS0,
			PIN_GRP_SPI0_3_SS1,
			PIN_GRP_SPI0_3_SS2,
			PIN_GRP_SPI0_4_SS0,
			PIN_GRP_SPI0_4_SS1,
			PIN_GRP_SPI0_4_SS2,
			PIN_GRP_SPI0_5_SS0,
			PIN_GRP_SPI0_5_SS1,
			PIN_GRP_SPI0_5_SS2,
		}),
	},
	[PIN_FUNC_SPI1] = {
		.Id = (u8)PIN_FUNC_SPI1,
		.Name = "spi1",
		.DevIdx = (u16)XPM_NODEIDX_DEV_SPI_1,
		.LmioRegMask = 0x80,
		.PmioRegMask = 0x100,
		.NumPins = 3,
		.NumGroups = 6,
		.Groups = ((u16 []) {
			PIN_GRP_SPI1_0,
			PIN_GRP_SPI1_1,
			PIN_GRP_SPI1_2,
			PIN_GRP_SPI1_3,
			PIN_GRP_SPI1_4,
			PIN_GRP_SPI1_5,
		}),
	},
	[PIN_FUNC_SPI1_SS] = {
		.Id = (u8)PIN_FUNC_SPI1_SS,
		.Name = "spi1_ss",
		.DevIdx = (u16)XPM_NODEIDX_DEV_SPI_1,
		.LmioRegMask = 0x80,
		.PmioRegMask = 0x100,
		.NumPins = 3,
		.NumGroups = 18,
		.Groups = ((u16 []) {
			PIN_GRP_SPI1_0_SS0,
			PIN_GRP_SPI1_0_SS1,
			PIN_GRP_SPI1_0_SS2,
			PIN_GRP_SPI1_1_SS0,
			PIN_GRP_SPI1_1_SS1,
			PIN_GRP_SPI1_1_SS2,
			PIN_GRP_SPI1_2_SS0,
			PIN_GRP_SPI1_2_SS1,
			PIN_GRP_SPI1_2_SS2,
			PIN_GRP_SPI1_3_SS0,
			PIN_GRP_SPI1_3_SS1,
			PIN_GRP_SPI1_3_SS2,
			PIN_GRP_SPI1_4_SS0,
			PIN_GRP_SPI1_4_SS1,
			PIN_GRP_SPI1_4_SS2,
			PIN_GRP_SPI1_5_SS0,
			PIN_GRP_SPI1_5_SS1,
			PIN_GRP_SPI1_5_SS2,
		}),
	},
	[PIN_FUNC_CAN0] = {
		.Id = (u8)PIN_FUNC_CAN0,
		.Name = "can0",
		.DevIdx = (u16)XPM_NODEIDX_DEV_CAN_FD_0,
		.LmioRegMask = 0x180,
		.PmioRegMask = 0x180,
		.NumPins = 2,
		.NumGroups = 18,
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
			PIN_GRP_CAN0_10,
			PIN_GRP_CAN0_11,
			PIN_GRP_CAN0_12,
			PIN_GRP_CAN0_13,
			PIN_GRP_CAN0_14,
			PIN_GRP_CAN0_15,
			PIN_GRP_CAN0_16,
			PIN_GRP_CAN0_17,
		}),
	},
	[PIN_FUNC_CAN1] = {
		.Id = (u8)PIN_FUNC_CAN1,
		.Name = "can1",
		.DevIdx = (u16)XPM_NODEIDX_DEV_CAN_FD_1,
		.LmioRegMask = 0x180,
		.PmioRegMask = 0x180,
		.NumPins = 2,
		.NumGroups = 19,
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
			PIN_GRP_CAN1_11,
			PIN_GRP_CAN1_12,
			PIN_GRP_CAN1_13,
			PIN_GRP_CAN1_14,
			PIN_GRP_CAN1_15,
			PIN_GRP_CAN1_16,
			PIN_GRP_CAN1_17,
			PIN_GRP_CAN1_18,
		}),
	},
	[PIN_FUNC_I2C0] = {
		.Id = (u8)PIN_FUNC_I2C0,
		.Name = "i2c0",
		.DevIdx = (u16)XPM_NODEIDX_DEV_I2C_0,
		.LmioRegMask = 0x200,
		.PmioRegMask = 0x80,
		.NumPins = 2,
		.NumGroups = 18,
		.Groups = ((u16 []) {
			PIN_GRP_I2C0_0,
			PIN_GRP_I2C0_1,
			PIN_GRP_I2C0_2,
			PIN_GRP_I2C0_3,
			PIN_GRP_I2C0_4,
			PIN_GRP_I2C0_5,
			PIN_GRP_I2C0_6,
			PIN_GRP_I2C0_7,
			PIN_GRP_I2C0_8,
			PIN_GRP_I2C0_9,
			PIN_GRP_I2C0_10,
			PIN_GRP_I2C0_11,
			PIN_GRP_I2C0_12,
			PIN_GRP_I2C0_13,
			PIN_GRP_I2C0_14,
			PIN_GRP_I2C0_15,
			PIN_GRP_I2C0_16,
			PIN_GRP_I2C0_17,
		}),
	},
	[PIN_FUNC_I2C1] = {
		.Id = (u8)PIN_FUNC_I2C1,
		.Name = "i2c1",
		.DevIdx = (u16)XPM_NODEIDX_DEV_I2C_1,
		.LmioRegMask = 0x200,
		.PmioRegMask = 0x80,
		.NumPins = 2,
		.NumGroups = 19,
		.Groups = ((u16 []) {
			PIN_GRP_I2C1_0,
			PIN_GRP_I2C1_1,
			PIN_GRP_I2C1_2,
			PIN_GRP_I2C1_3,
			PIN_GRP_I2C1_4,
			PIN_GRP_I2C1_5,
			PIN_GRP_I2C1_6,
			PIN_GRP_I2C1_7,
			PIN_GRP_I2C1_8,
			PIN_GRP_I2C1_9,
			PIN_GRP_I2C1_10,
			PIN_GRP_I2C1_11,
			PIN_GRP_I2C1_12,
			PIN_GRP_I2C1_13,
			PIN_GRP_I2C1_14,
			PIN_GRP_I2C1_15,
			PIN_GRP_I2C1_16,
			PIN_GRP_I2C1_17,
			PIN_GRP_I2C1_18,
		}),
	},
	[PIN_FUNC_I2C_PMC] = {
		.Id = (u8)PIN_FUNC_I2C_PMC,
		.Name = "i2c_pmc",
		.DevIdx = (u16)XPM_NODEIDX_DEV_I2C_PMC,
		.LmioRegMask = 0xFFF,
		.PmioRegMask = 0x300,
		.NumPins = 2,
		.NumGroups = 13,
		.Groups = ((u16 []) {
			PIN_GRP_I2C_PMC_0,
			PIN_GRP_I2C_PMC_1,
			PIN_GRP_I2C_PMC_2,
			PIN_GRP_I2C_PMC_3,
			PIN_GRP_I2C_PMC_4,
			PIN_GRP_I2C_PMC_5,
			PIN_GRP_I2C_PMC_6,
			PIN_GRP_I2C_PMC_7,
			PIN_GRP_I2C_PMC_8,
			PIN_GRP_I2C_PMC_9,
			PIN_GRP_I2C_PMC_10,
			PIN_GRP_I2C_PMC_11,
			PIN_GRP_I2C_PMC_12,
		}),
	},
	[PIN_FUNC_TTC0_CLK] = {
		.Id = (u8)PIN_FUNC_TTC0_CLK,
		.Name = "tt0_clk",
		.DevIdx = (u16)XPM_NODEIDX_DEV_TTC_0,
		.LmioRegMask = 0x280,
		.PmioRegMask = 0x280,
		.NumPins = 1,
		.NumGroups = 9,
		.Groups = ((u16 []) {
			PIN_GRP_TTC0_0_CLK,
			PIN_GRP_TTC0_1_CLK,
			PIN_GRP_TTC0_2_CLK,
			PIN_GRP_TTC0_3_CLK,
			PIN_GRP_TTC0_4_CLK,
			PIN_GRP_TTC0_5_CLK,
			PIN_GRP_TTC0_6_CLK,
			PIN_GRP_TTC0_7_CLK,
			PIN_GRP_TTC0_8_CLK,
		}),
	},
	[PIN_FUNC_TTC0_WAV] = {
		.Id = (u8)PIN_FUNC_TTC0_WAV,
		.Name = "ttc0_wav",
		.DevIdx = (u16)XPM_NODEIDX_DEV_TTC_0,
		.LmioRegMask = 0x280,
		.PmioRegMask = 0x280,
		.NumPins = 1,
		.NumGroups = 9,
		.Groups = ((u16 []) {
			PIN_GRP_TTC0_0_WAV,
			PIN_GRP_TTC0_1_WAV,
			PIN_GRP_TTC0_2_WAV,
			PIN_GRP_TTC0_3_WAV,
			PIN_GRP_TTC0_4_WAV,
			PIN_GRP_TTC0_5_WAV,
			PIN_GRP_TTC0_6_WAV,
			PIN_GRP_TTC0_7_WAV,
			PIN_GRP_TTC0_8_WAV,
		}),
	},
	[PIN_FUNC_TTC1_CLK] = {
		.Id = (u8)PIN_FUNC_TTC1_CLK,
		.Name = "ttc1_clk",
		.DevIdx = (u16)XPM_NODEIDX_DEV_TTC_1,
		.LmioRegMask = 0x280,
		.PmioRegMask = 0x280,
		.NumPins = 1,
		.NumGroups = 9,
		.Groups = ((u16 []) {
			PIN_GRP_TTC1_0_CLK,
			PIN_GRP_TTC1_1_CLK,
			PIN_GRP_TTC1_2_CLK,
			PIN_GRP_TTC1_3_CLK,
			PIN_GRP_TTC1_4_CLK,
			PIN_GRP_TTC1_5_CLK,
			PIN_GRP_TTC1_6_CLK,
			PIN_GRP_TTC1_7_CLK,
			PIN_GRP_TTC1_8_CLK,
		}),
	},
	[PIN_FUNC_TTC1_WAV] = {
		.Id = (u8)PIN_FUNC_TTC1_WAV,
		.Name = "ttc1_wav",
		.DevIdx = (u16)XPM_NODEIDX_DEV_TTC_1,
		.LmioRegMask = 0x280,
		.PmioRegMask = 0x280,
		.NumPins = 1,
		.NumGroups = 9,
		.Groups = ((u16 []) {
			PIN_GRP_TTC1_0_WAV,
			PIN_GRP_TTC1_1_WAV,
			PIN_GRP_TTC1_2_WAV,
			PIN_GRP_TTC1_3_WAV,
			PIN_GRP_TTC1_4_WAV,
			PIN_GRP_TTC1_5_WAV,
			PIN_GRP_TTC1_6_WAV,
			PIN_GRP_TTC1_7_WAV,
			PIN_GRP_TTC1_8_WAV,
		}),
	},
	[PIN_FUNC_TTC2_CLK] = {
		.Id = (u8)PIN_FUNC_TTC2_CLK,
		.Name = "ttc2_wav",
		.DevIdx = (u16)XPM_NODEIDX_DEV_TTC_2,
		.LmioRegMask = 0x280,
		.PmioRegMask = 0x280,
		.NumPins = 1,
		.NumGroups = 9,
		.Groups = ((u16 []) {
			PIN_GRP_TTC2_0_CLK,
			PIN_GRP_TTC2_1_CLK,
			PIN_GRP_TTC2_2_CLK,
			PIN_GRP_TTC2_3_CLK,
			PIN_GRP_TTC2_4_CLK,
			PIN_GRP_TTC2_5_CLK,
			PIN_GRP_TTC2_6_CLK,
			PIN_GRP_TTC2_7_CLK,
			PIN_GRP_TTC2_8_CLK,
		}),
	},
	[PIN_FUNC_TTC2_WAV] = {
		.Id = (u8)PIN_FUNC_TTC2_WAV,
		.Name = "ttc2_wav",
		.DevIdx = (u16)XPM_NODEIDX_DEV_TTC_2,
		.LmioRegMask = 0x280,
		.PmioRegMask = 0x280,
		.NumPins = 1,
		.NumGroups = 9,
		.Groups = ((u16 []) {
			PIN_GRP_TTC2_0_WAV,
			PIN_GRP_TTC2_1_WAV,
			PIN_GRP_TTC2_2_WAV,
			PIN_GRP_TTC2_3_WAV,
			PIN_GRP_TTC2_4_WAV,
			PIN_GRP_TTC2_5_WAV,
			PIN_GRP_TTC2_6_WAV,
			PIN_GRP_TTC2_7_WAV,
			PIN_GRP_TTC2_8_WAV,
		}),
	},
	[PIN_FUNC_TTC3_CLK] = {
		.Id = (u8)PIN_FUNC_TTC3_CLK,
		.Name = "ttc3_clk",
		.DevIdx = (u16)XPM_NODEIDX_DEV_TTC_3,
		.LmioRegMask = 0x280,
		.PmioRegMask = 0x280,
		.NumPins = 1,
		.NumGroups = 9,
		.Groups = ((u16 []) {
			PIN_GRP_TTC3_0_CLK,
			PIN_GRP_TTC3_1_CLK,
			PIN_GRP_TTC3_2_CLK,
			PIN_GRP_TTC3_3_CLK,
			PIN_GRP_TTC3_4_CLK,
			PIN_GRP_TTC3_5_CLK,
			PIN_GRP_TTC3_6_CLK,
			PIN_GRP_TTC3_7_CLK,
			PIN_GRP_TTC3_8_CLK,
		}),
	},
	[PIN_FUNC_TTC3_WAV] = {
		.Id = (u8)PIN_FUNC_TTC3_WAV,
		.Name = "ttc3_wav",
		.DevIdx = (u16)XPM_NODEIDX_DEV_TTC_3,
		.LmioRegMask = 0x280,
		.PmioRegMask = 0x280,
		.NumPins = 1,
		.NumGroups = 9,
		.Groups = ((u16 []) {
			PIN_GRP_TTC3_0_WAV,
			PIN_GRP_TTC3_1_WAV,
			PIN_GRP_TTC3_2_WAV,
			PIN_GRP_TTC3_3_WAV,
			PIN_GRP_TTC3_4_WAV,
			PIN_GRP_TTC3_5_WAV,
			PIN_GRP_TTC3_6_WAV,
			PIN_GRP_TTC3_7_WAV,
			PIN_GRP_TTC3_8_WAV,
		}),
	},
	[PIN_FUNC_WWDT0] = {
		.Id = (u8)PIN_FUNC_WWDT0,
		.Name = "wwdt0",
		.DevIdx = (u16)XPM_NODEIDX_DEV_SWDT_LPD,
		.LmioRegMask = 0x300,
		.PmioRegMask = 0x200,
		.NumPins = 6,
		.NumGroups = 6,
		.Groups = ((u16 []) {
			PIN_GRP_WWDT0_0,
			PIN_GRP_WWDT0_1,
			PIN_GRP_WWDT0_2,
			PIN_GRP_WWDT0_3,
			PIN_GRP_WWDT0_4,
			PIN_GRP_WWDT0_5,
		}),
	},
	[PIN_FUNC_WWDT1] = {
		.Id = (u8)PIN_FUNC_WWDT1,
		.Name = "wwdt1",
		.DevIdx = (u16)XPM_NODEIDX_DEV_SWDT_FPD,
		.LmioRegMask = 0x300,
		.PmioRegMask = 0x200,
		.NumPins = 6,
		.NumGroups = 6,
		.Groups = ((u16 []) {
			PIN_GRP_WWDT1_0,
			PIN_GRP_WWDT1_1,
			PIN_GRP_WWDT1_2,
			PIN_GRP_WWDT1_3,
			PIN_GRP_WWDT1_4,
			PIN_GRP_WWDT1_5,
		}),
	},
	[PIN_FUNC_SYSMON_I2C0] = {
		.Id = (u8)PIN_FUNC_SYSMON_I2C0,
		.Name = "sysmon_i2c0",
		.DevIdx = 0,
		.LmioRegMask = 0x380,
		.PmioRegMask = 0x00,
		.NumPins = 2,
		.NumGroups = 18,
		.Groups = ((u16 []) {
			PIN_GRP_SYSMON_I2C0_0,
			PIN_GRP_SYSMON_I2C0_1,
			PIN_GRP_SYSMON_I2C0_2,
			PIN_GRP_SYSMON_I2C0_3,
			PIN_GRP_SYSMON_I2C0_4,
			PIN_GRP_SYSMON_I2C0_5,
			PIN_GRP_SYSMON_I2C0_6,
			PIN_GRP_SYSMON_I2C0_7,
			PIN_GRP_SYSMON_I2C0_8,
			PIN_GRP_SYSMON_I2C0_9,
			PIN_GRP_SYSMON_I2C0_10,
			PIN_GRP_SYSMON_I2C0_11,
			PIN_GRP_SYSMON_I2C0_12,
			PIN_GRP_SYSMON_I2C0_13,
			PIN_GRP_SYSMON_I2C0_14,
			PIN_GRP_SYSMON_I2C0_15,
			PIN_GRP_SYSMON_I2C0_16,
			PIN_GRP_SYSMON_I2C0_17,
		}),
	},
	[PIN_FUNC_SYSMON_I2C0_ALERT] = {
		.Id = (u8)PIN_FUNC_SYSMON_I2C0_ALERT,
		.Name = "sysmon_i2c0_alrt",
		.DevIdx = 0,
		.LmioRegMask = 0x380,
		.PmioRegMask = 0x00,
		.NumPins = 1,
		.NumGroups = 18,
		.Groups = ((u16 []) {
			PIN_GRP_SYSMON_I2C0_0_ALERT,
			PIN_GRP_SYSMON_I2C0_1_ALERT,
			PIN_GRP_SYSMON_I2C0_2_ALERT,
			PIN_GRP_SYSMON_I2C0_3_ALERT,
			PIN_GRP_SYSMON_I2C0_4_ALERT,
			PIN_GRP_SYSMON_I2C0_5_ALERT,
			PIN_GRP_SYSMON_I2C0_6_ALERT,
			PIN_GRP_SYSMON_I2C0_7_ALERT,
			PIN_GRP_SYSMON_I2C0_8_ALERT,
			PIN_GRP_SYSMON_I2C0_9_ALERT,
			PIN_GRP_SYSMON_I2C0_10_ALERT,
			PIN_GRP_SYSMON_I2C0_11_ALERT,
			PIN_GRP_SYSMON_I2C0_12_ALERT,
			PIN_GRP_SYSMON_I2C0_13_ALERT,
			PIN_GRP_SYSMON_I2C0_14_ALERT,
			PIN_GRP_SYSMON_I2C0_15_ALERT,
			PIN_GRP_SYSMON_I2C0_16_ALERT,
			PIN_GRP_SYSMON_I2C0_17_ALERT,
		}),
	},
	[PIN_FUNC_UART0] = {
		.Id = (u8)PIN_FUNC_UART0,
		.Name = "uart0",
		.DevIdx = (u16)XPM_NODEIDX_DEV_UART_0,
		.LmioRegMask = 0x20,
		.PmioRegMask = 0x40,
		.NumPins = 2,
		.NumGroups = 9,
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
	[PIN_FUNC_UART0_CTRL] = {
		.Id = (u8)PIN_FUNC_UART0_CTRL,
		.Name = "uart0_ctrl",
		.DevIdx = (u16)XPM_NODEIDX_DEV_UART_0,
		.LmioRegMask = 0x20,
		.PmioRegMask = 0x40,
		.NumPins = 2,
		.NumGroups = 9,
		.Groups = ((u16 []) {
			PIN_GRP_UART0_0_CTRL,
			PIN_GRP_UART0_1_CTRL,
			PIN_GRP_UART0_2_CTRL,
			PIN_GRP_UART0_3_CTRL,
			PIN_GRP_UART0_4_CTRL,
			PIN_GRP_UART0_5_CTRL,
			PIN_GRP_UART0_6_CTRL,
			PIN_GRP_UART0_7_CTRL,
			PIN_GRP_UART0_8_CTRL,
		}),
	},
	[PIN_FUNC_UART1] = {
		.Id = (u8)PIN_FUNC_UART1,
		.Name = "uart1",
		.DevIdx = (u16)XPM_NODEIDX_DEV_UART_1,
		.LmioRegMask = 0x20,
		.PmioRegMask = 0x20,
		.NumPins = 2,
		.NumGroups = 9,
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
	[PIN_FUNC_UART1_CTRL] = {
		.Id = (u8)PIN_FUNC_UART1_CTRL,
		.Name = "uart1_ctrl",
		.DevIdx = (u16)XPM_NODEIDX_DEV_UART_1,
		.LmioRegMask = 0x20,
		.PmioRegMask = 0x40,
		.NumPins = 2,
		.NumGroups = 9,
		.Groups = ((u16 []) {
			PIN_GRP_UART1_0_CTRL,
			PIN_GRP_UART1_1_CTRL,
			PIN_GRP_UART1_2_CTRL,
			PIN_GRP_UART1_3_CTRL,
			PIN_GRP_UART1_4_CTRL,
			PIN_GRP_UART1_5_CTRL,
			PIN_GRP_UART1_6_CTRL,
			PIN_GRP_UART1_7_CTRL,
			PIN_GRP_UART1_8_CTRL,
		}),
	},
	[PIN_FUNC_GPIO0] = {
		.Id = (u8)PIN_FUNC_GPIO0,
		.Name = "gpio0",
		.DevIdx = (u16)XPM_NODEIDX_DEV_GPIO_PMC,
		.LmioRegMask = 0xFFF,
		.PmioRegMask = 0x60,
		.NumPins = 1,
		.NumGroups = 26,
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
		.LmioRegMask = 0xFFF,
		.PmioRegMask = 0x60,
		.NumPins = 1,
		.NumGroups = 26,
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
		.LmioRegMask = 0x40,
		.PmioRegMask = 0xFFF,
		.NumPins = 1,
		.NumGroups = 26,
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
	[PIN_FUNC_EMIO0] = {
		.Id = (u8)PIN_FUNC_EMIO0,
		.Name = "emio0",
		.DevIdx = 0,
		.LmioRegMask = 0x10,
		.PmioRegMask = 0x18,
		.NumPins = 1,
		.NumGroups = 78,
		.Groups = ((u16 []) {
			PIN_GRP_EMIO0_0,
			PIN_GRP_EMIO0_1,
			PIN_GRP_EMIO0_2,
			PIN_GRP_EMIO0_3,
			PIN_GRP_EMIO0_4,
			PIN_GRP_EMIO0_5,
			PIN_GRP_EMIO0_6,
			PIN_GRP_EMIO0_7,
			PIN_GRP_EMIO0_8,
			PIN_GRP_EMIO0_9,
			PIN_GRP_EMIO0_10,
			PIN_GRP_EMIO0_11,
			PIN_GRP_EMIO0_12,
			PIN_GRP_EMIO0_13,
			PIN_GRP_EMIO0_14,
			PIN_GRP_EMIO0_15,
			PIN_GRP_EMIO0_16,
			PIN_GRP_EMIO0_17,
			PIN_GRP_EMIO0_18,
			PIN_GRP_EMIO0_19,
			PIN_GRP_EMIO0_20,
			PIN_GRP_EMIO0_21,
			PIN_GRP_EMIO0_22,
			PIN_GRP_EMIO0_23,
			PIN_GRP_EMIO0_24,
			PIN_GRP_EMIO0_25,
			PIN_GRP_EMIO0_26,
			PIN_GRP_EMIO0_27,
			PIN_GRP_EMIO0_28,
			PIN_GRP_EMIO0_29,
			PIN_GRP_EMIO0_30,
			PIN_GRP_EMIO0_31,
			PIN_GRP_EMIO0_32,
			PIN_GRP_EMIO0_33,
			PIN_GRP_EMIO0_34,
			PIN_GRP_EMIO0_35,
			PIN_GRP_EMIO0_36,
			PIN_GRP_EMIO0_37,
			PIN_GRP_EMIO0_38,
			PIN_GRP_EMIO0_39,
			PIN_GRP_EMIO0_40,
			PIN_GRP_EMIO0_41,
			PIN_GRP_EMIO0_42,
			PIN_GRP_EMIO0_43,
			PIN_GRP_EMIO0_44,
			PIN_GRP_EMIO0_45,
			PIN_GRP_EMIO0_46,
			PIN_GRP_EMIO0_47,
			PIN_GRP_EMIO0_48,
			PIN_GRP_EMIO0_49,
			PIN_GRP_EMIO0_50,
			PIN_GRP_EMIO0_51,
			PIN_GRP_EMIO0_52,
			PIN_GRP_EMIO0_53,
			PIN_GRP_EMIO0_54,
			PIN_GRP_EMIO0_55,
			PIN_GRP_EMIO0_56,
			PIN_GRP_EMIO0_57,
			PIN_GRP_EMIO0_58,
			PIN_GRP_EMIO0_59,
			PIN_GRP_EMIO0_60,
			PIN_GRP_EMIO0_61,
			PIN_GRP_EMIO0_62,
			PIN_GRP_EMIO0_63,
			PIN_GRP_EMIO0_64,
			PIN_GRP_EMIO0_65,
			PIN_GRP_EMIO0_66,
			PIN_GRP_EMIO0_67,
			PIN_GRP_EMIO0_68,
			PIN_GRP_EMIO0_69,
			PIN_GRP_EMIO0_70,
			PIN_GRP_EMIO0_71,
			PIN_GRP_EMIO0_72,
			PIN_GRP_EMIO0_73,
			PIN_GRP_EMIO0_74,
			PIN_GRP_EMIO0_75,
			PIN_GRP_EMIO0_76,
			PIN_GRP_EMIO0_77,
		}),
	},
	[PIN_FUNC_GEM0] = {
		.Id = (u8)PIN_FUNC_GEM0,
		.Name = "gem0",
		.DevIdx = (u16)XPM_NODEIDX_DEV_GEM_0,
		.LmioRegMask = 0x4,
		.PmioRegMask = 0x6,
		.NumPins = 12,
		.NumGroups = 2,
		.Groups = ((u16 []) {
			PIN_GRP_GEM0_0,
			PIN_GRP_GEM0_1,
		}),
	},
	[PIN_FUNC_GEM1] = {
		.Id = (u8)PIN_FUNC_GEM1,
		.Name = "gem1",
		.DevIdx = (u16)XPM_NODEIDX_DEV_GEM_1,
		.LmioRegMask = 0x4,
		.PmioRegMask = 0x6,
		.NumPins = 12,
		.NumGroups = 2,
		.Groups = ((u16 []) {
			PIN_GRP_GEM1_0,
			PIN_GRP_GEM1_1,
		}),
	},
	[PIN_FUNC_TRACE0] = {
		.Id = (u8)PIN_FUNC_TRACE0,
		.Name = "trace0",
		.DevIdx = 0,
		.LmioRegMask = 0x8,
		.PmioRegMask = 0x10,
		.NumPins = 17,
		.NumGroups = 3,
		.Groups = ((u16 []) {
			PIN_GRP_TRACE0_0,
			PIN_GRP_TRACE0_1,
			PIN_GRP_TRACE0_2,
		}),
	},
	[PIN_FUNC_TRACE0_CLK] = {
		.Id = (u8)PIN_FUNC_TRACE0_CLK,
		.Name = "trace0_clk",
		.DevIdx = 0,
		.LmioRegMask = 0x8,
		.PmioRegMask = 0x10,
		.NumPins = 1,
		.NumGroups = 3,
		.Groups = ((u16 []) {
			PIN_GRP_TRACE0_0_CLK,
			PIN_GRP_TRACE0_1_CLK,
			PIN_GRP_TRACE0_2_CLK,
		}),
	},
	[PIN_FUNC_MDIO0] = {
		.Id = (u8)PIN_FUNC_MDIO0,
		.Name = "mdio0",
		.DevIdx = 0,
		.LmioRegMask = 0x280,
		.PmioRegMask = 0x180,
		.NumPins = 2,
		.NumGroups = 2,
		.Groups = ((u16 []) {
			PIN_GRP_MDIO0_0,
			PIN_GRP_MDIO0_1,
		}),
	},
	[PIN_FUNC_MDIO1] = {
		.Id = (u8)PIN_FUNC_MDIO1,
		.Name = "mdio1",
		.DevIdx = 0,
		.LmioRegMask = 0x300,
		.PmioRegMask = 0x200,
		.NumPins = 2,
		.NumGroups = 2,
		.Groups = ((u16 []) {
			PIN_GRP_MDIO1_0,
			PIN_GRP_MDIO1_1,
		}),
	},
	[PIN_FUNC_GEM_TSU0] = {
		.Id = (u8)PIN_FUNC_GEM_TSU0,
		.Name = "gem_tsu0",
		.DevIdx = 0,
		.LmioRegMask = 0x4,
		.PmioRegMask = 0x6,
		.NumPins = 1,
		.NumGroups = 4,
		.Groups = ((u16 []) {
			PIN_GRP_GEM_TSU0_0,
			PIN_GRP_GEM_TSU0_1,
			PIN_GRP_GEM_TSU0_2,
			PIN_GRP_GEM_TSU0_3,
		}),
	},
	[PIN_FUNC_PCIE0] = {
		.Id = (u8)PIN_FUNC_PCIE0,
		.Name = "pcie0",
		.DevIdx = 0,
		.LmioRegMask = 0x100,
		.PmioRegMask = 0x380,
		.NumPins = 2,
		.NumGroups = 3,
		.Groups = ((u16 []) {
			PIN_GRP_PCIE0_0,
			PIN_GRP_PCIE0_1,
			PIN_GRP_PCIE0_2,
		}),
	},
	[PIN_FUNC_SMAP0] = {
		.Id = (u8)PIN_FUNC_SMAP0,
		.Name = "smap0",
		.DevIdx = 0,
		.LmioRegMask = 0xFFF,
		.PmioRegMask = 0x4,
		.NumPins = 36,
		.NumGroups = 1,
		.Groups = ((u16 []) {
			PIN_GRP_SMAP0_0,
		}),
	},
	[PIN_FUNC_USB0] = {
		.Id = (u8)PIN_FUNC_USB0,
		.Name = "usb0",
		.DevIdx = (u16)XPM_NODEIDX_DEV_USB_0,
		.LmioRegMask = 0xFFF,
		.PmioRegMask = 0x6,
		.NumPins = 13,
		.NumGroups = 1,
		.Groups = ((u16 []) {
			PIN_GRP_USB0_0,
		}),
	},
	[PIN_FUNC_SD0] = {
		.Id = (u8)PIN_FUNC_SD0,
		.Name = "sd0",
		.DevIdx = (u16)XPM_NODEIDX_DEV_SDIO_0,
		.LmioRegMask = 0x3FF,
		.PmioRegMask = 0x2,
		.NumPins = 10,
		.NumGroups = 22,
		.Groups = ((u16 []) {
			PIN_GRP_SD0_0,
			PIN_GRP_SD0_4BIT_0_0,
			PIN_GRP_SD0_4BIT_0_1,
			PIN_GRP_SD0_1BIT_0_0,
			PIN_GRP_SD0_1BIT_0_1,
			PIN_GRP_SD0_1BIT_0_2,
			PIN_GRP_SD0_1BIT_0_3,
			PIN_GRP_SD0_1BIT_0_4,
			PIN_GRP_SD0_1BIT_0_5,
			PIN_GRP_SD0_1BIT_0_6,
			PIN_GRP_SD0_1BIT_0_7,
			PIN_GRP_SD0_1,
			PIN_GRP_SD0_4BIT_1_0,
			PIN_GRP_SD0_4BIT_1_1,
			PIN_GRP_SD0_1BIT_1_0,
			PIN_GRP_SD0_1BIT_1_1,
			PIN_GRP_SD0_1BIT_1_2,
			PIN_GRP_SD0_1BIT_1_3,
			PIN_GRP_SD0_1BIT_1_4,
			PIN_GRP_SD0_1BIT_1_5,
			PIN_GRP_SD0_1BIT_1_6,
			PIN_GRP_SD0_1BIT_1_7,
		}),
	},
	[PIN_FUNC_SD0_PC] = {
		.Id = (u8)PIN_FUNC_SD0_PC,
		.Name = "sd0_pc",
		.DevIdx = (u16)XPM_NODEIDX_DEV_SDIO_0,
		.LmioRegMask = 0x3FF,
		.PmioRegMask = 0x2,
		.NumPins = 1,
		.NumGroups = 2,
		.Groups = ((u16 []) {
			PIN_GRP_SD0_0_PC,
			PIN_GRP_SD0_1_PC,
		}),
	},
	[PIN_FUNC_SD0_CD] = {
		.Id = (u8)PIN_FUNC_SD0_CD,
		.Name = "sd0_cd",
		.DevIdx = (u16)XPM_NODEIDX_DEV_SDIO_0,
		.LmioRegMask = 0x3FF,
		.PmioRegMask = 0x2,
		.NumPins = 1,
		.NumGroups = 2,
		.Groups = ((u16 []) {
			PIN_GRP_SD0_0_CD,
			PIN_GRP_SD0_1_CD,
		}),
	},
	[PIN_FUNC_SD0_WP] = {
		.Id = (u8)PIN_FUNC_SD0_WP,
		.Name = "sd0_wp",
		.DevIdx = (u16)XPM_NODEIDX_DEV_SDIO_0,
		.LmioRegMask = 0x3FF,
		.PmioRegMask = 0x2,
		.NumPins = 1,
		.NumGroups = 2,
		.Groups = ((u16 []) {
			PIN_GRP_SD0_0_WP,
			PIN_GRP_SD0_1_WP,
		}),
	},
	[PIN_FUNC_SD1] = {
		.Id = (u8)PIN_FUNC_SD1,
		.Name = "sd1",
		.DevIdx = (u16)XPM_NODEIDX_DEV_SDIO_1,
		.LmioRegMask = 0x3FF,
		.PmioRegMask = 0x2,
		.NumPins = 10,
		.NumGroups = 22,
		.Groups = ((u16 []) {
			PIN_GRP_SD1_0,
			PIN_GRP_SD1_4BIT_0_0,
			PIN_GRP_SD1_4BIT_0_1,
			PIN_GRP_SD1_1BIT_0_0,
			PIN_GRP_SD1_1BIT_0_1,
			PIN_GRP_SD1_1BIT_0_2,
			PIN_GRP_SD1_1BIT_0_3,
			PIN_GRP_SD1_1BIT_0_4,
			PIN_GRP_SD1_1BIT_0_5,
			PIN_GRP_SD1_1BIT_0_6,
			PIN_GRP_SD1_1BIT_0_7,
			PIN_GRP_SD1_1,
			PIN_GRP_SD1_4BIT_1_0,
			PIN_GRP_SD1_4BIT_1_1,
			PIN_GRP_SD1_1BIT_1_0,
			PIN_GRP_SD1_1BIT_1_1,
			PIN_GRP_SD1_1BIT_1_2,
			PIN_GRP_SD1_1BIT_1_3,
			PIN_GRP_SD1_1BIT_1_4,
			PIN_GRP_SD1_1BIT_1_5,
			PIN_GRP_SD1_1BIT_1_6,
			PIN_GRP_SD1_1BIT_1_7,
		}),
	},
	[PIN_FUNC_SD1_PC] = {
		.Id = (u8)PIN_FUNC_SD1_PC,
		.Name = "sd1_pc",
		.DevIdx = (u16)XPM_NODEIDX_DEV_SDIO_1,
		.LmioRegMask = 0x3FF,
		.PmioRegMask = 0x2,
		.NumPins = 1,
		.NumGroups = 2,
		.Groups = ((u16 []) {
			PIN_GRP_SD1_0_PC,
			PIN_GRP_SD1_1_PC,
		}),
	},
	[PIN_FUNC_SD1_CD] = {
		.Id = (u8)PIN_FUNC_SD1_CD,
		.Name = "sd1_cd",
		.DevIdx = (u16)XPM_NODEIDX_DEV_SDIO_1,
		.LmioRegMask = 0x3FF,
		.PmioRegMask = 0x2,
		.NumPins = 1,
		.NumGroups = 2,
		.Groups = ((u16 []) {
			PIN_GRP_SD1_0_CD,
			PIN_GRP_SD1_1_CD,
		}),
	},
	[PIN_FUNC_SD1_WP] = {
		.Id = (u8)PIN_FUNC_SD1_WP,
		.Name = "sd1_wp",
		.DevIdx = (u16)XPM_NODEIDX_DEV_SDIO_1,
		.LmioRegMask = 0x3FF,
		.PmioRegMask = 0x2,
		.NumPins = 1,
		.NumGroups = 2,
		.Groups = ((u16 []) {
			PIN_GRP_SD1_0_WP,
			PIN_GRP_SD1_1_WP,
		}),
	},
	[PIN_FUNC_OSPI0] = {
		.Id = (u8)PIN_FUNC_OSPI0,
		.Name = "ospi0",
		.DevIdx = (u16)XPM_NODEIDX_DEV_OSPI,
		.LmioRegMask = 0x3FF,
		.PmioRegMask = 0x4,
		.NumPins = 10,
		.NumGroups = 1,
		.Groups = ((u16 []) {
			PIN_GRP_OSPI0_0,
		}),
	},
	[PIN_FUNC_OSPI0_SS] = {
		.Id = (u8)PIN_FUNC_OSPI0_SS,
		.Name = "ospi0_ss",
		.DevIdx = (u16)XPM_NODEIDX_DEV_OSPI,
		.LmioRegMask = 0x3FF,
		.PmioRegMask = 0x4,
		.NumPins = 2,
		.NumGroups = 1,
		.Groups = ((u16 []) {
			PIN_GRP_OSPI0_0_SS,
		}),
	},
	[PIN_FUNC_QSPI0] = {
		.Id = (u8)PIN_FUNC_QSPI0,
		.Name = "qspi0",
		.DevIdx = (u16)XPM_NODEIDX_DEV_QSPI,
		.LmioRegMask = 0x3FF,
		.PmioRegMask = 0x6,
		.NumPins = 10,
		.NumGroups = 1,
		.Groups = ((u16 []) {
			PIN_GRP_QSPI0_0,
		}),
	},
	[PIN_FUNC_QSPI0_FBCLK] = {
		.Id = (u8)PIN_FUNC_QSPI0_FBCLK,
		.Name = "qspi0_fbclk",
		.DevIdx = (u16)XPM_NODEIDX_DEV_QSPI,
		.LmioRegMask = 0x3FF,
		.PmioRegMask = 0x6,
		.NumPins = 1,
		.NumGroups = 1,
		.Groups = ((u16 []) {
			PIN_GRP_QSPI0_0_FBCLK,
		}),
	},
	[PIN_FUNC_QSPI0_SS] = {
		.Id = (u8)PIN_FUNC_QSPI0_SS,
		.Name = "qspi0_ss",
		.DevIdx = (u16)XPM_NODEIDX_DEV_QSPI,
		.LmioRegMask = 0x3FF,
		.PmioRegMask = 0x6,
		.NumPins = 2,
		.NumGroups = 1,
		.Groups = ((u16 []) {
			PIN_GRP_QSPI0_0_SS,
		}),
	},
	[PIN_FUNC_TEST_CLK] = {
		.Id = (u8)PIN_FUNC_TEST_CLK,
		.Name = "test_clk",
		.DevIdx = 0,
		.LmioRegMask = 0x3FF,
		.PmioRegMask = 0x8,
		.NumPins = 4,
		.NumGroups = 1,
		.Groups = ((u16 []) {
			PIN_GRP_TEST_CLK_0,
		}),
	},
	[PIN_FUNC_TEST_SCAN] = {
		.Id = (u8)PIN_FUNC_TEST_SCAN,
		.Name = "test_scan",
		.DevIdx = 0,
		.LmioRegMask = 0x3FF,
		.PmioRegMask = 0x20,
		.NumPins = 38,
		.NumGroups = 1,
		.Groups = ((u16 []) {
			PIN_GRP_TEST_SCAN_0,
		}),
	},
	[PIN_FUNC_TAMPER_TRIGGER] = {
		.Id = (u8)PIN_FUNC_TAMPER_TRIGGER,
		.Name = "tamper_trigger",
		.DevIdx = 0,
		.LmioRegMask = 0x3FF,
		.PmioRegMask = 0x380,
		.NumPins = 8,
		.NumGroups = 1,
		.Groups = ((u16 []) {
			PIN_GRP_TAMPER_TRIGGER_0,
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

	(void)memset(FuncName, 0, FUNC_QUERY_NAME_LEN);

	PinFunc = XPmPinFunc_GetById(FuncId);
	if (NULL == PinFunc) {
		goto done;
	}

	(void)memcpy(FuncName, &PinFunc->Name[0], FUNC_QUERY_NAME_LEN);

	Status = XST_SUCCESS;
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

	(void)memset(Groups, (s32)END_OF_GRP, (MAX_GROUPS_PER_RES * sizeof(u16)));

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
