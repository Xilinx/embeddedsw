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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/****************************************************************************/
/**
*
* @file xclockps_gate.c
* @addtogroup xclockps_v1_0
* @{
*
* This file handles Gate related definitions and operations
*
* <pre>
* MODIFICATION HISTORY:
* Ver   Who    Date     Changes
* ----- ------ -------- ---------------------------------------------
* 1.00  cjp    02/09/18 First release
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xclockps.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
/* Assign structure elements for output clock mappings */
#define XCLOCK_GATE_ASSIGN_INDICES(ClockIdVal, GateIndexVal) \
{ \
	.ClockId = ClockIdVal, \
	.GateIndex = GateIndexVal \
}

/* Assign structure elements for Gates */
#define XCLOCK_ASSIGN_GATES(ParentTypeVal, GateIndexVal, CtrlRegVal, BitVal) \
{ \
	.BitIndex = BitVal, \
	.EnableCount = 0, \
	.IsInit = FALSE, \
	.Parent = XCLOCK_GENERATE_PARENT_ID(ParentTypeVal, GateIndexVal), \
	.Rate = XCLOCK_INVALID_RATE, \
	.CtrlReg = CtrlRegVal, \
}

/**************************** Type Definitions *******************************/
/* This typedef holds information for output clocks mapped to gates */
typedef struct {
	XClock_OutputClks ClockId;
	XClock_GateIndices GateIndex;
} XClock_GateMappings;

/* This typedef holds information for gates */
typedef struct {
	u8          BitIndex;
	u8          EnableCount;
	u8          IsInit;
	u16         Parent;
	u32         CtrlReg;
	XClockRate  Rate;
} XClock_TypeGate;

/************************** Variable Definitions *****************************/
/* Output clock mapped to gates */
static XClock_GateMappings GateMap[] = {
	XCLOCK_GATE_ASSIGN_INDICES(ACPU,          ACPU_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(ACPU_HALF,     ACPU_HALF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(DBG_FPD,       DBG_FPD_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(DBG_LPD,       DBG_LPD_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(DBG_TRACE,     DBG_TRACE_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(DP_VIDEO_REF,  DP_VIDEO_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(DP_AUDIO_REF,  DP_AUDIO_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(DP_STC_REF,    DP_STC_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(GDMA_REF,      GDMA_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(DPDMA_REF,     DPDMA_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(SATA_REF,      SATA_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(PCIE_REF,      PCIE_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(GPU_REF,       GPU_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(GPU_PP0_REF,   GPU_PP0_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(GPU_PP1_REF,   GPU_PP1_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(TOPSW_MAIN,    TOPSW_MAIN_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(TOPSW_LSBUS,   TOPSW_LSBUS_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(GTGREF0_REF,   GTGREF0_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(LPD_SWITCH,    LPD_SWITCH_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(LPD_LSBUS,     LPD_LSBUS_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(USB0_BUS_REF,  USB0_BUS_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(USB1_BUS_REF,  USB1_BUS_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(USB3_DUAL_REF, USB3_DUAL_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(CPU_R5,        CPU_R5_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(CPU_R5_CORE,   CPU_R5_CORE_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(CSU_PLL,       CSU_PLL_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(PCAP,          PCAP_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(IOU_SWITCH,    IOU_SWITCH_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(GEM_TSU_REF,   GEM_TSU_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(GEM0_RX,       GEM0_TX_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(GEM0_REF,      GEM0_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(GEM1_RX,       GEM1_TX_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(GEM1_REF,      GEM1_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(GEM2_RX,       GEM2_TX_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(GEM2_REF,      GEM2_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(GEM3_RX,       GEM3_TX_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(GEM3_REF,      GEM3_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(QSPI_REF,      QSPI_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(SDIO0_REF,     SDIO0_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(SDIO1_REF,     SDIO1_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(UART0_REF,     UART0_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(UART1_REF,     UART1_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(SPI0_REF,      SPI0_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(SPI1_REF,      SPI1_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(NAND_REF,      NAND_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(I2C0_REF,      I2C0_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(I2C1_REF,      I2C1_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(CAN0_REF,      CAN0_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(CAN1_REF,      CAN1_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(ADMA_REF,      ADMA_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(TIMESTAMP_REF, TIMESTAMP_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(AMS_REF,       AMS_REF_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(PL0,           PL0_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(PL1,           PL1_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(PL2,           PL2_GATE),
	XCLOCK_GATE_ASSIGN_INDICES(PL3,           PL3_GATE),
};

/* Gates database */
static XClock_TypeGate Gates[] = {
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     ACPU_DIV0,          ACPU_CTRL,          24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_FIXEDFACTOR, ACPU_HALF_DIV_FF,   ACPU_CTRL,          25),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     PL0_DIV1,           PL0_REF_CTRL,       24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     PL1_DIV1,           PL1_REF_CTRL,       24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     PL2_DIV1,           PL2_REF_CTRL,       24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     PL3_DIV1,           PL3_REF_CTRL,       24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     DBG_TRACE_DIV0,     DBG_TRACE_CTRL,     24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     DBG_FPD_DIV0,       DBG_FPD_CTRL,       24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     DBG_LPD_DIV0,       DBG_LPD_CTRL,       24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     DP_VIDEO_REF_DIV1,  DP_VIDEO_REF_CTRL,  24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     DP_AUDIO_REF_DIV1,  DP_AUDIO_REF_CTRL,  24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     DP_STC_REF_DIV1,    DP_STC_REF_CTRL,    24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     GPU_REF_DIV0,       GPU_REF_CTRL,       24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     GPU_REF_DIV0,       GPU_REF_CTRL,       25),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     GPU_REF_DIV0,       GPU_REF_CTRL,       26),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     SATA_REF_DIV0,      SATA_REF_CTRL,      24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     PCIE_REF_DIV0,      PCIE_REF_CTRL,      24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     GDMA_REF_DIV0,      FPDDMA_REF_CTRL,    24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     DPDMA_REF_DIV0,     DPDMA_REF_CTRL,     24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     TOPSW_MAIN_DIV0,    TOPSW_MAIN_CTRL,    24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     TOPSW_LSBUS_DIV0,   TOPSW_LSBUS_CTRL,   24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     GTGREF0_REF_DIV0,   GTGREF0_REF_CTRL,   24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     USB3_DUAL_REF_DIV1, USB3_DUAL_REF_CTRL, 25),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     USB0_BUS_REF_DIV1,  USB0_BUS_REF_CTRL,  25),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     USB1_BUS_REF_DIV1,  USB1_BUS_REF_CTRL,  25),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_MUX,         GEM0_TX_MUX,        GEM0_REF_CTRL,      26),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     GEM0_REF_DIV1,      GEM0_REF_CTRL,      25),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_MUX,         GEM1_TX_MUX,        GEM1_REF_CTRL,      26),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     GEM1_REF_DIV1,      GEM1_REF_CTRL,      25),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_MUX,         GEM2_TX_MUX,        GEM2_REF_CTRL,      26),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     GEM2_REF_DIV1,      GEM2_REF_CTRL,      25),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_MUX,         GEM3_TX_MUX,        GEM3_REF_CTRL,      26),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     GEM3_REF_DIV1,      GEM3_REF_CTRL,      25),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     GEM_TSU_REF_DIV1,   GEM_TSU_REF_CTRL,   24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     QSPI_REF_DIV1,      QSPI_REF_CTRL,      24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     SDIO0_REF_DIV1,     SDIO0_REF_CTRL,     24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     SDIO1_REF_DIV1,     SDIO1_REF_CTRL,     24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     UART0_REF_DIV1,     UART0_REF_CTRL,     24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     UART1_REF_DIV1,     UART1_REF_CTRL,     24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     SPI0_REF_DIV1,      SPI0_REF_CTRL,      24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     SPI1_REF_DIV1,      SPI1_REF_CTRL,      24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     CAN0_REF_DIV1,      CAN0_REF_CTRL,      24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     CAN1_REF_DIV1,      CAN1_REF_CTRL,      24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     CPU_R5_DIV0,        CPU_R5_CTRL,        24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     CPU_R5_DIV0,        CPU_R5_CTRL,        25),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     IOU_SWITCH_DIV0,    IOU_SWITCH_CTRL,    24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     CSU_PLL_DIV0,       CSU_PLL_CTRL,       24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     PCAP_DIV0,          PCAP_CTRL,          24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     LPD_SWITCH_DIV0,    LPD_SWITCH_CTRL,    24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     LPD_LSBUS_DIV0,     LPD_LBUS_CTRL,      24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     NAND_REF_DIV1,      NAND_REF_CTRL,      24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     ADMA_REF_DIV0,      LPDDMA_REF_CTRL,    24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     AMS_REF_DIV1,       PSSYSMON_REF_CTRL,  24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     I2C0_REF_DIV1,      I2C0_REF_CTRL,      24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     I2C1_REF_DIV1,      I2C1_REF_CTRL,      24),
	XCLOCK_ASSIGN_GATES
	(XCLOCK_TYPE_DIVIDER,     TIMESTAMP_REF_DIV0, TSTMP_REF_CTRL,     24),
};

/*****************************************************************************/
/*
*
* The function checks if the gate is enabled.
*
* @param	GateIndex is the database index of gate to check for.
* @param	GateState is the pointer to variable holding state of the gate.
*			1 - Gate is enabled.
*			0 - Gate is disabled.
*
* @return	XST_SUCCESS if successful.
*		XST_INVALID_PARAM if invalid arguments.
*
* @note		None.
*
******************************************************************************/
static XStatus XClock_GateIsEnabled(u8 GateIndex, u8 *GateState)
{
	/* Validate arguments */
	XCLOCK_VALIDATE_INDEX(GATE, GateIndex);

	*GateState = !!(Gates[GateIndex].EnableCount);

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* The function enables the gate by writing into control register.
*
* @param	GateIndex is the database index of gate to enable.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XClock_GateEnableSet(u8 GateIndex)
{
	u8  GateState;
	u32 Value;

	/* Validate arguments */
	XCLOCK_VALIDATE_INDEX_WARN(GATE, GateIndex);

	if (XST_SUCCESS != XClock_GateIsEnabled(GateIndex, &GateState)) {
		xil_printf("Warning: Gate state fetch failed for %d\n",
								GateIndex);
		return;
	}

	if (GateState) {
		return;
	}

	if (XST_SUCCESS != XClock_ReadReg(Gates[GateIndex].CtrlReg, &Value)) {
		xil_printf("Warning: Gate enable failed for %d\n", GateIndex);
		return;
	}

	Value |= 1 << Gates[GateIndex].BitIndex;
	if (XST_SUCCESS != XClock_WriteReg(Gates[GateIndex].CtrlReg, Value)) {
		xil_printf("Warning: Gate enable failed for %d\n", GateIndex);
	}
}

/*****************************************************************************/
/*
*
* The function disables the gate by writing into control register.
*
* @param	GateIndex is the database index of gate to disable.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XClock_GateDisableSet(u8 GateIndex)
{
	u8  GateState;
	u32 Value;

	/* Validate arguments */
	XCLOCK_VALIDATE_INDEX_WARN(GATE, GateIndex);

	if (XST_SUCCESS != XClock_GateIsEnabled(GateIndex, &GateState)) {
		xil_printf("Warning: Gate state fetch failed for %d\n",
								GateIndex);
		return;
	}

	if (!GateState) {
		return;
	}

	if (XST_SUCCESS != XClock_ReadReg(Gates[GateIndex].CtrlReg, &Value)) {
		xil_printf("Warning: Gate disable failed for %d\n", GateIndex);
		return;
	}

	Value &= ~(1 << Gates[GateIndex].BitIndex);
	if(XST_SUCCESS != XClock_WriteReg(Gates[GateIndex].CtrlReg, Value)) {
		xil_printf("Warning: Gate disable failed for %d\n", GateIndex);
	}
}

/*****************************************************************************/
/*
*
* Initialize gate node.
*
* @param	GateIndex is the database index of gate to initialize.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XClock_GateInit(u8 GateIndex)
{
	u8 ParentType;
	u8 ParentIdx;

	/* Validate arguments */
	XCLOCK_VALIDATE_INDEX_WARN(GATE, GateIndex);

	if (!Gates[GateIndex].IsInit) {
		/* Init parent */
		ParentType = XCLOCK_FETCH_PARENT_TYPE(Gates[GateIndex].Parent);
		ParentIdx = XCLOCK_FETCH_PARENT_INDEX(Gates[GateIndex].Parent);
		XClock_InitClk((XClock_Types)ParentType, ParentIdx);

		/* Set rate */
		Gates[GateIndex].Rate = XClock_FetchRate((XClock_Types)ParentType, ParentIdx);
		Gates[GateIndex].IsInit = TRUE;
	}
}

/*****************************************************************************/
/*
*
* Enable function for the gate.
*
* @param	GateIndex is the database index of gate to enable.
*
* @return	XST_SUCCESS if successful.
*		XST_INVALID_PARAM if arguments are invalid.
*		XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
XStatus XClock_GateEnable(u8 GateIndex)
{
	u8  ParentType;
	u8  ParentIdx;

	/* Validate index */
	XCLOCK_VALIDATE_INDEX(GATE, GateIndex);

	/* Check for parent */
	ParentType = XCLOCK_FETCH_PARENT_TYPE(Gates[GateIndex].Parent);
	ParentIdx = XCLOCK_FETCH_PARENT_INDEX(Gates[GateIndex].Parent);

	/* Enable parent node */
	if (XST_SUCCESS != XClock_EnableClkNode((XClock_Types)ParentType, ParentIdx)) {
		return XST_FAILURE;
	}

	/* Enable the gate */
	if (!Gates[GateIndex].EnableCount) {
		XClock_GateEnableSet(GateIndex);
	}

	Gates[GateIndex].EnableCount++;

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* Disable function for the gate.
*
* @param	GateIndex is the database index of gate to disable.
*
* @return	XST_SUCCESS if successful.
*		XST_INVALID_PARAM if arguments are invalid.
*		XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
static XStatus XClock_GateDisable(u8 GateIndex)
{
	u8  ParentType;
	u8  ParentIdx;

	/* Validate index */
	XCLOCK_VALIDATE_INDEX(GATE, GateIndex);

	/* Check enabled status */
	if (!Gates[GateIndex].EnableCount) {
		return XST_SUCCESS;
	}

	/* Check for parent */
	ParentType = XCLOCK_FETCH_PARENT_TYPE(Gates[GateIndex].Parent);
	ParentIdx = XCLOCK_FETCH_PARENT_INDEX(Gates[GateIndex].Parent);

	/* Disable parent node */
	if (XST_SUCCESS != XClock_DisableClkNode((XClock_Types)ParentType, ParentIdx)) {
		return XST_FAILURE;
	}

	/* Disable the gate */
	Gates[GateIndex].EnableCount--;
	if (!Gates[GateIndex].EnableCount) {
		XClock_GateDisableSet(GateIndex);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* This function looks for the output clock mapped to gates and returns the
* gate index.
*
* @param	ClockId is the identifier for output clock.
* @param	GateIndex is the index of output clock in gate database.
*
* @return	XST_SUCCESS if successful.
*		XST_INVALID_PARAM if output clock mapping not found.
*
* @note		None.
*
******************************************************************************/
static XStatus XClock_GateFetchIdx(XClock_OutputClks ClockId, u8 *GateIndex)
{
	u8 Idx;

	/* Validate Args */
	XCLOCK_VALIDATE_INDEX(OP, ClockId);
	XCLOCK_VALIDATE_PTR(GateIndex);

	for (Idx = 0; Idx < ARRAY_SIZE(GateMap); Idx++) {
		if (GateMap[Idx].ClockId == ClockId) {
			*GateIndex = GateMap[Idx].GateIndex;
			return XST_SUCCESS;
		}
	}

	return XST_INVALID_PARAM;
}

/*****************************************************************************/
/*
*
* This function fetchs the parent based on index in database.
*
* @param	ClockId is the identifier for output clock.
* @param	NodeType is the pointer holding type of the node.
* @param	GateIndex is the pointer holding index of parent in database.
*
* @return	XST_SUCCESS if successful.
*		XST_INVALID_PARAM if output clock mapping not found.
*
* @note		None.
*
******************************************************************************/
static XStatus XClock_GateFetchParent(XClock_Types *NodeType, u8 *GateIndex)
{
	/* Validate Args */
	XCLOCK_VALIDATE_PTR(NodeType);
	XCLOCK_VALIDATE_PTR(GateIndex);
	XCLOCK_VALIDATE_INDEX(GATE, *GateIndex);

	*NodeType = (XClock_Types)XCLOCK_FETCH_PARENT_TYPE(Gates[*GateIndex].Parent);
	*GateIndex = XCLOCK_FETCH_PARENT_INDEX(Gates[*GateIndex].Parent);

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* Get gate rate.
*
* @param	GateIndex is the database index of gate to fetch rate for.
* @param	GetRate is pointer to variable holding gate rate.
*
* @return	XST_SUCCESS if successful.
*		XST_INVALID_PARAM for invalid function arguments.
*
* @note		None.
*
******************************************************************************/
static XStatus XClock_GateGetRate(u8 GateIndex, XClockRate *GetRate)
{
	/* Validate args */
	XCLOCK_VALIDATE_INDEX(GATE, GateIndex);
	XCLOCK_VALIDATE_PTR(GetRate);

	*GetRate = Gates[GateIndex].Rate;

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* The function updates the rate of gate node.
*
* @param	GateIndex is the database index of gate to update rate for.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XClock_GateUpdateRate(u8 GateIndex)
{
	u8  ParentType;
	u8  ParentIdx;

	XCLOCK_VALIDATE_INDEX_WARN(GATE, GateIndex);

	/* Fetch parent */
	ParentType = XCLOCK_FETCH_PARENT_TYPE(Gates[GateIndex].Parent);
	ParentIdx = XCLOCK_FETCH_PARENT_INDEX(Gates[GateIndex].Parent);
	XClock_UpdateRate((XClock_Types)ParentType, ParentIdx);

	/* Set rate */
	Gates[GateIndex].Rate = XClock_FetchRate((XClock_Types)ParentType, ParentIdx);
}

/*****************************************************************************/
/*
*
* Register functions for gate nodes.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XClock_GateRegisterFuncs(void)
{
	/* Register functions */
	XClock_NodeInit[XCLOCK_TYPE_GATE]        = &XClock_GateInit;
	XClock_NodeEnable[XCLOCK_TYPE_GATE]      = &XClock_GateEnable;
	XClock_NodeDisable[XCLOCK_TYPE_GATE]     = &XClock_GateDisable;
	XClock_NodeFetchIdx[XCLOCK_TYPE_GATE]    = &XClock_GateFetchIdx;
	XClock_NodeFetchParent[XCLOCK_TYPE_GATE] = &XClock_GateFetchParent;
	XClock_NodeGetRate[XCLOCK_TYPE_GATE]     = &XClock_GateGetRate;
	XClock_NodeUpdateRate[XCLOCK_TYPE_GATE]  = &XClock_GateUpdateRate;
}

/*****************************************************************************/
/*
*
* Begin Initialization of all gate nodes.
*
* @param	None.
*
* @return	None.
*
* @note		This function loops over all the gate nodes and initializes
*		them.
*
******************************************************************************/
void XClock_GateBeginInit(void)
{
	u8 Idx;

	for (Idx = 0; Idx < ARRAY_SIZE(Gates); Idx++) {
		XClock_GateInit(Idx);
	}
}

/** @} */
