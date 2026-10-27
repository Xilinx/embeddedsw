/******************************************************************************
* Copyright (C) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
* @file xdmapcie_common.h
* @addtogroup xdmapcie Overview
* @{
*
* @brief Defines the macros that are shared across the XDmaPcie driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 1.0	tk	01/30/2019	First release
* 3.3		10/24/2026	Added CPM6 (versal_2vp) controller helpers
* </pre>
*
*******************************************************************************/
#ifndef XDMAPCIE_COMMON_H_
#define XDMAPCIE_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "xil_printf.h"
#include "xil_types.h"
#include "xdmapcie_hw.h"
#if defined(versal_2vp)
#include <string.h>
#endif

#define DEBUG_MODE				/**< Enables the XDmaPcie_Dbg() prints */
/******************************** Include Files *******************************/
/****************************** Type Definitions ******************************/
/******************** Macros (Inline Functions) Definitions *******************/

/**
 * @name Bit and mask helpers
 * @{
 */
#define BIT(x) 								(1 << (x))	/**< Builds a mask with bit x set */

/** Derives the highest bus number from the ECAM size, 1 MB per bus */
#define GET_MAX_BUS_NO(ecam_sz) 			(((ecam_sz) / (1024 * 1024)) - 1)

#define BITSPERLONG 						64		/**< Width of the GENMASK() operand */
/** Builds a contiguous mask with bits h down to l set */
#define GENMASK(h, l) (((~0ULL) << (l)) & (~0ULL) >> (BITSPERLONG - 1 - (h)))
/** @} */

/**
 * @name Configuration read error detection
 * @{
 */
#define XDMAPCIE_ERROR_RESPONSE         (~0ULL)	/**< Value returned on a failed config read */
/** Evaluates to TRUE when a config read returned the all ones error response */
#define XDMAPCIE_POSSIBLE_ERROR(val)    ((val) == ((typeof(val)) XDMAPCIE_ERROR_RESPONSE))

/** Evaluates to TRUE when a BAR is unimplemented, that is too small or all ones */
#define XDMAPCIE_IS_BAR_UNIMPLEMENTED(size) (                                      \
    (((size) & ~0xFULL) == 0x00) ||      /* Size is zero or too small */     \
    XDMAPCIE_POSSIBLE_ERROR(size)         /* All 1s, read failure or unimpl */ \
)
/** @} */

/**
 * @name PCIe command register bit definitions
 * @{
 */
#define XDMAPCIE_CFG_CMD_MEM_EN 		0x00000002	/**< Memory access enable */
#define XDMAPCIE_CFG_CMD_BUSM_EN 		0x00000004	/**< Bus master enable */
/** @} */

/**
 * @name PCIe configuration space register offsets, in doublewords
 * @{
 */
#define XDMAPCIE_CFG_ID_REG 	0x0000	/**< Vendor ID and Device ID */
#define XDMAPCIE_CFG_CMD_STATUS_REG 	0x0001	/**< Command and Status */
#define XDMAPCIE_CFG_PRI_SEC_BUS_REG 	0x0006	/**< Primary and Secondary Bus */
#define XDMAPCIE_CFG_CAH_LAT_HD_REG 	0x0003	/**< Cache Line, Latency Timer, Header Type and BIST */
#define XDMAPCIE_CFG_BAR_MEM_TYPE_MASK 	0x1	/**< Memory or IO request */
#define XDMAPCIE_CFG_BAR_MEM_AS_MASK	0x8	/**< 32 bit or 64 bit address space */
#define XDMAPCIE_CFG_BAR_BASE_OFFSET 	0x0004	/**< First Base Address Register */
#define XDMAPCIE_CFG_BAR_0_REG 			0x0004	/**< Base Address Register 0 */
#define XDMAPCIE_CFG_BAR_1_REG 			0x0005	/**< Base Address Register 1 */
#define XDMAPCIE_CFG_BAR_2_REG 			0x0006	/**< Base Address Register 2 */
#define XDMAPCIE_CFG_BAR_3_REG 			0x0007	/**< Base Address Register 3 */
#define XDMAPCIE_CFG_BAR_4_REG 			0x0008	/**< Base Address Register 4 */
/** @} */

/**
 * @name Type 1 configuration header register offsets
 * @{
 */
#define XDMAPCIE_CFG_BUS_NUMS_T1_REG 	0X0006	/**< Bus number register */
#define XDMAPCIE_CFG_NP_MEM_T1_REG 		0X0008	/**< Non prefetchable memory base and limit */
#define XDMAPCIE_CFG_P_MEM_T1_REG 		0X0009	/**< Prefetchable memory base and limit */
#define XDMAPCIE_CFG_P_UPPER_MEM_T1_REG 0X000A	/**< Prefetchable memory base, upper 32 bits */
#define XDMAPCIE_CFG_P_LIMIT_MEM_T1_REG 0X000B	/**< Prefetchable memory limit, upper 32 bits */
/** @} */

/**
 * @name Configuration header field masks
 * @{
 */
#define XDMAPCIE_CFG_FUN_NOT_IMP_MASK 	0xFFFF	/**< Marks a function that is not implemented */
#define XDMAPCIE_CFG_HEADER_TYPE_MASK 	0x00010000	/**< Bridge or endpoint header type */
#define XDMAPCIE_CFG_MUL_FUN_DEV_MASK 	0x00800000	/**< Multi function device */
/** @} */

/**
 * @name Enumeration limits
 * @{
 */
#define XDMAPCIE_CFG_MAX_NUM_OF_BUS 	256	/**< Maximum number of buses */
#define XDMAPCIE_CFG_MAX_NUM_OF_DEV 	32	/**< Maximum number of devices per bus */
#define XDMAPCIE_CFG_MAX_NUM_OF_FUN 	8	/**< Maximum number of functions per device */
/** @} */

#define XDMAPCIE_CFG_HEADER_O_TYPE 		0x0000	/**< Endpoint header type */

/**
 * @name BAR address and type encodings
 * @{
 */
#define XDMAPCIE_BAR_IO_MEM 				1	/**< BAR maps IO space */
#define XDMAPCIE_BAR_ADDR_MEM 			0	/**< BAR maps memory space */
#define XDMAPCIE_BAR_MEM_TYPE_64 		0x4	/**< 64 bit BAR */
#define XDMAPCIE_BAR_MEM_TYPE_32 		0x0	/**< 32 bit BAR */
/** @} */

#define XDMAPCIE_PRIMARY_BUS   			0x18	/**< Primary bus number register offset */

#define XDMAPCIE_ECAM_MEMSIZE			16*1024*1024	/**< Default ECAM region size */

/**
 * @name Shift and nibble width helpers
 * @{
 */
#define MB_SHIFT 					20	/**< Shift for one megabyte */
#define HEX_NIBBLE 					4	/**< Bits in one hex nibble */
#define TWO_HEX_NIBBLES 			8	/**< Bits in two hex nibbles */
#define FOUR_HEX_NIBBLES 			16	/**< Bits in four hex nibbles */
#define EIGHT_HEX_NIBBLES 			32	/**< Bits in eight hex nibbles */
/** @} */

#define XDMAPCIE_LINKUP_SUCCESS				1	/**< The PCIe link is up */
#define XDMAPCIE_LINKUP_FAIL				0	/**< The PCIe link is down */

#define DATA_MASK_32				(0xFFFFFFFF)	/**< Mask for the lower 32 bits */

/**
 * @name Capability pointer register and masks
 * @{
 */
#define XDMAPCIE_CFG_P_CAP_PTR_T1_REG 	0X000D	/**< Capability pointer register */
#define XDMAPCIE_CFG_CAP_ID_LOC		 	GENMASK(7, 0)	/**< Capability ID field */
#define XDMAPCIE_CAP_PTR_LOC			GENMASK(7, 0)	/**< Next capability pointer field */
#define XDMAPCIE_CAP_SHIFT				8	/**< Shift to reach the next capability pointer */
#define XDMAPCIE_DOUBLEWORD(x)			(x / 4)	/**< Converts a byte offset to a doubleword offset */
#define CAP_PRESENT						(1)	/**< The capability is present */
#define CAP_NOT_PRESENT					(0)	/**< The capability is not present */
/** @} */

/**
 * @name Logging macros
 * @{
 */
#define XDmaPcie_Print(MSG, ...)	xil_printf("xdma_pcie: "MSG,##__VA_ARGS__)	/**< Prints an informational message */
#define XDmaPcie_Error(MSG, ...)	xil_printf("xdma_pcie: "MSG,##__VA_ARGS__)	/**< Prints an error message */

/** Prints an error message unconditionally */
#define XDmaPcie_Err(MSG, ...) \
		do { \
			XDmaPcie_Error(MSG, ##__VA_ARGS__); \
		} while (0)

#ifdef DEBUG_MODE
/** Prints a debug message when DEBUG_MODE is defined */
#define XDmaPcie_Dbg(MSG, ...) \
		do { \
			XDmaPcie_Print(MSG, ##__VA_ARGS__); \
		} while (0)
#else
	#define XDmaPcie_Dbg(MSG, ...)	{}
#endif
/** @} */

/***************************** Function Prototypes ****************************/

#if defined(versal_2vp)
/*****************************************************************************/
/**
* @brief Returns the CPM6 controller index named by a compatible string.
*
* The string is normally XDmaPcie_Config.Name, taken from the device tree.
*
* @param[in] Compatible Device tree compatible string.
*
* @return
*		- XDMAPCIE_CPM6_CONTROLLER_1 for the host1 compatible string.
*		- XDMAPCIE_CPM6_CONTROLLER_0 otherwise.
*
******************************************************************************/
static inline u32 XDmaPcie_Cpm6GetController(const char *Compatible)
{
	u32 Controller = XDMAPCIE_CPM6_CONTROLLER_0;

	if ((Compatible != NULL) &&
	    (strcmp(Compatible, XDMAPCIE_CPM6_HOST1_COMPATIBLE) == 0)) {
		Controller = XDMAPCIE_CPM6_CONTROLLER_1;
	}

	return Controller;
}

/*****************************************************************************/
/**
* @brief Returns the CPM6 interrupt register block base of a controller.
*
* The controller is selected by the compatible string. The device tree only
* supplies the CSR/SLCR (csr-slcr) base address, and the interrupt register
* block sits at a fixed offset from it.
*
* @param[in] Compatible Device tree compatible string.
* @param[in] CsrSlcrBase Base address of the csr-slcr block from the device
*            tree.
*
* @return Base address of the interrupt register block of the controller.
*
******************************************************************************/
static inline UINTPTR XDmaPcie_Cpm6CoreBase(const char *Compatible,
					    UINTPTR CsrSlcrBase)
{
	UINTPTR Offset = XDMAPCIE_CPM6_CORE0_INTR_OFFSET;

	if (XDmaPcie_Cpm6GetController(Compatible) ==
	    XDMAPCIE_CPM6_CONTROLLER_1) {
		Offset = XDMAPCIE_CPM6_CORE1_INTR_OFFSET;
	}

	return (CsrSlcrBase + Offset);
}

/*****************************************************************************/
/**
* @brief Selects the PERST# pin of the CPM6 controller named by the compatible
*        string.
*
* @param[in] Compatible Device tree compatible string.
* @param[in] Ctrl0Pin Pin wired to the controller 0 PERST#.
* @param[in] Ctrl1Pin Pin wired to the controller 1 PERST#.
*
* @return Ctrl1Pin for the host1 compatible string, Ctrl0Pin otherwise.
*
******************************************************************************/
static inline u32 XDmaPcie_Cpm6GetPerstPin(const char *Compatible,
					   u32 Ctrl0Pin, u32 Ctrl1Pin)
{
	u32 Pin = Ctrl0Pin;

	if (XDmaPcie_Cpm6GetController(Compatible) ==
	    XDMAPCIE_CPM6_CONTROLLER_1) {
		Pin = Ctrl1Pin;
	}

	return Pin;
}
#endif /* versal_2vp */

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XDMAPCIE_COMMON_H_ */
