/******************************************************************************
* Copyright (C) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XCLOCK_H /* prevent circular inclusions */
#define XCLOCK_H

/****************************************************************************/
/**
*
* @file xclockps.h
* @addtogroup clockps Overview
* @{
* @brief Defines the CLOCKPS driver software API.
*
* The Xilinx Clock controller driver provides APIs to control clock parameters.
* Below are the feature of the driver:
* - Manages database for mux, plls, gates, dividers and fixed factors for all
*   the supported clocks.
* - Allows to enable/disable specific clock.
* - Allows to change the parent for specific clock.
* - Allows to get/set rate for specific clock.
*
* This driver is intended to be RTOS and processor independent. It works with
* physical addresses only.  Any needs for dynamic memory management, threads
* or thread mutual exclusion, virtual memory, or cache control must be
* satisfied by the layer above this driver.
*
* <pre>
* MODIFICATION HISTORY:
* Ver   Who    Date     Changes
* ----- ------ -------- ---------------------------------------------
* 1.00  cjp    02/09/18 First release
* 1.00  sd     07/26/18 Fix Doxygen warnings
* 1.1   aru    03/20/19 Fix IAR issue by changing "XCLOCK_ABS_DIFF" to a
*                       function named "XClock_Absolute_Difference".
* 1.2   sd     02/13/20 Rename ARRAY_SIZE
* 1.4   sd     12/02/21 Fix compilation warnings reported with "-Wundef" flag.
* 1.5   sd     07/10/23 Added SDT support
* 1.6   sd     11/03/23 Fix the redefinition of the PSS_REF frequency
* 1.7   vlt    12/30/25 Update Doxygen comments to include SDT flow details.
* 1.7   vlt    04/14/26 Updated LookupConfig BaseAddress type from u32 to UINTPTR
*                       to support 64-bit addressing
* 1.8   tb     09/17/26 Align Doxygen comments with AMD standards.
* </pre>
*
******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xstatus.h"
#include "xil_types.h"
#include "xil_io.h"
#include "xclockps_hw.h"
#if defined (__aarch64__)
#include "xil_smc.h"
#endif

/************************** Constant Definitions *****************************/
/*
 * Device ID and Num Instances defines for resetps. Resetps uses common
 * hardware with other driver and hence this wrapper defines are required
 */
#ifndef SDT
/** Number of CLOCKPS instances. */
#define XPAR_XCLOCKPS_NUM_INSTANCES  (XPAR_XCRPSU_NUM_INSTANCES)
/** CLOCKPS device identifier. */
#define XPAR_XCLOCKPS_DEVICE_ID      (XPAR_XCRPSU_0_DEVICE_ID)
#endif

/* Fixed clock rate definitions */
/** Fixed video clock rate in Hz. */
#define XCLOCK_FIX_RATE_VIDEO_CLK    (27000000U)
#define XCLOCK_FIX_RATE_PSS_ALT_REF_CLK \
	(0U)	/**< Fixed PSS alternate reference clock rate in Hz. */
#define XCLOCK_FIX_RATE_GT_CRX_REF_CLK \
	(108000000U)	/**< Fixed GT CRX reference clock rate in Hz. */
/** Fixed auxiliary reference clock rate in Hz. */
#define XCLOCK_FIX_RATE_AUX_REF_CLK  (27000000U)
/** Fixed DisplayPort ACLK rate in Hz. */
#define XCLOCK_FIX_RATE_DP_ACLK      (100000000U)

/* Miscellaneous defines */
/** Invalid parent identifier. */
#define XCLOCK_INVALID_PARENT        (0xFFFF)
/** Parent-type field mask. */
#define XCLOCK_PARENT_TYPE_MASK      (0xFF00)
/** Parent-index field mask. */
#define XCLOCK_PARENT_IDX_MASK       (0x00FF)
/** Invalid clock rate value. */
#define XCLOCK_INVALID_RATE          (0)
/** Divider field width in bits. */
#define XCLOCK_DIVIDERS_BITWIDTH     (6)
/** Maximum divider value. */
#define XCLOCK_MAX_DIV_VAL           XCLOCK_VALUE_MASK(XCLOCK_DIVIDERS_BITWIDTH)
/** Invalid divider database index. */
#define XCLOCK_INVALID_DIV_INDEX     (MAX_DIVIDER + 1)
/** Invalid PLL database index. */
#define XCLOCK_INVALID_PLL_INDEX     (MAX_PLL + 1)

/* Clock Flags */
/** Clock is critical and is not disabled. */
#define XCLOCK_CLOCK_IS_CRITICAL     BIT(0)

/**
 * @brief Returns a bit mask of the given length.
 * @param[in] masklen Width of the mask in bits.
 *
 * @note
 * C-style signature
 *	u32 XCLOCK_VALUE_MASK(u32 masklen)
 */
#define XCLOCK_VALUE_MASK(masklen)   ((1 << masklen) - 1)

/**
 * @brief Returns a mask for bit n.
 * @param[in] n Bit index.
 *
 * @note
 * C-style signature
 *	u32 BIT(u32 n)
 */
#define BIT(n)		             (1 << (n))

/**
 * @brief Returns the number of elements in an array.
 * @param[in] x Array identifier.
 *
 * @note
 * C-style signature
 *	size_t CLK_ARRAY_SIZE(ElementType x[])
 */
#define CLK_ARRAY_SIZE(x)	             (sizeof(x) / sizeof((x)[0]))

/**
 * @brief Divides two values and rounds to the nearest integer.
 * @param[in] a Dividend.
 * @param[in] b Divisor.
 *
 * @note
 * C-style signature
 *	XClockRate XCLOCK_ROUND_DIV(XClockRate a, XClockRate b)
 */
#define XCLOCK_ROUND_DIV(a, b)       ((a + (b / 2)) / b)

/**
 * @brief Divides two values and rounds up.
 * @param[in] a Dividend.
 * @param[in] b Divisor.
 *
 * @note
 * C-style signature
 *	XClockRate XCLOCK_CEIL_DIV(XClockRate a, XClockRate b)
 */
#define XCLOCK_CEIL_DIV(a, b)        ((a + b - 1) / b)

/**
 * Gives the max value for unsigned type
 * Note:
 * Not to be used with signed types
 */
#define XCLOCK_MAX_VALUE_UNSIGNED_TYPE(type) \
	(type)(~0)

/**
 * Parent ID for node parents.
 * Parent ID is combination of parent type and node indices. Most significant
 * 8 bits represent type of parent and least significant 8 bits represents
 * the node index in nodes database
 */
#define XCLOCK_GENERATE_PARENT_ID(type, idx) \
	((u16)((u16)(type << 8) | (u8)(idx)))

/**
 * @brief Parses parent to fetch out parent type.
 *
 * @note
 * C-style signature
 *	u8 XCLOCK_FETCH_PARENT_TYPE(u16 parent)
 */
#define XCLOCK_FETCH_PARENT_TYPE(parent) \
	((parent & XCLOCK_PARENT_TYPE_MASK) >> 8)

/**
 * @brief Parses parent to fetch out parent database index.
 *
 * @note
 * C-style signature
 *	u8 XCLOCK_FETCH_PARENT_INDEX(u16 parent)
 */
#define XCLOCK_FETCH_PARENT_INDEX(parent) \
	(parent & XCLOCK_PARENT_IDX_MASK)

/**
 * @brief Limits the value within min and max.
 *
 * @note
 * C-style signature
 *	void XCLOCK_LIMIT_VALUE(u32 val, u32 min, u32 max)
 */
#define XCLOCK_LIMIT_VALUE(val, min, max) \
	do { \
		if (val < min) { \
			val = min; \
		} \
		if (val > max) { \
			val = max; \
		} \
	} while (0)

/**
 * This macro checks if index is valid.
 * type corresponds to node type and can have either of below:
 * - IP
 * - OP
 * - PLL
 * - MUX
 * - GATE
 * - DIVIDER
 * - FIXEDFACTOR
 *
 * The macro assumes the index to have unsigned data type and ignores lower
 * bound checks.
 */
#define XCLOCK_VALIDATE_INDEX(type, index) \
	do { \
		if (MAX_##type < index) { \
			return XST_INVALID_PARAM; \
		} \
	} while (0)

/**
 * This macro checks for valid index and returns no failure status.
 * The macro assumes the index to have unsigned data type and ignores lower
 * bound checks.
 */
#define XCLOCK_VALIDATE_INDEX_WARN(type, index) \
	do { \
		if (MAX_##type < index) { \
			xil_printf("Warning: Index %d out of bound for %s\n", \
				   index, #type); \
			return; \
		} \
	} while (0)

/**
 * @brief Validates pointer to not be null.
 *
 * @note
 * C-style signature
 *	void XCLOCK_VALIDATE_PTR(void *ptr)
 */
#define XCLOCK_VALIDATE_PTR(ptr) \
	do { \
		if (NULL == ptr) { \
			return XST_INVALID_PARAM; \
		} \
	} while (0)

/**************************** Type Definitions *******************************/
/**
 * @brief Holds a CLOCKPS rate value.
 *
 * Type defined for Rate.
 */
#if defined (__aarch64__)
typedef u64 XClockRate;
#else
typedef u32 XClockRate;
#endif

/**
 * @brief Holds CLOCKPS device configuration data.
 */
typedef struct {
#ifndef SDT
	u16 DeviceId;                    /**< Unique ID of device. */
#else
	char *Name;                       /**< Name of the device. */
	/** Base address of the device. */
	UINTPTR BaseAddress;
#endif
} XClockPs_Config;

/**
 * @brief Holds instance data for a CLOCKPS device.
 *
 * Allocate a variable of this type for every clock controller in the
 * system. Pass a pointer to that variable to the driver API
 * functions.
 */
typedef struct {
	XClockPs_Config Config;          /**< Hardware Configuration. */
} XClock;

/**
 * @brief Lists CLOCKPS node types.
 */
typedef enum {
	XCLOCK_TYPE_IP, /**< Fixed input-clock node type. */
	XCLOCK_TYPE_PLL, /**< PLL node type. */
	XCLOCK_TYPE_MUX, /**< Mux node type. */
	XCLOCK_TYPE_GATE, /**< Gate node type. */
	XCLOCK_TYPE_DIVIDER, /**< Divider node type. */
	XCLOCK_TYPE_FIXEDFACTOR, /**< Fixed-factor node type. */
	XCLOCK_TYPE_MAX /**< Sentinel for clock node types. */
} XClock_Types;

/**
 * @brief Lists CLOCKPS output clocks.
 *
 * Each enumerator identifies the output clock with the same name. MIN_OP
 * and MAX_OP mark the first and last identifiers.
 */
typedef enum  {
	MIN_OP,
	IOPLL = MIN_OP, RPLL, APLL, DPLL, VPLL,
	IOPLL_TO_FPD, RPLL_TO_FPD, APLL_TO_LPD, DPLL_TO_LPD, VPLL_TO_LPD,
	ACPU, ACPU_HALF,
	DBG_FPD, DBG_LPD, DBG_TRACE, DBG_TSTMP,
	DP_VIDEO_REF, DP_AUDIO_REF, DP_STC_REF,
	GDMA_REF, DPDMA_REF,
	DDR_REF,
	SATA_REF,
	PCIE_REF,
	GPU_REF, GPU_PP0_REF, GPU_PP1_REF,
	TOPSW_MAIN, TOPSW_LSBUS,
	GTGREF0_REF,
	LPD_SWITCH, LPD_LSBUS,
	USB0_BUS_REF, USB1_BUS_REF, USB3_DUAL_REF, USB0, USB1,
	CPU_R5, CPU_R5_CORE,
	CSU_SPB, CSU_PLL,
	PCAP,
	IOU_SWITCH,
	GEM_TSU_REF, GEM_TSU,
	GEM0_REF, GEM1_REF, GEM2_REF, GEM3_REF,
	GEM0_RX, GEM1_RX, GEM2_RX, GEM3_RX,
	QSPI_REF,
	SDIO0_REF, SDIO1_REF,
	UART0_REF, UART1_REF,
	SPI0_REF, SPI1_REF,
	NAND_REF,
	I2C0_REF, I2C1_REF,
	CAN0_REF, CAN1_REF, CAN0, CAN1,
	DLL_REF,
	ADMA_REF,
	TIMESTAMP_REF,
	AMS_REF,
	PL0, PL1, PL2, PL3,
	WDT,
	MAX_OP = WDT,
} XClock_OutputClks;

/**
 * @brief Lists CLOCKPS input clocks.
 */
typedef enum {
	MIN_IP, /**< First input-clock identifier. */
	PSS_REF_CLK = MIN_IP, /**< PSS REF CLK input clock. */
	VIDEO_CLK, /**< VIDEO CLK input clock. */
	PSS_ALT_REF_CLK, /**< PSS ALT REF CLK input clock. */
	GT_CRX_REF_CLK, /**< GT CRX REF CLK input clock. */
	AUX_REF_CLK, /**< AUX REF CLK input clock. */
	DP_ACLK, /**< DP ACLK input clock. */
	MAX_IP = DP_ACLK, /**< Last input-clock identifier. */
} XClock_InputClks;

/**
 * @brief Lists CLOCKPS divider database indices.
 */
typedef enum {
	MIN_DIVIDER, /**< First divider-database index. */
	PL0_DIV0 = MIN_DIVIDER, /**< PL0 DIV0 divider index. */
	PL0_DIV1, /**< PL0 DIV1 divider index. */
	PL1_DIV0, /**< PL1 DIV0 divider index. */
	PL1_DIV1, /**< PL1 DIV1 divider index. */
	PL2_DIV0, /**< PL2 DIV0 divider index. */
	PL2_DIV1, /**< PL2 DIV1 divider index. */
	PL3_DIV0, /**< PL3 DIV0 divider index. */
	PL3_DIV1, /**< PL3 DIV1 divider index. */
	APLL_TO_LPD_DIV0, /**< APLL TO LPD DIV0 divider index. */
	DPLL_TO_LPD_DIV0, /**< DPLL TO LPD DIV0 divider index. */
	VPLL_TO_LPD_DIV0, /**< VPLL TO LPD DIV0 divider index. */
	IOPLL_TO_FPD_DIV0, /**< IOPLL TO FPD DIV0 divider index. */
	RPLL_TO_FPD_DIV0, /**< RPLL TO FPD DIV0 divider index. */
	ACPU_DIV0, /**< ACPU DIV0 divider index. */
	DDR_REF_DIV0, /**< DDR REF DIV0 divider index. */
	GEM0_REF_DIV0, /**< GEM0 REF DIV0 divider index. */
	GEM0_REF_DIV1, /**< GEM0 REF DIV1 divider index. */
	GEM1_REF_DIV0, /**< GEM1 REF DIV0 divider index. */
	GEM1_REF_DIV1, /**< GEM1 REF DIV1 divider index. */
	GEM2_REF_DIV0, /**< GEM2 REF DIV0 divider index. */
	GEM2_REF_DIV1, /**< GEM2 REF DIV1 divider index. */
	GEM3_REF_DIV0, /**< GEM3 REF DIV0 divider index. */
	GEM3_REF_DIV1, /**< GEM3 REF DIV1 divider index. */
	TIMESTAMP_REF_DIV0, /**< TIMESTAMP REF DIV0 divider index. */
	DBG_TRACE_DIV0, /**< DBG TRACE DIV0 divider index. */
	DBG_FPD_DIV0, /**< DBG FPD DIV0 divider index. */
	DBG_LPD_DIV0, /**< DBG LPD DIV0 divider index. */
	DBG_TSTMP_DIV0, /**< DBG TSTMP DIV0 divider index. */
	DP_VIDEO_REF_DIV0, /**< DP VIDEO REF DIV0 divider index. */
	DP_VIDEO_REF_DIV1, /**< DP VIDEO REF DIV1 divider index. */
	DP_AUDIO_REF_DIV0, /**< DP AUDIO REF DIV0 divider index. */
	DP_AUDIO_REF_DIV1, /**< DP AUDIO REF DIV1 divider index. */
	DP_STC_REF_DIV0, /**< DP STC REF DIV0 divider index. */
	DP_STC_REF_DIV1, /**< DP STC REF DIV1 divider index. */
	GPU_REF_DIV0, /**< GPU REF DIV0 divider index. */
	SATA_REF_DIV0, /**< SATA REF DIV0 divider index. */
	PCIE_REF_DIV0, /**< PCIE REF DIV0 divider index. */
	GDMA_REF_DIV0, /**< GDMA REF DIV0 divider index. */
	DPDMA_REF_DIV0, /**< DPDMA REF DIV0 divider index. */
	TOPSW_MAIN_DIV0, /**< TOPSW MAIN DIV0 divider index. */
	TOPSW_LSBUS_DIV0, /**< TOPSW LSBUS DIV0 divider index. */
	GTGREF0_REF_DIV0, /**< GTGREF0 REF DIV0 divider index. */
	USB3_DUAL_REF_DIV0, /**< USB3 DUAL REF DIV0 divider index. */
	USB3_DUAL_REF_DIV1, /**< USB3 DUAL REF DIV1 divider index. */
	USB0_BUS_REF_DIV0, /**< USB0 BUS REF DIV0 divider index. */
	USB0_BUS_REF_DIV1, /**< USB0 BUS REF DIV1 divider index. */
	USB1_BUS_REF_DIV0, /**< USB1 BUS REF DIV0 divider index. */
	USB1_BUS_REF_DIV1, /**< USB1 BUS REF DIV1 divider index. */
	GEM_TSU_REF_DIV0, /**< GEM TSU REF DIV0 divider index. */
	GEM_TSU_REF_DIV1, /**< GEM TSU REF DIV1 divider index. */
	QSPI_REF_DIV0, /**< QSPI REF DIV0 divider index. */
	QSPI_REF_DIV1, /**< QSPI REF DIV1 divider index. */
	SDIO0_REF_DIV0, /**< SDIO0 REF DIV0 divider index. */
	SDIO0_REF_DIV1, /**< SDIO0 REF DIV1 divider index. */
	SDIO1_REF_DIV0, /**< SDIO1 REF DIV0 divider index. */
	SDIO1_REF_DIV1, /**< SDIO1 REF DIV1 divider index. */
	UART0_REF_DIV0, /**< UART0 REF DIV0 divider index. */
	UART0_REF_DIV1, /**< UART0 REF DIV1 divider index. */
	UART1_REF_DIV0, /**< UART1 REF DIV0 divider index. */
	UART1_REF_DIV1, /**< UART1 REF DIV1 divider index. */
	SPI0_REF_DIV0, /**< SPI0 REF DIV0 divider index. */
	SPI0_REF_DIV1, /**< SPI0 REF DIV1 divider index. */
	SPI1_REF_DIV0, /**< SPI1 REF DIV0 divider index. */
	SPI1_REF_DIV1, /**< SPI1 REF DIV1 divider index. */
	CAN0_REF_DIV0, /**< CAN0 REF DIV0 divider index. */
	CAN0_REF_DIV1, /**< CAN0 REF DIV1 divider index. */
	CAN1_REF_DIV0, /**< CAN1 REF DIV0 divider index. */
	CAN1_REF_DIV1, /**< CAN1 REF DIV1 divider index. */
	CPU_R5_DIV0, /**< CPU R5 DIV0 divider index. */
	IOU_SWITCH_DIV0, /**< IOU SWITCH DIV0 divider index. */
	CSU_PLL_DIV0, /**< CSU PLL DIV0 divider index. */
	PCAP_DIV0, /**< PCAP DIV0 divider index. */
	LPD_SWITCH_DIV0, /**< LPD SWITCH DIV0 divider index. */
	LPD_LSBUS_DIV0, /**< LPD LSBUS DIV0 divider index. */
	NAND_REF_DIV0, /**< NAND REF DIV0 divider index. */
	NAND_REF_DIV1, /**< NAND REF DIV1 divider index. */
	ADMA_REF_DIV0, /**< ADMA REF DIV0 divider index. */
	AMS_REF_DIV0, /**< AMS REF DIV0 divider index. */
	AMS_REF_DIV1, /**< AMS REF DIV1 divider index. */
	I2C0_REF_DIV0, /**< I2C0 REF DIV0 divider index. */
	I2C0_REF_DIV1, /**< I2C0 REF DIV1 divider index. */
	I2C1_REF_DIV0, /**< I2C1 REF DIV0 divider index. */
	I2C1_REF_DIV1, /**< I2C1 REF DIV1 divider index. */
	MAX_DIVIDER = I2C1_REF_DIV1, /**< Last divider-database index. */
} XClock_DivIndices;

/**
 * @brief Lists CLOCKPS mux database indices.
 */
typedef enum {
	MIN_MUX, /**< First mux-database index. */
	IOPLL_PRE_SRC_MUX = MIN_MUX, /**< IOPLL PRE SRC MUX mux index. */
	IOPLL_INT_MUX, /**< IOPLL INT MUX mux index. */
	IOPLL_POST_SRC_MUX, /**< IOPLL POST SRC MUX mux index. */
	IOPLL_MUX, /**< IOPLL MUX mux index. */
	RPLL_PRE_SRC_MUX, /**< RPLL PRE SRC MUX mux index. */
	RPLL_INT_MUX, /**< RPLL INT MUX mux index. */
	RPLL_POST_SRC_MUX, /**< RPLL POST SRC MUX mux index. */
	RPLL_MUX, /**< RPLL MUX mux index. */
	APLL_PRE_SRC_MUX, /**< APLL PRE SRC MUX mux index. */
	APLL_INT_MUX, /**< APLL INT MUX mux index. */
	APLL_POST_SRC_MUX, /**< APLL POST SRC MUX mux index. */
	APLL_MUX, /**< APLL MUX mux index. */
	DPLL_PRE_SRC_MUX, /**< DPLL PRE SRC MUX mux index. */
	DPLL_INT_MUX, /**< DPLL INT MUX mux index. */
	DPLL_POST_SRC_MUX, /**< DPLL POST SRC MUX mux index. */
	DPLL_MUX, /**< DPLL MUX mux index. */
	VPLL_PRE_SRC_MUX, /**< VPLL PRE SRC MUX mux index. */
	VPLL_INT_MUX, /**< VPLL INT MUX mux index. */
	VPLL_POST_SRC_MUX, /**< VPLL POST SRC MUX mux index. */
	VPLL_MUX, /**< VPLL MUX mux index. */
	ACPU_MUX, /**< ACPU MUX mux index. */
	WDT_MUX, /**< WDT MUX mux index. */
	DDR_MUX, /**< DDR MUX mux index. */
	PL0_MUX, /**< PL0 MUX mux index. */
	PL1_MUX, /**< PL1 MUX mux index. */
	PL2_MUX, /**< PL2 MUX mux index. */
	PL3_MUX, /**< PL3 MUX mux index. */
	DBG_TRACE_MUX, /**< DBG TRACE MUX mux index. */
	DBG_FPD_MUX, /**< DBG FPD MUX mux index. */
	DBG_LPD_MUX, /**< DBG LPD MUX mux index. */
	DBG_TSTMP_MUX, /**< DBG TSTMP MUX mux index. */
	DP_VIDEO_REF_MUX, /**< DP VIDEO REF MUX mux index. */
	DP_AUDIO_REF_MUX, /**< DP AUDIO REF MUX mux index. */
	DP_STC_REF_MUX, /**< DP STC REF MUX mux index. */
	GPU_REF_MUX, /**< GPU REF MUX mux index. */
	SATA_REF_MUX, /**< SATA REF MUX mux index. */
	PCIE_REF_MUX, /**< PCIE REF MUX mux index. */
	GDMA_REF_MUX, /**< GDMA REF MUX mux index. */
	DPDMA_REF_MUX, /**< DPDMA REF MUX mux index. */
	TOPSW_MAIN_MUX, /**< TOPSW MAIN MUX mux index. */
	TOPSW_LSBUS_MUX, /**< TOPSW LSBUS MUX mux index. */
	GTGREF0_REF_MUX, /**< GTGREF0 REF MUX mux index. */
	USB3_DUAL_REF_MUX, /**< USB3 DUAL REF MUX mux index. */
	USB0_BUS_REF_MUX, /**< USB0 BUS REF MUX mux index. */
	USB1_BUS_REF_MUX, /**< USB1 BUS REF MUX mux index. */
	GEM0_REF_MUX, /**< GEM0 REF MUX mux index. */
	GEM0_TX_MUX, /**< GEM0 TX MUX mux index. */
	GEM1_REF_MUX, /**< GEM1 REF MUX mux index. */
	GEM1_TX_MUX, /**< GEM1 TX MUX mux index. */
	GEM2_REF_MUX, /**< GEM2 REF MUX mux index. */
	GEM2_TX_MUX, /**< GEM2 TX MUX mux index. */
	GEM3_REF_MUX, /**< GEM3 REF MUX mux index. */
	GEM3_TX_MUX, /**< GEM3 TX MUX mux index. */
	GEM_TSU_REF_MUX, /**< GEM TSU REF MUX mux index. */
	GEM_TSU_MUX, /**< GEM TSU MUX mux index. */
	QSPI_REF_MUX, /**< QSPI REF MUX mux index. */
	SDIO0_REF_MUX, /**< SDIO0 REF MUX mux index. */
	SDIO1_REF_MUX, /**< SDIO1 REF MUX mux index. */
	UART0_REF_MUX, /**< UART0 REF MUX mux index. */
	UART1_REF_MUX, /**< UART1 REF MUX mux index. */
	SPI0_REF_MUX, /**< SPI0 REF MUX mux index. */
	SPI1_REF_MUX, /**< SPI1 REF MUX mux index. */
	CAN0_REF_MUX, /**< CAN0 REF MUX mux index. */
	CAN0_MIO_MUX, /**< CAN0 MIO MUX mux index. */
	CAN0_MUX, /**< CAN0 MUX mux index. */
	CAN1_REF_MUX, /**< CAN1 REF MUX mux index. */
	CAN1_MIO_MUX, /**< CAN1 MIO MUX mux index. */
	CAN1_MUX, /**< CAN1 MUX mux index. */
	CPU_R5_MUX, /**< CPU R5 MUX mux index. */
	IOU_SWITCH_MUX, /**< IOU SWITCH MUX mux index. */
	CSU_PLL_MUX, /**< CSU PLL MUX mux index. */
	PCAP_MUX, /**< PCAP MUX mux index. */
	LPD_SWITCH_MUX, /**< LPD SWITCH MUX mux index. */
	LPD_LSBUS_MUX, /**< LPD LSBUS MUX mux index. */
	NAND_REF_MUX, /**< NAND REF MUX mux index. */
	ADMA_REF_MUX, /**< ADMA REF MUX mux index. */
	DLL_REF_MUX, /**< DLL REF MUX mux index. */
	AMS_REF_MUX, /**< AMS REF MUX mux index. */
	I2C0_REF_MUX, /**< I2C0 REF MUX mux index. */
	I2C1_REF_MUX, /**< I2C1 REF MUX mux index. */
	TIMESTAMP_REF_MUX, /**< TIMESTAMP REF MUX mux index. */
	MAX_MUX = TIMESTAMP_REF_MUX, /**< Last mux-database index. */
} XClock_MuxIndices;

/**
 * @brief Lists CLOCKPS fixed-factor database indices.
 */
typedef enum {
	MIN_FIXEDFACTOR, /**< First fixed-factor database index. */
	/** IOPLL INT HALF FF fixed-factor index. */
	IOPLL_INT_HALF_FF = MIN_FIXEDFACTOR,
	RPLL_INT_HALF_FF, /**< RPLL INT HALF FF fixed-factor index. */
	APLL_INT_HALF_FF, /**< APLL INT HALF FF fixed-factor index. */
	DPLL_INT_HALF_FF, /**< DPLL INT HALF FF fixed-factor index. */
	VPLL_INT_HALF_FF, /**< VPLL INT HALF FF fixed-factor index. */
	ACPU_HALF_DIV_FF, /**< ACPU HALF DIV FF fixed-factor index. */
	/** Last fixed-factor database index. */
	MAX_FIXEDFACTOR = ACPU_HALF_DIV_FF,
} XClock_FixFactIndices;

/**
 * @brief Lists CLOCKPS gate database indices.
 */
typedef enum {
	MIN_GATE, /**< First gate-database index. */
	ACPU_GATE = MIN_GATE, /**< ACPU GATE gate index. */
	ACPU_HALF_GATE, /**< ACPU HALF GATE gate index. */
	PL0_GATE, /**< PL0 GATE gate index. */
	PL1_GATE, /**< PL1 GATE gate index. */
	PL2_GATE, /**< PL2 GATE gate index. */
	PL3_GATE, /**< PL3 GATE gate index. */
	DBG_TRACE_GATE, /**< DBG TRACE GATE gate index. */
	DBG_FPD_GATE, /**< DBG FPD GATE gate index. */
	DBG_LPD_GATE, /**< DBG LPD GATE gate index. */
	DP_VIDEO_REF_GATE, /**< DP VIDEO REF GATE gate index. */
	DP_AUDIO_REF_GATE, /**< DP AUDIO REF GATE gate index. */
	DP_STC_REF_GATE, /**< DP STC REF GATE gate index. */
	GPU_REF_GATE, /**< GPU REF GATE gate index. */
	GPU_PP0_REF_GATE, /**< GPU PP0 REF GATE gate index. */
	GPU_PP1_REF_GATE, /**< GPU PP1 REF GATE gate index. */
	SATA_REF_GATE, /**< SATA REF GATE gate index. */
	PCIE_REF_GATE, /**< PCIE REF GATE gate index. */
	GDMA_REF_GATE, /**< GDMA REF GATE gate index. */
	DPDMA_REF_GATE, /**< DPDMA REF GATE gate index. */
	TOPSW_MAIN_GATE, /**< TOPSW MAIN GATE gate index. */
	TOPSW_LSBUS_GATE, /**< TOPSW LSBUS GATE gate index. */
	GTGREF0_REF_GATE, /**< GTGREF0 REF GATE gate index. */
	USB3_DUAL_REF_GATE, /**< USB3 DUAL REF GATE gate index. */
	USB0_BUS_REF_GATE, /**< USB0 BUS REF GATE gate index. */
	USB1_BUS_REF_GATE, /**< USB1 BUS REF GATE gate index. */
	GEM0_TX_GATE, /**< GEM0 TX GATE gate index. */
	GEM0_REF_GATE, /**< GEM0 REF GATE gate index. */
	GEM1_TX_GATE, /**< GEM1 TX GATE gate index. */
	GEM1_REF_GATE, /**< GEM1 REF GATE gate index. */
	GEM2_TX_GATE, /**< GEM2 TX GATE gate index. */
	GEM2_REF_GATE, /**< GEM2 REF GATE gate index. */
	GEM3_TX_GATE, /**< GEM3 TX GATE gate index. */
	GEM3_REF_GATE, /**< GEM3 REF GATE gate index. */
	GEM_TSU_REF_GATE, /**< GEM TSU REF GATE gate index. */
	QSPI_REF_GATE, /**< QSPI REF GATE gate index. */
	SDIO0_REF_GATE, /**< SDIO0 REF GATE gate index. */
	SDIO1_REF_GATE, /**< SDIO1 REF GATE gate index. */
	UART0_REF_GATE, /**< UART0 REF GATE gate index. */
	UART1_REF_GATE, /**< UART1 REF GATE gate index. */
	SPI0_REF_GATE, /**< SPI0 REF GATE gate index. */
	SPI1_REF_GATE, /**< SPI1 REF GATE gate index. */
	CAN0_REF_GATE, /**< CAN0 REF GATE gate index. */
	CAN1_REF_GATE, /**< CAN1 REF GATE gate index. */
	CPU_R5_GATE, /**< CPU R5 GATE gate index. */
	CPU_R5_CORE_GATE, /**< CPU R5 CORE GATE gate index. */
	IOU_SWITCH_GATE, /**< IOU SWITCH GATE gate index. */
	CSU_PLL_GATE, /**< CSU PLL GATE gate index. */
	PCAP_GATE, /**< PCAP GATE gate index. */
	LPD_SWITCH_GATE, /**< LPD SWITCH GATE gate index. */
	LPD_LSBUS_GATE, /**< LPD LSBUS GATE gate index. */
	NAND_REF_GATE, /**< NAND REF GATE gate index. */
	ADMA_REF_GATE, /**< ADMA REF GATE gate index. */
	AMS_REF_GATE, /**< AMS REF GATE gate index. */
	I2C0_REF_GATE, /**< I2C0 REF GATE gate index. */
	I2C1_REF_GATE, /**< I2C1 REF GATE gate index. */
	TIMESTAMP_REF_GATE, /**< TIMESTAMP REF GATE gate index. */
	MAX_GATE = TIMESTAMP_REF_GATE, /**< Last gate-database index. */
} XClock_GateIndices;

/**
 * @brief Lists CLOCKPS PLL database indices.
 */
typedef enum {
	MIN_PLL, /**< First PLL-database index. */
	IOPLL_INT_PLL = MIN_PLL, /**< IOPLL INT PLL PLL index. */
	RPLL_INT_PLL, /**< RPLL INT PLL PLL index. */
	APLL_INT_PLL, /**< APLL INT PLL PLL index. */
	DPLL_INT_PLL, /**< DPLL INT PLL PLL index. */
	VPLL_INT_PLL, /**< VPLL INT PLL PLL index. */
	MAX_PLL = VPLL_INT_PLL, /**< Last PLL-database index. */
} XClock_PllIndices;

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
/****************************************************************************/
/**
* @brief Reads a 32-bit CLOCKPS register.
*
* Read the given register.
*
* @param[in] RegAddr Address of the register to read.
* @param[out] Value Pointer to variable holding read value.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if unsuccessful.
*
* @note         Read from  register may fail in case driver doesn't have proper
* 		access at EL1 NONSECURE
*
******************************************************************************/
static inline XStatus XClock_ReadReg(u32 RegAddr, u32 *Value)
{
#if defined (__aarch64__) && (EL1_NONSECURE == 1)
	XSmc_OutVar RegValue;

	RegValue = Xil_Smc(MMIO_READ_SMC_FID, (u64)(RegAddr), 0, 0, 0, 0, 0, 0);
	if (0x00 == (RegValue.Arg0 & 0xFFFFFFFF)) {
		*Value = RegValue.Arg0 >> 32;
		return XST_SUCCESS;
	}

	return XST_FAILURE;
#else
	*Value = Xil_In32((u32)RegAddr);
	return XST_SUCCESS;
#endif
}

/****************************************************************************/
/**
* @brief Writes a 32-bit CLOCKPS register.
*
* Write the given register.
*
* @param[in] RegAddr Address of the register to write.
* @param[in] Value 32-bit value to write to the register.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if unsuccessful.
*
* @note         Write to register may fail in case driver doesn't have proper
* 		access at EL1 NONSECURE.
*
******************************************************************************/
static inline XStatus XClock_WriteReg(u32 RegAddr, u32 Value)
{
#if defined (__aarch64__) && (EL1_NONSECURE == 1)
	XSmc_OutVar RegValue;
	RegValue = Xil_Smc(MMIO_WRITE_SMC_FID,
			   (u64)(RegAddr) | ((u64)(0xFFFFFFFF) << 32),
			   (u64)Value, 0, 0, 0, 0, 0);
	if (0x00 == (RegValue.Arg0 & 0xFFFFFFFF)) {
		return XST_SUCCESS;
	}

	return XST_FAILURE;
#else
	Xil_Out32((u32)RegAddr, (u32)Value);
	return XST_SUCCESS;
#endif
}

/* Nodes init functions */
void XClock_PllBeginInit(void);
void XClock_MuxBeginInit(void);
void XClock_GateBeginInit(void);
void XClock_DivBeginInit(void);
void XClock_FixedFactorBeginInit(void);

/* Nodes callback register functions */
void XClock_PllRegisterFuncs(void);
void XClock_MuxRegisterFuncs(void);
void XClock_GateRegisterFuncs(void);
void XClock_DivRegisterFuncs(void);
void XClock_FixedFactorRegisterFuncs(void);

/**
 * @brief Points to a node's update-rate handler.
 */
typedef void (*XClock_UpdateRateFuncPtr)(u8 NodeIdx);
extern XClock_UpdateRateFuncPtr XClock_NodeUpdateRate[XCLOCK_TYPE_MAX];

/**
 * @brief Points to a node's get-rate handler.
 */
typedef XStatus (*XClock_FetchRateFuncPtr)(u8 NodeIdx, XClockRate *GetRate);
extern XClock_FetchRateFuncPtr XClock_NodeGetRate[XCLOCK_TYPE_MAX];

/**
 * @brief Points to a node's set-rate handler.
 */
typedef XStatus (*XClock_SetRateFuncPtr)(u8 NodeIdx, XClockRate ParentRate,
		XClockRate Rate, XClockRate *SetRate, u8 DryRun);
extern XClock_SetRateFuncPtr XClock_NodeSetRate[XCLOCK_TYPE_MAX];

/**
 * @brief Points to a node's fetch-parent handler.
 */
typedef XStatus (*XClock_FetchParentFuncPtr)(XClock_Types *NodeType,
		u8 *NodeIdx);
extern XClock_FetchParentFuncPtr XClock_NodeFetchParent[XCLOCK_TYPE_MAX];

/**
 * @brief Points to a node's fetch-index handler.
 */
typedef XStatus (*XClock_FetchIdxFuncPtr)(XClock_OutputClks ClockId,
		u8 *NodeIdx);
extern XClock_FetchIdxFuncPtr XClock_NodeFetchIdx[XCLOCK_TYPE_MAX];

/**
 * @brief Points to a node's disable handler.
 */
typedef XStatus (*XClock_DisableNodeFuncPtr)(u8 NodeIdx);
extern XClock_DisableNodeFuncPtr XClock_NodeDisable[XCLOCK_TYPE_MAX];

/**
 * @brief Points to a node's enable handler.
 */
typedef XStatus (*XClock_EnableNodeFuncPtr)(u8 NodeIdx);
extern XClock_EnableNodeFuncPtr XClock_NodeEnable[XCLOCK_TYPE_MAX];

/**
 * @brief Points to a node's init handler.
 */
typedef void (*XClock_InitClkFuncPtr)(u8 NodeIdx);
extern XClock_InitClkFuncPtr XClock_NodeInit[XCLOCK_TYPE_MAX];

/**
 * @brief Points to the mux set-parent handler.
 */
typedef XStatus (*XClock_SetParentFuncPtr)(u8 NodeIdx, u8 SetParentIdx);
extern XClock_SetParentFuncPtr XClock_MuxSetParent;

/* Wrapper functions */
XStatus XClock_EnableClkNode(XClock_Types NodeType, u8 NodeIdx);
XStatus XClock_DisableClkNode(XClock_Types NodeType, u8 NodeIdx);
void XClock_InitClk(XClock_Types NodeType, u8 NodeIdx);
void XClock_UpdateRate(XClock_Types NodeType, u8 NodeIdx);
XClockRate XClock_FetchRate(XClock_Types NodeType, u8 NodeIdx);

/* APIs */
#ifndef SDT
XClockPs_Config *XClock_LookupConfig(u16 DeviceId);
#else
XClockPs_Config *XClock_LookupConfig(UINTPTR BaseAddress);
#endif
XStatus XClock_CfgInitialize(XClock *InstancePtr, XClockPs_Config *ConfigPtr);
XStatus XClock_EnableClock(XClock_OutputClks ClockId);
XStatus XClock_DisableClock(XClock_OutputClks ClockId);
XStatus XClock_GetParent(XClock_OutputClks ClockId,
			 XClock_Types *NodeType, u8 *NodeIdx);
XStatus XClock_GetRate(XClock_OutputClks ClockId, XClockRate *Rate);
XStatus XClock_SetParent(XClock_OutputClks ClockId, u8 MuxIdx,
			 u8 SetParentIdx);
XStatus XClock_SetRate(XClock_OutputClks ClockId, XClockRate Rate,
		       XClockRate *SetRate);

#ifdef __cplusplus
}
#endif
#endif /* end of protection macro */

/** @} */
