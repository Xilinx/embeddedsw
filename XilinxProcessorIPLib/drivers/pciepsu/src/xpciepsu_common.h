/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
* @file xpciepsu_common.h
*
* Definitions of commonly used macros.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 1.0	bs	08/21/2018	First release
* </pre>
*
*******************************************************************************/
#ifndef XPCIEPSU_COMMON_H_
#define XPCIEPSU_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "xil_printf.h"

#define DEBUG_MODE
/******************************** Include Files *******************************/
/****************************** Type Definitions ******************************/
/******************** Macros (Inline Functions) Definitions *******************/
/* Command register offsets */

/* Memory access enable */
#define XPCIEPSU_CFG_CMD_MEM_EN		0x00000002U

/* Bus master enable */
#define XPCIEPSU_CFG_CMD_BUSM_EN	0x00000004U

/* I/O access enable */
#define XPCIEPSU_CFG_CMD_IO_EN		0x00000001U

/* Parity errors response */
#define XPCIEPSU_CFG_CMD_PARITY_EN	0x00000040U

/* SERR report enable */
#define XPCIEPSU_CFG_CMD_SERR_EN	0x00000100U

/* PCIe Configuration registers offsets */

/* Vendor ID/Device ID offset */
#define XPCIEPSU_CFG_ID_REG		0x0000U

/* Command/Status Register Offset */
#define XPCIEPSU_CFG_CMD_STATUS_REG	0x0001U

/* Cache Line/Latency Timer / Header Type / BIST Register Offset */
#define XPCIEPSU_CFG_CAH_LAT_HD_REG	0x0003U

#define XPCIEPSU_CFG_BAR_MEM_TYPE_MASK	0x1U	/* Memory or IO request */

#define XPCIEPSU_CFG_BAR_MEM_AS_MASK	0x8U	/* 32b or 64b address space */

/* PCIe Base Addr */
#define XPCIEPSU_CFG_BAR_BASE_OFFSET	0x0004U

#define XPCIEPSU_CFG_BUS_NUMS_T1_REG	0x0006U
#define XPCIEPSU_CFG_NP_MEM_T1_REG 	0x0008U
#define XPCIEPSU_CFG_P_MEM_T1_REG 	0x0009U
#define XPCIEPSU_CFG_P_UPPER_MEM_T1_REG 0x000AU
#define XPCIEPSU_CFG_P_LIMIT_MEM_T1_REG 0x000BU

#define XPCIEPSU_CFG_FUN_NOT_IMP_MASK 	0xFFFFU
#define XPCIEPSU_CFG_HEADER_TYPE_MASK 	0x00010000U
#define XPCIEPSU_CFG_MUL_FUN_DEV_MASK 	0x00800000U

#define XPCIEPSU_CFG_MAX_NUM_OF_DEV 	32U
#define XPCIEPSU_CFG_MAX_NUM_OF_FUN 	8U

#define XPCIEPSU_CFG_HEADER_O_TYPE	0x0000U

#define XPCIEPSU_BAR_MEM_TYPE_IO	0x1U
#define XPCIEPSU_BAR_MEM_TYPE_64 	0x4U
#define XPCIEPSU_BAR_MEM_TYPE_32 	0x0U

#define XPCIEPSU_PRIMARY_BUS   		0x18U

#define XPCIEPSU_ECAM_MEMSIZE		16U * 1024U * 1024U

#define MB_SHIFT 			20U
#define TWO_HEX_NIBBLES 		8U
#define FOUR_HEX_NIBBLES		16U
#define EIGHT_HEX_NIBBLES 		32U

#define XPCIEPSU_LINKUP_SUCCESS		1U
#define XPCIEPSU_LINKUP_FAIL		0U

#define DATA_MASK_32			(0xFFFFFFFFU)

/* Capability pointer Doubleword and masks */
#define XPCIEPSU_CFG_P_CAP_PTR_T1_REG 	0X000DU
#define XPCIEPSU_CFG_CAP_ID_LOC		GENMASK(7U, 0U)
#define XPCIEPSU_CAP_PTR_LOC		GENMASK(7U, 0U)
#define XPCIEPSU_CAP_SHIFT		8U
#define CAP_PRESENT			(1U)
#define CAP_NOT_PRESENT			(0U)

/* PCIe mode */
#define XPCIEPSU_MODE_ENDPOINT		0x0U

/* Conditional debugging prints */
#define XPciePsu_Err(...) \
		do { \
			xil_printf("pcie_psu: "); \
			xil_printf(__VA_ARGS__);  \
		} while (0U != FALSE)

#ifdef DEBUG_MODE
#define XPciePsu_Dbg(...) \
		do { \
			xil_printf("pcie_psu: "); \
			xil_printf(__VA_ARGS__);  \
		} while (0U != FALSE)
#else
	#define XPciePsu_Dbg(...)	{}
#endif

/***************************** Function Prototypes ****************************/

#ifdef __cplusplus
}
#endif

#endif /* XPCIEPSU_COMMON_H_ */
