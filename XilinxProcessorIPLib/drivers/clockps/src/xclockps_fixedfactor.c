/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xclockps_fixedfactor.c
* @brief Implements CLOCKPS fixed-factor operations.
*
* This file handles fixed factor related definition and operations.
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

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
/**
 * @brief Assigns structure elements for fixed factors.
 *
 * @note
 * C-style signature
 *	XClock_TypeFixedFactor XCLOCK_ASSIGN_FIXEDFACTORS(
 *		XClock_Types ParentTypeVal, u8 FixFactIndexVal)
 */
#define XCLOCK_ASSIGN_FIXEDFACTORS(ParentTypeVal, FixFactIndexVal) \
{ \
	.Multiplier = 1, \
	.Divisor = 2, \
	.IsInit = FALSE, \
	.EnableCount = 0, \
	.Parent = XCLOCK_GENERATE_PARENT_ID(ParentTypeVal, FixFactIndexVal), \
	.Rate = XCLOCK_INVALID_RATE, \
}

/**************************** Type Definitions *******************************/
/**
 * @brief Holds fixed factors.
 */
typedef struct {
	u8          Multiplier; /**< Multiplier. */
	u8          Divisor; /**< Divisor. */
	u8          IsInit; /**< Isinit. */
	u8          EnableCount; /**< Enablecount. */
	u16         Parent; /**< Parent. */
	XClockRate  Rate; /**< Rate. */

} XClock_TypeFixedFactor;

/************************** Variable Definitions *****************************/
/* Fixed factor database */
static XClock_TypeFixedFactor FixedFactors[] = {
	XCLOCK_ASSIGN_FIXEDFACTORS(XCLOCK_TYPE_PLL,     IOPLL_INT_PLL),
	XCLOCK_ASSIGN_FIXEDFACTORS(XCLOCK_TYPE_PLL,     RPLL_INT_PLL),
	XCLOCK_ASSIGN_FIXEDFACTORS(XCLOCK_TYPE_PLL,     APLL_INT_PLL),
	XCLOCK_ASSIGN_FIXEDFACTORS(XCLOCK_TYPE_PLL,     DPLL_INT_PLL),
	XCLOCK_ASSIGN_FIXEDFACTORS(XCLOCK_TYPE_PLL,     VPLL_INT_PLL),
	XCLOCK_ASSIGN_FIXEDFACTORS(XCLOCK_TYPE_DIVIDER, ACPU_DIV0),
};

/*****************************************************************************/
/**
* @brief Recalculates a fixed-factor rate.
*
* Recalculate fixed factor rate.
*
* @param[in] FixFactIndex Database index of fixed factor to recalculate rate
*   for.
* @param[in] ParentRate Rate of fixed factor parent.
* @param[out] Rate Pointer to variable holding recalculated rate.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_INVALID_PARAM if invalid argument.
*
******************************************************************************/
static XStatus XClock_FixedFactorRecalcRate(u8 FixFactIndex,
		XClockRate ParentRate, XClockRate *Rate)
{
	/* Validate Index */
	XCLOCK_VALIDATE_INDEX(FIXEDFACTOR, FixFactIndex);
	XCLOCK_VALIDATE_PTR(Rate);

	*Rate = ParentRate * FixedFactors[FixFactIndex].Multiplier;
	*Rate = *Rate / FixedFactors[FixFactIndex].Divisor;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Initializes a fixed-factor node.
*
* Initialize fixed factor node.
*
* @param[in] FixFactIndex Database index of fixed factor to initialize.
*
******************************************************************************/
static void XClock_FixedFactorInit(u8 FixFactIndex)
{
	u8         ParentType;
	u8         ParentIdx;
	XClockRate ParentRate;
	XClockRate Rate;

	XCLOCK_VALIDATE_INDEX_WARN(FIXEDFACTOR, FixFactIndex);

	if (!FixedFactors[FixFactIndex].IsInit) {
		/* Init parent */
		ParentType = XCLOCK_FETCH_PARENT_TYPE
			     (FixedFactors[FixFactIndex].Parent);
		ParentIdx = XCLOCK_FETCH_PARENT_INDEX
			    (FixedFactors[FixFactIndex].Parent);
		XClock_InitClk((XClock_Types)ParentType, ParentIdx);

		/* Set rate */
		ParentRate = XClock_FetchRate((XClock_Types)ParentType, ParentIdx);
		if (XST_SUCCESS !=
		    XClock_FixedFactorRecalcRate(FixFactIndex, ParentRate, &Rate)) {
			xil_printf("Warning: Failed to Recalculate rate for "
				   "Fixedfactor at index %d", FixFactIndex);
			return;
		}

		FixedFactors[FixFactIndex].Rate = Rate;
		FixedFactors[FixFactIndex].IsInit = TRUE;
	}
}

/*****************************************************************************/
/**
* @brief Enables a fixed-factor node.
*
* Enable function for fixed factors.
*
* @param[in] FixFactIndex Database index of fixed factor to enable.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_INVALID_PARAM if arguments are invalid.
*		- XST_FAILURE otherwise.
*
******************************************************************************/
static XStatus XClock_FixedFactorEnable(u8 FixFactIndex)
{
	u8  ParentType;
	u8  ParentIdx;

	/* Validate arguments */
	XCLOCK_VALIDATE_INDEX(FIXEDFACTOR, FixFactIndex);

	/* Fetch parent information */
	ParentType = XCLOCK_FETCH_PARENT_TYPE
		     (FixedFactors[FixFactIndex].Parent);
	ParentIdx = XCLOCK_FETCH_PARENT_INDEX
		    (FixedFactors[FixFactIndex].Parent);

	/* Enable parent node */
	if (XST_SUCCESS != XClock_EnableClkNode((XClock_Types)ParentType, ParentIdx)) {
		return XST_FAILURE;
	}

	FixedFactors[FixFactIndex].EnableCount++;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Disables a fixed-factor node.
*
* Disable function for fixed factors.
*
* @param[in] FixFactIndex Database index of fixed factor to disable.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_INVALID_PARAM if arguments are invalid.
*		- XST_FAILURE otherwise.
*
******************************************************************************/
static XStatus XClock_FixedFactorDisable(u8 FixFactIndex)
{
	u8  ParentType;
	u8  ParentIdx;

	/* Validate arguments */
	XCLOCK_VALIDATE_INDEX(FIXEDFACTOR, FixFactIndex);

	/* Check enable status */
	if (!FixedFactors[FixFactIndex].EnableCount) {
		return XST_SUCCESS;
	}

	/* Fetch parent information */
	ParentType = XCLOCK_FETCH_PARENT_TYPE
		     (FixedFactors[FixFactIndex].Parent);
	ParentIdx = XCLOCK_FETCH_PARENT_INDEX
		    (FixedFactors[FixFactIndex].Parent);

	/* Disable parent node */
	if (XST_SUCCESS != XClock_DisableClkNode((XClock_Types)ParentType, ParentIdx)) {
		return XST_FAILURE;
	}

	FixedFactors[FixFactIndex].EnableCount--;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Fetches the parent of a fixed-factor node.
*
* This function fetches the parent based on index in database.
*
* @param[out] NodeType Pointer receiving the parent node type.
* @param[in,out] FixFactIndex Pointer holding index of parent in database.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_INVALID_PARAM if output clock mapping not found.
*
******************************************************************************/
static XStatus XClock_FixedFactorFetchParent(XClock_Types *NodeType,
		u8 *FixFactIndex)
{
	/* Validate Args */
	XCLOCK_VALIDATE_PTR(NodeType);
	XCLOCK_VALIDATE_PTR(FixFactIndex);
	XCLOCK_VALIDATE_INDEX(FIXEDFACTOR, *FixFactIndex);

	*NodeType = (XClock_Types)XCLOCK_FETCH_PARENT_TYPE
		    (FixedFactors[*FixFactIndex].Parent);
	*FixFactIndex = XCLOCK_FETCH_PARENT_INDEX
			(FixedFactors[*FixFactIndex].Parent);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Gets a fixed-factor rate.
*
* Get Fixed factor rate.
*
* @param[in] FixFactIndex Database index of fixed factor to fetch rate for.
* @param[out] GetRate Pointer to variable holding fixed factor rate.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_INVALID_PARAM for invalid function arguments.
*
******************************************************************************/
static XStatus XClock_FixedFactorGetRate(u8 FixFactIndex, XClockRate *GetRate)
{
	/* Validate args */
	XCLOCK_VALIDATE_INDEX(FIXEDFACTOR, FixFactIndex);
	XCLOCK_VALIDATE_PTR(GetRate);

	*GetRate = FixedFactors[FixFactIndex].Rate;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Updates the rate of a fixed-factor node.
*
* The function updates the rate of fixed factor node.
*
* @param[in] FixFactIndex Database index of fixed factor to update rate for.
*
******************************************************************************/
static void XClock_FixedFactorUpdateRate(u8 FixFactIndex)
{
	u8         ParentType;
	u8         ParentIdx;
	XClockRate ParentRate;
	XClockRate Rate;

	XCLOCK_VALIDATE_INDEX_WARN(FIXEDFACTOR, FixFactIndex);

	/* Fetch parent */
	ParentType = XCLOCK_FETCH_PARENT_TYPE
		     (FixedFactors[FixFactIndex].Parent);
	ParentIdx = XCLOCK_FETCH_PARENT_INDEX
		    (FixedFactors[FixFactIndex].Parent);
	XClock_UpdateRate((XClock_Types)ParentType, ParentIdx);

	/* Set rate */
	ParentRate = XClock_FetchRate((XClock_Types)ParentType, ParentIdx);
	if (XST_SUCCESS !=
	    XClock_FixedFactorRecalcRate(FixFactIndex, ParentRate, &Rate)) {
		xil_printf("Warning: Failed to Recalculate rate for "
			   "Fixedfactor at index %d", FixFactIndex);
		return;
	}

	FixedFactors[FixFactIndex].Rate = Rate;
}

/*****************************************************************************/
/**
* @brief Registers fixed-factor callback functions.
*
* Register functions for fixed factor node.
*
******************************************************************************/
void XClock_FixedFactorRegisterFuncs(void)
{
	/* Register functions */
	XClock_NodeInit[XCLOCK_TYPE_FIXEDFACTOR]        =
		&XClock_FixedFactorInit;
	XClock_NodeEnable[XCLOCK_TYPE_FIXEDFACTOR]      =
		&XClock_FixedFactorEnable;
	XClock_NodeDisable[XCLOCK_TYPE_FIXEDFACTOR]     =
		&XClock_FixedFactorDisable;
	XClock_NodeFetchParent[XCLOCK_TYPE_FIXEDFACTOR] =
		&XClock_FixedFactorFetchParent;
	XClock_NodeGetRate[XCLOCK_TYPE_FIXEDFACTOR]     =
		&XClock_FixedFactorGetRate;
	XClock_NodeUpdateRate[XCLOCK_TYPE_FIXEDFACTOR]  =
		&XClock_FixedFactorUpdateRate;
}
/*****************************************************************************/
/**
* @brief Initializes all fixed-factor nodes.
*
* Begin Initialization of all fixed factor node.
*
* @note		This function loops over all the fixed factor nodes and
*		initializes them.
*
******************************************************************************/
void XClock_FixedFactorBeginInit(void)
{
	u8 Idx;

	for (Idx = 0; Idx < CLK_ARRAY_SIZE(FixedFactors); Idx++) {
		XClock_FixedFactorInit(Idx);
	}
}

/** @} */
