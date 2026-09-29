/******************************************************************************
* Copyright (C) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xclockps_pll.c
* @brief Implements CLOCKPS PLL node operations.
*
* This file handles PLL related definition and operations.
*
* <pre>
* MODIFICATION HISTORY:
* Ver   Who    Date     Changes
* ----- ------ -------- ---------------------------------------------
* 1.00  cjp    02/09/18 First release
* 1.2   sd     02/13/20 Rename ARRAY_SIZE
* 1.7   vlt    01/27/26 Fixed codespell issues
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
#include "sleep.h"

/************************** Constant Definitions *****************************/
/* PLL rate defines */
/** Pll rate val min. */
#define XCLOCK_PLL_RATE_VAL_MIN            1500000000
/** Pll rate val max. */
#define XCLOCK_PLL_RATE_VAL_MAX            3000000000

/* PLL fractional defines */
/** Pll frac register offset. */
#define XCLOCK_PLL_FRAC_REGISTER_OFFSET    0x8
#define XCLOCK_PLL_FRAC_DIV                0x10000	/**< Pll frac div. */
/** Pll frac enable mask. */
#define XCLOCK_PLL_FRAC_ENABLE_MASK	   0x80000000
/** Pll frac data mask. */
#define XCLOCK_PLL_FRAC_DATA_MASK          0x0000FFFF

/* PLL reset defines */
#define XCLOCK_PLL_CTRL_RESET_SET          1	/**< Pll ctrl reset set. */
/** Pll ctrl reset shift. */
#define XCLOCK_PLL_CTRL_RESET_SHIFT        0
/** Pll ctrl reset mask. */
#define XCLOCK_PLL_CTRL_RESET_MASK         0x00000001

/* PLL bypass defines */
#define XCLOCK_PLL_CTRL_BYPASS_SET         1	/**< Pll ctrl bypass set. */
/** Pll ctrl bypass shift. */
#define XCLOCK_PLL_CTRL_BYPASS_SHIFT       3

/* PLL feedback defines */
/** Pll ctrl fbdiv val min. */
#define XCLOCK_PLL_CTRL_FBDIV_VAL_MIN      25
/** Pll ctrl fbdiv val max. */
#define XCLOCK_PLL_CTRL_FBDIV_VAL_MAX      125
/** Pll ctrl fbdiv shift. */
#define XCLOCK_PLL_CTRL_FBDIV_SHIFT        8
/** Pll ctrl fbdiv mask. */
#define XCLOCK_PLL_CTRL_FBDIV_MASK         0x00007F00

/***************** Macros (Inline Functions) Definitions *********************/
/**
 * @brief Assigns structure elements for output clock mappings.
 *
 * @note
 * C-style signature
 *	XClock_PllMappings XCLOCK_PLL_ASSIGN_INDICES(
 *		XClock_OutputClks ClockIdVal, XClock_PllIndices PllIndexVal)
 */
#define XCLOCK_PLL_ASSIGN_INDICES(ClockIdVal, PllIndexVal) \
{ \
	.ClockId = ClockIdVal, \
	.PllIndex = PllIndexVal \
}

/**
 * @brief Assigns structure elements for plls.
 *
 * @note
 * C-style signature
 *	XClock_TypePll XCLOCK_ASSIGN_PLLS(XClock_Types ParentTypeVal,
 *		u8 PllIndexVal, u32 CtrlRegVal)
 */
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
/**
 * @brief Holds output clocks mapped to plls.
 */
typedef struct {
	XClock_OutputClks ClockId; /**< Clockid. */
	XClock_PllIndices PllIndex; /**< Pllindex. */

} XClock_PllMappings;

/**
 * @brief Holds plls.
 */
typedef struct {
	u8         LockIndex; /**< Lockindex. */
	u8         EnableCount; /**< Enablecount. */
	u8         IsInit; /**< Isinit. */
	u8         Flags; /**< Flags. */
	u16        Parent; /**< Parent. */
	u32        CtrlReg; /**< Ctrlreg. */
	u32        StatusReg; /**< Statusreg. */
	XClockRate Rate; /**< Rate. */

} XClock_TypePll;

/**
 * @brief Holds pll operating modes.
 */
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
/**
* @brief Reads the current PLL operating mode.
*
* The function is used to check the current pll operating mode.
*
* @param[in] PllIndex Database index of pll to check mode for.
* @param[out] PllMode Pointer to variable holding pll operating mode.
*   XCLOCK_PLL_MODE_FRACTIONAL - Pll operating in fractional mode.
*   XCLOCK_PLL_MODE_INTEGER - Pll operating in integral mode.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_INVALID_PARAM if invalid arguments.
*		- XST_FAILURE if read/write fails.
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
/**
* @brief Sets the PLL operating mode.
*
* The function is used to set pll operating mode.
*
* @param[in] PllIndex Database index of PLL to set mode for.
* @param[in] Mode PLL mode to set.
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
/**
* @brief Checks whether a PLL is enabled.
*
* This function checks if the pll is enabled.
*
* @param[in] PllIndex Index of output clock in pll database.
* @param[out] PllState Pointer to variable holding Pll state. 1 - Pll is
*   enabled. 0 - Pll is disabled.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_INVALID_PARAM if invalid arguments.
*		- XST_FAILURE if read/write fails.
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
/**
* @brief Enables a PLL in the control register.
*
* The function enables the pll by writing into control register.
*
* @param[in] PllIndex Database index of pll to enable.
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
	if (XST_SUCCESS != XClock_WriteReg(Plls[PllIndex].CtrlReg, Value)) {
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
/**
* @brief Disables a PLL in the control register.
*
* The function disables the pll by writing into control register.
*
* @param[in] PllIndex Database index of pll to disable.
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
/**
* @brief Rounds a PLL rate for the current operating mode.
*
* Rounds pll rate based on current operating mode.
*
* @param[in] PllIndex Database index of pll to round rate for.
* @param[in] Rate Rate of pll.
* @param[in] ParentRate Rate of pll parent.
* @param[out] RoundRate Pointer to variable holding rounded rate.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_INVALID_PARAM if invalid arguments.
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
/**
* @brief Recalculates a PLL rate from hardware.
*
* Recalculate pll rate.
*
* @param[in] PllIndex Database index of pll to recalculate rate for.
* @param[in] ParentRate Rate of pll parent.
* @param[out] Rate Pointer to variable holding recalculated rate.
*
* @return	Recalculated rate.
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
/**
* @brief Initializes a PLL node.
*
* Initialize pll node.
*
* @param[in] PllIndex Database index of pll to initialize.
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
/**
* @brief Enables a PLL node.
*
* Enable function for the pll.
*
* @param[in] PllIndex Database index of pll to enable.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_INVALID_PARAM if arguments are invalid.
*		- XST_FAILURE otherwise.
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
/**
* @brief Disables a PLL node.
*
* Disable function for the pll.
*
* @param[in] PllIndex Database index of pll to disable.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_INVALID_PARAM if arguments are invalid.
*		- XST_FAILURE otherwise.
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
/**
* @brief Fetches the parent of a PLL node.
*
* This function fetches the parent based on index in database.
*
* @param[out] NodeType Pointer receiving the parent node type.
* @param[in,out] PllIndex Pointer holding index of parent in database.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_INVALID_PARAM if output clock mapping not found.
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
/**
* @brief Sets a PLL rate.
*
* Set pll rate.
*
* @param[in] PllIndex Database index of divider to set rate for.
* @param[in] ParentRate Rate of pll parent.
* @param[in] Rate Rate of pll to set.
* @param[out] SetRate Pointer to variable holding actual rate, set for pll.
* @param[in] DryRun If set, plls are not configured and only achievable
*   frequency is calculated.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_INVALID_PARAM for invalid function arguments.
*		- XST_FAILURE if read/write failure.
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
/**
* @brief Gets a PLL rate.
*
* Get pll rate.
*
* @param[in] PllIndex Database index of pll to fetch rate for.
* @param[out] GetRate Pointer to variable holding pll rate.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_INVALID_PARAM for invalid function arguments.
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
/**
* @brief Updates the rate of a PLL node.
*
* The function updates the rate of pll node.
*
* @param[in] PllIndex Database index of pll to update rate for.
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
/**
* @brief Registers PLL node callback functions.
*
* Register functions for pll node.
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
/**
* @brief Initializes all PLL nodes.
*
* Begin Initialization of all pll node.
*
* @note		This function loops over all the pll nodes and initializes
*		them.
*
******************************************************************************/
void XClock_PllBeginInit(void)
{
	u8 Idx;

	for (Idx = 0; Idx < CLK_ARRAY_SIZE(Plls); Idx++) {
		XClock_PllInit(Idx);
	}
}

/** @} */
