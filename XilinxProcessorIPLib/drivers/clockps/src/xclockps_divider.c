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
* @file xclockps_divider.c
* @addtogroup xclockps_v1_0
* @{
*
* Contains the implementation of interface functions of the clock driver.
* See xclock.h for a description of the driver.
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
#define XCLOCK_DIV_ASSIGN_INDICES(ClockIdVal, DivIndexVal) \
{ \
	.ClockId = ClockIdVal, \
	.DivIndex = DivIndexVal \
}

/**
 * Assign structure elements for dividers with control register shift of
 * 8 bits.
 */
#define XCLOCK_ASSIGN_DIVIDERS_S08(ParentTypeVal, DivIndexVal, CtrlRegVal) \
{ \
	.Shift = 8, \
	.IsInit = FALSE, \
	.EnableCount = 0, \
	.Parent = XCLOCK_GENERATE_PARENT_ID(ParentTypeVal, DivIndexVal), \
	.CtrlReg = CtrlRegVal, \
	.Rate = XCLOCK_INVALID_RATE, \
}

/**
 * Assign structure elements for dividers with control register shift of
 * 16 bits.
 */
#define XCLOCK_ASSIGN_DIVIDERS_S16(ParentTypeVal, DivIndexVal, CtrlRegVal) \
{ \
	.Shift = 16, \
	.IsInit = FALSE, \
	.EnableCount = 0, \
	.Parent = XCLOCK_GENERATE_PARENT_ID(ParentTypeVal, DivIndexVal), \
	.CtrlReg = CtrlRegVal, \
	.Rate = XCLOCK_INVALID_RATE, \
}

/**************************** Type Definitions *******************************/
/* This typedef holds information for output clocks mapped to divisors */
typedef struct {
	XClock_OutputClks ClockId;
	XClock_DivIndices DivIndex;
} XClock_DivMappings;

/* This typedef holds information for divisors */
typedef struct {
	u8          Shift;
	u8          IsInit;
	u8          EnableCount;
	u16         Parent;
	u32         CtrlReg;
	XClockRate  Rate;
} XClock_TypeDiv;

/************************** Variable Definitions *****************************/
/* Output clock mapped to dividers */
static XClock_DivMappings DivMap[] = {
	XCLOCK_DIV_ASSIGN_INDICES(IOPLL_TO_FPD, IOPLL_TO_FPD_DIV0),
	XCLOCK_DIV_ASSIGN_INDICES(RPLL_TO_FPD,  RPLL_TO_FPD_DIV0),
	XCLOCK_DIV_ASSIGN_INDICES(APLL_TO_LPD,  APLL_TO_LPD_DIV0),
	XCLOCK_DIV_ASSIGN_INDICES(DPLL_TO_LPD,  DPLL_TO_LPD_DIV0),
	XCLOCK_DIV_ASSIGN_INDICES(VPLL_TO_LPD,  VPLL_TO_LPD_DIV0),
	XCLOCK_DIV_ASSIGN_INDICES(DBG_TSTMP,    DBG_TSTMP_DIV0),
	XCLOCK_DIV_ASSIGN_INDICES(DDR_REF,      DDR_REF_DIV0),
};

/* Dividers database */
static XClock_TypeDiv Dividers[] = {
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     PL0_MUX,           PL0_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S16
	(XCLOCK_TYPE_DIVIDER, PL0_DIV0,          PL0_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     PL1_MUX,           PL1_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S16
	(XCLOCK_TYPE_DIVIDER, PL1_DIV0,          PL1_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     PL2_MUX,           PL2_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S16
	(XCLOCK_TYPE_DIVIDER, PL2_DIV0,          PL2_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     PL3_MUX,           PL3_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S16
	(XCLOCK_TYPE_DIVIDER, PL3_DIV0,          PL3_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     APLL_MUX,          APLL_TO_LPD_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     DPLL_MUX,          DPLL_TO_LPD_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     VPLL_MUX,          VPLL_TO_LPD_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     IOPLL_MUX,         IOPLL_TO_FPD_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     RPLL_MUX,          RPLL_TO_FPD_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     ACPU_MUX,          ACPU_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     DDR_MUX,           DDR_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     GEM0_REF_MUX,      GEM0_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S16
	(XCLOCK_TYPE_DIVIDER, GEM0_REF_DIV0,     GEM0_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     GEM1_REF_MUX,      GEM1_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S16
	(XCLOCK_TYPE_DIVIDER, GEM1_REF_DIV0,     GEM1_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     GEM2_REF_MUX,      GEM2_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S16
	(XCLOCK_TYPE_DIVIDER, GEM2_REF_DIV0,     GEM2_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     GEM3_REF_MUX,      GEM3_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S16
	(XCLOCK_TYPE_DIVIDER, GEM3_REF_DIV0,     GEM3_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     TIMESTAMP_REF_MUX, TSTMP_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     DBG_TRACE_MUX,     DBG_TRACE_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     DBG_FPD_MUX,       DBG_FPD_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     DBG_LPD_MUX,       DBG_LPD_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     DBG_TSTMP_MUX,     DBG_TSTMP_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     DP_VIDEO_REF_MUX,  DP_VIDEO_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S16
	(XCLOCK_TYPE_DIVIDER, DP_VIDEO_REF_DIV0, DP_VIDEO_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     DP_AUDIO_REF_MUX,  DP_AUDIO_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S16
	(XCLOCK_TYPE_DIVIDER, DP_AUDIO_REF_DIV0, DP_AUDIO_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     DP_STC_REF_MUX,    DP_STC_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S16
	(XCLOCK_TYPE_DIVIDER, DP_STC_REF_DIV0,   DP_STC_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     GPU_REF_MUX,       GPU_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     SATA_REF_MUX,      SATA_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     PCIE_REF_MUX,      PCIE_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     GDMA_REF_MUX,      FPDDMA_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     DPDMA_REF_MUX,     DPDMA_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     TOPSW_MAIN_MUX,    TOPSW_MAIN_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     TOPSW_LSBUS_MUX,   TOPSW_LSBUS_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     GTGREF0_REF_MUX,   GTGREF0_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     USB3_DUAL_REF_MUX, USB3_DUAL_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S16
	(XCLOCK_TYPE_DIVIDER, USB3_DUAL_REF_DIV0, USB3_DUAL_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     USB0_BUS_REF_MUX,  USB0_BUS_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S16
	(XCLOCK_TYPE_DIVIDER, USB0_BUS_REF_DIV0, USB0_BUS_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     USB1_BUS_REF_MUX,  USB1_BUS_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S16
	(XCLOCK_TYPE_DIVIDER, USB1_BUS_REF_DIV0, USB1_BUS_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     GEM_TSU_REF_MUX,   GEM_TSU_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S16
	(XCLOCK_TYPE_DIVIDER, GEM_TSU_REF_DIV0,  GEM_TSU_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     QSPI_REF_MUX,      QSPI_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S16
	(XCLOCK_TYPE_DIVIDER, QSPI_REF_DIV0,     QSPI_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     SDIO0_REF_MUX,     SDIO0_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S16
	(XCLOCK_TYPE_DIVIDER, SDIO0_REF_DIV0,    SDIO0_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     SDIO1_REF_MUX,     SDIO1_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S16
	(XCLOCK_TYPE_DIVIDER, SDIO1_REF_DIV0,    SDIO1_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     UART0_REF_MUX,     UART0_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S16
	(XCLOCK_TYPE_DIVIDER, UART0_REF_DIV0,    UART0_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     UART1_REF_MUX,     UART1_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S16
	(XCLOCK_TYPE_DIVIDER, UART1_REF_DIV0,    UART1_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     SPI0_REF_MUX,      SPI0_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S16
	(XCLOCK_TYPE_DIVIDER, SPI0_REF_DIV0,     SPI0_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     SPI1_REF_MUX,      SPI1_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S16
	(XCLOCK_TYPE_DIVIDER, SPI1_REF_DIV0,     SPI1_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     CAN0_REF_MUX,      CAN0_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S16
	(XCLOCK_TYPE_DIVIDER, CAN0_REF_DIV0,     CAN0_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     CAN1_REF_MUX,      CAN1_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S16
	(XCLOCK_TYPE_DIVIDER, CAN1_REF_DIV0,     CAN1_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     CPU_R5_MUX,        CPU_R5_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     IOU_SWITCH_MUX,    IOU_SWITCH_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     CSU_PLL_MUX,       CSU_PLL_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     PCAP_MUX,          PCAP_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     LPD_SWITCH_MUX,    LPD_SWITCH_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     LPD_LSBUS_MUX,     LPD_LBUS_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     NAND_REF_MUX,      NAND_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S16
	(XCLOCK_TYPE_DIVIDER, NAND_REF_DIV0,     NAND_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     ADMA_REF_MUX,      LPDDMA_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     AMS_REF_MUX,       PSSYSMON_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S16
	(XCLOCK_TYPE_DIVIDER, AMS_REF_DIV0,      PSSYSMON_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     I2C0_REF_MUX,      I2C0_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S16
	(XCLOCK_TYPE_DIVIDER, I2C0_REF_DIV0,     I2C0_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S08
	(XCLOCK_TYPE_MUX,     I2C1_REF_MUX,      I2C1_REF_CTRL),
	XCLOCK_ASSIGN_DIVIDERS_S16
	(XCLOCK_TYPE_DIVIDER, I2C1_REF_DIV0,     I2C1_REF_CTRL)
};

/*****************************************************************************/
/*
*
* Recalculate divider rate.
*
* @param	DivIndex is the database index of divider to recalculate rate
*		for.
* @param	ParentRate is the rate of divider parent.
* @param	Rate is the pointer to variable holding recalculated rate.
*
* @return	XST_SUCCESS if successful.
*		XST_INVALID_PARAM if invalid arguments.
*		XST_FAILURE if read/write fails.
*
* @note		None.
*
******************************************************************************/
static XStatus XClock_DivRecalcRate(u8 DivIndex, XClockRate ParentRate,
							XClockRate *Rate)
{
	u32 Value;

	/* Validate Args */
	XCLOCK_VALIDATE_INDEX(DIVIDER, DivIndex);
	XCLOCK_VALIDATE_PTR(Rate);

	if (XST_SUCCESS != XClock_ReadReg(Dividers[DivIndex].CtrlReg, &Value)) {
		return XST_FAILURE;
	}

	Value = Value >> Dividers[DivIndex].Shift;
	Value &= XCLOCK_VALUE_MASK(XCLOCK_DIVIDERS_BITWIDTH);

	if (!Value) {
		*Rate = ParentRate;
	} else {
		*Rate = (XClockRate)XCLOCK_CEIL_DIV(ParentRate, Value);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* Initialize divider node.
*
* @param	DivIndex is the database index of divider to initialize.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XClock_DivInit(u8 DivIndex)
{
	u8         ParentType;
	u8         ParentIdx;
	XClockRate ParentRate;
	XClockRate Rate;

	XCLOCK_VALIDATE_INDEX_WARN(DIVIDER, DivIndex);

	if (!Dividers[DivIndex].IsInit) {
		/* Init parent */
		ParentType = XCLOCK_FETCH_PARENT_TYPE
						(Dividers[DivIndex].Parent);
		ParentIdx = XCLOCK_FETCH_PARENT_INDEX
						(Dividers[DivIndex].Parent);
		XClock_InitClk((XClock_Types)ParentType, ParentIdx);

		/* Set rate */
		ParentRate = XClock_FetchRate((XClock_Types)ParentType, ParentIdx);
		if (XST_SUCCESS !=
			XClock_DivRecalcRate(DivIndex, ParentRate, &Rate)) {
			return;
		}

		Dividers[DivIndex].Rate = Rate;
		Dividers[DivIndex].IsInit = TRUE;
	}
}

/*****************************************************************************/
/*
*
* Enable function for the divider.
*
* @param	DivIndex is the database index of divider to enable.
*
* @return	XST_SUCCESS if successful.
*		XST_INVALID_PARAM if arguments are invalid.
*		XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
static XStatus XClock_DivEnable(u8 DivIndex)
{
	u8  ParentType;
	u8  ParentIdx;

	/* Validate arguments */
	XCLOCK_VALIDATE_INDEX(DIVIDER, DivIndex);

	/* Check for parent */
	ParentType = XCLOCK_FETCH_PARENT_TYPE(Dividers[DivIndex].Parent);
	ParentIdx = XCLOCK_FETCH_PARENT_INDEX(Dividers[DivIndex].Parent);

	/* Enable parent node */
	if (XST_SUCCESS != XClock_EnableClkNode((XClock_Types)ParentType, ParentIdx)) {
		return XST_FAILURE;
	}

	Dividers[DivIndex].EnableCount++;

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* Disable function for the divider.
*
* @param	DivIndex is the database index of divider to disable.
*
* @return	XST_SUCCESS if successful.
*		XST_INVALID_PARAM if arguments are invalid.
*		XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
static XStatus XClock_DivDisable(u8 DivIndex)
{
	u8  ParentType;
	u8  ParentIdx;

	/* Validate arguments */
	XCLOCK_VALIDATE_INDEX(DIVIDER, DivIndex);

	/* check enable status */
	if (!Dividers[DivIndex].EnableCount) {
		return XST_SUCCESS;
	}

	/* Check for parent */
	ParentType = XCLOCK_FETCH_PARENT_TYPE(Dividers[DivIndex].Parent);
	ParentIdx =  XCLOCK_FETCH_PARENT_INDEX(Dividers[DivIndex].Parent);

	/* Disable parent node */
	if (XST_SUCCESS != XClock_DisableClkNode((XClock_Types)ParentType, ParentIdx)) {
		return XST_FAILURE;
	}

	Dividers[DivIndex].EnableCount--;

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* This function looks for the output clock mapped to divider and returns the
* divider index.
*
* @param	ClockId is the identifier for output clock.
* @param	DivIndex is the index of output clock in divider database.
*
* @return	XST_SUCCESS if successful.
*		XST_INVALID_PARAM if output clock mapping not found.
*
* @note		None.
*
******************************************************************************/
static XStatus XClock_DivFetchIdx(XClock_OutputClks ClockId, u8 *DivIndex)
{
	u8 Idx;

	/* Validate Args */
	XCLOCK_VALIDATE_INDEX(OP, ClockId);
	XCLOCK_VALIDATE_PTR(DivIndex);

	for (Idx = 0; Idx < ARRAY_SIZE(DivMap); Idx++) {
		if (DivMap[Idx].ClockId == ClockId) {
			*DivIndex = DivMap[Idx].DivIndex;
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
* @param	DivIndex is the pointer holding index of parent in database.
*
* @return	XST_SUCCESS if successful.
*		XST_INVALID_PARAM if output clock mapping not found.
*
* @note		None.
*
******************************************************************************/
static XStatus XClock_DivFetchParent(XClock_Types *NodeType, u8 *DivIndex)
{
	/* Validate Args */
	XCLOCK_VALIDATE_PTR(NodeType);
	XCLOCK_VALIDATE_PTR(DivIndex);
	XCLOCK_VALIDATE_INDEX(DIVIDER, *DivIndex);

	*NodeType = (XClock_Types)XCLOCK_FETCH_PARENT_TYPE(Dividers[*DivIndex].Parent);
	*DivIndex = XCLOCK_FETCH_PARENT_INDEX(Dividers[*DivIndex].Parent);

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* Set divider rate.
*
* @param	DivIndex is the database index of divider to set rate for.
* @param	ParentRate is the rate of divider parent.
* @param	Rate is the rate of divider to set.
* @param	SetRate is pointer to variable holding actual rate, set for
*		divider.
* @param	DryRun if set, divisors are not configured and only achievable
*		frequency is calculated.
*
* @return	XST_SUCCESS if successful.
*		XST_INVALID_PARAM for invalid function arguments.
*		XST_FAILURE if read/write fails.
*
* @note		None.
*
******************************************************************************/
static XStatus XClock_DivSetRate(u8 DivIndex, XClockRate ParentRate,
			XClockRate Rate, XClockRate *SetRate, u8 DryRun)
{
	u8  Div;
	u32 Value;

	/* Validate args */
	XCLOCK_VALIDATE_INDEX(DIVIDER, DivIndex);
	XCLOCK_VALIDATE_PTR(SetRate);

	/* Rate to set must be less than parent rate */
	if (ParentRate < Rate) {
		return XST_SUCCESS;
	}

	Div = (u8)XCLOCK_ROUND_DIV(ParentRate, Rate);
	XCLOCK_LIMIT_VALUE(Div, 1, XCLOCK_MAX_DIV_VAL);

	*SetRate = (XClockRate)XCLOCK_CEIL_DIV(ParentRate, Div);

	if (!DryRun) {
		if (XST_SUCCESS !=
			XClock_ReadReg(Dividers[DivIndex].CtrlReg, &Value)) {
			return XST_FAILURE;
		}

		Value &= ~(XCLOCK_VALUE_MASK(XCLOCK_DIVIDERS_BITWIDTH) <<
						Dividers[DivIndex].Shift);
		Value |= Div << (Dividers[DivIndex].Shift);
		if (XST_SUCCESS !=
			XClock_WriteReg(Dividers[DivIndex].CtrlReg, Value)) {
			*SetRate = XCLOCK_INVALID_RATE;
			return XST_FAILURE;
		}

		Dividers[DivIndex].Rate = *SetRate;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* Get divider rate.
*
* @param	DivIndex is the database index of divider to fetch rate for.
* @param	GetRate is pointer to variable holding divider rate.
*
* @return	XST_SUCCESS if successful.
*		XST_INVALID_PARAM for invalid function arguments.
*
* @note		None.
*
******************************************************************************/
static XStatus XClock_DivGetRate(u8 DivIndex, XClockRate *GetRate)
{
	XCLOCK_VALIDATE_INDEX(DIVIDER, DivIndex);
	XCLOCK_VALIDATE_PTR(GetRate);

	*GetRate = Dividers[DivIndex].Rate;

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* The function updates the rate of divider node.
*
* @param	DivIndex is the database index of divider to update rate for.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XClock_DivUpdateRate(u8 DivIndex)
{
	u8         ParentType;
	u8         ParentIdx;
	XClockRate ParentRate;
	XClockRate Rate;

	XCLOCK_VALIDATE_INDEX_WARN(DIVIDER, DivIndex);

	/* Fetch parent */
	ParentType = XCLOCK_FETCH_PARENT_TYPE(Dividers[DivIndex].Parent);
	ParentIdx = XCLOCK_FETCH_PARENT_INDEX(Dividers[DivIndex].Parent);
	XClock_UpdateRate((XClock_Types)ParentType, ParentIdx);

	/* Set rate */
	ParentRate = XClock_FetchRate((XClock_Types)ParentType, ParentIdx);
	if (XST_SUCCESS != XClock_DivRecalcRate(DivIndex, ParentRate, &Rate)) {
		return;
	}

	Dividers[DivIndex].Rate = Rate;
}

/*****************************************************************************/
/*
*
* Register functions for divider node.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XClock_DivRegisterFuncs(void)
{
	/* Register functions */
	XClock_NodeInit[XCLOCK_TYPE_DIVIDER]        = &XClock_DivInit;
	XClock_NodeEnable[XCLOCK_TYPE_DIVIDER]      = &XClock_DivEnable;
	XClock_NodeDisable[XCLOCK_TYPE_DIVIDER]     = &XClock_DivDisable;
	XClock_NodeFetchIdx[XCLOCK_TYPE_DIVIDER]    = &XClock_DivFetchIdx;
	XClock_NodeFetchParent[XCLOCK_TYPE_DIVIDER] = &XClock_DivFetchParent;
	XClock_NodeSetRate[XCLOCK_TYPE_DIVIDER]     = &XClock_DivSetRate;
	XClock_NodeGetRate[XCLOCK_TYPE_DIVIDER]     = &XClock_DivGetRate;
	XClock_NodeUpdateRate[XCLOCK_TYPE_DIVIDER]  = &XClock_DivUpdateRate;
}

/*****************************************************************************/
/*
*
* Begin Initialization of all divider node.
*
* @param	None.
*
* @return	None.
*
* @note		This function loops over all the divider nodes and initializes
*		them.
*
******************************************************************************/
void XClock_DivBeginInit(void)
{
	u8 Idx;

	for (Idx = 0; Idx < ARRAY_SIZE(Dividers); Idx++) {
		XClock_DivInit(Idx);
	}
}

/** @} */
