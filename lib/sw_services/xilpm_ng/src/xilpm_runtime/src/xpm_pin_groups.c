/******************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*
 * This is an automatically generated file from script.
 * Please do not modify this!
 */

#include "xil_util.h"
#include "xpm_node.h"
#include "xpm_pin.h"
#include "xpm_pinfunc.h"
#include "xpm_runtime_pin.h"

static const XPm_PinGroup PmPinGroups[XPM_NODEIDX_STMIC_MAX] = {
	[XPM_NODEIDX_STMIC_LMIO_0] = {
		.GroupCount = 7U,
		.GroupList = ((u16 []) {
			PIN_GRP_GEM0_0,
			PIN_GRP_UART0_0,
			PIN_GRP_GPIO2_0,
			PIN_GRP_SPI0_0,
			PIN_GRP_CAN1_0,
			PIN_GRP_I2C1_0,
			PIN_GRP_SYSMON_I2C_0,
		})
	},
	[XPM_NODEIDX_STMIC_LMIO_1] = {
		.GroupCount = 7U,
		.GroupList = ((u16 []) {
			PIN_GRP_GEM0_0,
			PIN_GRP_UART0_0,
			PIN_GRP_GPIO2_1,
			PIN_GRP_SPI0_SS_0,
			PIN_GRP_CAN1_0,
			PIN_GRP_I2C1_0,
			PIN_GRP_SYSMON_I2C_0,
		})
	},
	[XPM_NODEIDX_STMIC_LMIO_2] = {
		.GroupCount = 8U,
		.GroupList = ((u16 []) {
			PIN_GRP_GEM0_0,
			PIN_GRP_TRACE_0,
			PIN_GRP_UART0_0,
			PIN_GRP_GPIO2_2,
			PIN_GRP_SPI0_SS_0,
			PIN_GRP_CAN0_0,
			PIN_GRP_I2C0_0,
			PIN_GRP_SYSMON_I2C_ALERT_0,
		})
	},
	[XPM_NODEIDX_STMIC_LMIO_3] = {
		.GroupCount = 8U,
		.GroupList = ((u16 []) {
			PIN_GRP_GEM0_0,
			PIN_GRP_TRACE_0,
			PIN_GRP_UART0_0,
			PIN_GRP_GPIO2_3,
			PIN_GRP_SPI0_SS_0,
			PIN_GRP_PCIE_0,
			PIN_GRP_CAN0_0,
			PIN_GRP_I2C0_0,
		})
	},
	[XPM_NODEIDX_STMIC_LMIO_4] = {
		.GroupCount = 8U,
		.GroupList = ((u16 []) {
			PIN_GRP_GEM0_0,
			PIN_GRP_TRACE_0,
			PIN_GRP_UART1_0,
			PIN_GRP_GPIO2_4,
			PIN_GRP_SPI0_0,
			PIN_GRP_CAN3_0,
			PIN_GRP_TTC1_CLK_0,
			PIN_GRP_SYSMON_I2C_1,
		})
	},
	[XPM_NODEIDX_STMIC_LMIO_5] = {
		.GroupCount = 8U,
		.GroupList = ((u16 []) {
			PIN_GRP_GEM0_0,
			PIN_GRP_TRACE_0,
			PIN_GRP_UART1_0,
			PIN_GRP_GPIO2_5,
			PIN_GRP_SPI0_0,
			PIN_GRP_CAN3_0,
			PIN_GRP_TTC1_WAV_0,
			PIN_GRP_SYSMON_I2C_1,
		})
	},
	[XPM_NODEIDX_STMIC_LMIO_6] = {
		.GroupCount = 8U,
		.GroupList = ((u16 []) {
			PIN_GRP_GEM0_0,
			PIN_GRP_TRACE_CLK_0,
			PIN_GRP_UART1_0,
			PIN_GRP_GPIO2_6,
			PIN_GRP_SPI1_0,
			PIN_GRP_CAN2_0,
			PIN_GRP_TTC0_CLK_0,
			PIN_GRP_SYSMON_I2C_ALERT_1,
		})
	},
	[XPM_NODEIDX_STMIC_LMIO_7] = {
		.GroupCount = 8U,
		.GroupList = ((u16 []) {
			PIN_GRP_GEM0_0,
			PIN_GRP_TRACE_0,
			PIN_GRP_UART1_0,
			PIN_GRP_GPIO2_7,
			PIN_GRP_SPI1_SS_0,
			PIN_GRP_CAN2_0,
			PIN_GRP_MMI_DP_HPD_0,
			PIN_GRP_TTC0_WAV_0,
		})
	},
	[XPM_NODEIDX_STMIC_LMIO_8] = {
		.GroupCount = 8U,
		.GroupList = ((u16 []) {
			PIN_GRP_GEM0_0,
			PIN_GRP_TRACE_0,
			PIN_GRP_UART0_1,
			PIN_GRP_GPIO2_8,
			PIN_GRP_SPI1_SS_0,
			PIN_GRP_PCIE_0,
			PIN_GRP_CAN1_1,
			PIN_GRP_MDIO2_0,
		})
	},
	[XPM_NODEIDX_STMIC_LMIO_9] = {
		.GroupCount = 8U,
		.GroupList = ((u16 []) {
			PIN_GRP_GEM0_0,
			PIN_GRP_TRACE_0,
			PIN_GRP_UART0_1,
			PIN_GRP_GPIO2_9,
			PIN_GRP_SPI1_SS_0,
			PIN_GRP_CAN1_1,
			PIN_GRP_MDIO2_0,
			PIN_GRP_SYSMON_I2C_2,
		})
	},
	[XPM_NODEIDX_STMIC_LMIO_10] = {
		.GroupCount = 8U,
		.GroupList = ((u16 []) {
			PIN_GRP_GEM0_0,
			PIN_GRP_TRACE_0,
			PIN_GRP_UART0_1,
			PIN_GRP_GPIO2_10,
			PIN_GRP_SPI1_0,
			PIN_GRP_CAN0_1,
			PIN_GRP_TTC2_CLK_0,
			PIN_GRP_SYSMON_I2C_2,
		})
	},
	[XPM_NODEIDX_STMIC_LMIO_11] = {
		.GroupCount = 8U,
		.GroupList = ((u16 []) {
			PIN_GRP_GEM0_0,
			PIN_GRP_TRACE_0,
			PIN_GRP_UART0_1,
			PIN_GRP_GPIO2_11,
			PIN_GRP_SPI1_0,
			PIN_GRP_CAN0_1,
			PIN_GRP_TTC2_WAV_0,
			PIN_GRP_SYSMON_I2C_ALERT_2,
		})
	},
	[XPM_NODEIDX_STMIC_LMIO_12] = {
		.GroupCount = 8U,
		.GroupList = ((u16 []) {
			PIN_GRP_GEM1_0,
			PIN_GRP_TRACE_0,
			PIN_GRP_UART1_1,
			PIN_GRP_GPIO2_12,
			PIN_GRP_SPI0_1,
			PIN_GRP_CAN3_1,
			PIN_GRP_MMI_DP_HPD_1,
			PIN_GRP_TTC1_CLK_1,
		})
	},
	[XPM_NODEIDX_STMIC_LMIO_13] = {
		.GroupCount = 8U,
		.GroupList = ((u16 []) {
			PIN_GRP_GEM1_0,
			PIN_GRP_TRACE_0,
			PIN_GRP_UART1_1,
			PIN_GRP_GPIO2_13,
			PIN_GRP_SPI0_SS_1,
			PIN_GRP_CAN3_1,
			PIN_GRP_TTC1_WAV_1,
			PIN_GRP_SYSMON_I2C_3,
		})
	},
	[XPM_NODEIDX_STMIC_LMIO_14] = {
		.GroupCount = 7U,
		.GroupList = ((u16 []) {
			PIN_GRP_GEM1_0,
			PIN_GRP_UART1_1,
			PIN_GRP_GPIO2_14,
			PIN_GRP_SPI0_SS_1,
			PIN_GRP_CAN2_1,
			PIN_GRP_TTC0_CLK_1,
			PIN_GRP_SYSMON_I2C_3,
		})
	},
	[XPM_NODEIDX_STMIC_LMIO_15] = {
		.GroupCount = 7U,
		.GroupList = ((u16 []) {
			PIN_GRP_GEM1_0,
			PIN_GRP_UART1_1,
			PIN_GRP_GPIO2_15,
			PIN_GRP_SPI0_SS_1,
			PIN_GRP_CAN2_1,
			PIN_GRP_TTC0_WAV_1,
			PIN_GRP_SYSMON_I2C_ALERT_3,
		})
	},
	[XPM_NODEIDX_STMIC_LMIO_16] = {
		.GroupCount = 7U,
		.GroupList = ((u16 []) {
			PIN_GRP_GEM1_0,
			PIN_GRP_UART0_2,
			PIN_GRP_GPIO2_16,
			PIN_GRP_SPI0_1,
			PIN_GRP_CAN1_2,
			PIN_GRP_I2C1_1,
			PIN_GRP_MDIO2_1,
		})
	},
	[XPM_NODEIDX_STMIC_LMIO_17] = {
		.GroupCount = 7U,
		.GroupList = ((u16 []) {
			PIN_GRP_GEM1_0,
			PIN_GRP_UART0_2,
			PIN_GRP_GPIO2_17,
			PIN_GRP_SPI0_1,
			PIN_GRP_CAN1_2,
			PIN_GRP_I2C1_1,
			PIN_GRP_MDIO2_1,
		})
	},
	[XPM_NODEIDX_STMIC_LMIO_18] = {
		.GroupCount = 8U,
		.GroupList = ((u16 []) {
			PIN_GRP_GEM1_0,
			PIN_GRP_TRACE_0,
			PIN_GRP_UART0_2,
			PIN_GRP_GPIO2_18,
			PIN_GRP_SPI1_1,
			PIN_GRP_CAN0_2,
			PIN_GRP_I2C0_1,
			PIN_GRP_SYSMON_I2C_4,
		})
	},
	[XPM_NODEIDX_STMIC_LMIO_19] = {
		.GroupCount = 8U,
		.GroupList = ((u16 []) {
			PIN_GRP_GEM1_0,
			PIN_GRP_TRACE_0,
			PIN_GRP_UART0_2,
			PIN_GRP_GPIO2_19,
			PIN_GRP_SPI1_SS_1,
			PIN_GRP_CAN0_2,
			PIN_GRP_I2C0_1,
			PIN_GRP_SYSMON_I2C_4,
		})
	},
	[XPM_NODEIDX_STMIC_LMIO_20] = {
		.GroupCount = 8U,
		.GroupList = ((u16 []) {
			PIN_GRP_GEM1_0,
			PIN_GRP_TRACE_0,
			PIN_GRP_UART1_2,
			PIN_GRP_GPIO2_20,
			PIN_GRP_SPI1_SS_1,
			PIN_GRP_CAN1_3,
			PIN_GRP_TTC1_CLK_2,
			PIN_GRP_SYSMON_I2C_ALERT_4,
		})
	},
	[XPM_NODEIDX_STMIC_LMIO_21] = {
		.GroupCount = 8U,
		.GroupList = ((u16 []) {
			PIN_GRP_GEM1_0,
			PIN_GRP_TRACE_0,
			PIN_GRP_UART1_2,
			PIN_GRP_GPIO2_21,
			PIN_GRP_SPI1_SS_1,
			PIN_GRP_CAN1_3,
			PIN_GRP_TTC1_WAV_2,
			PIN_GRP_MDIO1_0,
		})
	},
	[XPM_NODEIDX_STMIC_LMIO_22] = {
		.GroupCount = 8U,
		.GroupList = ((u16 []) {
			PIN_GRP_GEM1_0,
			PIN_GRP_TRACE_0,
			PIN_GRP_UART1_2,
			PIN_GRP_GPIO2_22,
			PIN_GRP_SPI1_1,
			PIN_GRP_CAN0_3,
			PIN_GRP_TTC0_CLK_2,
			PIN_GRP_MDIO1_0,
		})
	},
	[XPM_NODEIDX_STMIC_LMIO_23] = {
		.GroupCount = 8U,
		.GroupList = ((u16 []) {
			PIN_GRP_GEM1_0,
			PIN_GRP_TRACE_0,
			PIN_GRP_UART1_2,
			PIN_GRP_GPIO2_23,
			PIN_GRP_SPI1_1,
			PIN_GRP_CAN0_3,
			PIN_GRP_TTC0_WAV_2,
			PIN_GRP_SYSMON_I2C_5,
		})
	},
	[XPM_NODEIDX_STMIC_LMIO_24] = {
		.GroupCount = 6U,
		.GroupList = ((u16 []) {
			PIN_GRP_GEM_TSU_0,
			PIN_GRP_GPIO2_24,
			PIN_GRP_MDIO0_0,
			PIN_GRP_CAN1_4,
			PIN_GRP_I2C1_2,
			PIN_GRP_SYSMON_I2C_5,
		})
	},
	[XPM_NODEIDX_STMIC_LMIO_25] = {
		.GroupCount = 6U,
		.GroupList = ((u16 []) {
			PIN_GRP_GEM_TSU_1,
			PIN_GRP_GPIO2_25,
			PIN_GRP_MDIO0_0,
			PIN_GRP_CAN1_4,
			PIN_GRP_I2C1_2,
			PIN_GRP_SYSMON_I2C_ALERT_5,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_0] = {
		.GroupCount = 10U,
		.GroupList = ((u16 []) {
			PIN_GRP_OSPI_0,
			PIN_GRP_SMAP_0,
			PIN_GRP_QSPI_0,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART0_3,
			PIN_GRP_GPIO0_0,
			PIN_GRP_SYSMON_I2C_6,
			PIN_GRP_I2C1_3,
			PIN_GRP_SPI0_2,
			PIN_GRP_CAN0_4,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_1] = {
		.GroupCount = 11U,
		.GroupList = ((u16 []) {
			PIN_GRP_OSPI_0,
			PIN_GRP_SMAP_0,
			PIN_GRP_QSPI_0,
			PIN_GRP_UFS_0,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART0_3,
			PIN_GRP_GPIO0_1,
			PIN_GRP_SYSMON_I2C_6,
			PIN_GRP_I2C1_3,
			PIN_GRP_SPI0_SS_2,
			PIN_GRP_CAN0_4,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_2] = {
		.GroupCount = 11U,
		.GroupList = ((u16 []) {
			PIN_GRP_OSPI_0,
			PIN_GRP_SMAP_0,
			PIN_GRP_QSPI_0,
			PIN_GRP_UFS_0,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART0_3,
			PIN_GRP_GPIO0_2,
			PIN_GRP_SYSMON_I2C_ALERT_6,
			PIN_GRP_I2C0_2,
			PIN_GRP_SPI0_SS_2,
			PIN_GRP_CAN1_5,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_3] = {
		.GroupCount = 10U,
		.GroupList = ((u16 []) {
			PIN_GRP_OSPI_0,
			PIN_GRP_SMAP_0,
			PIN_GRP_QSPI_0,
			PIN_GRP_UFS_0,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART0_3,
			PIN_GRP_GPIO0_3,
			PIN_GRP_I2C0_2,
			PIN_GRP_SPI0_SS_2,
			PIN_GRP_CAN1_5,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_4] = {
		.GroupCount = 11U,
		.GroupList = ((u16 []) {
			PIN_GRP_OSPI_0,
			PIN_GRP_SMAP_0,
			PIN_GRP_QSPI_0,
			PIN_GRP_TRACE_1,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART1_3,
			PIN_GRP_GPIO0_4,
			PIN_GRP_SYSMON_I2C_7,
			PIN_GRP_SPI0_2,
			PIN_GRP_CAN2_2,
			PIN_GRP_WWDT_0,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_5] = {
		.GroupCount = 10U,
		.GroupList = ((u16 []) {
			PIN_GRP_OSPI_0,
			PIN_GRP_SMAP_0,
			PIN_GRP_QSPI_SS_0,
			PIN_GRP_TRACE_1,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART1_3,
			PIN_GRP_GPIO0_5,
			PIN_GRP_SYSMON_I2C_7,
			PIN_GRP_SPI0_2,
			PIN_GRP_CAN2_2,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_6] = {
		.GroupCount = 11U,
		.GroupList = ((u16 []) {
			PIN_GRP_OSPI_0,
			PIN_GRP_SMAP_0,
			PIN_GRP_QSPI_FBCLK_0,
			PIN_GRP_TRACE_CLK_1,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART1_3,
			PIN_GRP_GPIO0_6,
			PIN_GRP_SYSMON_I2C_ALERT_7,
			PIN_GRP_SPI1_2,
			PIN_GRP_CAN3_2,
			PIN_GRP_TTC1_CLK_3,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_7] = {
		.GroupCount = 11U,
		.GroupList = ((u16 []) {
			PIN_GRP_OSPI_0,
			PIN_GRP_SMAP_0,
			PIN_GRP_QSPI_SS_0,
			PIN_GRP_TRACE_1,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART1_3,
			PIN_GRP_GPIO0_7,
			PIN_GRP_SPI1_SS_2,
			PIN_GRP_CAN3_2,
			PIN_GRP_TTC1_WAV_3,
			PIN_GRP_PCIE_1,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_8] = {
		.GroupCount = 11U,
		.GroupList = ((u16 []) {
			PIN_GRP_OSPI_0,
			PIN_GRP_SMAP_0,
			PIN_GRP_QSPI_0,
			PIN_GRP_TRACE_1,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART0_4,
			PIN_GRP_GPIO0_8,
			PIN_GRP_SPI1_SS_2,
			PIN_GRP_CAN0_5,
			PIN_GRP_TTC0_CLK_3,
			PIN_GRP_PCIE_1,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_9] = {
		.GroupCount = 11U,
		.GroupList = ((u16 []) {
			PIN_GRP_OSPI_0,
			PIN_GRP_SMAP_0,
			PIN_GRP_QSPI_0,
			PIN_GRP_TRACE_1,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART0_4,
			PIN_GRP_GPIO0_9,
			PIN_GRP_SYSMON_I2C_8,
			PIN_GRP_SPI1_SS_2,
			PIN_GRP_CAN0_5,
			PIN_GRP_TTC0_WAV_3,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_10] = {
		.GroupCount = 11U,
		.GroupList = ((u16 []) {
			PIN_GRP_OSPI_SS_0,
			PIN_GRP_SMAP_0,
			PIN_GRP_QSPI_0,
			PIN_GRP_TRACE_1,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART0_4,
			PIN_GRP_GPIO0_10,
			PIN_GRP_SYSMON_I2C_8,
			PIN_GRP_SPI1_2,
			PIN_GRP_CAN1_6,
			PIN_GRP_WWDT_1,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_11] = {
		.GroupCount = 11U,
		.GroupList = ((u16 []) {
			PIN_GRP_OSPI_SS_0,
			PIN_GRP_SMAP_0,
			PIN_GRP_QSPI_0,
			PIN_GRP_TRACE_1,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART0_4,
			PIN_GRP_GPIO0_11,
			PIN_GRP_SYSMON_I2C_ALERT_8,
			PIN_GRP_SPI1_2,
			PIN_GRP_CAN1_6,
			PIN_GRP_TTC3_CLK_0,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_12] = {
		.GroupCount = 9U,
		.GroupList = ((u16 []) {
			PIN_GRP_OSPI_RST_N_0,
			PIN_GRP_QSPI_0,
			PIN_GRP_TRACE_1,
			PIN_GRP_UART1_4,
			PIN_GRP_GPIO0_12,
			PIN_GRP_SPI0_3,
			PIN_GRP_CAN2_3,
			PIN_GRP_TTC3_WAV_0,
			PIN_GRP_EXT_TAMPER_TRIG_0,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_13] = {
		.GroupCount = 10U,
		.GroupList = ((u16 []) {
			PIN_GRP_OSPI_ECC_FAIL_0,
			PIN_GRP_SD0_0,
			PIN_GRP_USB0_0,
			PIN_GRP_TRACE_1,
			PIN_GRP_UART1_4,
			PIN_GRP_GPIO0_13,
			PIN_GRP_SYSMON_I2C_9,
			PIN_GRP_SPI0_SS_3,
			PIN_GRP_CAN2_3,
			PIN_GRP_EXT_TAMPER_TRIG_1,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_14] = {
		.GroupCount = 10U,
		.GroupList = ((u16 []) {
			PIN_GRP_SD1_0,
			PIN_GRP_SD0_0,
			PIN_GRP_USB0_0,
			PIN_GRP_TRACE_1,
			PIN_GRP_UART1_4,
			PIN_GRP_GPIO0_14,
			PIN_GRP_SYSMON_I2C_9,
			PIN_GRP_SPI0_SS_3,
			PIN_GRP_CAN3_3,
			PIN_GRP_TTC1_CLK_4,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_15] = {
		.GroupCount = 10U,
		.GroupList = ((u16 []) {
			PIN_GRP_SD1_0,
			PIN_GRP_SD0_0,
			PIN_GRP_USB0_0,
			PIN_GRP_TRACE_1,
			PIN_GRP_UART1_4,
			PIN_GRP_GPIO0_15,
			PIN_GRP_SYSMON_I2C_ALERT_9,
			PIN_GRP_SPI0_SS_3,
			PIN_GRP_CAN3_3,
			PIN_GRP_TTC1_WAV_4,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_16] = {
		.GroupCount = 10U,
		.GroupList = ((u16 []) {
			PIN_GRP_SD1_0,
			PIN_GRP_SD0_0,
			PIN_GRP_USB0_0,
			PIN_GRP_TRACE_1,
			PIN_GRP_UART0_5,
			PIN_GRP_GPIO0_16,
			PIN_GRP_I2C1_4,
			PIN_GRP_SPI0_3,
			PIN_GRP_CAN0_6,
			PIN_GRP_TTC0_CLK_4,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_17] = {
		.GroupCount = 10U,
		.GroupList = ((u16 []) {
			PIN_GRP_SD1_0,
			PIN_GRP_SD0_PC_0,
			PIN_GRP_USB0_0,
			PIN_GRP_TRACE_1,
			PIN_GRP_UART0_5,
			PIN_GRP_GPIO0_17,
			PIN_GRP_I2C1_4,
			PIN_GRP_SPI0_3,
			PIN_GRP_CAN0_6,
			PIN_GRP_TTC0_WAV_4,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_18] = {
		.GroupCount = 10U,
		.GroupList = ((u16 []) {
			PIN_GRP_SD1_0,
			PIN_GRP_SD0_0,
			PIN_GRP_USB0_0,
			PIN_GRP_TRACE_1,
			PIN_GRP_UART0_5,
			PIN_GRP_GPIO0_18,
			PIN_GRP_SYSMON_I2C_10,
			PIN_GRP_I2C0_3,
			PIN_GRP_SPI1_3,
			PIN_GRP_CAN1_7,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_19] = {
		.GroupCount = 10U,
		.GroupList = ((u16 []) {
			PIN_GRP_SD1_0,
			PIN_GRP_SD0_0,
			PIN_GRP_USB0_0,
			PIN_GRP_TRACE_1,
			PIN_GRP_UART0_5,
			PIN_GRP_GPIO0_19,
			PIN_GRP_SYSMON_I2C_10,
			PIN_GRP_I2C0_3,
			PIN_GRP_SPI1_SS_3,
			PIN_GRP_CAN1_7,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_20] = {
		.GroupCount = 10U,
		.GroupList = ((u16 []) {
			PIN_GRP_SD1_0,
			PIN_GRP_SD0_0,
			PIN_GRP_USB0_0,
			PIN_GRP_TRACE_1,
			PIN_GRP_UART1_5,
			PIN_GRP_GPIO0_20,
			PIN_GRP_SYSMON_I2C_ALERT_10,
			PIN_GRP_SPI1_SS_3,
			PIN_GRP_CAN2_4,
			PIN_GRP_TTC2_CLK_1,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_21] = {
		.GroupCount = 10U,
		.GroupList = ((u16 []) {
			PIN_GRP_SD1_0,
			PIN_GRP_SD0_0,
			PIN_GRP_USB0_0,
			PIN_GRP_TRACE_1,
			PIN_GRP_UART1_5,
			PIN_GRP_GPIO0_21,
			PIN_GRP_SPI1_SS_3,
			PIN_GRP_CAN2_4,
			PIN_GRP_TTC2_WAV_1,
			PIN_GRP_EXT_TAMPER_TRIG_2,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_22] = {
		.GroupCount = 10U,
		.GroupList = ((u16 []) {
			PIN_GRP_SD1_0,
			PIN_GRP_SD0_0,
			PIN_GRP_USB0_0,
			PIN_GRP_TEST_CLK_0,
			PIN_GRP_UART1_5,
			PIN_GRP_GPIO0_22,
			PIN_GRP_SPI1_3,
			PIN_GRP_CAN3_4,
			PIN_GRP_TTC1_CLK_5,
			PIN_GRP_EXT_TAMPER_TRIG_3,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_23] = {
		.GroupCount = 10U,
		.GroupList = ((u16 []) {
			PIN_GRP_SD1_0,
			PIN_GRP_SD0_0,
			PIN_GRP_USB0_0,
			PIN_GRP_TEST_CLK_0,
			PIN_GRP_UART1_5,
			PIN_GRP_GPIO0_23,
			PIN_GRP_SYSMON_I2C_11,
			PIN_GRP_SPI1_3,
			PIN_GRP_CAN3_4,
			PIN_GRP_TTC1_WAV_5,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_24] = {
		.GroupCount = 9U,
		.GroupList = ((u16 []) {
			PIN_GRP_SD1_0,
			PIN_GRP_SD0_CD_0,
			PIN_GRP_USB0_0,
			PIN_GRP_UFS_1,
			PIN_GRP_TEST_CLK_0,
			PIN_GRP_GPIO0_24,
			PIN_GRP_SYSMON_I2C_11,
			PIN_GRP_WWDT_2,
			PIN_GRP_TTC0_CLK_5,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_25] = {
		.GroupCount = 8U,
		.GroupList = ((u16 []) {
			PIN_GRP_SD1_PC_0,
			PIN_GRP_SD0_WP_0,
			PIN_GRP_USB0_0,
			PIN_GRP_UFS_1,
			PIN_GRP_TEST_CLK_0,
			PIN_GRP_GPIO0_25,
			PIN_GRP_SYSMON_I2C_ALERT_11,
			PIN_GRP_TTC0_WAV_5,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_26] = {
		.GroupCount = 11U,
		.GroupList = ((u16 []) {
			PIN_GRP_OSPI_ECC_FAIL_1,
			PIN_GRP_SD0_PC_1,
			PIN_GRP_GEM0_1,
			PIN_GRP_UFS_1,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART0_6,
			PIN_GRP_GPIO1_0,
			PIN_GRP_I2C0_4,
			PIN_GRP_SPI0_4,
			PIN_GRP_CAN0_7,
			PIN_GRP_EXT_TAMPER_TRIG_4,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_27] = {
		.GroupCount = 10U,
		.GroupList = ((u16 []) {
			PIN_GRP_SD0_1,
			PIN_GRP_GEM0_1,
			PIN_GRP_USB1_0,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART0_6,
			PIN_GRP_GPIO1_1,
			PIN_GRP_I2C0_4,
			PIN_GRP_SPI0_SS_4,
			PIN_GRP_CAN0_7,
			PIN_GRP_TTC1_CLK_6,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_28] = {
		.GroupCount = 11U,
		.GroupList = ((u16 []) {
			PIN_GRP_SMAP_0,
			PIN_GRP_SD0_1,
			PIN_GRP_GEM0_1,
			PIN_GRP_USB1_0,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART0_6,
			PIN_GRP_GPIO1_2,
			PIN_GRP_I2C1_5,
			PIN_GRP_SPI0_SS_4,
			PIN_GRP_CAN1_8,
			PIN_GRP_TTC1_WAV_6,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_29] = {
		.GroupCount = 11U,
		.GroupList = ((u16 []) {
			PIN_GRP_SMAP_0,
			PIN_GRP_SD0_1,
			PIN_GRP_GEM0_1,
			PIN_GRP_USB1_0,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART0_6,
			PIN_GRP_GPIO1_3,
			PIN_GRP_I2C1_5,
			PIN_GRP_SPI0_SS_4,
			PIN_GRP_CAN1_8,
			PIN_GRP_MMI_DP_HPD_2,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_30] = {
		.GroupCount = 12U,
		.GroupList = ((u16 []) {
			PIN_GRP_SMAP_0,
			PIN_GRP_SD0_1,
			PIN_GRP_GEM0_1,
			PIN_GRP_USB1_0,
			PIN_GRP_TRACE_2,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART1_6,
			PIN_GRP_GPIO1_4,
			PIN_GRP_SYSMON_I2C_12,
			PIN_GRP_SPI0_4,
			PIN_GRP_CAN2_5,
			PIN_GRP_PCIE_2,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_31] = {
		.GroupCount = 12U,
		.GroupList = ((u16 []) {
			PIN_GRP_SMAP_0,
			PIN_GRP_SD0_1,
			PIN_GRP_GEM0_1,
			PIN_GRP_USB1_0,
			PIN_GRP_TRACE_2,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART1_6,
			PIN_GRP_GPIO1_5,
			PIN_GRP_SYSMON_I2C_12,
			PIN_GRP_SPI0_4,
			PIN_GRP_CAN2_5,
			PIN_GRP_TTC0_CLK_6,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_32] = {
		.GroupCount = 12U,
		.GroupList = ((u16 []) {
			PIN_GRP_SMAP_0,
			PIN_GRP_SD0_1,
			PIN_GRP_GEM0_1,
			PIN_GRP_USB1_0,
			PIN_GRP_TRACE_CLK_2,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART1_6,
			PIN_GRP_GPIO1_6,
			PIN_GRP_SYSMON_I2C_ALERT_12,
			PIN_GRP_SPI1_4,
			PIN_GRP_CAN3_5,
			PIN_GRP_TTC0_WAV_6,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_33] = {
		.GroupCount = 12U,
		.GroupList = ((u16 []) {
			PIN_GRP_SMAP_0,
			PIN_GRP_SD0_1,
			PIN_GRP_GEM0_1,
			PIN_GRP_USB1_0,
			PIN_GRP_TRACE_2,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART1_6,
			PIN_GRP_GPIO1_7,
			PIN_GRP_MDIO2_2,
			PIN_GRP_SPI1_SS_4,
			PIN_GRP_CAN3_5,
			PIN_GRP_WWDT_3,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_34] = {
		.GroupCount = 12U,
		.GroupList = ((u16 []) {
			PIN_GRP_SMAP_0,
			PIN_GRP_SD0_1,
			PIN_GRP_GEM0_1,
			PIN_GRP_USB1_0,
			PIN_GRP_TRACE_2,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART0_7,
			PIN_GRP_GPIO1_8,
			PIN_GRP_MDIO2_2,
			PIN_GRP_SPI1_SS_4,
			PIN_GRP_CAN0_8,
			PIN_GRP_EXT_TAMPER_TRIG_5,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_35] = {
		.GroupCount = 12U,
		.GroupList = ((u16 []) {
			PIN_GRP_SMAP_0,
			PIN_GRP_SD0_1,
			PIN_GRP_GEM0_1,
			PIN_GRP_USB1_0,
			PIN_GRP_TRACE_2,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART0_7,
			PIN_GRP_GPIO1_9,
			PIN_GRP_SYSMON_I2C_13,
			PIN_GRP_SPI1_SS_4,
			PIN_GRP_CAN0_8,
			PIN_GRP_TTC3_CLK_1,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_36] = {
		.GroupCount = 12U,
		.GroupList = ((u16 []) {
			PIN_GRP_SMAP_0,
			PIN_GRP_SD0_1,
			PIN_GRP_GEM0_1,
			PIN_GRP_USB1_0,
			PIN_GRP_TRACE_2,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART0_7,
			PIN_GRP_GPIO1_10,
			PIN_GRP_SYSMON_I2C_13,
			PIN_GRP_SPI1_4,
			PIN_GRP_CAN1_9,
			PIN_GRP_TTC3_WAV_1,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_37] = {
		.GroupCount = 12U,
		.GroupList = ((u16 []) {
			PIN_GRP_SMAP_0,
			PIN_GRP_SD0_CD_1,
			PIN_GRP_GEM0_1,
			PIN_GRP_USB1_0,
			PIN_GRP_TRACE_2,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART0_7,
			PIN_GRP_GPIO1_11,
			PIN_GRP_SYSMON_I2C_ALERT_13,
			PIN_GRP_SPI1_4,
			PIN_GRP_CAN1_9,
			PIN_GRP_TTC2_CLK_2,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_38] = {
		.GroupCount = 12U,
		.GroupList = ((u16 []) {
			PIN_GRP_SMAP_0,
			PIN_GRP_SD0_WP_1,
			PIN_GRP_GEM1_1,
			PIN_GRP_USB1_0,
			PIN_GRP_TRACE_2,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART1_7,
			PIN_GRP_GPIO1_12,
			PIN_GRP_SPI0_5,
			PIN_GRP_CAN2_6,
			PIN_GRP_TTC2_WAV_2,
			PIN_GRP_PCIE_2,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_39] = {
		.GroupCount = 11U,
		.GroupList = ((u16 []) {
			PIN_GRP_SMAP_0,
			PIN_GRP_GEM1_1,
			PIN_GRP_USB1_0,
			PIN_GRP_TRACE_2,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART1_7,
			PIN_GRP_GPIO1_13,
			PIN_GRP_SYSMON_I2C_14,
			PIN_GRP_SPI0_SS_5,
			PIN_GRP_CAN2_6,
			PIN_GRP_TTC1_CLK_7,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_40] = {
		.GroupCount = 11U,
		.GroupList = ((u16 []) {
			PIN_GRP_SD1_1,
			PIN_GRP_SMAP_0,
			PIN_GRP_GEM1_1,
			PIN_GRP_TRACE_2,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART1_7,
			PIN_GRP_GPIO1_14,
			PIN_GRP_SYSMON_I2C_14,
			PIN_GRP_SPI0_SS_5,
			PIN_GRP_CAN3_6,
			PIN_GRP_TTC1_WAV_7,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_41] = {
		.GroupCount = 11U,
		.GroupList = ((u16 []) {
			PIN_GRP_SD1_1,
			PIN_GRP_SMAP_0,
			PIN_GRP_GEM1_1,
			PIN_GRP_TRACE_2,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART1_7,
			PIN_GRP_GPIO1_15,
			PIN_GRP_SYSMON_I2C_ALERT_14,
			PIN_GRP_SPI0_SS_5,
			PIN_GRP_CAN3_6,
			PIN_GRP_WWDT_4,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_42] = {
		.GroupCount = 10U,
		.GroupList = ((u16 []) {
			PIN_GRP_SD1_1,
			PIN_GRP_SMAP_0,
			PIN_GRP_GEM1_1,
			PIN_GRP_TRACE_2,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART0_8,
			PIN_GRP_GPIO1_16,
			PIN_GRP_I2C0_5,
			PIN_GRP_SPI0_5,
			PIN_GRP_CAN0_9,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_43] = {
		.GroupCount = 11U,
		.GroupList = ((u16 []) {
			PIN_GRP_SD1_1,
			PIN_GRP_SMAP_0,
			PIN_GRP_GEM1_1,
			PIN_GRP_TRACE_2,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART0_8,
			PIN_GRP_GPIO1_17,
			PIN_GRP_MDIO2_3,
			PIN_GRP_I2C0_5,
			PIN_GRP_SPI0_5,
			PIN_GRP_CAN0_9,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_44] = {
		.GroupCount = 11U,
		.GroupList = ((u16 []) {
			PIN_GRP_SD1_1,
			PIN_GRP_SMAP_0,
			PIN_GRP_GEM1_1,
			PIN_GRP_TRACE_2,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART0_8,
			PIN_GRP_GPIO1_18,
			PIN_GRP_MDIO2_3,
			PIN_GRP_I2C1_6,
			PIN_GRP_SPI1_5,
			PIN_GRP_CAN1_10,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_45] = {
		.GroupCount = 10U,
		.GroupList = ((u16 []) {
			PIN_GRP_SD1_1,
			PIN_GRP_SMAP_0,
			PIN_GRP_GEM1_1,
			PIN_GRP_TRACE_2,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART0_8,
			PIN_GRP_GPIO1_19,
			PIN_GRP_I2C1_6,
			PIN_GRP_SPI1_SS_5,
			PIN_GRP_CAN1_10,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_46] = {
		.GroupCount = 11U,
		.GroupList = ((u16 []) {
			PIN_GRP_SD1_1,
			PIN_GRP_SMAP_0,
			PIN_GRP_GEM1_1,
			PIN_GRP_UFS_2,
			PIN_GRP_TRACE_2,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART1_8,
			PIN_GRP_GPIO1_20,
			PIN_GRP_SPI1_SS_5,
			PIN_GRP_CAN2_7,
			PIN_GRP_TTC1_CLK_8,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_47] = {
		.GroupCount = 12U,
		.GroupList = ((u16 []) {
			PIN_GRP_SD1_1,
			PIN_GRP_SMAP_0,
			PIN_GRP_GEM1_1,
			PIN_GRP_UFS_2,
			PIN_GRP_TRACE_2,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART1_8,
			PIN_GRP_GPIO1_21,
			PIN_GRP_SPI1_SS_5,
			PIN_GRP_CAN2_7,
			PIN_GRP_TTC1_WAV_8,
			PIN_GRP_EXT_TAMPER_TRIG_6,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_48] = {
		.GroupCount = 11U,
		.GroupList = ((u16 []) {
			PIN_GRP_SD1_1,
			PIN_GRP_SMAP_0,
			PIN_GRP_GEM1_1,
			PIN_GRP_UFS_2,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART1_8,
			PIN_GRP_GPIO1_22,
			PIN_GRP_SPI1_5,
			PIN_GRP_CAN3_7,
			PIN_GRP_MMI_DP_HPD_3,
			PIN_GRP_TTC0_CLK_7,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_49] = {
		.GroupCount = 11U,
		.GroupList = ((u16 []) {
			PIN_GRP_SD1_1,
			PIN_GRP_SMAP_0,
			PIN_GRP_GEM1_1,
			PIN_GRP_UFS_3,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_UART1_8,
			PIN_GRP_GPIO1_23,
			PIN_GRP_SYSMON_I2C_15,
			PIN_GRP_SPI1_5,
			PIN_GRP_CAN3_7,
			PIN_GRP_TTC0_WAV_7,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_50] = {
		.GroupCount = 9U,
		.GroupList = ((u16 []) {
			PIN_GRP_SD1_1,
			PIN_GRP_SMAP_0,
			PIN_GRP_GEM_TSU_2,
			PIN_GRP_UFS_3,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_GPIO1_24,
			PIN_GRP_SYSMON_I2C_15,
			PIN_GRP_MDIO0_1,
			PIN_GRP_MDIO1_1,
		})
	},
	[XPM_NODEIDX_STMIC_PMIO_51] = {
		.GroupCount = 9U,
		.GroupList = ((u16 []) {
			PIN_GRP_SD1_PC_1,
			PIN_GRP_SMAP_0,
			PIN_GRP_GEM_TSU_3,
			PIN_GRP_UFS_3,
			PIN_GRP_TEST_SCAN_0,
			PIN_GRP_GPIO1_25,
			PIN_GRP_SYSMON_I2C_ALERT_15,
			PIN_GRP_MDIO0_1,
			PIN_GRP_MDIO1_1,
		})
	},
};

/****************************************************************************/
/**
 * @brief  Get requested pin group by node index
 *
 * @param PinIndex     Pin Index.
 *
 * @return Pointer to requested XPm_PinGroup, NULL otherwise
 *
 * @note Requires only node index
 *
 ****************************************************************************/
const XPm_PinGroup *XPmPin_GetGroupByIdx(const u32 PinIndex)
{
	return &PmPinGroups[PinIndex];
}
