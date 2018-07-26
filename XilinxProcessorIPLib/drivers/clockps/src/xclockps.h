/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/****************************************************************************/
/**
*
* @file xclockps.h
* @addtogroup xclock_v1_0
* @{
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
* </pre>
*
******************************************************************************/
#ifndef XCLOCK_H		/* prevent circular inclusions */
#define XCLOCK_H		/* by using protection macros */

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
#define XPAR_XCLOCKPS_NUM_INSTANCES  (XPAR_XCRPSU_NUM_INSTANCES)
#define XPAR_XCLOCKPS_DEVICE_ID      (XPAR_XCRPSU_0_DEVICE_ID)

/* Fixed clock rate definitions */
#define XCLOCK_FIX_RATE_VIDEO_CLK    (27000000U)
#define XCLOCK_FIX_RATE_PSS_ALT_REF_CLK \
				     (0U)
#define XCLOCK_FIX_RATE_GT_CRX_REF_CLK \
				     (108000000U)
#define XCLOCK_FIX_RATE_AUX_REF_CLK  (27000000U)
#define XCLOCK_FIX_RATE_DP_ACLK      (100000000U)

/* Miscellaneous defines */
#define XCLOCK_INVALID_PARENT        (0xFFFF)
#define XCLOCK_PARENT_TYPE_MASK      (0xFF00)
#define XCLOCK_PARENT_IDX_MASK       (0x00FF)
#define XCLOCK_INVALID_RATE          (0)
#define XCLOCK_DIVIDERS_BITWIDTH     (6)
#define XCLOCK_MAX_DIV_VAL           XCLOCK_VALUE_MASK(XCLOCK_DIVIDERS_BITWIDTH)
#define XCLOCK_INVALID_DIV_INDEX     (MAX_DIVIDER + 1)
#define XCLOCK_INVALID_PLL_INDEX     (MAX_PLL + 1)

/* Clock Flags */
#define XCLOCK_CLOCK_IS_CRITICAL     BIT(0)	/**< Clock is critical and will not be disabled */

/* Bit mask of specific length */
#define XCLOCK_VALUE_MASK(masklen)   ((1 << masklen) - 1)

/* Bit shift */
#define BIT(n)		             (1 << (n))

/* Calculates array size */
#define ARRAY_SIZE(x)	             (sizeof(x) / sizeof((x)[0]))

/* Division rounded to closest integer */
#define XCLOCK_ROUND_DIV(a, b)       ((a + (b / 2)) / b)

/* Division rounded to integer greater than actual value */
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

/* Parse parent to fetch out parent type */
#define XCLOCK_FETCH_PARENT_TYPE(parent) \
				((parent & XCLOCK_PARENT_TYPE_MASK) >> 8)

/* Parse parent to fetch out parent database index */
#define XCLOCK_FETCH_PARENT_INDEX(parent) \
				(parent & XCLOCK_PARENT_IDX_MASK)

/* Absolute difference of 2 values */
#define XCLOCK_ABS_DIFF(a, b) \
	({ \
		u32 diff; \
		if (a < b) { \
			diff = b - a; \
		} else { \
			diff = a - b; \
		} \
		diff; \
	})

/* Limits the value within min and max */
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

/* This macro validates pointer to not be null */
#define XCLOCK_VALIDATE_PTR(ptr) \
	do { \
		if (NULL == ptr) { \
			return XST_INVALID_PARAM; \
		} \
	} while (0)

/**************************** Type Definitions *******************************/
/* Type defined for Rate */
#if defined (__aarch64__)
typedef u64 XClockRate;
#else
typedef u32 XClockRate;
#endif

/* This typedef contains configuration information for the device */
typedef struct {
	u16 DeviceId;                    /**< Unique ID of device */
} XClockPs_Config;

/**
 * The XClockPs driver instance data. The user is required to allocate a
 * variable of this type for every clock controller device in the system.
 * A pointer to a variable of this type is then passed to the driver API
 * functions.
 */
typedef struct {
	XClockPs_Config Config;          /**< Hardware Configuration */
} XClock;

/* Type of clock nodes */
typedef enum {
	XCLOCK_TYPE_IP,
	XCLOCK_TYPE_PLL,
	XCLOCK_TYPE_MUX,
	XCLOCK_TYPE_GATE,
	XCLOCK_TYPE_DIVIDER,
	XCLOCK_TYPE_FIXEDFACTOR,
	XCLOCK_TYPE_MAX
} XClock_Types;

/* List of output clocks */
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

/* List of input clocks */
typedef enum {
	MIN_IP,
	PSS_REF_CLK = MIN_IP,
	VIDEO_CLK,
	PSS_ALT_REF_CLK,
	GT_CRX_REF_CLK,
	AUX_REF_CLK,
	DP_ACLK,
	MAX_IP = DP_ACLK,
} XClock_InputClks;

/* Divider database indices */
typedef enum {
	MIN_DIVIDER,
	PL0_DIV0 = MIN_DIVIDER,
	PL0_DIV1,
	PL1_DIV0,
	PL1_DIV1,
	PL2_DIV0,
	PL2_DIV1,
	PL3_DIV0,
	PL3_DIV1,
	APLL_TO_LPD_DIV0,
	DPLL_TO_LPD_DIV0,
	VPLL_TO_LPD_DIV0,
	IOPLL_TO_FPD_DIV0,
	RPLL_TO_FPD_DIV0,
	ACPU_DIV0,
	DDR_REF_DIV0,
	GEM0_REF_DIV0,
	GEM0_REF_DIV1,
	GEM1_REF_DIV0,
	GEM1_REF_DIV1,
	GEM2_REF_DIV0,
	GEM2_REF_DIV1,
	GEM3_REF_DIV0,
	GEM3_REF_DIV1,
	TIMESTAMP_REF_DIV0,
	DBG_TRACE_DIV0,
	DBG_FPD_DIV0,
	DBG_LPD_DIV0,
	DBG_TSTMP_DIV0,
	DP_VIDEO_REF_DIV0,
	DP_VIDEO_REF_DIV1,
	DP_AUDIO_REF_DIV0,
	DP_AUDIO_REF_DIV1,
	DP_STC_REF_DIV0,
	DP_STC_REF_DIV1,
	GPU_REF_DIV0,
	SATA_REF_DIV0,
	PCIE_REF_DIV0,
	GDMA_REF_DIV0,
	DPDMA_REF_DIV0,
	TOPSW_MAIN_DIV0,
	TOPSW_LSBUS_DIV0,
	GTGREF0_REF_DIV0,
	USB3_DUAL_REF_DIV0,
	USB3_DUAL_REF_DIV1,
	USB0_BUS_REF_DIV0,
	USB0_BUS_REF_DIV1,
	USB1_BUS_REF_DIV0,
	USB1_BUS_REF_DIV1,
	GEM_TSU_REF_DIV0,
	GEM_TSU_REF_DIV1,
	QSPI_REF_DIV0,
	QSPI_REF_DIV1,
	SDIO0_REF_DIV0,
	SDIO0_REF_DIV1,
	SDIO1_REF_DIV0,
	SDIO1_REF_DIV1,
	UART0_REF_DIV0,
	UART0_REF_DIV1,
	UART1_REF_DIV0,
	UART1_REF_DIV1,
	SPI0_REF_DIV0,
	SPI0_REF_DIV1,
	SPI1_REF_DIV0,
	SPI1_REF_DIV1,
	CAN0_REF_DIV0,
	CAN0_REF_DIV1,
	CAN1_REF_DIV0,
	CAN1_REF_DIV1,
	CPU_R5_DIV0,
	IOU_SWITCH_DIV0,
	CSU_PLL_DIV0,
	PCAP_DIV0,
	LPD_SWITCH_DIV0,
	LPD_LSBUS_DIV0,
	NAND_REF_DIV0,
	NAND_REF_DIV1,
	ADMA_REF_DIV0,
	AMS_REF_DIV0,
	AMS_REF_DIV1,
	I2C0_REF_DIV0,
	I2C0_REF_DIV1,
	I2C1_REF_DIV0,
	I2C1_REF_DIV1,
	MAX_DIVIDER = I2C1_REF_DIV1,
} XClock_DivIndices;

/* Mux database indices */
typedef enum {
	MIN_MUX,
	IOPLL_PRE_SRC_MUX = MIN_MUX,
	IOPLL_INT_MUX,
	IOPLL_POST_SRC_MUX,
	IOPLL_MUX,
	RPLL_PRE_SRC_MUX,
	RPLL_INT_MUX,
	RPLL_POST_SRC_MUX,
	RPLL_MUX,
	APLL_PRE_SRC_MUX,
	APLL_INT_MUX,
	APLL_POST_SRC_MUX,
	APLL_MUX,
	DPLL_PRE_SRC_MUX,
	DPLL_INT_MUX,
	DPLL_POST_SRC_MUX,
	DPLL_MUX,
	VPLL_PRE_SRC_MUX,
	VPLL_INT_MUX,
	VPLL_POST_SRC_MUX,
	VPLL_MUX,
	ACPU_MUX,
	WDT_MUX,
	DDR_MUX,
	PL0_MUX,
	PL1_MUX,
	PL2_MUX,
	PL3_MUX,
	DBG_TRACE_MUX,
	DBG_FPD_MUX,
	DBG_LPD_MUX,
	DBG_TSTMP_MUX,
	DP_VIDEO_REF_MUX,
	DP_AUDIO_REF_MUX,
	DP_STC_REF_MUX,
	GPU_REF_MUX,
	SATA_REF_MUX,
	PCIE_REF_MUX,
	GDMA_REF_MUX,
	DPDMA_REF_MUX,
	TOPSW_MAIN_MUX,
	TOPSW_LSBUS_MUX,
	GTGREF0_REF_MUX,
	USB3_DUAL_REF_MUX,
	USB0_BUS_REF_MUX,
	USB1_BUS_REF_MUX,
	GEM0_REF_MUX,
	GEM0_TX_MUX,
	GEM1_REF_MUX,
	GEM1_TX_MUX,
	GEM2_REF_MUX,
	GEM2_TX_MUX,
	GEM3_REF_MUX,
	GEM3_TX_MUX,
	GEM_TSU_REF_MUX,
	GEM_TSU_MUX,
	QSPI_REF_MUX,
	SDIO0_REF_MUX,
	SDIO1_REF_MUX,
	UART0_REF_MUX,
	UART1_REF_MUX,
	SPI0_REF_MUX,
	SPI1_REF_MUX,
	CAN0_REF_MUX,
	CAN0_MIO_MUX,
	CAN0_MUX,
	CAN1_REF_MUX,
	CAN1_MIO_MUX,
	CAN1_MUX,
	CPU_R5_MUX,
	IOU_SWITCH_MUX,
	CSU_PLL_MUX,
	PCAP_MUX,
	LPD_SWITCH_MUX,
	LPD_LSBUS_MUX,
	NAND_REF_MUX,
	ADMA_REF_MUX,
	DLL_REF_MUX,
	AMS_REF_MUX,
	I2C0_REF_MUX,
	I2C1_REF_MUX,
	TIMESTAMP_REF_MUX,
	MAX_MUX = TIMESTAMP_REF_MUX,
} XClock_MuxIndices;

/* Fixed factor database indices */
typedef enum {
	MIN_FIXEDFACTOR,
	IOPLL_INT_HALF_FF = MIN_FIXEDFACTOR,
	RPLL_INT_HALF_FF,
	APLL_INT_HALF_FF,
	DPLL_INT_HALF_FF,
	VPLL_INT_HALF_FF,
	ACPU_HALF_DIV_FF,
	MAX_FIXEDFACTOR = ACPU_HALF_DIV_FF,
} XClock_FixFactIndices;

/* Gate database indices */
typedef enum {
	MIN_GATE,
	ACPU_GATE = MIN_GATE,
	ACPU_HALF_GATE,
	PL0_GATE,
	PL1_GATE,
	PL2_GATE,
	PL3_GATE,
	DBG_TRACE_GATE,
	DBG_FPD_GATE,
	DBG_LPD_GATE,
	DP_VIDEO_REF_GATE,
	DP_AUDIO_REF_GATE,
	DP_STC_REF_GATE,
	GPU_REF_GATE,
	GPU_PP0_REF_GATE,
	GPU_PP1_REF_GATE,
	SATA_REF_GATE,
	PCIE_REF_GATE,
	GDMA_REF_GATE,
	DPDMA_REF_GATE,
	TOPSW_MAIN_GATE,
	TOPSW_LSBUS_GATE,
	GTGREF0_REF_GATE,
	USB3_DUAL_REF_GATE,
	USB0_BUS_REF_GATE,
	USB1_BUS_REF_GATE,
	GEM0_TX_GATE,
	GEM0_REF_GATE,
	GEM1_TX_GATE,
	GEM1_REF_GATE,
	GEM2_TX_GATE,
	GEM2_REF_GATE,
	GEM3_TX_GATE,
	GEM3_REF_GATE,
	GEM_TSU_REF_GATE,
	QSPI_REF_GATE,
	SDIO0_REF_GATE,
	SDIO1_REF_GATE,
	UART0_REF_GATE,
	UART1_REF_GATE,
	SPI0_REF_GATE,
	SPI1_REF_GATE,
	CAN0_REF_GATE,
	CAN1_REF_GATE,
	CPU_R5_GATE,
	CPU_R5_CORE_GATE,
	IOU_SWITCH_GATE,
	CSU_PLL_GATE,
	PCAP_GATE,
	LPD_SWITCH_GATE,
	LPD_LSBUS_GATE,
	NAND_REF_GATE,
	ADMA_REF_GATE,
	AMS_REF_GATE,
	I2C0_REF_GATE,
	I2C1_REF_GATE,
	TIMESTAMP_REF_GATE,
	MAX_GATE = TIMESTAMP_REF_GATE,
} XClock_GateIndices;

/* Pll database indices */
typedef enum {
	MIN_PLL,
	IOPLL_INT_PLL = MIN_PLL,
	RPLL_INT_PLL,
	APLL_INT_PLL,
	DPLL_INT_PLL,
	VPLL_INT_PLL,
	MAX_PLL = VPLL_INT_PLL,
} XClock_PllIndices;

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
/****************************************************************************/
/**
* Read the given register.
*
* @param        RegAddr is the address of the register to read
* @param        Value is the pointer to variable holding read value
*
* @return       XST_SUCCESS if successful
* 		XST_FAILURE if unsuccessful
*
* @note         Read from  register may fail in case driver doesnot have proper
* 		access at EL1 NONSECURE
*
******************************************************************************/
static inline XStatus XClock_ReadReg(u32 RegAddr, u32 *Value)
{
#if EL1_NONSECURE
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
*
* Write the given register.
*
* @param        RegAddr is the address of the register to write
* @param        Value is the 32-bit value to write to the register
*
* @return       XST_SUCCESS if successful
* 		XST_FAILURE if unsuccessful
*
* @note         Write to register may fail in case driver doesnot have proper
* 		access at EL1 NONSECURE.
*
******************************************************************************/
static inline XStatus XClock_WriteReg(u32 RegAddr, u32 Value)
{
#if EL1_NONSECURE
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

/* Function pointer holding update rate functions for nodes */
typedef void (*XClock_UpdateRateFuncPtr)(u8 NodeIdx);
extern XClock_UpdateRateFuncPtr XClock_NodeUpdateRate[XCLOCK_TYPE_MAX];

/* Function pointer holding Get Rate functions for nodes */
typedef XStatus (*XClock_FetchRateFuncPtr)(u8 NodeIdx, XClockRate *GetRate);
extern XClock_FetchRateFuncPtr XClock_NodeGetRate[XCLOCK_TYPE_MAX];

/* Function pointer holding Set Rate functions for nodes */
typedef XStatus (*XClock_SetRateFuncPtr)(u8 NodeIdx, XClockRate ParentRate,
			XClockRate Rate, XClockRate *SetRate, u8 DryRun);
extern XClock_SetRateFuncPtr XClock_NodeSetRate[XCLOCK_TYPE_MAX];

/* Function pointer holding parent fetch functions for nodes */
typedef XStatus (*XClock_FetchParentFuncPtr)(XClock_Types *NodeType,
								u8 *NodeIdx);
extern XClock_FetchParentFuncPtr XClock_NodeFetchParent[XCLOCK_TYPE_MAX];

/* Function pointer holding database index fetch functions for nodes */
typedef XStatus (*XClock_FetchIdxFuncPtr)(XClock_OutputClks ClockId,
								u8 *NodeIdx);
extern XClock_FetchIdxFuncPtr XClock_NodeFetchIdx[XCLOCK_TYPE_MAX];

/* Function pointer holding disable functions for nodes */
typedef XStatus (*XClock_DisableNodeFuncPtr)(u8 NodeIdx);
extern XClock_DisableNodeFuncPtr XClock_NodeDisable[XCLOCK_TYPE_MAX];

/* Function pointer holding Enable functions for nodes */
typedef XStatus (*XClock_EnableNodeFuncPtr)(u8 NodeIdx);
extern XClock_EnableNodeFuncPtr XClock_NodeEnable[XCLOCK_TYPE_MAX];

/* Function pointer holding Init functions for nodes */
typedef void (*XClock_InitClkFuncPtr)(u8 NodeIdx);
extern XClock_InitClkFuncPtr XClock_NodeInit[XCLOCK_TYPE_MAX];

/* Function pointer holding parent set function for mux */
typedef XStatus (*XClock_SetParentFuncPtr)(u8 NodeIdx, u8 SetParentIdx);
extern XClock_SetParentFuncPtr XClock_MuxSetParent;

/* Wrapper functions */
XStatus XClock_EnableClkNode(XClock_Types NodeType, u8 NodeIdx);
XStatus XClock_DisableClkNode(XClock_Types NodeType, u8 NodeIdx);
void XClock_InitClk(XClock_Types NodeType, u8 NodeIdx);
void XClock_UpdateRate(XClock_Types NodeType, u8 NodeIdx);
XClockRate XClock_FetchRate(XClock_Types NodeType, u8 NodeIdx);

/* APIs */
XClockPs_Config *XClock_LookupConfig(u16 DeviceId);
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
