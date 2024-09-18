/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xil_util.h"
#include "xpm_pin.h"

static XPm_PinNode *PmMioPins[XPM_NODEIDX_STMIC_MAX];
static u16 PmNumPins;

/****************************************************************************/
/**
 * @brief  This function initializes the XPm_PinNode data staructure.
 *
 * @param Pin		XPm_PinNode data staructure.
 * @param PinId		PinNode ID.
 * @param BaseAddress	BaseAddress of the pin.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code.
 *
 ****************************************************************************/
XStatus XPmPin_Init(XPm_PinNode *Pin, u32 PinId, u32 BaseAddress)
{
	XStatus Status = XST_FAILURE;
	u32 PinIdx;

	PinIdx = NODEINDEX(PinId);
	if ((PinIdx >= (u32)XPM_NODEIDX_STMIC_MAX) ||
	    (PinIdx == (u32)XPM_NODEIDX_STMIC_MIN)) {
		goto done;
	}

	XPmNode_Init(&Pin->Node, PinId, (u8)XPM_PINSTATE_UNUSED, BaseAddress);

	const XPm_PinGroup *Grp = XPmPin_GetGroupByIdx(PinIdx);
	Pin->Groups = Grp->GroupList;
	Pin->NumGroups = (u8)(Grp->GroupCount);
	Pin->FuncId = (u8)MAX_FUNCTION;
	Pin->SubsysIdx = (u16)NODEINDEX(INVALID_SUBSYSID);

	if (PinIdx <= PINS_PER_BANK) {
		Pin->Bank = 0;
	} else {
		Pin->Bank =
			(u8)(((PinIdx - PINS_PER_BANK - 1U) / PINS_PER_BANK) &
			BITMASK(PIN_NODE_BANK_BIT_FIELD_SIZE));
	}

	Pin->BiasStatus = (u8)PINCTRL_BIAS_ENABLE;
	Pin->PullCtrl = (u8)PINCTRL_BIAS_PULL_UP;
	Pin->TriState = (u8)PINCTRL_TRI_STATE_ENABLE;

	PmMioPins[PinIdx] = Pin;
	PmNumPins++;
	Status = XST_SUCCESS;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function returns instance to XPm_PinNode based on PinId.
 *
 * @param PinId		PinNode ID.
 *
 * @return Instance of XPm_PinNode if successful else NULL.
 *
 ****************************************************************************/
XPm_PinNode *XPmPin_GetById(u32 PinId)
{
	XPm_PinNode *PinNode = NULL;
	u32 PinIndex = NODEINDEX(PinId);

	if ((u32)XPM_NODECLASS_STMIC != NODECLASS(PinId)) {
		goto done;
	} else if ((u32)XPM_NODESUBCL_PIN != NODESUBCLASS(PinId)) {
		goto done;
	} else if (((u32)XPM_NODETYPE_LPD_MIO != NODETYPE(PinId)) &&
		   ((u32)XPM_NODETYPE_PMC_MIO != NODETYPE(PinId))) {
		goto done;
	} else if (PinIndex >= (u32)XPM_NODEIDX_STMIC_MAX) {
		goto done;
	} else {
		/* Required by MISRA */
	}

	PinNode =  PmMioPins[PinIndex];

done:
	return PinNode;
}

/****************************************************************************/
/**
 * @brief  This function returns total number of pins added.
 *
 * @param NumPins	Number of pins.
 *
 * @return XST_SUCCESS.
 *
 ****************************************************************************/
XStatus XPmPin_GetNumPins(u32 *NumPins)
{
	*NumPins = PmNumPins;
	return XST_SUCCESS;
}

/****************************************************************************/
/**
 * @brief  Get requested pin node by node index
 *
 * @param PinIndex     Pin Index.
 *
 * @return Pointer to requested XPm_PinNode, NULL otherwise
 *
 * @note Requires only node index
 *
 ****************************************************************************/
XPm_PinNode *XPmPin_GetByIndex(const u32 PinIndex)
{
	XPm_PinNode *Pin = NULL;

	Pin = PmMioPins[PinIndex];

	return Pin;
}

/****************************************************************************/
/**
 * @brief  This function validates PinFunction availability in given Pin.
 *
 * @param Pin		Pin Node.
 * @param PinFunc	Pin Function.
 *
 * @return 1 if function is available on given pin else 0.
 *
 ****************************************************************************/
static u8 ValidatePinFunc(const XPm_PinNode *Pin, const XPm_PinFunc *PinFunc)
{
	u16 FGrpIdx, PGrpIdx;
	const u16 *FunGrps;
	const u16 *PinGrps;
	u8 IsValid = 0;

	FunGrps = PinFunc->Groups;
	PinGrps = Pin->Groups;

	for (PGrpIdx = 0; PGrpIdx < Pin->NumGroups; PGrpIdx++) {
		for (FGrpIdx = 0; FGrpIdx < PinFunc->NumGroups; FGrpIdx++) {
			if (FunGrps[FGrpIdx] == PinGrps[PGrpIdx]) {
				IsValid = 1;
				break;
			}
		}
		if (1U == IsValid) {
			break;
		}
	}

	return IsValid;
}

/****************************************************************************/
/**
 * @brief  This function sets pin function on given pin.
 *
 * @param PinId		Pin ID.
 * @param FuncId	Function ID.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code.
 *
 ****************************************************************************/
XStatus XPmPin_SetPinFunction(u32 PinId, u32 FuncId)
{
	XStatus Status = XST_FAILURE;
	XPm_PinNode *Pin;
	const XPm_PinFunc *PinFunc;
	u32 PinBaseAddr;

	Pin = XPmPin_GetById(PinId);
	if (NULL == Pin) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	PinFunc = XPmPinFunc_GetById(FuncId);
	if ((NULL == PinFunc) || (0U == ValidatePinFunc(Pin, PinFunc))) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	PinBaseAddr = (Pin->Node.BaseAddress + (ABS_PINNUM(Pin->Node.Id, Pin->Bank) * 4U));
	if ((u32)XPM_NODETYPE_LPD_MIO == NODETYPE(PinId)) {
		PmOut32(PinBaseAddr, PinFunc->LmioRegMask);
	} else if ((u32)XPM_NODETYPE_PMC_MIO == NODETYPE(PinId)) {
		PmOut32(PinBaseAddr, PinFunc->PmioRegMask);
	} else {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}
	Pin->FuncId = (u8)(FuncId & 0xFFU);
	Pin->Node.State = (u8)XPM_PINSTATE_ASSIGNED;

	Status = XST_SUCCESS;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function returns pin function on given pin.
 *
 * @param PinId		Pin ID.
 * @param FuncId	Function ID.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code.
 *
 ****************************************************************************/
XStatus XPmPin_GetPinFunction(u32 PinId, u32 *FuncId)
{
	XStatus Status = XST_FAILURE;
	const XPm_PinNode *Pin;

	Pin = XPmPin_GetById(PinId);

	if (NULL == Pin) {
		Status = XST_INVALID_PARAM;
		goto done;
	} else if (NULL == XPmPinFunc_GetById(Pin->FuncId)) {
		*FuncId = INVALID_FUNC_ID;
		Status = XST_SUCCESS;
	} else {
		*FuncId = Pin->FuncId;
		Status = XST_SUCCESS;
	}

done:
	return Status;
}

static XStatus XPmPin_ConfigSlewRate(u32 BaseAddr, u32 BitMask, u32 Value)
{
	XStatus Status = XST_FAILURE;

	if ((u32)PINCTRL_SLEW_RATE_SLOW == Value) {
		XPm_RMW32((BaseAddr + SEL_SLEW), BitMask, 0);
	} else if ((u32)PINCTRL_SLEW_RATE_FAST == Value) {
		XPm_RMW32((BaseAddr + SEL_SLEW), BitMask, BitMask);
	} else {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	Status = XST_SUCCESS;
done:
	return Status;
}

static XStatus XPmPin_ConfigBiasStatus(XPm_PinNode *Pin, u32 BaseAddr,
				       u32 BitMask, u32 Value)
{
	XStatus Status = XST_FAILURE;
	u32 RegPuAddr, RegPdAddr;

	RegPuAddr = BaseAddr + EN_WK_PU;
	RegPdAddr = BaseAddr + EN_WK_PD;

	if (((u32)PINCTRL_BIAS_ENABLE != Value) &&
	    ((u32)PINCTRL_BIAS_DISABLE != Value)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (((u32)PINCTRL_BIAS_ENABLE == Value) &&
	    ((u32)PINCTRL_BIAS_DISABLE == Pin->BiasStatus)) {
		if ((u32)PINCTRL_BIAS_PULL_UP == Pin->PullCtrl) {
			PmRmw32(RegPuAddr, BitMask, BitMask);
		} else {
			PmRmw32(RegPdAddr, BitMask, BitMask);
		}
	} else if (((u32)PINCTRL_BIAS_DISABLE == Value) &&
		   ((u32)PINCTRL_BIAS_ENABLE == Pin->BiasStatus)) {
		PmRmw32(RegPdAddr, BitMask, 0);
		PmRmw32(RegPuAddr, BitMask, 0);
	} else {
		/* Required by MISRA */
	}
	Pin->BiasStatus = (u8)(Value & BITMASK(PIN_NODE_BIASSTATUS_BIT_FIELD_SIZE));
	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus XPmPin_ConfigPullCtrl(XPm_PinNode *Pin, u32 BaseAddr, u32 BitMask,
				     u32 Value)
{
	XStatus Status = XST_FAILURE;
	u32 RegPuAddr, RegPdAddr;

	RegPuAddr = BaseAddr + EN_WK_PU;
	RegPdAddr = BaseAddr + EN_WK_PD;

	if ((u32)PINCTRL_BIAS_ENABLE == Pin->BiasStatus) {
		if ((u32)PINCTRL_BIAS_PULL_UP == Value) {
			/* Disable weak pull-down */
			PmRmw32(RegPdAddr, BitMask, 0);
			/* Enable weak pull-up */
			PmRmw32(RegPuAddr, BitMask, BitMask);
		} else if ((u32)PINCTRL_BIAS_PULL_DOWN == Value) {
			/* Disable weak pull-up */
			PmRmw32(RegPuAddr, BitMask, 0);
			/* Enable weak pull-down */
			PmRmw32(RegPdAddr, BitMask, BitMask);
		} else {
			Status = XST_INVALID_PARAM;
			goto done;
		}
	}
	Pin->PullCtrl = (u8)(Value & BITMASK(PIN_NODE_PULLCTRL_BIT_FIELD_SIZE));
	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus XPmPin_ConfigSchmittCmos(u32 BaseAddr, u32 BitMask, u32 Value)
{
	XStatus Status = XST_FAILURE;

	if ((u32)PINCTRL_INPUT_TYPE_CMOS == Value) {
		PmRmw32((BaseAddr + EN_RX_SCHMITT_HYST), BitMask, 0);
	} else if ((u32)PINCTRL_INPUT_TYPE_SCHMITT == Value) {
		PmRmw32((BaseAddr + EN_RX_SCHMITT_HYST), BitMask, BitMask);
	} else {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	Status = XST_SUCCESS;
done:
	return Status;
}

static XStatus XPmPin_ConfigDriveStrength(const XPm_PinNode *Pin, u32 BaseAddr, u32 Value)
{
	XStatus Status = XST_FAILURE;
	u32 Val = Value;
	u32 BitMask;

	if ((u32)PINCTRL_DRIVE_STRENGTH_MAX <= Val) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if ((PINNUM(Pin->Node.Id) * SEL_DRV_WIDTH) < BITS_IN_REG) {
		Val <<= PINNUM(Pin->Node.Id);
		BitMask = SEL_DRV0_MASK(Pin->Node.Id);
		PmRmw32((BaseAddr + SEL_DRV0), BitMask, Val);
	} else {
		Val <<= (PINNUM(Pin->Node.Id) - (BITS_IN_REG / SEL_DRV_WIDTH));
		BitMask = SEL_DRV1_MASK(Pin->Node.Id);
		PmRmw32((BaseAddr + SEL_DRV1), BitMask, Val);
	}
	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus XPmPin_ConfigTriState(XPm_PinNode *Pin, u32 BitMask, u32 Value)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddr;

	if ((u32)XPM_NODETYPE_LPD_MIO == NODETYPE(Pin->Node.Id)) {
		BaseAddr = Pin->Node.BaseAddress + TRI_STATE + 4U;
	} else {
		BaseAddr = Pin->Node.BaseAddress + TRI_STATE + ((Pin->Bank) * 4U);
	}

	if ((u32)PINCTRL_TRI_STATE_ENABLE == Value) {
		PmRmw32(BaseAddr, BitMask, BitMask);
	} else if ((u32)PINCTRL_TRI_STATE_DISABLE == Value) {
		/* Add check to make sure that domain is powered on */
		PmRmw32(BaseAddr, BitMask, 0);
	} else {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	Pin->TriState = (u8)(Value & BITMASK(PIN_NODE_TRISTATE_BIT_FIELD_SIZE));
	Status = XST_SUCCESS;

done:
	return Status;
}
/****************************************************************************/
/**
 * @brief  This function sets pin configuration on given pin.
 *
 * @param PinId		Pin ID.
 * @param Param		Configuration parameter type.
 * @param ParamValue	Configuration parameter value.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code.
 *
 ****************************************************************************/
XStatus XPmPin_SetPinConfig(u32 PinId, u32 Param, u32 ParamValue)
{
	XStatus Status = XST_FAILURE;
	XPm_PinNode *Pin;
	u32 BitMask, BaseAddr;
	u32 Value = ParamValue;

	Pin = XPmPin_GetById(PinId);
	if (NULL == Pin) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	BitMask = (u32)1U << (PINNUM(Pin->Node.Id) % PINS_PER_BANK);
	BaseAddr = Pin->Node.BaseAddress + ((Pin->Bank) * ((u32)BNK_OFFSET));

	switch (Param) {
	case (u32)PINCTRL_CONFIG_SLEW_RATE:
		Status = XPmPin_ConfigSlewRate(BaseAddr, BitMask, Value);
		break;
	case (u32)PINCTRL_CONFIG_BIAS_STATUS:
		Status = XPmPin_ConfigBiasStatus(Pin, BaseAddr, BitMask, Value);
		break;
	case (u32)PINCTRL_CONFIG_PULL_CTRL:
		Status = XPmPin_ConfigPullCtrl(Pin, BaseAddr, BitMask, Value);
		break;
	case (u32)PINCTRL_CONFIG_SCHMITT_CMOS:
		Status = XPmPin_ConfigSchmittCmos(BaseAddr, BitMask, Value);
		break;
	case (u32)PINCTRL_CONFIG_DRIVE_STRENGTH:
		Status = XPmPin_ConfigDriveStrength(Pin, BaseAddr, Value);
		break;
	case (u32)PINCTRL_CONFIG_TRI_STATE:
		Status = XPmPin_ConfigTriState(Pin, BitMask, Value);
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function returns pin configuration of given pin.
 *
 * @param PinId		Pin ID.
 * @param Param		Configuration parameter type.
 * @param Value		Configuration parameter value.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code.
 *
 ****************************************************************************/
XStatus XPmPin_GetPinConfig(u32 PinId, u32 Param, u32 *Value)
{
	XStatus Status = XST_FAILURE;
	const XPm_PinNode *Pin;
	u32 BitMask;
	u32 Reg;
	u32 BaseAddr;

	Pin = XPmPin_GetById(PinId);

	if (NULL == Pin) {
		Status = XST_INVALID_PARAM;
		goto done;
	} else {
		/* Required by MISRA */
	}

	BitMask = (u32)1U << (PINNUM(Pin->Node.Id) % PINS_PER_BANK);
	BaseAddr = Pin->Node.BaseAddress + ((Pin->Bank) * ((u32)BNK_OFFSET));

	switch (Param) {
	case (u32)PINCTRL_CONFIG_SLEW_RATE:
		PmIn32((BaseAddr + SEL_SLEW), Reg);
		if (BitMask == (Reg & BitMask)) {
			*Value = (u32)PINCTRL_SLEW_RATE_FAST;
		} else {
			*Value = (u32)PINCTRL_SLEW_RATE_SLOW;
		}
		Status = XST_SUCCESS;
		break;
	case (u32)PINCTRL_CONFIG_BIAS_STATUS:
		*Value = Pin->BiasStatus;
		Status = XST_SUCCESS;
		break;
	case (u32)PINCTRL_CONFIG_PULL_CTRL:
		*Value = Pin->PullCtrl;
		Status = XST_SUCCESS;
		break;
	case (u32)PINCTRL_CONFIG_SCHMITT_CMOS:
		PmIn32((BaseAddr + EN_RX_SCHMITT_HYST), Reg);
		if (0U == (Reg & BitMask)) {
			*Value = (u32)PINCTRL_INPUT_TYPE_CMOS;
		} else {
			*Value = (u32)PINCTRL_INPUT_TYPE_SCHMITT;
		}
		Status = XST_SUCCESS;
		break;
	case (u32)PINCTRL_CONFIG_DRIVE_STRENGTH:
		if ((PINNUM(Pin->Node.Id) * SEL_DRV_WIDTH) < BITS_IN_REG) {
			BitMask = SEL_DRV0_MASK(Pin->Node.Id);
			PmIn32((BaseAddr + SEL_DRV0), *Value);
			*Value &= BitMask;
			*Value >>= PINNUM(Pin->Node.Id);
		} else {
			BitMask = SEL_DRV1_MASK(Pin->Node.Id);
			PmIn32((BaseAddr + SEL_DRV1), *Value);
			*Value &= BitMask;
			*Value >>= (PINNUM(Pin->Node.Id) - (BITS_IN_REG / SEL_DRV_WIDTH));
		}
		Status = XST_SUCCESS;
		break;
	case (u32)PINCTRL_CONFIG_VOLTAGE_STATUS:
		PmIn32((BaseAddr + VMODE), *Value);
		*Value &= VMODE_MASK;
		Status = XST_SUCCESS;
		break;
	case (u32)PINCTRL_CONFIG_TRI_STATE:
		*Value = Pin->TriState;
		Status = XST_SUCCESS;
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function returns groups present in pin based on
 *	   pin ID. Index 0 returns the first 6 group IDs, index 6
 *	   returns the next 6 group IDs, and so forth.
 *
 * @param PinId		Pin ID.
 * @param Index		Index of next function groups
 * @param Groups	Function groups.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code.
 *
 ****************************************************************************/
XStatus XPmPin_GetPinGroups(u32 PinId, u32 Index, u16 *Groups)
{
	XStatus Status = XST_FAILURE;
	u32 i;
	u32 NumRead;
	const XPm_PinNode *Pin;
	u32 Size = MAX_GROUPS_PER_RES * sizeof(u16);

	Pin = XPmPin_GetById(PinId);

	Status = Xil_SMemSet(Groups, Size, (s32)END_OF_GRP, Size);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	if (NULL == Pin) {
		Status = XST_INVALID_PARAM;
		goto done;
	} else {
		/* Required by MISRA */
	}

	/* Read up to 6 group IDs from Index */
	if ((Pin->NumGroups - Index) > MAX_GROUPS_PER_RES) {
		NumRead = MAX_GROUPS_PER_RES;
	} else {
		NumRead = Pin->NumGroups - Index;
	}

	for (i = 0; i < NumRead; i++) {
		Groups[i] = Pin->Groups[i + Index];
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function returns pin attributes like type, subclass
 *	   and class based on the pin index.
 *
 * @param PinIndex	Pin Index.
 * @param Resp		Attributes Response.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code.
 *
 ****************************************************************************/
XStatus XPmPin_QueryAttributes(const u32 PinIndex, u32 *Resp)
{
	XStatus Status = XST_FAILURE;
	const XPm_PinNode *Pin;

	/* Check for valid pin index */
	if (PinIndex >= (u32)XPM_NODEIDX_STMIC_MAX) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* As per TRM, MIO PIN starts from 0 but the PIN node ID
	 * generated from the toplogy CDO starts from 1 and thus the
	 * mismatch is happening. Add +1 hack to avoid the mismatch*/
	Pin = XPmPin_GetByIndex(PinIndex + 1U);
	if (NULL == Pin) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	*Resp = Pin->Node.Id;

	Status = XST_SUCCESS;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function requests pin.
 *
 * @param SubsystemId	Subsystem ID.
 * @param PinId		Pin ID.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code.
 *
 ****************************************************************************/
XStatus XPmPin_Request(const u32 SubsystemId, const u32 PinId)
{
	XStatus Status = XST_FAILURE;
	XPm_PinNode *Pin;

	Pin = XPmPin_GetById(PinId);
	if (NULL == Pin) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (Pin->SubsysIdx != (u16)NODEINDEX(INVALID_SUBSYSID)) {
		if (Pin->SubsysIdx == NODEINDEX(SubsystemId)) {
			/* Pin requested again by same subsystem */
			Status = XST_SUCCESS;
			goto done;
		}
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	Pin->SubsysIdx = (u16)(NODEINDEX(SubsystemId));

	Status = XST_SUCCESS;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function releases pin.
 *
 * @param SubsystemId	Subsystem ID.
 * @param PinId		Pin ID.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code.
 *
 ****************************************************************************/
XStatus XPmPin_Release(const u32 SubsystemId, const u32 PinId)
{
	XStatus Status = XST_FAILURE;
	XPm_PinNode *Pin;

	Pin = XPmPin_GetById(PinId);
	if (NULL == Pin) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (Pin->SubsysIdx != NODEINDEX(SubsystemId)) {
		Status = XST_FAILURE;
		goto done;
	}

	Pin->SubsysIdx = (u16)NODEINDEX(INVALID_SUBSYSID);

	Status = XST_SUCCESS;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function checks permission for given pin. If subsystem
 *	   requested this pin it returns success else error code.
 *
 * @param SubsystemId	Subsystem ID.
 * @param PinId		Pin ID.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code.
 *
 ****************************************************************************/
XStatus XPmPin_CheckPerms(const u32 SubsystemId, const u32 PinId)
{
	XStatus Status = XST_FAILURE;
	const XPm_PinNode *Pin;

	Pin = XPmPin_GetById(PinId);
	if (NULL == Pin) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (Pin->SubsysIdx != NODEINDEX(SubsystemId)) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}
