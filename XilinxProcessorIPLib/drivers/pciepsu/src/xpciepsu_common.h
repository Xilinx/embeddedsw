/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
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
#define XPCIEPSU_CFG_CMD_MEM_EN 		0x00000002

/* Bus master enable */
#define XPCIEPSU_CFG_CMD_BUSM_EN 		0x00000004

/* PCIe Configuration registers offsets */

/* Vendor ID/Device ID offset */
#define XPCIEPSU_CFG_ID_REG 	0x0000

/* Command/Status Register Offset */
#define XPCIEPSU_CFG_CMD_STATUS_REG 	0x0001

/* Primary/Sec.Bus Register Offset */
#define XPCIEPSU_CFG_PRI_SEC_BUS_REG 	0x0006

/* Cache Line/Latency Timer / Header Type / BIST Register Offset */
#define XPCIEPSU_CFG_CAH_LAT_HD_REG 	0x0003

#define XPCIEPSU_CFG_BAR_MEM_TYPE_MASK 	0x1

/* PCIe Base Addr */
#define XPCIEPSU_CFG_BAR_BASE_OFFSET 	0x0004

/* PCIe Base Addr 0 */
#define XPCIEPSU_CFG_BAR_0_REG 			0x0004

/* PCIe Base Addr 1 */
#define XPCIEPSU_CFG_BAR_1_REG 			0x0005

/* PCIe Base Addr 2 */
#define XPCIEPSU_CFG_BAR_2_REG 			0x0006

/* PCIe Base Addr 3 */
#define XPCIEPSU_CFG_BAR_3_REG 			0x0007

/* PCIe Base Addr 4 */
#define XPCIEPSU_CFG_BAR_4_REG 			0x0008

#define XPCIEPSU_CFG_BUS_NUMS_T1_REG 	0X0006
#define XPCIEPSU_CFG_NP_MEM_T1_REG 		0X0008
#define XPCIEPSU_CFG_P_MEM_T1_REG 		0X0009
#define XPCIEPSU_CFG_P_UPPER_MEM_T1_REG 0X000A
#define XPCIEPSU_CFG_P_LIMIT_MEM_T1_REG 0X000B

#define XPCIEPSU_CFG_FUN_NOT_IMP_MASK 	0xFFFF
#define XPCIEPSU_CFG_HEADER_TYPE_MASK 	0x00EF0000
#define XPCIEPSU_CFG_MUL_FUN_DEV_MASK 	0x00800000

#define XPCIEPSU_CFG_MAX_NUM_OF_BUS 	256
#define XPCIEPSU_CFG_MAX_NUM_OF_DEV 	32
#define XPCIEPSU_CFG_MAX_NUM_OF_FUN 	8

#define XPCIEPSU_CFG_HEADER_O_TYPE 		0x0000

#define XPCIEPSU_BAR_IO_MEM 				1
#define XPCIEPSU_BAR_ADDR_MEM 			0

#define XPCIEPSU_BAR_MEM_TYPE_64 		1
#define XPCIEPSU_BAR_MEM_TYPE_32 		0
#define XPCIEPSU_PRIMARY_BUS   			0x18

#define MB_SHIFT 					20
#define HEX_NIBBLE 					4
#define TWO_HEX_NIBBLES 			8
#define FOUR_HEX_NIBBLES 			16
#define EIGHT_HEX_NIBBLES 			32

#define XPCIEPSU_LINKUP_SUCCESS				1
#define XPCIEPSU_LINKUP_FAIL				0

#define DATA_MASK_32				(0xFFFFFFFF)

/* Capability pointer Doubleword and masks */
#define XPCIEPSU_CFG_P_CAP_PTR_T1_REG 	0X000D
#define XPCIEPSU_CFG_CAP_ID_LOC		 	GENMASK(7, 0)
#define XPCIEPSU_CAP_PTR_LOC			GENMASK(7, 0)
#define XPCIEPSU_CAP_SHIFT				8
#define XPCIEPSU_DOUBLEWORD(x)			(x / 4)
#define CAP_PRESENT						(1)
#define CAP_NOT_PRESENT					(0)

/* PCIe mode */
#define XPCIEPSU_MODE_ENDPOINT		0X0
#define XPCIEPSU_MODE_ROOTCOMPLEX	0X1

/* Conditional debugging prints */
#define XPciePsu_Err(...) \
		do { \
			xil_printf("pcie_psu: "); \
			xil_printf(__VA_ARGS__);  \
		} while (0)

#ifdef DEBUG_MODE
#define XPciePsu_Dbg(...) \
		do { \
			xil_printf("pcie_psu: "); \
			xil_printf(__VA_ARGS__);  \
		} while (0)
#else
	#define XPciePsu_Dbg(...)	{}
#endif

/***************************** Function Prototypes ****************************/

#ifdef __cplusplus
}
#endif

#endif /* XPCIEPSU_COMMON_H_ */
