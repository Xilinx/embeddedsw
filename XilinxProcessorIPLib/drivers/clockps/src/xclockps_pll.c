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
* @file xclockps_pll.c
* @addtogroup xclockps_v1_0
* @{
*
* This file handles PLL related definition and operations.
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
#include "sleep.h"

/************************** Constant Definitions *****************************/
/* PLL rate defines */
#define XCLOCK_PLL_RATE_VAL_MIN            1500000000
#define XCLOCK_PLL_RATE_VAL_MAX            3000000000

/* PLL fractional defines */
#define XCLOCK_PLL_FRAC_REGISTER_OFFSET    0x8
#define XCLOCK_PLL_FRAC_DIV                0x10000
#define XCLOCK_PLL_FRAC_ENABLE_MASK	   0x80000000
#define XCLOCK_PLL_FRAC_DATA_MASK          0x0000FFFF

/* PLL reset defines */
#define XCLOCK_PLL_CTRL_RESET_SET          1
#define XCLOCK_PLL_CTRL_RESET_SHIFT        0
#define XCLOCK_PLL_CTRL_RESET_MASK         0x00000001

/* PLL bypass defines */
#define XCLOCK_PLL_CTRL_BYPASS_SET         1
#define XCLOCK_PLL_CTRL_BYPASS_SHIFT       3

/* PLL feedback defines */
#define XCLOCK_PLL_CTRL_FBDIV_VAL_MIN      25
#define XCLOCK_PLL_CTRL_FBDIV_VAL_MAX      125
#define XCLOCK_PLL_CTRL_FBDIV_SHIFT        8
#define XCLOCK_PLL_CTRL_FBDIV_MASK         0x00007F00

/***************** Macros (Inline Functions) Definitions *********************/
/* Assign structure elements for output clock mappings */
#define XCLOCK_PLL_ASSIGN_INDICES(ClockIdVal, PllIndexVal) \
{ \
	.ClockId = ClockIdVal, \
	.PllIndex = PllIndexVal \
}

/* Assign structure elements for plls */
#define XCLOCK_ASSIGN_PLLS(ParentTypeVal, PllIndexVal, CtrlRegVal) \
{ \
	.LockIndex = 0, \
	.EnableCount = 0, \
	.IsInit = FALSE, \
	.Flags = XCLOCK_CLOCK_IS_CRITICAL, \
	.Parent = XCLOCK_GENERATE_PARENT_ID(ParentTypeVal, PllIndexVal), \
	.CtrlReg = CtrlRegVal, \
	.StatusReg = PLL_STATUS, \
	.Rate = XCLOCK_INVALID_RATE, \
}

/**************************** Type Definitions *******************************/
/* This typedef holds information for output clocks mapped to plls */
typedef struct {
	XClock_OutputClks ClockId;
	XClock_PllIndices PllIndex;
} XClock_PllMappings;

/* This typedef holds information for plls */
typedef struct {
	u8         LockIndex;
	u8         EnableCount;
	u8         IsInit;
	u8         Flags;
	u16        Parent;
	u32        CtrlReg;
	u32        StatusReg;
	XClockRate Rate;
} XClock_TypePll;

/* This typedef holds information for pll operating modes */
typedef enum {
	XCLOCK_PLL_MODE_FRACTIONAL,
	XCLOCK_PLL_MODE_INTEGER,
} XClock_PllMode;

/************************** Variable Definitions *****************************/
/* Plls database */
static XClock_TypePll Plls[] = {
	XCLOCK_ASSIGN_PLLS(XCLOCK_TYPE_MUX, IOPLL_PRE_SRC_MUX, IOPLL_CTRL),
	XCLOCK_ASSIGN_PLLS(XCLOCK_TYPE_MUX, RPLL_PRE_SRC_MUX,  RPLL_CTRL),
	XCLOCK_ASSIGN_PLLS(XCLOCK_TYPE_MUX, APLL_PRE_SRC_MUX,  APLL_CTRL),
	XCLOCK_ASSIGN_PLLS(XCLOCK_TYPE_MUX, DPLL_PRE_SRC_MUX,  DPLL_CTRL),
	XCLOCK_ASSIGN_PLLS(XCLOCK_TYPE_MUX, VPLL_PRE_SRC_MUX,  VPLL_CTRL),
};

/*****************************************************************************/
/*
*
* The function is used to check the current pll operating mode.
*
* @param	PllIndex is the database index of pll to check mode for.
* @param	PllMode is the pointer to variable holding pll operating mode.
*		XCLOCK_PLL_MODE_FRACTIONAL - Pll operating in fractional mode.
*		XCLOCK_PLL_MODE_INTEGER - Pll operating in integral mode.
*
* @return	XST_SUCCESS if successful
*		XST_INVALID_PARAM if invalid arguments.
*		XST_FAILURE if read/write fails.
*
* @note		None.
*
******************************************************************************/
static XStatus XClock_PllGetMode(u8 PllIndex, XClock_PllMode *PllMode)
{
	u32 Value;

	/* Validate arguments */
	 XCLOCK_VALIDATE_INDEX(PLL, PllIndex);

	if (XST_SUCCESS != XClock_ReadReg(Plls[PllIndex].CtrlReg +
				XCLOCK_PLL_FRAC_REGISTER_OFFSET, &Value)) {
		return XST_FAILURE;
	}

	*PllMode = (Value & XCLOCK_PLL_FRAC_ENABLE_MASK) ?
			XCLOCK_PLL_MODE_FRACTIONAL : XCLOCK_PLL_MODE_INTEGER;

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* The function is used to set pll operating mode.
*
* @param	PllIndex is the database index of pll to check mode for.
*		Mode is the pll mode to set
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XClock_PllSetMode(u8 PllIndex, XClock_PllMode Mode)
{
	u32 Value = 0;

	/* Validate arguments */
	XCLOCK_VALIDATE_INDEX_WARN(PLL, PllIndex);

	if (XCLOCK_PLL_MODE_FRACTIONAL == Mode) {
		Value = XCLOCK_PLL_FRAC_ENABLE_MASK;
	}

	if (XST_SUCCESS != XClock_WriteReg(Plls[PllIndex].CtrlReg +
				XCLOCK_PLL_FRAC_REGISTER_OFFSET, Value)) {
		xil_printf("Warning: Failed to set PLL mode for %d\n",
								PllIndex);
	}
}

/*****************************************************************************/
/*
*
* This function checks if the pll is enabled.
*
* @param	PllIndex is the index of output clock in pll database.
* @param	PllState is the pointer to variable holding Pll state.
*			1 - Pll is enabled.
*			0 - Pll is disabled.
*
* @return	XST_SUCCESS if successful
*		XST_INVALID_PARAM if invalid arguments.
*		XST_FAILURE if read/write fails.
*
* @note		None.
*
******************************************************************************/
static u8 XClock_PllIsEnabled(u8 PllIndex, u8 *PllState)
{
	u32 Value;

	/* Validate arguments */
	XCLOCK_VALIDATE_INDEX(PLL, PllIndex);

	if (XST_SUCCESS != XClock_ReadReg(Plls[PllIndex].CtrlReg, &Value)) {
		return XST_FAILURE;
	}

	Value = Value & XCLOCK_PLL_CTRL_RESET_MASK;
	*PllState = !Value;

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* The function enables the pll by writing into control register.
*
* @param	PllIndex is the database index of pll to enable.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XClock_PllEnableSet(u8 PllIndex)
{
	u8  PllState;
	u32 Value;

	/* Validate arguments */
	XCLOCK_VALIDATE_INDEX_WARN(PLL, PllIndex);

	if (XST_SUCCESS != XClock_PllIsEnabled(PllIndex, &PllState)) {
		xil_printf("Warning: Pll state fetch failed for %d\n",
								PllIndex);
		return;
	}

	if (PllState) {
		return;
	}

	if (XST_SUCCESS != XClock_ReadReg(Plls[PllIndex].CtrlReg, &Value)) {
		xil_printf("Warning: Failed to enable PLL %d\n", PllIndex);
		return;
	}

	Value |= XCLOCK_PLL_CTRL_BYPASS_SET << XCLOCK_PLL_CTRL_BYPASS_SHIFT;
	if (XST_SUCCESS != XClock_WriteReg(Plls[PllIndex].CtrlReg, Value)) {
		xil_printf("Warning: Failed to enable PLL %d\n", PllIndex);
		return;
	}

	Value |= XCLOCK_PLL_CTRL_RESET_SET << XCLOCK_PLL_CTRL_RESET_SHIFT;
	if(XST_SUCCESS != XClock_WriteReg(Plls[PllIndex].CtrlReg, Value)) {
		xil_printf("Warning: Failed to enable PLL %d\n", PllIndex);
		return;
	}

	Value &= ~(XCLOCK_PLL_CTRL_RESET_SET << XCLOCK_PLL_CTRL_RESET_SHIFT);
	if (XST_SUCCESS != XClock_WriteReg(Plls[PllIndex].CtrlReg, Value)) {
		xil_printf("Warning: Failed to enable PLL %d\n", PllIndex);
		return;
	}

	do {
		if (XST_SUCCESS !=
			XClock_ReadReg(Plls[PllIndex].StatusReg, &Value)) {
			xil_printf("Warning: Failed to enable PLL %d\n",
								PllIndex);
			return;
		}
		usleep(10);
	} while (!(Value & Plls[PllIndex].LockIndex));

	Value &= ~(XCLOCK_PLL_CTRL_BYPASS_SET << XCLOCK_PLL_CTRL_BYPASS_SHIFT);
	if (XST_SUCCESS != XClock_WriteReg(Plls[PllIndex].CtrlReg, Value)) {
		xil_printf("Warning: Failed to enable PLL %d\n", PllIndex);
		return;
	}
}

/*****************************************************************************/
/*
*
* The function disables the pll by writing into control register.
*
* @param	PllIndex is the database index of pll to disable.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XClock_PllDisableSet(u8 PllIndex)
{
	u8  PllState;
	u32 Value;

	/* Validate arguments */
	XCLOCK_VALIDATE_INDEX_WARN(PLL, PllIndex);

	if (XST_SUCCESS != XClock_PllIsEnabled(PllIndex, &PllState)) {
		xil_printf("Warning: Pll state fetch failed for %d\n",
								PllIndex);
		return;
	}

	if (!PllState || (XCLOCK_CLOCK_IS_CRITICAL == Plls[PllIndex].Flags)) {
		return;
	}

	if (XST_SUCCESS != XClock_ReadReg(Plls[PllIndex].CtrlReg, &Value)) {
		xil_printf("Warning: Failed to disable PLL %d\n", PllIndex);
		return;
	}

	Value |= XCLOCK_PLL_CTRL_BYPASS_SET << XCLOCK_PLL_CTRL_BYPASS_SHIFT;
	if (XST_SUCCESS != XClock_WriteReg(Plls[PllIndex].CtrlReg, Value)) {
		xil_printf("Warning: Failed to disable PLL %d\n", PllIndex);
		return;
	}

	Value |= XCLOCK_PLL_CTRL_RESET_SET << XCLOCK_PLL_CTRL_RESET_SHIFT;
	if (XST_SUCCESS != XClock_WriteReg(Plls[PllIndex].CtrlReg, Value)) {
		xil_printf("Warning: Failed to disable PLL %d\n", PllIndex);
		return;
	}
}

/*****************************************************************************/
/*
*
* Rounds pll rate based on current operating mode.
*
* @param        PllIndex is the database index of pll to round rate for.
* @param        Rate is the rate of pll.
* @param        ParentRate is the rate of pll parent.
* @param        RoundRate is the pointer to variable holding rounded rate.
*
* @return       XST_SUCCESS if successful.
*		XST_INVALID_PARAM if invalid arguments.
*
* @note         None.
*
******************************************************************************/
static XStatus XClock_PllRoundRate(u8 PllIndex, XClockRate Rate,
				XClockRate ParentRate, XClockRate *RoundRate)
{
	u32            FeedbackDiv;
	XClockRate     RateRem;
	XClockRate     RateDiv;
	XClock_PllMode PllMode;

	/* Validate arguments */
	XCLOCK_VALIDATE_INDEX(PLL, PllIndex);
	XCLOCK_VALIDATE_PTR(RoundRate);

	RateDiv = ((Rate * XCLOCK_PLL_FRAC_DIV) / ParentRate);
	RateRem = RateDiv % XCLOCK_PLL_FRAC_DIV;
	XClock_PllSetMode(PllIndex, (XClock_PllMode)(!!RateRem));

	if (XST_SUCCESS != XClock_PllGetMode(PllIndex, &PllMode)) {
		return XST_FAILURE;
	}

	if (PllMode == XCLOCK_PLL_MODE_FRACTIONAL) {
		if (Rate > XCLOCK_PLL_RATE_VAL_MAX) {
			FeedbackDiv = Rate / XCLOCK_PLL_RATE_VAL_MAX;
			Rate = Rate / (FeedbackDiv + 1);
		}
		if (Rate < XCLOCK_PLL_RATE_VAL_MIN) {
			FeedbackDiv = XCLOCK_CEIL_DIV
						(XCLOCK_PLL_RATE_VAL_MIN, Rate);
			Rate = Rate * FeedbackDiv;
		}

		*RoundRate = Rate;

		return XST_SUCCESS;
	}

	FeedbackDiv = XCLOCK_ROUND_DIV(Rate, ParentRate);
	XCLOCK_LIMIT_VALUE(FeedbackDiv, XCLOCK_PLL_CTRL_FBDIV_VAL_MIN,
						XCLOCK_PLL_CTRL_FBDIV_VAL_MAX);

	*RoundRate = (ParentRate * FeedbackDiv);

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* Recalculate pll rate.
*
* @param	PllIndex is the database index of pll to recalculate rate for.
* @param	ParentRate is the rate of pll parent.
* @param	Rate is the pointer to variable holding recalculated rate.
*
* @return	Recalculated rate.
*
* @note		None.
*
******************************************************************************/
static XStatus XClock_PllRecalcRate(u8 PllIndex, XClockRate ParentRate,
							XClockRate *Rate)
{
	u32            Value;
	u32            FeedbackDiv;
	u32            FeedbackData;
	XClockRate     RateOffset;
	XClock_PllMode PllMode;

	/* Validate arguments */
	XCLOCK_VALIDATE_INDEX(PLL, PllIndex);
	XCLOCK_VALIDATE_PTR(Rate);

	if (XST_SUCCESS != XClock_ReadReg(Plls[PllIndex].CtrlReg, &Value)) {
		return XST_FAILURE;
	}

	FeedbackDiv = (Value & XCLOCK_PLL_CTRL_FBDIV_MASK) >>
						XCLOCK_PLL_CTRL_FBDIV_SHIFT;
	*Rate =  ParentRate * FeedbackDiv;

	if (XST_SUCCESS != XClock_PllGetMode(PllIndex, &PllMode)) {
		return XST_FAILURE;
	}

	if (PllMode == XCLOCK_PLL_MODE_FRACTIONAL) {
		if (XST_SUCCESS != XClock_ReadReg(Plls[PllIndex].CtrlReg +
				XCLOCK_PLL_FRAC_REGISTER_OFFSET, &Value)) {
			return XST_FAILURE;
		}

		FeedbackData = Value & XCLOCK_PLL_FRAC_DATA_MASK;
		RateOffset = (ParentRate * FeedbackData) / XCLOCK_PLL_FRAC_DIV;
		*Rate = *Rate + RateOffset;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* Initialize pll node.
*
* @param	PllIndex is the database index of pll to initialize.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XClock_PllInit(u8 PllIndex)
{
	u8         ParentType;
	u8         ParentIdx;
	XClockRate ParentRate;
	XClockRate Rate;

	/* Validate arguments */
	XCLOCK_VALIDATE_INDEX_WARN(PLL, PllIndex);

	if (!Plls[PllIndex].IsInit) {
		/* Init parent */
		ParentType = XCLOCK_FETCH_PARENT_TYPE(Plls[PllIndex].Parent);
		ParentIdx = XCLOCK_FETCH_PARENT_INDEX(Plls[PllIndex].Parent);
		XClock_InitClk((XClock_Types)ParentType, ParentIdx);

		/* Set rate */
		ParentRate = XClock_FetchRate((XClock_Types)ParentType, ParentIdx);
		if (XST_SUCCESS !=
			XClock_PllRecalcRate(PllIndex, ParentRate, &Rate)) {
			xil_printf("Warning: Failed to Recalculate rate for "
					"Pll at index %d", PllIndex);
			return;
		}

		Plls[PllIndex].Rate = Rate;
		Plls[PllIndex].IsInit = TRUE;
	}
}

/*****************************************************************************/
/*
*
* Enable function for the pll.
*
* @param	PllIndex is the database index of pll to enable.
*
* @return	XST_SUCCESS if successful.
*		XST_INVALID_PARAM if arguments are invalid.
*		XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
static XStatus XClock_PllEnable(u8 PllIndex)
{
	u8  ParentType;
	u8  ParentIdx;

	/* Validate index */
	XCLOCK_VALIDATE_INDEX(PLL, PllIndex);

	/* Check for parent */
	ParentType = XCLOCK_FETCH_PARENT_TYPE(Plls[PllIndex].Parent);
	ParentIdx =  XCLOCK_FETCH_PARENT_INDEX(Plls[PllIndex].Parent);

	/* Enable parent node */
	if (XST_SUCCESS != XClock_EnableClkNode((XClock_Types)ParentType, ParentIdx)) {
		return XST_FAILURE;
	}

	/* Enable PLL */
	if (!Plls[PllIndex].EnableCount) {
		XClock_PllEnableSet(PllIndex);
	}

	Plls[PllIndex].EnableCount++;

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* Disable function for the pll.
*
* @param	PllIndex is the database index of pll to disable.
*
* @return	XST_SUCCESS if successful.
*		XST_INVALID_PARAM if arguments are invalid.
*		XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
static XStatus XClock_PllDisable(u8 PllIndex)
{
	u8  ParentType;
	u8  ParentIdx;

	/* Validate index */
	XCLOCK_VALIDATE_INDEX(PLL, PllIndex);

	/* Check enable status */
	if (!Plls[PllIndex].EnableCount) {
		return XST_SUCCESS;
	}

	/* Check for parent */
	ParentType = XCLOCK_FETCH_PARENT_TYPE(Plls[PllIndex].Parent);
	ParentIdx =  XCLOCK_FETCH_PARENT_INDEX(Plls[PllIndex].Parent);

	/* Disable parent node */
	if (XST_SUCCESS != XClock_DisableClkNode((XClock_Types)ParentType, ParentIdx)) {
		return XST_FAILURE;
	}

	/* Enable PLL */
	Plls[PllIndex].EnableCount--;
	if (!Plls[PllIndex].EnableCount) {
		XClock_PllDisableSet(PllIndex);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* This function fetchs the parent based on index in database.
*
* @param	ClockId is the identifier for output clock.
* @param	NodeType is the pointer holding type of the node.
* @param	PllIndex is the pointer holding index of parent in database.
*
* @return	XST_SUCCESS if successful.
*		XST_INVALID_PARAM if output clock mapping not found.
*
* @note		None.
*
******************************************************************************/
static XStatus XClock_PllFetchParent(XClock_Types *NodeType, u8 *PllIndex)
{
	/* Validate Args */
	XCLOCK_VALIDATE_PTR(NodeType);
	XCLOCK_VALIDATE_PTR(PllIndex);
	XCLOCK_VALIDATE_INDEX(PLL, *PllIndex);

	*NodeType = (XClock_Types)XCLOCK_FETCH_PARENT_TYPE(Plls[*PllIndex].Parent);
	*PllIndex = XCLOCK_FETCH_PARENT_INDEX(Plls[*PllIndex].Parent);

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* Set pll rate.
*
* @param	PllIndex is the database index of divider to set rate for.
* @param	ParentRate is the rate of pll parent.
* @param	Rate is the rate of pll to set.
* @param	SetRate is pointer to variable holding actual rate, set for
*		pll.
* @param	DryRun if set, plls are not configured and only achievable
*		frequency is calculated.
*
* @return	XST_SUCCESS if successful.
*		XST_INVALID_PARAM for invalid function arguments.
*		XST_FAILURE if read/write failure.
*
* @note		None.
*
******************************************************************************/
static XStatus XClock_PllSetRate(u8 PllIndex, XClockRate ParentRate,
				XClockRate Rate, XClockRate *SetRate, u8 DryRun)
{
	u32            Value;
	u32            RateDiv;
	u32            RateOffset;
	u32            FeedbackDiv;
	u32            FeedbackRem;
	u32            FeedbackData;
	XClock_PllMode PllMode;

	/* Validate args */
	XCLOCK_VALIDATE_INDEX(PLL, PllIndex);
	XCLOCK_VALIDATE_PTR(SetRate);

	/* Round rate to set */
	if (XST_SUCCESS !=
		XClock_PllRoundRate(PllIndex, Rate, ParentRate, &Rate)) {
		return XST_FAILURE;
	}

	if (XST_SUCCESS != XClock_PllGetMode(PllIndex, &PllMode)) {
		return XST_FAILURE;
	}

	if (PllMode == XCLOCK_PLL_MODE_FRACTIONAL) {
		RateDiv = ((Rate * XCLOCK_PLL_FRAC_DIV) / ParentRate);
		FeedbackDiv = RateDiv / XCLOCK_PLL_FRAC_DIV;
		FeedbackRem = RateDiv % XCLOCK_PLL_FRAC_DIV;
		XCLOCK_LIMIT_VALUE(FeedbackDiv, XCLOCK_PLL_CTRL_FBDIV_VAL_MIN,
						XCLOCK_PLL_CTRL_FBDIV_VAL_MAX);
		Rate = ParentRate * FeedbackDiv;
		RateOffset = (ParentRate * FeedbackRem) / XCLOCK_PLL_FRAC_DIV;
		FeedbackData =
		(XCLOCK_PLL_FRAC_DIV * FeedbackRem) / XCLOCK_PLL_FRAC_DIV;

		*SetRate = Rate + RateOffset;

		if (!DryRun) {
			if (XST_SUCCESS !=
			XClock_ReadReg(Plls[PllIndex].CtrlReg, &Value)) {
				*SetRate = XCLOCK_INVALID_RATE;
				return XST_FAILURE;
			}

			Value &= ~XCLOCK_PLL_CTRL_FBDIV_MASK;
			Value |= FeedbackDiv << XCLOCK_PLL_CTRL_FBDIV_SHIFT;
			if (XST_SUCCESS !=
			XClock_WriteReg(Plls[PllIndex].CtrlReg, Value)) {
				*SetRate = XCLOCK_INVALID_RATE;
				return XST_FAILURE;
			}

			if (XST_SUCCESS !=
				XClock_ReadReg(Plls[PllIndex].CtrlReg +
				XCLOCK_PLL_FRAC_REGISTER_OFFSET, &Value)) {
				*SetRate = XCLOCK_INVALID_RATE;
				return XST_FAILURE;
			}

			Value &= ~XCLOCK_PLL_FRAC_DATA_MASK;
			Value |= FeedbackData & XCLOCK_PLL_FRAC_DATA_MASK;
			if (XST_SUCCESS !=
				XClock_WriteReg(Plls[PllIndex].CtrlReg +
				XCLOCK_PLL_FRAC_REGISTER_OFFSET, Value)) {
				*SetRate = XCLOCK_INVALID_RATE;
				return XST_FAILURE;
			}

			Plls[PllIndex].Rate = *SetRate;
		}

		return XST_SUCCESS;
	}

	FeedbackDiv = XCLOCK_ROUND_DIV(Rate, ParentRate);
	XCLOCK_LIMIT_VALUE(FeedbackDiv, XCLOCK_PLL_CTRL_FBDIV_VAL_MIN,
					XCLOCK_PLL_CTRL_FBDIV_VAL_MAX);

	*SetRate = ParentRate * FeedbackDiv;

	if (!DryRun) {
		if (XST_SUCCESS !=
			XClock_ReadReg(Plls[PllIndex].CtrlReg, &Value)) {
			*SetRate = XCLOCK_INVALID_RATE;
			return XST_FAILURE;
		}

		Value &= ~XCLOCK_PLL_CTRL_FBDIV_MASK;
		Value |= FeedbackDiv << XCLOCK_PLL_CTRL_FBDIV_SHIFT;
		if (XST_SUCCESS !=
			XClock_WriteReg(Plls[PllIndex].CtrlReg, Value)) {
			*SetRate = XCLOCK_INVALID_RATE;
			return XST_FAILURE;
		}

		Plls[PllIndex].Rate = *SetRate;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* Get pll rate.
*
* @param	PllIndex is the database index of pll to fetch rate for.
* @param	GetRate is pointer to variable holding pll rate.
*
* @return	XST_SUCCESS if successful.
*		XST_INVALID_PARAM for invalid function arguments.
*
* @note		None.
*
******************************************************************************/
static XStatus XClock_PllGetRate(u8 PllIndex, XClockRate *GetRate)
{
	/* Validate args */
	XCLOCK_VALIDATE_INDEX(PLL, PllIndex);
	XCLOCK_VALIDATE_PTR(GetRate);

	*GetRate = Plls[PllIndex].Rate;

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* The function updates the rate of pll node.
*
* @param	PllIndex is the database index of pll to update rate for.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XClock_PllUpdateRate(u8 PllIndex)
{
	u8         ParentType;
	u8         ParentIdx;
	XClockRate ParentRate;
	XClockRate Rate;

	/* Validate arguments */
	XCLOCK_VALIDATE_INDEX_WARN(PLL, PllIndex);

	/* Fetch parent */
	ParentType = XCLOCK_FETCH_PARENT_TYPE(Plls[PllIndex].Parent);
	ParentIdx = XCLOCK_FETCH_PARENT_INDEX(Plls[PllIndex].Parent);
	XClock_UpdateRate((XClock_Types)ParentType, ParentIdx);

	/* Set rate */
	ParentRate = XClock_FetchRate((XClock_Types)ParentType, ParentIdx);
	if (XST_SUCCESS !=
			XClock_PllRecalcRate(PllIndex, ParentRate, &Rate)) {
		xil_printf("Warning: Failed to Recalculate rate for "
						"Pll at index %d", PllIndex);
		return;
	}

	Plls[PllIndex].Rate = Rate;
}

/*****************************************************************************/
/*
*
* Register functions for pll node.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XClock_PllRegisterFuncs(void)
{
	/* Register functions */
	XClock_NodeInit[XCLOCK_TYPE_PLL]        = &XClock_PllInit;
	XClock_NodeEnable[XCLOCK_TYPE_PLL]      = &XClock_PllEnable;
	XClock_NodeDisable[XCLOCK_TYPE_PLL]     = &XClock_PllDisable;
	XClock_NodeFetchParent[XCLOCK_TYPE_PLL] = &XClock_PllFetchParent;
	XClock_NodeSetRate[XCLOCK_TYPE_PLL]     = &XClock_PllSetRate;
	XClock_NodeGetRate[XCLOCK_TYPE_PLL]     = &XClock_PllGetRate;
	XClock_NodeUpdateRate[XCLOCK_TYPE_PLL]  = &XClock_PllUpdateRate;
}

/*****************************************************************************/
/*
*
* Begin Initialization of all pll node.
*
* @param	None.
*
* @return	None.
*
* @note		This function loops over all the pll nodes and initializes
*		them.
*
******************************************************************************/
void XClock_PllBeginInit(void)
{
	u8 Idx;

	for (Idx = 0; Idx < ARRAY_SIZE(Plls); Idx++) {
		XClock_PllInit(Idx);
	}
}

/** @} */
