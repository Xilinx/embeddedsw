/******************************************************************************
* Copyright (C) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xclockps_mux.c
* @brief Implements CLOCKPS mux node operations.
*
* This file handles Mux related definition and operations.
*
* <pre>
* MODIFICATION HISTORY:
* Ver   Who    Date     Changes
* ----- ------ -------- ---------------------------------------------
* 1.00  cjp    02/09/18 First release
* 1.2   sd     02/13/20 Rename ARRAY_SIZE
* 1.7   vlt    01/27/26 Fixed codespell issues
* 1.8   ss     09/10/26 Fix invalid parent handling in MuxFetchParent
* 1.8   tb    09/17/26 Align Doxygen comments with AMD standards.
* </pre>
*
******************************************************************************/

/**
* @addtogroup clockps_api CLOCKPS APIs
* @{
*/

/***************************** Include Files *********************************/
#include "xclockps.h"

/************************** Constant Definitions *****************************/
/* Register offset defines */
#define XCLOCK_PLL_NXT_REG_OFFSET    4	/**< Pll nxt reg offset. */
#define XCLOCK_GEM_NXT_REG_OFFSET    2	/**< Gem nxt reg offset. */
#define XCLOCK_CAN_NXT_REG_OFFSET    3	/**< Can nxt reg offset. */

/* Mux parent types defines */
#define XCLOCK_PARENT_DIVS_MUX       (BIT(2))	/**< Parent divs mux. */
/** Parent muxs div. */
#define XCLOCK_PARENT_MUXS_DIV       (BIT(0) | BIT(1))
/** Parent div muxs. */
#define XCLOCK_PARENT_DIV_MUXS       (BIT(1) | BIT(2))
/** Parent mux div mux. */
#define XCLOCK_PARENT_MUX_DIV_MUX    (BIT(0) | BIT(2))
/** Parent all muxs. */
#define XCLOCK_PARENT_ALL_MUXS       (BIT(0) | BIT(1) | BIT(2))
#define XCLOCK_DEFAULT_MUX_PARENT    0	/**< Default mux parent. */
#define XCLOCK_PARENT_EXTERNAL       0xFFFE	/**< Parent external. */

/* Number of parents */
#define XCLOCK_NUM_PLL_SRC_PARENTS   8	/**< Num pll src parents. */
#define XCLOCK_NUM_PLL_PARENTS       2	/**< Num pll parents. */
#define XCLOCK_NUM_PERI_PARENTS      4	/**< Num peri parents. */
#define XCLOCK_NUM_DDR_PARENTS       2	/**< Num ddr parents. */
#define XCLOCK_NUM_WDT_PARENTS       2	/**< Num wdt parents. */
#define XCLOCK_NUM_TSTMP_PARENTS     8	/**< Num tstmp parents. */
#define XCLOCK_NUM_GEM_PARENTS       2	/**< Num gem parents. */
#define XCLOCK_NUM_GEM_TSU_PARENTS   4	/**< Num gem tsu parents. */
#define XCLOCK_NUM_CAN_PARENTS       2	/**< Num can parents. */
#define XCLOCK_NUM_DLL_REF_PARENTS   2	/**< Num dll ref parents. */
#define XCLOCK_NUM_CAN_MIO_PARENTS   77	/**< Num can mio parents. */

/* Number of peripherals */
#define XCLOCK_NUM_PLL               5	/**< Num pll. */
#define XCLOCK_NUM_GEM               4	/**< Num gem. */
#define XCLOCK_NUM_CAN               2	/**< Num can. */

/***************** Macros (Inline Functions) Definitions *********************/
/**
 * @brief Assigns structure elements for output clock mappings.
 *
 * @note
 * C-style signature
 *	XClock_MuxMappings XCLOCK_MUX_ASSIGN_INDICES(
 *		XClock_OutputClks ClockIdVal, XClock_MuxIndices MuxIndexVal)
 */
#define XCLOCK_MUX_ASSIGN_INDICES(ClockIdVal, MuxIndexVal) \
{ \
	.ClockId = (XClock_OutputClks)ClockIdVal, \
	.MuxIndex = (XClock_MuxIndices)MuxIndexVal \
}

/**
 * @brief Assigns structure elements for muxes.
 *
 * @note
 * C-style signature
 *	XClock_TypeMux XCLOCK_ASSIGN_MUXES(u16 *ParentVal, u8 NumParVal,
 *		u32 CtrlRegVal, u8 ShiftVal, u8 WidthVal)
 */
#define XCLOCK_ASSIGN_MUXES(ParentVal, \
				NumParVal, CtrlRegVal, ShiftVal, WidthVal) \
{ \
	.Shift = ShiftVal, \
	.Width = WidthVal, \
	.ActiveParent = (u8)XCLOCK_INVALID_PARENT, \
	.DefaultParent = 0, \
	.IsInit = FALSE, \
	.NumParents = NumParVal, \
	.EnableCount = 0, \
	.Parent = ParentVal, \
	.CtrlReg = CtrlRegVal, \
	.Rate = XCLOCK_INVALID_RATE, \
}

/**
 * @brief Assigns structure elements for muxes having 8 parents.
 *
 * @note
 * C-style signature
 *	XClock_TypeMux XCLOCK_ASSIGN_MUXES_P8(u16 *ParentVal, u32 CtrlRegVal,
 *		u8 ShiftVal, u8 WidthVal)
 */
#define XCLOCK_ASSIGN_MUXES_P8(ParentVal, CtrlRegVal, ShiftVal, WidthVal) \
{ \
	.Shift = ShiftVal, \
	.Width = WidthVal, \
	.ActiveParent = (u8)XCLOCK_INVALID_PARENT, \
	.DefaultParent = 0, \
	.IsInit = FALSE, \
	.NumParents = 8, \
	.EnableCount = 0, \
	.Parent = ParentVal, \
	.CtrlReg = CtrlRegVal, \
	.Rate = XCLOCK_INVALID_RATE, \
}

/**
 * @brief Assigns structure elements for muxes having 4 parents.
 *
 * @note
 * C-style signature
 *	XClock_TypeMux XCLOCK_ASSIGN_MUXES_P4(u16 *ParentVal, u32 CtrlRegVal,
 *		u8 ShiftVal, u8 WidthVal)
 */
#define XCLOCK_ASSIGN_MUXES_P4(ParentVal, CtrlRegVal, ShiftVal, WidthVal) \
{ \
	.Shift = ShiftVal, \
	.Width = WidthVal, \
	.ActiveParent = (u8)XCLOCK_INVALID_PARENT, \
	.DefaultParent = 0, \
	.IsInit = FALSE, \
	.NumParents = 4, \
	.EnableCount = 0, \
	.Parent = ParentVal, \
	.CtrlReg = CtrlRegVal, \
	.Rate = XCLOCK_INVALID_RATE, \
}

/**
 * @brief Assigns structure elements for muxes having 2 parents.
 *
 * @note
 * C-style signature
 *	XClock_TypeMux XCLOCK_ASSIGN_MUXES_P2(u16 *ParentVal, u32 CtrlRegVal,
 *		u8 ShiftVal, u8 WidthVal)
 */
#define XCLOCK_ASSIGN_MUXES_P2(ParentVal, CtrlRegVal, ShiftVal, WidthVal) \
{ \
	.Shift = ShiftVal, \
	.Width = WidthVal, \
	.ActiveParent = (u8)XCLOCK_INVALID_PARENT, \
	.DefaultParent = 0, \
	.IsInit = FALSE, \
	.NumParents = 2, \
	.EnableCount = 0, \
	.Parent = ParentVal, \
	.CtrlReg = CtrlRegVal, \
	.Rate = XCLOCK_INVALID_RATE, \
}

/**************************** Type Definitions *******************************/
/**
 * @brief Holds output clocks mapped to muxes.
 */
typedef struct {
	XClock_OutputClks ClockId; /**< Clockid. */
	XClock_MuxIndices MuxIndex; /**< Muxindex. */

} XClock_MuxMappings;

/**
 * @brief Holds muxes.
 */
typedef struct {
	u8          Shift; /**< Shift. */
	u8          Width; /**< Width. */
	u8          ActiveParent; /**< Activeparent. */
	u8          DefaultParent; /**< Defaultparent. */
	u8          IsInit; /**< Isinit. */
	u8          NumParents; /**< Numparents. */
	u8          EnableCount; /**< Enablecount. */
	u16         *Parent; /**< Parent. */
	u32         CtrlReg; /**< Ctrlreg. */
	XClockRate  Rate; /**< Rate. */

} XClock_TypeMux;

/* This typedef defines parent indices for peripherals */
typedef enum {
	IDX_PL,
	IDX_ACPU,
	IDX_DBG_FPD,
	IDX_DBG_LPD,
	IDX_DP,
	IDX_GPU_REF,
	IDX_SATA_REF,
	IDX_PCIE_REF,
	IDX_DMA_REF,
	IDX_TOPSW_MAIN,
	IDX_TOPSW_LSBUS,
	IDX_GTG_REF,
	IDX_USB3_DUAL_REF,
	IDX_USB0_BUS_REF,
	IDX_USB1_BUS_REF,
	IDX_GEM_REF,
	IDX_QSPI_REF,
	IDX_SDIO_REF,
	IDX_UART_REF,
	IDX_SPI_REF,
	IDX_CAN_REF,
	IDX_CPU_R5,
	IDX_IOU_SWITCH,
	IDX_CSU_PLL,
	IDX_PCAP,
	IDX_LPD_SWITCH,
	IDX_LPD_LSBUS,
	IDX_NAND_REF,
	IDX_ADMA_REF,
	IDX_GEM_TSU_REF,
	IDX_AMS_REF,
	IDX_I2C_REF,
	IDX_PERIPH_MAX
} XClock_PeriParentsIndices;

/************************** Variable Definitions *****************************/
/* Mux Parents */
static u16 PllSrcParents[XCLOCK_NUM_PLL_SRC_PARENTS];
static u16 PllIntMuxParents[XCLOCK_NUM_PLL][XCLOCK_NUM_PLL_PARENTS];
static u16 PllParents[XCLOCK_NUM_PLL][XCLOCK_NUM_PLL_PARENTS];
static u16 PeriParents[IDX_PERIPH_MAX][XCLOCK_NUM_PERI_PARENTS];
static u16 DdrParents[XCLOCK_NUM_DDR_PARENTS];
static u16 WdtParents[XCLOCK_NUM_WDT_PARENTS];
static u16 TstmpParents[XCLOCK_NUM_TSTMP_PARENTS];
static u16 GemParents[XCLOCK_NUM_GEM][XCLOCK_NUM_GEM_PARENTS];
static u16 GemTsuParents[XCLOCK_NUM_GEM_TSU_PARENTS];
static u16 CanParents[XCLOCK_NUM_CAN][XCLOCK_NUM_CAN_PARENTS];
static u16 CanMioParents[XCLOCK_NUM_CAN_MIO_PARENTS];
static u16 DllRefParents[XCLOCK_NUM_DLL_REF_PARENTS];

/* Output clock mapped to muxes */
static XClock_MuxMappings MuxMap[] = {
	XCLOCK_MUX_ASSIGN_INDICES(GEM_TSU, GEM_TSU),
	XCLOCK_MUX_ASSIGN_INDICES(CAN0, CAN0),
	XCLOCK_MUX_ASSIGN_INDICES(CAN1, CAN1),
	XCLOCK_MUX_ASSIGN_INDICES(DLL_REF, DLL_REF),
	XCLOCK_MUX_ASSIGN_INDICES(WDT, WDT),
};

/* Muxes database */
static XClock_TypeMux Muxes[] = {
	XCLOCK_ASSIGN_MUXES_P8
	(PllSrcParents,                  IOPLL_CTRL,         20, 3),
	XCLOCK_ASSIGN_MUXES_P2
	(PllIntMuxParents[IOPLL],        IOPLL_CTRL,         16, 1),
	XCLOCK_ASSIGN_MUXES_P8
	(PllSrcParents,                  IOPLL_CTRL,         24, 3),
	XCLOCK_ASSIGN_MUXES_P2
	(PllParents[IOPLL],              IOPLL_CTRL,         3,  1),
	XCLOCK_ASSIGN_MUXES_P8
	(PllSrcParents,                  RPLL_CTRL,          20, 3),
	XCLOCK_ASSIGN_MUXES_P2
	(PllIntMuxParents[RPLL],         RPLL_CTRL,          16, 1),
	XCLOCK_ASSIGN_MUXES_P8
	(PllSrcParents,                  RPLL_CTRL,          24, 3),
	XCLOCK_ASSIGN_MUXES_P2
	(PllParents[RPLL],               RPLL_CTRL,          3,  1),
	XCLOCK_ASSIGN_MUXES_P8
	(PllSrcParents,                  APLL_CTRL,          20, 3),
	XCLOCK_ASSIGN_MUXES_P2
	(PllIntMuxParents[APLL],         APLL_CTRL,          16, 1),
	XCLOCK_ASSIGN_MUXES_P8
	(PllSrcParents,                  APLL_CTRL,          24, 3),
	XCLOCK_ASSIGN_MUXES_P2
	(PllParents[APLL],               APLL_CTRL,          3,  1),
	XCLOCK_ASSIGN_MUXES_P8
	(PllSrcParents,                  DPLL_CTRL,          20, 3),
	XCLOCK_ASSIGN_MUXES_P2
	(PllIntMuxParents[DPLL],         DPLL_CTRL,          16, 1),
	XCLOCK_ASSIGN_MUXES_P8
	(PllSrcParents,                  DPLL_CTRL,          24, 3),
	XCLOCK_ASSIGN_MUXES_P2
	(PllParents[DPLL],               DPLL_CTRL,          3,  1),
	XCLOCK_ASSIGN_MUXES_P8
	(PllSrcParents,                  VPLL_CTRL,          20, 3),
	XCLOCK_ASSIGN_MUXES_P2
	(PllIntMuxParents[VPLL],         VPLL_CTRL,          16, 1),
	XCLOCK_ASSIGN_MUXES_P8
	(PllSrcParents,                  VPLL_CTRL,          24, 3),
	XCLOCK_ASSIGN_MUXES_P2
	(PllParents[VPLL],               VPLL_CTRL,          3,  1),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_ACPU],          ACPU_CTRL,          0,  3),
	XCLOCK_ASSIGN_MUXES_P2
	(WdtParents,                     WDT_CLK_SEL,        0,  1),
	XCLOCK_ASSIGN_MUXES_P2
	(DdrParents,                     DDR_CTRL,           0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_PL],            PL0_REF_CTRL,       0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_PL],            PL1_REF_CTRL,       0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_PL],            PL2_REF_CTRL,       0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_PL],            PL3_REF_CTRL,       0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_DBG_FPD],       DBG_TRACE_CTRL,     0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_DBG_FPD],       DBG_FPD_CTRL,       0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_DBG_LPD],       DBG_LPD_CTRL,       0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_DBG_FPD],       DBG_TSTMP_CTRL,     0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_DP],            DP_VIDEO_REF_CTRL,  0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_DP],            DP_AUDIO_REF_CTRL,  0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_DP],            DP_STC_REF_CTRL,    0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_GPU_REF],       GPU_REF_CTRL,       0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_SATA_REF],      SATA_REF_CTRL,      0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_PCIE_REF],      PCIE_REF_CTRL,      0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_DMA_REF],       FPDDMA_REF_CTRL,    0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_DMA_REF],       DPDMA_REF_CTRL,     0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_TOPSW_MAIN],    TOPSW_MAIN_CTRL,    0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_TOPSW_LSBUS],   TOPSW_LSBUS_CTRL,   0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_GTG_REF],       GTGREF0_REF_CTRL,   0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_USB3_DUAL_REF], USB3_DUAL_REF_CTRL, 0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_USB0_BUS_REF],  USB0_BUS_REF_CTRL,  0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_USB1_BUS_REF],  USB1_BUS_REF_CTRL,  0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_GEM_REF],       GEM0_REF_CTRL,      0,  3),
	XCLOCK_ASSIGN_MUXES_P2
	(GemParents[0],                  GEM_CLK_CTRL,       1,  1),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_GEM_REF],       GEM1_REF_CTRL,      0,  3),
	XCLOCK_ASSIGN_MUXES_P2
	(GemParents[1],                  GEM_CLK_CTRL,       6,  1),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_GEM_REF],       GEM2_REF_CTRL,      0,  3),
	XCLOCK_ASSIGN_MUXES_P2
	(GemParents[2],                  GEM_CLK_CTRL,       11, 1),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_GEM_REF],       GEM3_REF_CTRL,      0,  3),
	XCLOCK_ASSIGN_MUXES_P2
	(GemParents[3],                  GEM_CLK_CTRL,       16, 1),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_GEM_TSU_REF],   GEM_TSU_REF_CTRL,   0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(GemTsuParents,                  GEM_CLK_CTRL,       20, 2),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_QSPI_REF],      QSPI_REF_CTRL,      0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_SDIO_REF],      SDIO0_REF_CTRL,     0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_SDIO_REF],      SDIO1_REF_CTRL,     0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_UART_REF],      UART0_REF_CTRL,     0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_UART_REF],      UART1_REF_CTRL,     0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_SPI_REF],       SPI0_REF_CTRL,      0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_SPI_REF],       SPI1_REF_CTRL,      0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_CAN_REF],       CAN0_REF_CTRL,      0,  3),
	XCLOCK_ASSIGN_MUXES
	(CanMioParents, XCLOCK_NUM_CAN_MIO_PARENTS, CAN_CLK_CTRL, 0, 7),
	XCLOCK_ASSIGN_MUXES_P2
	(CanParents[0],                  CAN_CLK_CTRL,       7,  1),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_CAN_REF],       CAN1_REF_CTRL,      0,  3),
	XCLOCK_ASSIGN_MUXES
	(CanMioParents, XCLOCK_NUM_CAN_MIO_PARENTS, CAN_CLK_CTRL, 15, 7),
	XCLOCK_ASSIGN_MUXES_P2
	(CanParents[1],                  CAN_CLK_CTRL,       22, 1),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_CPU_R5],        CPU_R5_CTRL,        0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_IOU_SWITCH],    IOU_SWITCH_CTRL,    0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_CSU_PLL],       CSU_PLL_CTRL,       0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_PCAP],          PCAP_CTRL,          0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_LPD_SWITCH],    LPD_SWITCH_CTRL,    0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_LPD_LSBUS],     LPD_LBUS_CTRL,      0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_NAND_REF],      NAND_REF_CTRL,      0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_ADMA_REF],      LPDDMA_REF_CTRL,    0,  3),
	XCLOCK_ASSIGN_MUXES_P2
	(DllRefParents,                  DLL_REF_CTRL,       0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_AMS_REF],       PSSYSMON_REF_CTRL,  0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_I2C_REF],       I2C0_REF_CTRL,      0,  3),
	XCLOCK_ASSIGN_MUXES_P4
	(PeriParents[IDX_I2C_REF],       I2C1_REF_CTRL,      0,  3),
	XCLOCK_ASSIGN_MUXES_P8
	(TstmpParents,                   TSTMP_REF_CTRL,     0,  3)
};

/*****************************************************************************/
/**
* @brief Allocates peripheral parent IDs for a mux.
*
* This function allocates peripheral parents for the mux based on type of the
* parent.
*
* @param[out] ParentGroup Pointer to mux parent variable.
* @param[in] GroupType Type of group parents belong to.
* @param[in] Par1 Parent belonging to parent group.
* @param[in] Par2 Parent belonging to parent group.
* @param[in] Par3 Parent belonging to parent group.
*
* @note		Here the bits in group type indicate the type of parent.
*		Specific bit corresponds to specific parent as in Bit 0
*		indicates type for parent 1, Bit 1 for parent 2 and Bit 2 for
*		parent 3. If the bit is set then the parent corresponds to
*		PLL, else the parent corresponds to divider.
*
******************************************************************************/
static void XClock_AllocPeriphMuxParents(u16 *ParentGroup, u8 GroupType,
		u8 Par1, u8 Par2, u8 Par3)
{
	if (GroupType & BIT(0)) {
		ParentGroup[0] = XCLOCK_GENERATE_PARENT_ID(XCLOCK_TYPE_MUX,
				 Par1);
	} else {
		ParentGroup[0] = XCLOCK_GENERATE_PARENT_ID(XCLOCK_TYPE_DIVIDER,
				 Par1);
	}

	ParentGroup[1] = XCLOCK_INVALID_PARENT;

	if (GroupType & BIT(1)) {
		ParentGroup[2] = XCLOCK_GENERATE_PARENT_ID(XCLOCK_TYPE_MUX,
				 Par2);
	} else {
		ParentGroup[2] = XCLOCK_GENERATE_PARENT_ID(XCLOCK_TYPE_DIVIDER,
				 Par2);
	}

	if (GroupType & BIT(2)) {
		ParentGroup[3] = XCLOCK_GENERATE_PARENT_ID(XCLOCK_TYPE_MUX,
				 Par3);
	} else {
		ParentGroup[3] = XCLOCK_GENERATE_PARENT_ID(XCLOCK_TYPE_DIVIDER,
				 Par3);
	}
}

/*****************************************************************************/
/**
* @brief Fetches the default parent of a mux.
*
* This function fetches current default parent for the mux specified by
* mux index.
*
* @param[in] NodeIdx Index of mux in database for which parent is to be fetched.
*
* @return	Index of default parent.
*
* @note		This function returns XCLOCK_INVALID_PARENT in case invalid
*		node index occurs.
*
******************************************************************************/
static u8 XClock_MuxFetchDefaultParent(u8 NodeIdx)
{
	u8  Parent;
	u8  Shift;
	u32 Value;
	u32 Reg;

	switch (NodeIdx) {
		case IOPLL_INT_MUX:
			Reg = IOPLL_CTRL;
			Shift = 16;
			break;
		case IOPLL_MUX:
			Reg = IOPLL_CTRL;
			Shift = 3;
			break;
		case RPLL_INT_MUX:
			Reg = RPLL_CTRL;
			Shift = 16;
			break;
		case RPLL_MUX:
			Reg = RPLL_CTRL;
			Shift = 3;
			break;
		case APLL_INT_MUX:
			Reg = APLL_CTRL;
			Shift = 16;
			break;
		case APLL_MUX:
			Reg = APLL_CTRL;
			Shift = 3;
			break;
		case DPLL_INT_MUX:
			Reg = DPLL_CTRL;
			Shift = 16;
			break;
		case DPLL_MUX:
			Reg = DPLL_CTRL;
			Shift = 3;
			break;
		case VPLL_INT_MUX:
			Reg = VPLL_CTRL;
			Shift = 16;
			break;
		case VPLL_MUX:
			Reg = VPLL_CTRL;
			Shift = 3;
			break;
		default:
			return ((u8)XCLOCK_INVALID_PARENT);
			break;
	}

	if (XST_SUCCESS != XClock_ReadReg(Reg, &Value)) {
		Parent = (u8)XCLOCK_INVALID_PARENT;
	} else {
		Parent = (Value & BIT(Shift)) >> Shift;
	}

	return Parent;
}

/*****************************************************************************/
/**
* @brief Allocates parents for supported muxes.
*
* This function allocates parents for supported peripherals.
*
* @note		This function must be called before calling any other function
*		related to mux operations. Without call to this function no
*		mux has parent information.
*
******************************************************************************/
static void XClock_AllocMuxParents(void)
{
	u8  NodeIdx;
	u32 Idx;

	/* Allocate PLL Source parents */
	PllSrcParents[0] = XCLOCK_GENERATE_PARENT_ID
			   (XCLOCK_TYPE_IP, PSS_REF_CLK);
	PllSrcParents[1] = XCLOCK_GENERATE_PARENT_ID
			   (XCLOCK_TYPE_IP, PSS_REF_CLK);
	PllSrcParents[2] = XCLOCK_GENERATE_PARENT_ID
			   (XCLOCK_TYPE_IP, PSS_REF_CLK);
	PllSrcParents[3] = XCLOCK_GENERATE_PARENT_ID
			   (XCLOCK_TYPE_IP, PSS_REF_CLK);
	PllSrcParents[4] = XCLOCK_GENERATE_PARENT_ID
			   (XCLOCK_TYPE_IP, VIDEO_CLK);
	PllSrcParents[5] = XCLOCK_GENERATE_PARENT_ID
			   (XCLOCK_TYPE_IP, PSS_ALT_REF_CLK);
	PllSrcParents[6] = XCLOCK_GENERATE_PARENT_ID
			   (XCLOCK_TYPE_IP, AUX_REF_CLK);
	PllSrcParents[7] = XCLOCK_GENERATE_PARENT_ID
			   (XCLOCK_TYPE_IP, GT_CRX_REF_CLK);

	/* Allocate PLL mux parents */
	for (Idx = 0; Idx < XCLOCK_NUM_PLL; Idx++) {
		PllIntMuxParents[Idx][0] = XCLOCK_GENERATE_PARENT_ID
					   (XCLOCK_TYPE_PLL, IOPLL_INT_PLL + Idx);
		PllIntMuxParents[Idx][1] = XCLOCK_GENERATE_PARENT_ID
					   (XCLOCK_TYPE_FIXEDFACTOR, IOPLL_INT_HALF_FF + Idx);

		/* Set default parent */
		NodeIdx = IOPLL_INT_MUX + (Idx * XCLOCK_PLL_NXT_REG_OFFSET);
		Muxes[NodeIdx].DefaultParent =
			XClock_MuxFetchDefaultParent(NodeIdx);

		PllParents[Idx][0] = XCLOCK_GENERATE_PARENT_ID
				     (XCLOCK_TYPE_MUX, IOPLL_INT_MUX +
				      (XCLOCK_PLL_NXT_REG_OFFSET * Idx));
		PllParents[Idx][1] = XCLOCK_GENERATE_PARENT_ID
				     (XCLOCK_TYPE_MUX, IOPLL_POST_SRC_MUX +
				      (XCLOCK_PLL_NXT_REG_OFFSET * Idx));

		/* Set default parent */
		NodeIdx = IOPLL_MUX + (Idx * XCLOCK_PLL_NXT_REG_OFFSET);
		Muxes[NodeIdx].DefaultParent =
			XClock_MuxFetchDefaultParent(NodeIdx);
	}

	/* Allocate DDR parents */
	DdrParents[0] = XCLOCK_GENERATE_PARENT_ID
			(XCLOCK_TYPE_MUX, DPLL_INT_MUX);
	DdrParents[1] = XCLOCK_GENERATE_PARENT_ID
			(XCLOCK_TYPE_PLL, VPLL_INT_MUX);

	/* Allocate watchdog parents */
	WdtParents[0] = XCLOCK_GENERATE_PARENT_ID
			(XCLOCK_TYPE_GATE, TOPSW_LSBUS_GATE);
	WdtParents[1] = XCLOCK_PARENT_EXTERNAL;

	/* Allocate timestamp mux parents */
	TstmpParents[0] = XCLOCK_GENERATE_PARENT_ID
			  (XCLOCK_TYPE_MUX, IOPLL_INT_MUX);
	TstmpParents[1] = XCLOCK_INVALID_PARENT;
	TstmpParents[2] = XCLOCK_GENERATE_PARENT_ID
			  (XCLOCK_TYPE_MUX, RPLL_INT_MUX);
	TstmpParents[3] = XCLOCK_GENERATE_PARENT_ID
			  (XCLOCK_TYPE_DIVIDER, DPLL_TO_LPD_DIV0);
	TstmpParents[4] = XCLOCK_GENERATE_PARENT_ID
			  (XCLOCK_TYPE_IP, PSS_REF_CLK);
	TstmpParents[5] = XCLOCK_GENERATE_PARENT_ID
			  (XCLOCK_TYPE_IP, PSS_REF_CLK);
	TstmpParents[6] = XCLOCK_GENERATE_PARENT_ID
			  (XCLOCK_TYPE_IP, PSS_REF_CLK);
	TstmpParents[7] = XCLOCK_GENERATE_PARENT_ID
			  (XCLOCK_TYPE_IP, PSS_REF_CLK);

	/* Allocate gem parents */
	for (Idx = 0; Idx < XCLOCK_NUM_GEM; Idx++) {
		GemParents[Idx][0] = XCLOCK_GENERATE_PARENT_ID
				     (XCLOCK_TYPE_DIVIDER, (GEM0_REF_DIV1 +
						     (XCLOCK_GEM_NXT_REG_OFFSET * Idx)));
		GemParents[Idx][1] = XCLOCK_PARENT_EXTERNAL;
	}

	/* Allocate gem tsu parents */
	GemTsuParents[0] = XCLOCK_GENERATE_PARENT_ID
			   (XCLOCK_TYPE_GATE, GEM_TSU_REF_GATE);
	GemTsuParents[1] = XCLOCK_GENERATE_PARENT_ID
			   (XCLOCK_TYPE_GATE, GEM_TSU_REF_GATE);
	GemTsuParents[2] = XCLOCK_PARENT_EXTERNAL;
	GemTsuParents[3] = XCLOCK_PARENT_EXTERNAL;

	/* Allocate can parents  */
	for (Idx = 0; Idx < XCLOCK_NUM_CAN; Idx++) {
		CanParents[Idx][0] = XCLOCK_GENERATE_PARENT_ID
				     (XCLOCK_TYPE_GATE, (CAN0_REF_GATE + Idx));
		CanParents[Idx][1] = XCLOCK_GENERATE_PARENT_ID
				     (XCLOCK_TYPE_MUX, (CAN0_MIO_MUX +
							XCLOCK_CAN_NXT_REG_OFFSET));
	}

	/* Allocate can mio parents */
	for (Idx = 0; Idx < XCLOCK_NUM_CAN_MIO_PARENTS; Idx++) {
		CanMioParents[Idx] = XCLOCK_PARENT_EXTERNAL;
	}

	/* Allocate dll ref parents */
	DllRefParents[0] = XCLOCK_GENERATE_PARENT_ID
			   (XCLOCK_TYPE_MUX, IOPLL_INT_MUX);
	DllRefParents[1] = XCLOCK_GENERATE_PARENT_ID
			   (XCLOCK_TYPE_MUX, RPLL_INT_MUX);

	/* Allocate Peripheral parents */
	XClock_AllocPeriphMuxParents(PeriParents[IDX_PL],
				     XCLOCK_PARENT_MUXS_DIV,    IOPLL_INT_MUX,
				     RPLL_INT_MUX,     DPLL_TO_LPD_DIV0);
	XClock_AllocPeriphMuxParents(PeriParents[IDX_ACPU],
				     XCLOCK_PARENT_ALL_MUXS,    APLL_INT_MUX,
				     DPLL_INT_MUX,     VPLL_INT_MUX);
	XClock_AllocPeriphMuxParents(PeriParents[IDX_DBG_FPD],
				     XCLOCK_PARENT_DIV_MUXS,    IOPLL_TO_FPD_DIV0,
				     DPLL_INT_MUX,     APLL_INT_MUX);
	XClock_AllocPeriphMuxParents(PeriParents[IDX_DBG_LPD],
				     XCLOCK_PARENT_MUXS_DIV,    RPLL_INT_MUX,
				     IOPLL_INT_MUX,    DPLL_TO_LPD_DIV0);
	XClock_AllocPeriphMuxParents(PeriParents[IDX_DP],
				     XCLOCK_PARENT_MUXS_DIV,    VPLL_INT_MUX,
				     DPLL_INT_MUX,     RPLL_TO_FPD_DIV0);
	XClock_AllocPeriphMuxParents(PeriParents[IDX_GPU_REF],
				     XCLOCK_PARENT_DIV_MUXS,    IOPLL_TO_FPD_DIV0,
				     VPLL_INT_MUX,     DPLL_INT_MUX);
	XClock_AllocPeriphMuxParents(PeriParents[IDX_SATA_REF],
				     XCLOCK_PARENT_DIV_MUXS,    IOPLL_TO_FPD_DIV0,
				     APLL_INT_MUX,     DPLL_INT_MUX);
	XClock_AllocPeriphMuxParents(PeriParents[IDX_PCIE_REF],
				     XCLOCK_PARENT_DIVS_MUX,    IOPLL_TO_FPD_DIV0,
				     RPLL_TO_FPD_DIV0,  DPLL_INT_MUX);
	XClock_AllocPeriphMuxParents(PeriParents[IDX_DMA_REF],
				     XCLOCK_PARENT_ALL_MUXS,    APLL_INT_MUX,
				     VPLL_INT_MUX,     DPLL_INT_MUX);
	XClock_AllocPeriphMuxParents(PeriParents[IDX_TOPSW_MAIN],
				     XCLOCK_PARENT_ALL_MUXS,    APLL_INT_MUX,
				     VPLL_INT_MUX,     DPLL_INT_MUX);
	XClock_AllocPeriphMuxParents(PeriParents[IDX_TOPSW_LSBUS],
				     XCLOCK_PARENT_MUX_DIV_MUX, APLL_INT_MUX,
				     IOPLL_TO_FPD_DIV0, DPLL_INT_MUX);
	XClock_AllocPeriphMuxParents(PeriParents[IDX_GTG_REF],
				     XCLOCK_PARENT_DIV_MUXS,    IOPLL_TO_FPD_DIV0,
				     APLL_INT_MUX,     DPLL_INT_MUX);
	XClock_AllocPeriphMuxParents(PeriParents[IDX_USB3_DUAL_REF],
				     XCLOCK_PARENT_MUXS_DIV,    IOPLL_INT_MUX,
				     RPLL_INT_MUX,     DPLL_TO_LPD_DIV0);
	XClock_AllocPeriphMuxParents(PeriParents[IDX_USB0_BUS_REF],
				     XCLOCK_PARENT_MUXS_DIV,    IOPLL_INT_MUX,
				     RPLL_INT_MUX,     DPLL_TO_LPD_DIV0);
	XClock_AllocPeriphMuxParents(PeriParents[IDX_USB1_BUS_REF],
				     XCLOCK_PARENT_MUXS_DIV,    IOPLL_INT_MUX,
				     APLL_INT_MUX,     DPLL_TO_LPD_DIV0);
	XClock_AllocPeriphMuxParents(PeriParents[IDX_GEM_REF],
				     XCLOCK_PARENT_MUXS_DIV,    IOPLL_INT_MUX,
				     RPLL_INT_MUX,     DPLL_TO_LPD_DIV0);
	XClock_AllocPeriphMuxParents(PeriParents[IDX_QSPI_REF],
				     XCLOCK_PARENT_MUXS_DIV,    IOPLL_INT_MUX,
				     RPLL_INT_MUX,     DPLL_TO_LPD_DIV0);
	XClock_AllocPeriphMuxParents(PeriParents[IDX_SDIO_REF],
				     XCLOCK_PARENT_MUXS_DIV,    IOPLL_INT_MUX,
				     RPLL_INT_MUX,     VPLL_TO_LPD_DIV0);
	XClock_AllocPeriphMuxParents(PeriParents[IDX_UART_REF],
				     XCLOCK_PARENT_MUXS_DIV,    IOPLL_INT_MUX,
				     RPLL_INT_MUX,     DPLL_TO_LPD_DIV0);
	XClock_AllocPeriphMuxParents(PeriParents[IDX_SPI_REF],
				     XCLOCK_PARENT_MUXS_DIV,    IOPLL_INT_MUX,
				     RPLL_INT_MUX,     DPLL_TO_LPD_DIV0);
	XClock_AllocPeriphMuxParents(PeriParents[IDX_CAN_REF],
				     XCLOCK_PARENT_MUXS_DIV,    IOPLL_INT_MUX,
				     RPLL_INT_MUX,     DPLL_TO_LPD_DIV0);
	XClock_AllocPeriphMuxParents(PeriParents[IDX_CPU_R5],
				     XCLOCK_PARENT_MUXS_DIV,    RPLL_INT_MUX,
				     IOPLL_INT_MUX,    DPLL_TO_LPD_DIV0);
	XClock_AllocPeriphMuxParents(PeriParents[IDX_IOU_SWITCH],
				     XCLOCK_PARENT_MUXS_DIV,    RPLL_INT_MUX,
				     IOPLL_INT_MUX,    DPLL_TO_LPD_DIV0);
	XClock_AllocPeriphMuxParents(PeriParents[IDX_CSU_PLL],
				     XCLOCK_PARENT_MUXS_DIV,    IOPLL_INT_MUX,
				     RPLL_INT_MUX,     DPLL_TO_LPD_DIV0);
	XClock_AllocPeriphMuxParents(PeriParents[IDX_PCAP],
				     XCLOCK_PARENT_MUXS_DIV,    IOPLL_INT_MUX,
				     RPLL_INT_MUX,     DPLL_TO_LPD_DIV0);
	XClock_AllocPeriphMuxParents(PeriParents[IDX_LPD_SWITCH],
				     XCLOCK_PARENT_MUXS_DIV,    RPLL_INT_MUX,
				     IOPLL_INT_MUX,    DPLL_TO_LPD_DIV0);
	XClock_AllocPeriphMuxParents(PeriParents[IDX_LPD_LSBUS],
				     XCLOCK_PARENT_MUXS_DIV,    RPLL_INT_MUX,
				     IOPLL_INT_MUX,    DPLL_TO_LPD_DIV0);
	XClock_AllocPeriphMuxParents(PeriParents[IDX_NAND_REF],
				     XCLOCK_PARENT_MUXS_DIV,    IOPLL_INT_MUX,
				     RPLL_INT_MUX,     DPLL_TO_LPD_DIV0);
	XClock_AllocPeriphMuxParents(PeriParents[IDX_ADMA_REF],
				     XCLOCK_PARENT_MUXS_DIV,    RPLL_INT_MUX,
				     IOPLL_INT_MUX,    DPLL_TO_LPD_DIV0);
	XClock_AllocPeriphMuxParents(PeriParents[IDX_GEM_TSU_REF],
				     XCLOCK_PARENT_MUXS_DIV,    IOPLL_INT_MUX,
				     RPLL_INT_MUX,     DPLL_TO_LPD_DIV0);
	XClock_AllocPeriphMuxParents(PeriParents[IDX_AMS_REF],
				     XCLOCK_PARENT_MUXS_DIV,    RPLL_INT_MUX,
				     IOPLL_INT_MUX,    DPLL_TO_LPD_DIV0);
	XClock_AllocPeriphMuxParents(PeriParents[IDX_I2C_REF],
				     XCLOCK_PARENT_MUXS_DIV,    IOPLL_INT_MUX,
				     RPLL_INT_MUX,     DPLL_TO_LPD_DIV0);

	return;
};

/*****************************************************************************/
/**
* @brief Sets the active parent of a mux.
*
* @param[in] MuxIndex Database index of mux to look parent for.
* @param[in] SetParentIdx Index of parent to set for mux.
*
* @return
*		- XST_SUCCESS if successful. XCLOCK_INVALID_PARAM for invalid
*		function arguments.
*		- XST_FAILURE if read/write fails.
*
******************************************************************************/
static XStatus XClock_MuxSetActiveParent(u8 MuxIndex, u8 SetParentIdx)
{
	u8  ParentType;
	u8  ParentIdx;
	u16 *ParentPtr;
	u32 Value;
	u32 ParentMask;

	/* Validate arguments */
	XCLOCK_VALIDATE_INDEX(MUX, MuxIndex);
	if (SetParentIdx > Muxes[MuxIndex].NumParents) {
		return XST_INVALID_PARAM;
	}

	ParentPtr = Muxes[MuxIndex].Parent;

	if (XCLOCK_INVALID_PARENT == ParentPtr[SetParentIdx]) {
		/* Invalid parent */
		return XST_INVALID_PARAM;
	}

	/* Enable the parent */
	if (XCLOCK_PARENT_EXTERNAL != ParentPtr[SetParentIdx]) {
		ParentType = XCLOCK_FETCH_PARENT_TYPE(ParentPtr[SetParentIdx]);
		ParentIdx = XCLOCK_FETCH_PARENT_INDEX(ParentPtr[SetParentIdx]);

		if (XST_SUCCESS !=
		    XClock_EnableClkNode((XClock_Types)ParentType, ParentIdx)) {
			return XST_FAILURE;
		}
	}

	/* Set parent */
	if (XST_SUCCESS != XClock_ReadReg(Muxes[MuxIndex].CtrlReg, &Value)) {
		return XST_FAILURE;
	}

	ParentMask = ((1 << Muxes[MuxIndex].Width) - 1) <<
		     Muxes[MuxIndex].Shift;
	Value &= ~(ParentMask);
	Value |= (SetParentIdx << Muxes[MuxIndex].Shift);
	if (XST_SUCCESS != XClock_WriteReg(Muxes[MuxIndex].CtrlReg, Value)) {
		return XST_FAILURE;
	}

	Muxes[MuxIndex].ActiveParent = SetParentIdx;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Initializes a mux node.
*
* Initialize mux node.
*
* @param[in] MuxIndex Database index of mux to initialize.
*
******************************************************************************/
static void XClock_MuxInit(u8 MuxIndex)
{
	u8  ParentType;
	u8  ParentIdx;
	u16 Parent;

	XCLOCK_VALIDATE_INDEX_WARN(MUX, MuxIndex);

	if (!Muxes[MuxIndex].IsInit) {
		Parent = Muxes[MuxIndex].Parent[Muxes[MuxIndex].DefaultParent];

		/* Ignore if external parent */
		if (XCLOCK_PARENT_EXTERNAL == Parent) {
			return;
		}

		/* Init parent */
		ParentType = XCLOCK_FETCH_PARENT_TYPE(Parent);
		ParentIdx = XCLOCK_FETCH_PARENT_INDEX(Parent);
		XClock_InitClk((XClock_Types)ParentType, ParentIdx);

		/* Set rate */
		Muxes[MuxIndex].Rate = XClock_FetchRate((XClock_Types)ParentType, ParentIdx);
		Muxes[MuxIndex].ActiveParent = Muxes[MuxIndex].DefaultParent;
		Muxes[MuxIndex].IsInit = TRUE;
	}
}

/*****************************************************************************/
/**
* @brief Enables a mux node.
*
* Enable function for the mux.
*
* @param[in] MuxIndex Database index of mux to enable.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_INVALID_PARAM if arguments are invalid.
*		- XST_FAILURE otherwise.
*
******************************************************************************/
static XStatus XClock_MuxEnable(u8 MuxIndex)
{
	u8  ParentType;
	u8  ParentIdx;
	u16 Parent;

	/* Validate arguments */
	XCLOCK_VALIDATE_INDEX(MUX, MuxIndex);

	/* Check for parent */
	if (((u8)XCLOCK_INVALID_PARENT) == Muxes[MuxIndex].ActiveParent) {
		Muxes[MuxIndex].ActiveParent = Muxes[MuxIndex].DefaultParent;
		Parent = Muxes[MuxIndex].Parent[Muxes[MuxIndex].DefaultParent];
	} else {
		Parent = Muxes[MuxIndex].Parent[Muxes[MuxIndex].ActiveParent];
	}

	if (XCLOCK_PARENT_EXTERNAL != Parent) {
		ParentType = XCLOCK_FETCH_PARENT_TYPE(Parent);
		ParentIdx = XCLOCK_FETCH_PARENT_INDEX(Parent);

		/* Enable parent node */
		if (XST_SUCCESS !=
		    XClock_EnableClkNode((XClock_Types)ParentType, ParentIdx)) {
			return XST_FAILURE;
		}
	}

	Muxes[MuxIndex].EnableCount++;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Disables a mux node.
*
* Disable function for the mux.
*
* @param[in] MuxIndex Database index of mux to disable.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_INVALID_PARAM if arguments are invalid.
*		- XST_FAILURE otherwise.
*
******************************************************************************/
static XStatus XClock_MuxDisable(u8 MuxIndex)
{
	u8  ParentType;
	u8  ParentIdx;
	u16 Parent;

	/* Validate arguments */
	XCLOCK_VALIDATE_INDEX(MUX, MuxIndex);

	/* Check enabled status */
	if (!Muxes[MuxIndex].EnableCount) {
		return XST_SUCCESS;
	}

	/* check for parent */
	Parent = Muxes[MuxIndex].Parent[Muxes[MuxIndex].ActiveParent];

	if (((u8)XCLOCK_INVALID_PARENT) == Muxes[MuxIndex].ActiveParent) {
		if (!Muxes[MuxIndex].EnableCount) {
			return XST_FAILURE;
		} else {
			return XST_SUCCESS;
		}
	}

	if (XCLOCK_PARENT_EXTERNAL != Parent) {
		ParentType = XCLOCK_FETCH_PARENT_TYPE(Parent);
		ParentIdx = XCLOCK_FETCH_PARENT_INDEX(Parent);

		/* Disable parent node */
		if (XST_SUCCESS !=
		    XClock_DisableClkNode((XClock_Types)ParentType, ParentIdx)) {
			return XST_FAILURE;
		}
	}

	Muxes[MuxIndex].EnableCount--;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Looks up the mux index for an output clock.
*
* This function looks for the output clock mapped to mux and returns the
* mux index.
*
* @param[in] ClockId Identifier for output clock.
* @param[out] MuxIndex Index of output clock in mux database.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_INVALID_PARAM if output clock mapping not found.
*
******************************************************************************/
static XStatus XClock_MuxFetchIdx(XClock_OutputClks ClockId, u8 *MuxIndex)
{
	u8 Idx;

	/* Validate Args */
	XCLOCK_VALIDATE_INDEX(OP, ClockId);
	XCLOCK_VALIDATE_PTR(MuxIndex);

	for (Idx = 0; Idx < CLK_ARRAY_SIZE(MuxMap); Idx++) {
		if (MuxMap[Idx].ClockId == ClockId) {
			*MuxIndex = MuxMap[Idx].MuxIndex;
			return XST_SUCCESS;
		}
	}

	return XST_INVALID_PARAM;
}

/*****************************************************************************/
/**
* @brief Fetches the parent of a mux node.
*
* This function fetches the parent based on index in database.
*
* @param[out] NodeType Pointer receiving the parent node type.
* @param[in,out] MuxIndex Pointer holding index of parent in database.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_INVALID_PARAM if output clock mapping not found.
*
******************************************************************************/
static XStatus XClock_MuxFetchParent(XClock_Types *NodeType, u8 *MuxIndex)
{
	u16 Parent;
	u8  ActiveParent;

	/* Validate Args */
	XCLOCK_VALIDATE_PTR(NodeType);
	XCLOCK_VALIDATE_PTR(MuxIndex);
	XCLOCK_VALIDATE_INDEX(MUX, *MuxIndex);

	/* Fall back to default parent if active parent was never set */
	ActiveParent = Muxes[*MuxIndex].ActiveParent;
	if (((u8)XCLOCK_INVALID_PARENT) == ActiveParent) {
		ActiveParent = Muxes[*MuxIndex].DefaultParent;
	}

	/* Validate parent index before using it to index Parent[] */
	if (ActiveParent >= Muxes[*MuxIndex].NumParents) {
		return XST_INVALID_PARAM;
	}

	Parent = Muxes[*MuxIndex].Parent[ActiveParent];

	/* Reject sentinel encodings; they do not decode to a real node */
	if ((XCLOCK_PARENT_EXTERNAL == Parent) ||
	    (XCLOCK_INVALID_PARENT == Parent)) {
		return XST_INVALID_PARAM;
	}

	*NodeType = (XClock_Types)XCLOCK_FETCH_PARENT_TYPE(Parent);
	*MuxIndex = XCLOCK_FETCH_PARENT_INDEX(Parent);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Gets a mux rate.
*
* Get mux rate.
*
* @param[in] MuxIndex Database index of mux to fetch rate for.
* @param[out] GetRate Pointer to variable holding mux rate.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_INVALID_PARAM for invalid function arguments.
*
******************************************************************************/
static XStatus XClock_MuxGetRate(u8 MuxIndex, XClockRate *GetRate)
{
	/* Validate args */
	XCLOCK_VALIDATE_INDEX(MUX, MuxIndex);
	XCLOCK_VALIDATE_PTR(GetRate);

	*GetRate = Muxes[MuxIndex].Rate;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Updates the rate of a mux node.
*
* The function updates the rate of mux node.
*
* @param[in] MuxIndex Database index of mux to update rate for.
*
******************************************************************************/
static void XClock_MuxUpdateRate(u8 MuxIndex)
{

	u8  ParentType;
	u8  ParentIdx;
	u16 Parent;

	/* Validate arguments */
	XCLOCK_VALIDATE_INDEX_WARN(MUX, MuxIndex);

	/* Ignore if external parent */
	Parent = Muxes[MuxIndex].Parent[Muxes[MuxIndex].ActiveParent];
	if (XCLOCK_PARENT_EXTERNAL == Parent) {
		return;
	}

	/* Init parent */
	ParentType = XCLOCK_FETCH_PARENT_TYPE(Parent);
	ParentIdx = XCLOCK_FETCH_PARENT_INDEX(Parent);
	XClock_UpdateRate((XClock_Types)ParentType, ParentIdx);

	/* Set rate */
	Muxes[MuxIndex].Rate = XClock_FetchRate((XClock_Types)ParentType, ParentIdx);
}

/*****************************************************************************/
/**
* @brief Registers mux node callback functions.
*
* Register functions for mux node.
*
******************************************************************************/
void XClock_MuxRegisterFuncs(void)
{
	/* Register functions */
	XClock_MuxSetParent                     = &XClock_MuxSetActiveParent;
	XClock_NodeInit[XCLOCK_TYPE_MUX]        = &XClock_MuxInit;
	XClock_NodeEnable[XCLOCK_TYPE_MUX]      = &XClock_MuxEnable;
	XClock_NodeDisable[XCLOCK_TYPE_MUX]     = &XClock_MuxDisable;
	XClock_NodeFetchIdx[XCLOCK_TYPE_MUX]    = &XClock_MuxFetchIdx;
	XClock_NodeFetchParent[XCLOCK_TYPE_MUX] = &XClock_MuxFetchParent;
	XClock_NodeGetRate[XCLOCK_TYPE_MUX]     = &XClock_MuxGetRate;
	XClock_NodeUpdateRate[XCLOCK_TYPE_MUX]  = &XClock_MuxUpdateRate;
}

/*****************************************************************************/
/**
* @brief Initializes all mux nodes.
*
* Begin Initialization of all mux node.
*
* @note		This function loops over all the mux nodes and initializes
*		them.
*
******************************************************************************/
void XClock_MuxBeginInit(void)
{
	u8 Idx;

	/* Allocate mux parents */
	XClock_AllocMuxParents();

	for (Idx = 0; Idx < CLK_ARRAY_SIZE(Muxes); Idx++) {
		XClock_MuxInit(Idx);
	}
}

/** @} */
