/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xclockps.c
* @brief Implements CLOCKPS interface functions.
*
* Contains the implementation of interface functions of the clock driver.
* See xclock.h for a description of the driver.
*
* <pre>
* MODIFICATION HISTORY:
* Ver   Who    Date     Changes
* ----- ------ -------- ---------------------------------------------
* 1.00  cjp    02/09/18 First release
* 1.00  sd     07/26/18 Fix coverity warnings
* 1.1	aru    03/20/19 Fix IAR issue by changing "XCLOCK_ABS_DIFF" to a
*			function named "XClock_Absolute_Difference".
* 1.2   sd     02/13/20 Rename ARRAY_SIZE
* 1.5   sd     07/10/23 Added SDT support
* 1.8   vn     07/30/26 Fix Doxygen warnings
* 1.8   tb     09/17/26 Align Doxygen comments with AMD standards.
* </pre>
*
******************************************************************************/

/**
* @addtogroup clockps_api CLOCKPS APIs
* @{
*/

/***************************** Include Files *********************************/
#include "xclockps.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes ******************************/
static void XClock_SetupClockModules(void);
static inline u32 XClock_Absolute_Difference(XClockRate Rate, XClockRate CurrRate);

/***************** Macros (Inline Functions) Definitions *********************/
/**
 * @brief Assigns structure elements for input clocks.
 *
 * @note
 * C-style signature
 *	XClock_TypeIp XCLOCK_ASSIGN_IP(XClockRate RateVal)
 */
#define XCLOCK_ASSIGN_IP(RateVal) \
	{ \
		.Rate = RateVal, \
	}

/**************************** Type Definitions *******************************/
/**
 * @brief Holds input clock.
 */
typedef struct {
	XClockRate Rate; /**< Rate. */

} XClock_TypeIp;

/************************** Variable Definitions *****************************/
/**
 * Input clock database.
 *
 * @note
 * PSS REF CLK frequency is obtained from xparamters.h and has minor variations
 * based on board revisions. Other frequencies are independent to board
 * revisions and no macros for them are defined in xparameters.h
 *
 */
static XClock_TypeIp Ips[] = {
	XCLOCK_ASSIGN_IP(XPAR_PSU_PSS_REF_CLK_FREQ_HZ),
	XCLOCK_ASSIGN_IP(XCLOCK_FIX_RATE_VIDEO_CLK),
	XCLOCK_ASSIGN_IP(XCLOCK_FIX_RATE_PSS_ALT_REF_CLK),
	XCLOCK_ASSIGN_IP(XCLOCK_FIX_RATE_GT_CRX_REF_CLK),
	XCLOCK_ASSIGN_IP(XCLOCK_FIX_RATE_AUX_REF_CLK),
	XCLOCK_ASSIGN_IP(XCLOCK_FIX_RATE_DP_ACLK)
};

XClock_UpdateRateFuncPtr  XClock_NodeUpdateRate[XCLOCK_TYPE_MAX];
XClock_FetchRateFuncPtr   XClock_NodeGetRate[XCLOCK_TYPE_MAX];
XClock_SetRateFuncPtr     XClock_NodeSetRate[XCLOCK_TYPE_MAX];
XClock_FetchParentFuncPtr XClock_NodeFetchParent[XCLOCK_TYPE_MAX];
XClock_FetchIdxFuncPtr    XClock_NodeFetchIdx[XCLOCK_TYPE_MAX];
XClock_DisableNodeFuncPtr XClock_NodeDisable[XCLOCK_TYPE_MAX];
XClock_EnableNodeFuncPtr  XClock_NodeEnable[XCLOCK_TYPE_MAX];
XClock_InitClkFuncPtr     XClock_NodeInit[XCLOCK_TYPE_MAX];
XClock_SetParentFuncPtr   XClock_MuxSetParent;

/*****************************************************************************/
/**
* @brief Initializes a CLOCKPS driver instance.
*
* This function initializes a XClockPs instance/driver.
*
* @param[out] InstancePtr Pointer to the XClockPs instance.
* @param[in] ConfigPtr Points to the XClockPs device configuration structure.
*
* @return	XST_SUCCESS always.
*
* @note		This function allocates device Id to instance pointer. The
*		required base address comes from xparameters.h and are not
*		required to be allocated. This function also sets up basic
*		clock nodes and handles initialization for the same.
*
******************************************************************************/
XStatus XClock_CfgInitialize(XClock *InstancePtr, XClockPs_Config *ConfigPtr)
{
	/* Arguments validation */
	XCLOCK_VALIDATE_PTR(InstancePtr);
	XCLOCK_VALIDATE_PTR(ConfigPtr);

	/* Copying instance */
#ifdef SDT
	InstancePtr->Config.BaseAddress = ConfigPtr->BaseAddress;
#else
	InstancePtr->Config.DeviceId = ConfigPtr->DeviceId;
#endif

	/* Setup clock modules */
	XClock_SetupClockModules();

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Initializes CLOCKPS clock nodes.
*
* This function handles initialization for clock nodes.
*
******************************************************************************/
static void XClock_NodeBeginInit(void)
{
	/* Register node functions */
	XClock_PllRegisterFuncs();
	XClock_MuxRegisterFuncs();
	XClock_GateRegisterFuncs();
	XClock_DivRegisterFuncs();
	XClock_FixedFactorRegisterFuncs();

	/* Begin node initialization */
	XClock_PllBeginInit();
	XClock_MuxBeginInit();
	XClock_GateBeginInit();
	XClock_DivBeginInit();
	XClock_FixedFactorBeginInit();
}

/*****************************************************************************/
/**
* @brief Enables clocks required at initialization.
*
* This function enables acpu and ddr_ref clocks during initialization.
*
* @note		An array with list of clocks to be enabled at initialization
*		is maintained. This function walks through that array and
*		initializes all the clocks.
*
******************************************************************************/
static void XClock_EnableInitClocks(void)
{
	u8                Idx;
	XClock_OutputClks InitClks[] = {ACPU, DDR_REF};

	for (Idx = 0; Idx < CLK_ARRAY_SIZE(InitClks); Idx++) {
		if (XST_SUCCESS != XClock_EnableClock(InitClks[Idx])) {
			xil_printf("Warning: Failed to enable clock at "
				   "index %d\n\r", Idx);
		}
	}
}

/*****************************************************************************/
/**
* @brief Sets up CLOCKPS modules before clock operations.
*
* This function handles prerequisites required for clock operations.
*
* @note		Prerequisites for clock operations are handled here. Anything
*		that is to be done before clock operations must be included
*		here.
*
******************************************************************************/
static void XClock_SetupClockModules(void)
{
	/* Init clock nodes */
	XClock_NodeBeginInit();

	/* Enable Init clocks */
	XClock_EnableInitClocks();
}

/*****************************************************************************/
/**
* @brief Enables a CLOCKPS clock sub-node.
*
* This function is a wrapper to enable clock sub nodes.
*
* @param[in] NodeType Type of clock node.
* @param[in] NodeIdx Index of the node.
*
* @return	XST_SUCCESS if successful else failure error code.
*
******************************************************************************/
XStatus XClock_EnableClkNode(XClock_Types NodeType, u8 NodeIdx)
{
	XStatus Status = XST_SUCCESS;

	if (XCLOCK_TYPE_IP == NodeType) {
		/* No need to enable fixed clocks, validating node index */
		XCLOCK_VALIDATE_INDEX(IP, NodeIdx);
		Status = XST_SUCCESS;
	}

	if (NULL != XClock_NodeEnable[NodeType]) {
		Status = XClock_NodeEnable[NodeType](NodeIdx);
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief Disables a CLOCKPS clock sub-node.
*
* This function is a wrapper to disable clock sub nodes.
*
* @param[in] NodeType Type of clock node.
* @param[in] NodeIdx Index of the node.
*
* @return	XST_SUCCESS if successful else failure error code.
*
******************************************************************************/
XStatus XClock_DisableClkNode(XClock_Types NodeType, u8 NodeIdx)
{
	XStatus Status = XST_SUCCESS;

	if (XCLOCK_TYPE_IP == NodeType) {
		/* No need to disable fixed clocks, validating node index */
		XCLOCK_VALIDATE_INDEX(IP, NodeIdx);
		Status = XST_SUCCESS;
	}

	if (NULL != XClock_NodeDisable[NodeType]) {
		Status = XClock_NodeDisable[NodeType](NodeIdx);
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief Fetches node type and index for an output clock.
*
* This function queries each of the sub nodes for output clock information.
*
* @param[in] ClockId Identifier for output clock.
* @param[out] NodeType Pointer to variable storing node type.
* @param[out] NodeIdx Pointer to variable storing node index.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_INVALID_PARAM if no node has output clock information.
*
******************************************************************************/
static XStatus XClock_FetchClockInfo(XClock_OutputClks ClockId,
				     XClock_Types *NodeType, u8 *NodeIdx)
{
	u8      Idx;
	XStatus Status;

	/* Validate Arguments */
	XCLOCK_VALIDATE_PTR(NodeType);

	for (Idx = 0; Idx < XCLOCK_TYPE_MAX; Idx++) {
		if (NULL != XClock_NodeFetchIdx[Idx]) {
			Status = XClock_NodeFetchIdx[Idx](ClockId, NodeIdx);
			if (XST_SUCCESS == Status) {
				*NodeType = (XClock_Types)Idx;
				return Status;
			}
		}
	}

	/* Output clock not found in any possible node */
	return XST_INVALID_PARAM;
}

/*****************************************************************************/
/**
* @brief Fetches configurable node data for an output clock.
*
* Fetch Configurable node information for output clock.
*
* @param[in] ClockId Output clock id.
* @param[out] PllIndex Pointer to variable holding Pll database index.
* @param[out] DivIndex Pointer to variable holding Div database index.
* @param[out] PllParentRate Pointer to variable holding rate of Pll parent.
* @param[out] DivParentRate Pointer to variable holding rate of Div parent.
*
* @return	XST_SUCCESS if successful else failure error code.
*
* @note		Max supported Plls is 1 and Max supported Dividers are 2 for
*		ZynqMP.
*
******************************************************************************/
static XStatus XClock_GetConfigNodeInfo(XClock_OutputClks ClockId,
					u8 *PllIndex, u8 *DivIndex, XClockRate *PllParentRate,
					XClockRate *DivParentRate)
{
	u8           NodeIdx;
	u8           FetchDivRate = FALSE;
	u8           FetchPllRate = FALSE;
	u8           DivIdx = 0;
	XStatus      Status;
	XClock_Types NodeType;

	/* Validate arguments */
	XCLOCK_VALIDATE_INDEX(OP, ClockId);
	XCLOCK_VALIDATE_PTR(PllIndex);
	XCLOCK_VALIDATE_PTR(DivIndex);
	XCLOCK_VALIDATE_PTR(PllParentRate);
	XCLOCK_VALIDATE_PTR(DivParentRate);

	/* Fetch end node information */
	Status = XClock_FetchClockInfo(ClockId, &NodeType, &NodeIdx);
	if (XST_SUCCESS != Status) {
		return Status;
	}

	do {
		if (XCLOCK_TYPE_DIVIDER == NodeType) {
			DivIndex[DivIdx] = NodeIdx;
			FetchDivRate = TRUE;
		}

		if (XCLOCK_TYPE_PLL == NodeType) {
			*PllIndex = NodeIdx;
			FetchPllRate = TRUE;
		}

		/* Fetch Node parent */
		if (NULL != XClock_NodeFetchParent[NodeType]) {
			Status = XClock_NodeFetchParent[NodeType](&NodeType,
					&NodeIdx);
			if (Status) {
				return Status;
			}
		}

		if (TRUE == FetchDivRate) {
			if (DivIdx > 2) {
				/* Max 2 divisors supported */
				return XST_FAILURE;
			}

			*DivParentRate = XClock_FetchRate(NodeType, NodeIdx);
			DivIdx++;
			FetchDivRate = FALSE;
		}

		if (TRUE == FetchPllRate) {

			*PllParentRate = XClock_FetchRate(NodeType, NodeIdx);
			FetchPllRate = FALSE;
		}
	} while (XCLOCK_TYPE_IP != NodeType);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Configures dividers to reach a requested rate.
*
* Configure Divs to achieve requested rate. The rate is rounded so the nearest
* possible value is returned.
*
* @param[in] DivIdx Pointer to database index of Dividers.
* @param[in] ParRate Rate of parent.
* @param[in] Rate Clock rate to set.
* @param[out] SetRate Pointer to variable holding rate that is set.
*
* @return	XST_SUCCESS if successful else failure error code.
*
******************************************************************************/
static XStatus XClock_ConfigDivs(u8 *DivIdx, XClockRate ParRate,
				 XClockRate Rate, XClockRate *SetRate)
{
	u8         CurrDiv;
	u8         MinDiv;
	u8         MinRem;
	u8         Div[2];
	u16        ExpDiv;
	u32        MaxDiv;
	XStatus    Status;
	XClockRate DivRate;
	XClockRate AchRate;
	const u8   NodeType = XCLOCK_TYPE_DIVIDER;

	/* Validate if set rate is registered */
	if (NULL == XClock_NodeSetRate[NodeType]) {
		return XST_FAILURE;
	}

	/* Compute expected values */
	ExpDiv = XCLOCK_ROUND_DIV(ParRate, Rate);

	if (DivIdx[1] != XCLOCK_INVALID_DIV_INDEX) {
		MaxDiv = (XCLOCK_MAX_DIV_VAL * XCLOCK_MAX_DIV_VAL);
	} else {
		MaxDiv = XCLOCK_MAX_DIV_VAL;
	}

	/* Expected rate is more than achievable */
	if (ExpDiv > MaxDiv) {
		AchRate = Rate;
		Status = XClock_NodeSetRate[NodeType](DivIdx[0], ParRate, Rate,
						      &DivRate, 0);
		if (XST_SUCCESS != Status) {
			return Status;
		}

		if (DivIdx[1] != XCLOCK_INVALID_DIV_INDEX) {
			AchRate = DivRate / XCLOCK_MAX_DIV_VAL;
			Status = XClock_NodeSetRate[NodeType](DivIdx[1],
							      DivRate, AchRate, &DivRate, 0);
			if (XST_SUCCESS != Status) {
				return Status;
			}
		}

		*SetRate = DivRate;

		return XST_SUCCESS;
	}

	/* Only one divisor available or rate achievable by one divisor */
	if (ExpDiv <= XCLOCK_MAX_DIV_VAL) {
		AchRate = Rate;
		Status = XClock_NodeSetRate[NodeType](DivIdx[0],
						      ParRate, AchRate, &DivRate, 0);
		if (XST_SUCCESS != Status) {
			return Status;
		}

		/* Clear second divisor if available */
		if (DivIdx[1] != XCLOCK_INVALID_DIV_INDEX) {
			Status = XClock_NodeSetRate[NodeType](DivIdx[1],
							      DivRate, DivRate, &DivRate, 0);
			if (XST_SUCCESS != Status) {
				return Status;
			}
		}

		*SetRate = DivRate;

		return XST_SUCCESS;
	}

	/* Rate achievable by two divisors */
	CurrDiv = 2;
	MinRem = XCLOCK_MAX_DIV_VAL - 1;
	MinDiv = 1;

	do {
		if (MinRem > (ExpDiv % CurrDiv)) {
			if (XCLOCK_MAX_DIV_VAL < (ExpDiv / CurrDiv)) {
				CurrDiv++;
				continue;
			}

			MinRem = ExpDiv % CurrDiv;
			MinDiv = CurrDiv;
		}

		if (0 == MinRem) {
			break;
		}
	} while (++CurrDiv <= XCLOCK_MAX_DIV_VAL);

	/* Calculate best achievable rate */
	Div[0] = ExpDiv / MinDiv;
	Div[1] = ExpDiv / Div[0];

	/* Dry run to calculate achievable rate */
	Status = XClock_NodeSetRate[NodeType](DivIdx[0], ParRate,
					      (ParRate / Div[1]), &DivRate, 0);
	if (Status) {
		return Status;
	}

	Status = XClock_NodeSetRate[NodeType](DivIdx[1], DivRate,
					      (DivRate / Div[0]), &DivRate, 0);
	if (Status) {
		return Status;
	}

	*SetRate = DivRate;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Configures PLLs to reach a requested rate.
*
* Configure Plls to achieve desirable rate.
*
* @param[in] PllIdx Pointer to database index of Pll.
* @param[in] ParRate Rate of parent.
* @param[in] Rate Clock rate to set.
* @param[out] SetRate Pointer to variable holding rate that is set.
*
* @return	XST_SUCCESS if successful else failure error code.
*
******************************************************************************/
static XStatus XClock_ConfigPlls(u8 *PllIdx, XClockRate ParRate,
				 XClockRate Rate, XClockRate *SetRate)
{
	XStatus    Status;
	XClockRate CurrRate;

	/* Validate arguments */
	XCLOCK_VALIDATE_INDEX(PLL, *PllIdx);
	XCLOCK_VALIDATE_PTR(SetRate);

	/* Get current rate */
	Status = XClock_NodeGetRate[XCLOCK_TYPE_PLL](*PllIdx, &CurrRate);
	if (XST_SUCCESS != Status) {
		return Status;
	}

	/* Dry run to get achievable rate */
	Status = XClock_NodeSetRate[XCLOCK_TYPE_PLL](*PllIdx, ParRate, Rate,
			SetRate, 1);
	if (XST_SUCCESS != Status) {
		return Status;
	}

	if (XClock_Absolute_Difference(Rate, CurrRate) >
	    XClock_Absolute_Difference(Rate, *SetRate)) {
		Status = XClock_NodeSetRate[XCLOCK_TYPE_PLL](*PllIdx, ParRate,
				Rate, SetRate, 0);
		if (XST_SUCCESS != Status) {
			return Status;
		}

		*SetRate = Rate;
	} else {
		/*
		 * @NOTE:
		 * If rate is not updated then SetRate is set to invalid
		 * so that the calling function can act upon as per
		 * requirement.
		 */
		*SetRate = XCLOCK_INVALID_RATE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Initializes a CLOCKPS clock node.
*
* This function is a wrapper to init clock nodes.
*
* @param[in] NodeType Node type of clock to init.
* @param[in] NodeIdx Database index of node to init.
*
******************************************************************************/
void XClock_InitClk(XClock_Types NodeType, u8 NodeIdx)
{
	if (NULL != XClock_NodeInit[NodeType]) {
		XClock_NodeInit[NodeType](NodeIdx);
	}
}

/*****************************************************************************/
/**
* @brief Updates rates for topologies that match a node.
*
* This function is updates rate for the topology having matching node type and
* index.
*
* @param[in] MatchType Node type of clock to match.
* @param[in] MatchIdx Database index of node to match.
*
******************************************************************************/
void XClock_UpdateTopologyRates(XClock_Types MatchType, u8 MatchIdx)
{
	u8           Idx;
	u8           ClockIdx;
	u8           NodeIdx;
	XClock_Types ClockType;
	XClock_Types NodeType;

	for (Idx = 0; Idx < MAX_OP; Idx++) {
		/* Fetch output clock information */
		if (XST_SUCCESS !=
		    XClock_FetchClockInfo((XClock_OutputClks)Idx, &ClockType, &ClockIdx)) {
			return;
		}

		NodeType = ClockType;
		NodeIdx = ClockIdx;

		do {
			if (MatchType == NodeType &&
			    MatchIdx == NodeIdx) {
				/* Update rates for the topology */
				XClock_UpdateRate(ClockType, ClockIdx);
				break;
			}

			/* No further parent information can be fetched*/
			if (NULL == XClock_NodeFetchParent[NodeType]) {
				break;
			}

			/* Get node parent for further lookup */
			if (XST_SUCCESS != XClock_NodeFetchParent[NodeType]
			    (&NodeType, &NodeIdx)) {
				break;
			}
		} while (XCLOCK_TYPE_IP != NodeType);
	}
}

/*****************************************************************************/
/**
* @brief Updates the rate for a CLOCKPS node.
*
* This function is a wrapper to update rates.
*
* @param[in] NodeType Node type of clock to update rate for.
* @param[in] NodeIdx Database index of node to update rate for.
*
******************************************************************************/
void XClock_UpdateRate(XClock_Types NodeType, u8 NodeIdx)
{
	if (NULL != XClock_NodeUpdateRate[NodeType]) {
		XClock_NodeUpdateRate[NodeType](NodeIdx);
	}
}

/*****************************************************************************/
/**
* @brief Fetches the rate of a CLOCKPS node.
*
* This function is a wrapper to fetch clock node rates.
*
* @param[in] NodeType Type of the clock node.
* @param[in] NodeIdx Database index of the clock node.
*
* @return	Rate of the node.
*
******************************************************************************/
XClockRate XClock_FetchRate(XClock_Types NodeType, u8 NodeIdx)
{
	XStatus    Status;
	XClockRate Rate;

	if (XCLOCK_TYPE_IP == NodeType) {
		return Ips[NodeIdx].Rate;
	}

	if (NULL != XClock_NodeGetRate[NodeType]) {
		Status = XClock_NodeGetRate[NodeType](NodeIdx, &Rate);
		if (XST_SUCCESS != Status) {
			Rate = XCLOCK_INVALID_RATE;
		}
	} else {
		Rate = XCLOCK_INVALID_RATE;
	}

	return Rate;
}

/*****************************************************************************/
/**
* @brief Fetches the parent of an output clock.
*
* @param[in] ClockId Identifier for output clock.
* @param[out] NodeType Pointer to variable storing node type.
* @param[out] NodeIdx Pointer to variable storing node index.
*
* @return	XST_SUCCESS if successful else failure error code.
*
******************************************************************************/
XStatus XClock_GetParent(XClock_OutputClks ClockId,
			 XClock_Types *NodeType, u8 *NodeIdx)
{
	XStatus Status;

	/* Validate Arguments */
	XCLOCK_VALIDATE_INDEX(OP, ClockId);

	/* Fetch clock information */
	Status = XClock_FetchClockInfo(ClockId, NodeType, NodeIdx);
	if (XST_SUCCESS != Status) {
		return Status;
	}

	if (NULL != XClock_NodeFetchParent[*NodeType]) {
		return XClock_NodeFetchParent[*NodeType](NodeType, NodeIdx);
	}

	/* No fetch parent function registered */
	return XST_FAILURE;
}

/*****************************************************************************/
/**
* @brief Sets the parent of an output clock.
*
* @param[in] ClockId Identifier for output clock.
* @param[in] MuxIdx Index of mux to set parent for.
* @param[in] SetParentIdx Index of parent to set.
*
* @return	XST_SUCCESS if successful else failure error code.
*
* @note		SetParentIdx corresponds to the value to be written in parent
*		configuration register. Parents are mapped in a way that the
*		parent index in parent array corresponds to the value to be
*		written in parent configuration register.
*		List of Mux index can be obtained from xclockps.h
*
******************************************************************************/
XStatus XClock_SetParent(XClock_OutputClks ClockId, u8 MuxIdx,
			 u8 SetParentIdx)
{
	u8           LookupDone;
	u8           NodeIdx;
	u8           ClockIdx;
	XStatus      Status;
	XClock_Types NodeType;
	XClock_Types ClockType;

	/* Validate Arguments */
	XCLOCK_VALIDATE_INDEX(OP, ClockId);
	XCLOCK_VALIDATE_INDEX(MUX, MuxIdx);

	/* Fetch clock information */
	Status = XClock_FetchClockInfo(ClockId, &NodeType, &NodeIdx);
	if (XST_SUCCESS != Status) {
		return Status;
	}

	ClockType = NodeType;
	ClockIdx = NodeIdx;

	/* Look for mux in the topology */
	LookupDone = FALSE;
	do {
		if (NULL != XClock_NodeFetchParent[NodeType]) {
			Status =
				XClock_NodeFetchParent[NodeType](&NodeType, &NodeIdx);
			if (XST_SUCCESS != Status) {
				return Status;
			}

			if (XCLOCK_TYPE_MUX == NodeType && MuxIdx == NodeIdx) {
				/* Set mux parent */
				Status =
					XClock_MuxSetParent(NodeIdx, SetParentIdx);
				if (XST_SUCCESS != Status) {
					return Status;
				}

				break;
			}
		} else {
			LookupDone = TRUE;
		}
	} while (FALSE == LookupDone);

	/* Update rates for the topology */
	XClock_UpdateRate(ClockType, ClockIdx);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Fetches the rate of an output clock.
*
* @param[in] ClockId Identifier for output clock.
* @param[out] Rate Pointer to variable storing rate.
*
* @return	XST_SUCCESS if successful else failure error code.
*
******************************************************************************/
XStatus XClock_GetRate(XClock_OutputClks ClockId, XClockRate *Rate)
{
	u8           NodeIdx;
	XStatus      Status;
	XClock_Types NodeType;

	/* Validate Arguments */
	XCLOCK_VALIDATE_INDEX(OP, ClockId);

	/* Fetch clock information */
	Status = XClock_FetchClockInfo(ClockId, &NodeType, &NodeIdx);
	if (XST_SUCCESS != Status) {
		return Status;
	}

	*Rate = XClock_FetchRate(NodeType, NodeIdx);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Sets the rate of an output clock.
*
* @param[in] ClockId Identifier for output clock.
* @param[in] Rate Clock rate to set.
* @param[out] SetRate Pointer to variable holding rate that is set.
*
* @return	XST_SUCCESS if successful else failure error code.
*
******************************************************************************/
XStatus XClock_SetRate(XClock_OutputClks ClockId, XClockRate Rate,
		       XClockRate *SetRate)
{
	u8           ClockIdx;
	u8           PllIndex = XCLOCK_INVALID_PLL_INDEX;
	u8           DivIndex[2] = {XCLOCK_INVALID_DIV_INDEX,
				    XCLOCK_INVALID_DIV_INDEX
				   };
	XStatus      Status;
	XClockRate   DivParentRate = XCLOCK_INVALID_RATE;
	XClockRate   PllParentRate = XCLOCK_INVALID_RATE;
	XClockRate   CurrRate;
	XClock_Types ClockType;

	/* Validate arguments */
	XCLOCK_VALIDATE_INDEX(OP, ClockId);
	XCLOCK_VALIDATE_PTR(SetRate);

	/* Fetch clock information */
	Status = XClock_FetchClockInfo(ClockId, &ClockType, &ClockIdx);
	if (XST_SUCCESS != Status) {
		return Status;
	}

	/* Fetch the clock rate and return if same as requested rate */
	Status = XClock_GetRate(ClockId, &CurrRate);
	if (XST_SUCCESS != Status) {
		return Status;
	}

	if (Rate == CurrRate) {
		*SetRate = CurrRate;
		return XST_SUCCESS;
	}

	/* Get configurable node information */
	Status = XClock_GetConfigNodeInfo(ClockId, &PllIndex, DivIndex,
					  &PllParentRate, &DivParentRate);
	if (XST_SUCCESS != Status) {
		return Status;
	}

	/* Configure Divisors */
	if ((XCLOCK_INVALID_DIV_INDEX != DivIndex[0]) &&
	    (Rate < DivParentRate)) {
		Status = XClock_ConfigDivs(DivIndex, DivParentRate, Rate,
					   SetRate);
		if (XST_SUCCESS == Status) {
			/* Update rates for the topology */
			XClock_UpdateRate(ClockType, ClockIdx);

			return Status;
		}
	}

	/* Configure Plls */
	if ((XCLOCK_INVALID_PLL_INDEX != PllIndex) && (Rate > PllParentRate)) {
		Status = XClock_ConfigPlls(&PllIndex, PllParentRate, Rate,
					   SetRate);
		if (XST_SUCCESS == Status) {
			if (*SetRate != XCLOCK_INVALID_RATE) {
				/*
				 * Rate updated. Update rates for all the
				 * topologies using this PLL
				 */
				XClock_UpdateTopologyRates(XCLOCK_TYPE_PLL,
							   PllIndex);
			}

			return Status;
		}
	}

	return XST_FAILURE;
}

/*****************************************************************************/
/**
* @brief Enables an output clock by clock ID.
*
* This function enables output clock based on clock ID. The API looks up for the
* end node of the specified clock Id. It then enables the node and recursively
* looks up for the parents and enables them if they are disabled. The recursive
* lookup and enable occurs until fixed input clock appears in the topology.
*
* @param[in] ClockId Identifier for output clock.
*
* @return	XST_SUCCESS if successful else failure error code.
*
******************************************************************************/
XStatus XClock_EnableClock(XClock_OutputClks ClockId)
{
	u8           NodeIdx;
	XStatus      Status;
	XClock_Types ClockType;

	/* Validate Arguments */
	XCLOCK_VALIDATE_INDEX(OP, ClockId);

	/* Fetch clock information */
	Status = XClock_FetchClockInfo(ClockId, &ClockType, &NodeIdx);
	if (XST_SUCCESS == Status) {
		/* Transfer enable call to concerned node */
		Status = XClock_EnableClkNode(ClockType, NodeIdx);
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief Disables an output clock by clock ID.
*
* This function disables output clock based on clock ID.
*
* @param[in] ClockId Identifier for output clock.
*
* @return	XST_SUCCESS if successful else failure error code.
*
******************************************************************************/
XStatus XClock_DisableClock(XClock_OutputClks ClockId)
{
	u8           NodeIdx;
	XStatus      Status;
	XClock_Types ClockType;

	/* Validate Arguments */
	XCLOCK_VALIDATE_INDEX(OP, ClockId);

	/* Fetch clock information */
	Status = XClock_FetchClockInfo(ClockId, &ClockType, &NodeIdx);
	if (XST_SUCCESS == Status) {
		/* Transfer disable call to concerned node */
		Status = XClock_DisableClkNode(ClockType, NodeIdx);
	}

	return Status;
}
/*****************************************************************************/
/**
* @brief Returns the absolute difference of two rates.
*
* This function returns the absolute difference of 2 values.
* @param[in] Rate First rate value.
* @param[in] CurrRate Second rate value.
*
* @return	Difference between the two rate values.
*
******************************************************************************/
static inline u32 XClock_Absolute_Difference(XClockRate Rate, XClockRate CurrRate)
{
	u32 AbsDiff;
	if (Rate < CurrRate) {
		AbsDiff = (u32)(CurrRate - Rate);
	} else {
		AbsDiff = (u32)(Rate - CurrRate);
	}
	return AbsDiff;
}

/** @} */
