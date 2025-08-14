/******************************************************************************
* Copyright (C) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
* @file xdmapcie_common.h
*
* Definitions of commonly used macros.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 1.0	tk	01/30/2019	First release
* </pre>
*
*******************************************************************************/
#ifndef XDMAPCIE_COMMON_H_
#define XDMAPCIE_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "xil_printf.h"

#define DEBUG_MODE
/******************************** Include Files *******************************/
/****************************** Type Definitions ******************************/
/******************** Macros (Inline Functions) Definitions *******************/

#define BIT(x) 								(1 << (x))

/* each bus required 1 MB ecam space */
#define GET_MAX_BUS_NO(ecam_sz) 			(((ecam_sz) / (1024 * 1024)) - 1)

#define BITSPERLONG 						64
#define GENMASK(h, l) (((~0ULL) << (l)) & (~0ULL) >> (BITSPERLONG - 1 - (h)))

#define XDMAPCIE_ERROR_RESPONSE         (~0ULL)
#define XDMAPCIE_POSSIBLE_ERROR(val)    ((val) == ((typeof(val)) XDMAPCIE_ERROR_RESPONSE))

#define XDMAPCIE_IS_BAR_UNIMPLEMENTED(size) (                                      \
    (((size) & ~0xFULL) == 0x00) ||      /* Size is zero or too small */     \
    XDMAPCIE_POSSIBLE_ERROR(size)         /* All 1s — read failure or unimpl */ \
)

/* Command register offsets */

/* Memory access enable */
#define XDMAPCIE_CFG_CMD_MEM_EN 		0x00000002

/* Bus master enable */
#define XDMAPCIE_CFG_CMD_BUSM_EN 		0x00000004

/* PCIe Configuration registers offsets */

/* Vendor ID/Device ID offset */
#define XDMAPCIE_CFG_ID_REG 	0x0000

/* Command/Status Register Offset */
#define XDMAPCIE_CFG_CMD_STATUS_REG 	0x0001

/* Primary/Sec.Bus Register Offset */
#define XDMAPCIE_CFG_PRI_SEC_BUS_REG 	0x0006

/* Cache Line/Latency Timer / Header Type / BIST Register Offset */
#define XDMAPCIE_CFG_CAH_LAT_HD_REG 	0x0003

#define XDMAPCIE_CFG_BAR_MEM_TYPE_MASK 	0x1	/* Memory or IO request */

#define XDMAPCIE_CFG_BAR_MEM_AS_MASK	0x8	/* 32b or 64b address space */

/* PCIe Base Addr */
#define XDMAPCIE_CFG_BAR_BASE_OFFSET 	0x0004

/* PCIe Base Addr 0 */
#define XDMAPCIE_CFG_BAR_0_REG 			0x0004

/* PCIe Base Addr 1 */
#define XDMAPCIE_CFG_BAR_1_REG 			0x0005

/* PCIe Base Addr 2 */
#define XDMAPCIE_CFG_BAR_2_REG 			0x0006

/* PCIe Base Addr 3 */
#define XDMAPCIE_CFG_BAR_3_REG 			0x0007

/* PCIe Base Addr 4 */
#define XDMAPCIE_CFG_BAR_4_REG 			0x0008

#define XDMAPCIE_CFG_BUS_NUMS_T1_REG 	0X0006
#define XDMAPCIE_CFG_NP_MEM_T1_REG 		0X0008
#define XDMAPCIE_CFG_P_MEM_T1_REG 		0X0009
#define XDMAPCIE_CFG_P_UPPER_MEM_T1_REG 0X000A
#define XDMAPCIE_CFG_P_LIMIT_MEM_T1_REG 0X000B

#define XDMAPCIE_CFG_FUN_NOT_IMP_MASK 	0xFFFF
#define XDMAPCIE_CFG_HEADER_TYPE_MASK 	0x00010000
#define XDMAPCIE_CFG_MUL_FUN_DEV_MASK 	0x00800000

#define XDMAPCIE_CFG_MAX_NUM_OF_BUS 	256
#define XDMAPCIE_CFG_MAX_NUM_OF_DEV 	32
#define XDMAPCIE_CFG_MAX_NUM_OF_FUN 	8

#define XDMAPCIE_CFG_HEADER_O_TYPE 		0x0000

#define XDMAPCIE_BAR_IO_MEM 				1
#define XDMAPCIE_BAR_ADDR_MEM 			0

#define XDMAPCIE_BAR_MEM_TYPE_64 		0x4
#define XDMAPCIE_BAR_MEM_TYPE_32 		0x0

#define XDMAPCIE_PRIMARY_BUS   			0x18

#define XDMAPCIE_ECAM_MEMSIZE			16*1024*1024

#define MB_SHIFT 					20
#define HEX_NIBBLE 					4
#define TWO_HEX_NIBBLES 			8
#define FOUR_HEX_NIBBLES 			16
#define EIGHT_HEX_NIBBLES 			32

#define XDMAPCIE_LINKUP_SUCCESS				1
#define XDMAPCIE_LINKUP_FAIL				0

#define DATA_MASK_32				(0xFFFFFFFF)

/* Capability pointer Doubleword and masks */
#define XDMAPCIE_CFG_P_CAP_PTR_T1_REG 	0X000D
#define XDMAPCIE_CFG_CAP_ID_LOC		 	GENMASK(7, 0)
#define XDMAPCIE_CAP_PTR_LOC			GENMASK(7, 0)
#define XDMAPCIE_CAP_SHIFT				8
#define XDMAPCIE_DOUBLEWORD(x)			(x / 4)
#define CAP_PRESENT						(1)
#define CAP_NOT_PRESENT					(0)

/* Print log macros */
#define XDmaPcie_Print(MSG, ...)	xil_printf("xdma_pcie: "MSG,##__VA_ARGS__)
#define XDmaPcie_Error(MSG, ...)	xil_printf("xdma_pcie: "MSG,##__VA_ARGS__)

/* Conditional debugging prints */
#define XDmaPcie_Err(MSG, ...) \
		do { \
			XDmaPcie_Error(MSG, ##__VA_ARGS__); \
		} while (0)

#ifdef DEBUG_MODE
#define XDmaPcie_Dbg(MSG, ...) \
		do { \
			XDmaPcie_Print(MSG, ##__VA_ARGS__); \
		} while (0)
#else
	#define XDmaPcie_Dbg(MSG, ...)	{}
#endif

/***************************** Function Prototypes ****************************/

#ifdef __cplusplus
}
#endif

#endif /* XDMAPCIE_COMMON_H_ */
