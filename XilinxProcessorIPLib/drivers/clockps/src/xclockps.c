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
* @file xclockps.c
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
* 1.00  sd     07/26/18 Fix coverity warnings
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xclockps.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes ******************************/
static void XClock_SetupClockModules(void);

/***************** Macros (Inline Functions) Definitions *********************/
/* Assign structure elements for input clocks */
#define XCLOCK_ASSIGN_IP(RateVal) \
{ \
	.Rate = RateVal, \
}

/**************************** Type Definitions *******************************/
/* This typedef holds information for input clock */
typedef struct {
	XClockRate Rate;
} XClock_TypeIp;

/************************** Variable Definitions *****************************/
/**
 * Input clock database.
 *
 * @Note:
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
/*
*
* This function initializes a XClockPs instance/driver.
*
* @param	InstancePtr is a pointer to the XClockPs instance.
* @param	ConfigPtr points to the XClockPs device configuration
*		structure.
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
	InstancePtr->Config.DeviceId = ConfigPtr->DeviceId;

	/* Setup clock modules */
	XClock_SetupClockModules();

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* This function handles initialization for clock nodes.
*
* @param	None.
*
* @return	None.
*
* @note	None.
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
/*
*
* This function enables acpu and ddr_ref clocks during initialization.
*
* @param	None.
*
* @return	None.
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

	for (Idx = 0; Idx < ARRAY_SIZE(InitClks); Idx++) {
		if (XST_SUCCESS != XClock_EnableClock(InitClks[Idx])) {
			xil_printf("Warning: Failed to enable clock at "
							"index %d\n\r", Idx);
		}
	}
}

/*****************************************************************************/
/*
*
* This function handles prerequisites required for clock operations.
*
* @param	None.
*
* @return	None.
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
/*
*
* This function is a wrapper to enable clock sub nodes.
*
* @param	NodeType is type of clock node.
* @param	NodeIdx is the index of the node.
*
* @return	XST_SUCCESS if successful else failure error code.
*
* @note		None.
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
/*
*
* This function is a wrapper to disable clock sub nodes.
*
* @param	NodeType is type of clock node.
* @param	NodeIdx is the index of the node.
*
* @return	XST_SUCCESS if successful else failure error code.
*
* @note		None.
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
/*
*
* This function queries each of the sub nodes for output clock information.
*
* @param	ClockId is the identifier for output clock.
* @param	NodeType is pointer to variable storing node type.
* @param	NodeIdx is pointer to variable storing node index.
*
* @return	XST_SUCCESS if successful.
*		XST_INVALID_PARAM if no node has output clock information.
*
* @note		None.
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
				*NodeType = Idx;
				return Status;
			}
		}
	}

	/* Output clock not found in any possible node */
	return XST_INVALID_PARAM;
}

/*****************************************************************************/
/*
*
* Fetch Configurable node information for output clock.
*
* @param	ClockId is the output clock id.
* @param	PllIndex is the pointer to variable holding Pll database index.
* @param	DivIndex is the pointer to variable holding Div database index.
* @param	PllParentRate is the pointer to variable holding rate of Pll
*		parent.
* @param	DivParentRate is the pointer to variable holding rate of Div
*		parent.
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
/*
*
* Configure Divs to achieve requested rate. The rate is rounded so the nearest
* possible value is returned.
*
* @param	DivIdx is the pointer to database index of Dividers.
* @param	ParRate is the rate of parent.
* @param	Rate is the clock rate to set.
* @param	SetRate is the pointer to variable holding rate that is set.
*
* @return	XST_SUCCESS if successful else failure error code.
*
* @note		None.
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
/*
*
* Configure Plls to achieve desirable rate.
*
* @param	PllIdx is the pointer to database index of Pll.
* @param	ParRate is the rate of parent.
* @param	Rate is the clock rate to set.
* @param	SetRate is the pointer to variable holding rate that is set.
*
* @return	XST_SUCCESS if successful else failure error code.
*
* @note		None.
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

	/* Get curent rate */
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

	if (XCLOCK_ABS_DIFF(Rate, CurrRate) > XCLOCK_ABS_DIFF(Rate, *SetRate)) {
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
/*
*
* This function is a wrapper to init clock nodes.
*
* @param	NodeType is the node type of clock to init.
* @param	NodeIdx is the database index of node to init.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XClock_InitClk(XClock_Types NodeType, u8 NodeIdx)
{
	if (NULL != XClock_NodeInit[NodeType]) {
		XClock_NodeInit[NodeType](NodeIdx);
	}
}

/*****************************************************************************/
/*
*
* This function is updates rate for the topology having matching node type and
* index.
*
* @param	NodeType is the node type of clock to match
* @param	NodeIdx is the database index of node to match
*
* @return	None.
*
* @note		None.
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
			XClock_FetchClockInfo(Idx, &ClockType, &ClockIdx)) {
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
/*
*
* This function is a wrapper to update rates.
*
* @param	NodeType is the node type of clock to update rate for.
* @param	NodeIdx is the database index of node to update rate for.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XClock_UpdateRate(XClock_Types NodeType, u8 NodeIdx)
{
	if (NULL != XClock_NodeUpdateRate[NodeType]) {
		XClock_NodeUpdateRate[NodeType](NodeIdx);
	}
}

/*****************************************************************************/
/*
*
* This function is a wrapper to fetch clock node rates.
*
* @param	NodeType is pointer to variable storing node type.
* @param	NodeIdx is pointer to variable storing node index.
*
* @return	Rate of the node.
*
* @note		None.
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
/*
*
* This function is used to fetch intermediate parent for output clock.
*
* @param	ClockId is the identifier for output clock.
* @param	NodeType is pointer to variable storing node type.
* @param	NodeIdx is pointer to variable storing node index.
*
* @return	XST_SUCCESS if successful else failure error code.
*
* @note		None.
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
/*
*
* This function is used to set parent for output clock.
*
* @param	ClockId is the identifier for output clock.
* @param	MuxIdx is the index of mux to set parent for.
* @param	SetParentIdx is the index of parent to set.
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
/*
*
* This function is used to fetch rate of output clock.
*
* @param	ClockId is the identifier for output clock.
* @param	Rate is pointer to variable storing rate.
*
* @return	XST_SUCCESS if successful else failure error code.
*
* @note		None.
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
/*
*
* This function is used to set rate for output clock.
*
* @param	ClockId is the identifier for output clock.
* @param	Rate is the clock rate to set.
* @param	SetRate is the pointer to variable holding rate that is set.
*
* @return	XST_SUCCESS if successful else failure error code.
*
* @note		None.
*
******************************************************************************/
XStatus XClock_SetRate(XClock_OutputClks ClockId, XClockRate Rate,
							XClockRate *SetRate)
{
	u8           ClockIdx;
	u8           PllIndex = XCLOCK_INVALID_PLL_INDEX;
	u8           DivIndex[2] = {XCLOCK_INVALID_DIV_INDEX,
					XCLOCK_INVALID_DIV_INDEX};
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
				 * Rate upated. Update rates for all the
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
/*
*
* This function enables output clock based on clock ID. The API looks up for the
* end node of the specified clock Id. It then enables the node and recursively
* looks up for the parents and enables them if they are disabled. The recursive
* lookup and enable occurs until fixed input clock appears in the topology.
*
* @param	ClockId is the identifier for output clock.
*
* @return	XST_SUCCESS if successful else failure error code.
*
* @note		None.
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
/*
*
* This function disables output clock based on clock ID.
*
* @param	ClockId is the identifier for output clock.
*
* @return	XST_SUCCESS if successful else failure error code.
*
* @note		None.
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

/** @} */
