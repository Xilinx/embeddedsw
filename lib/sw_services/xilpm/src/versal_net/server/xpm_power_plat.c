/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_power.h"
#include "xpm_regs.h"
#include "xpm_powerdomain.h"
#include "xpm_psm.h"

XStatus XPmPower_PlatSendPowerUpReq(XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;

	switch (NODEINDEX(Node->Id)) {
	case (u32)XPM_NODEIDX_POWER_CPM5N:
		Status = XPm_PowerUpCPM5N(Node);
		break;
	case (u32)XPM_NODEIDX_POWER_HNICX:
		Status = XPm_PowerUpHnicx();
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

	return Status;
}

XStatus XPmPower_PlatSendPowerDownReq(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;

	switch (NODEINDEX(Node->Id)) {
	case (u32)XPM_NODEIDX_POWER_CPM5N:
		Status = XPm_PowerDwnCPM5N(Node);
		break;
	case (u32)XPM_NODEIDX_POWER_HNICX:
		Status = XPm_PowerDwnHnicx();
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

	return Status;
}

void XPmPower_SetPsmRegInfo(XPm_Power *Power, const u32 *Args)
{
	u32 Offset, Width;

	Power->PwrUpEnOffset = ((Args[7] & 0xFFU) == 0U) ?
				PSMX_GLOBAL_REQ_PWRUP0_EN_OFFSET :
				PSMX_GLOBAL_REQ_PWRUP1_EN_OFFSET;
	Power->PwrDwnEnOffset = ((Args[7] & 0xFF00U) == 0U) ?
				 PSMX_GLOBAL_REQ_PWRDWN0_EN_OFFSET :
				 PSMX_GLOBAL_REQ_PWRDWN1_EN_OFFSET;
	Power->PwrStatOffset = ((Args[7] & 0xFF0000U) == 0U) ?
				PSMX_GLOBAL_PWR_STATE0_OFFSET :
				PSMX_GLOBAL_PWR_STATE1_OFFSET;
	Offset = (Args[1] & 0xFFU);
	Width = (Args[1] & 0xFF00U) >> 8U;
	Power->PwrUpMask = BITNMASK(Offset, Width);
	Offset = (Args[8] & 0xFFU);
	Width = (Args[8] & 0xFF00U) >> 8U;
	Power->PwrDwnMask = BITNMASK(Offset, Width);
	Offset = (Args[8] & 0xFF0000U) >> 16U;
	Width = (Args[8] & 0xFF000000U) >> 24U;
	Power->PwrStatMask = BITNMASK(Offset, Width);
}

XStatus XPmPower_SendIslandPowerUpReq(const XPm_Node *Node)
{
	return XPmPsm_SendPowerUpReq((XPm_Power *)Node);
}

XStatus XPmPower_SendIslandPowerDwnReq(const XPm_Node *Node)
{
	return XPmPsm_SendPowerDownReq((XPm_Power *)Node);
}

static XStatus SavePowerNode(XPm_Power* ThisData) {
	XStatus Status = XST_FAILURE;
	u32* SavedData = NULL;

	BEGIN_SAVE_STRUCT(SavedData, XPmNode_SaveNode, ((XPm_Node*)ThisData));
	SaveStruct(Status, done, ((ThisData->UseCount)<<8)|(ThisData->WfParentUseCnt));
	END_SAVE_STRUCT(SavedData);

	Status = XST_SUCCESS;
done:
	XPM_UPDATE_THROW_IF_ERROR(Status, ThisData);
	return Status;
}

static XStatus RestorePowerNode(u32* SavedData, XPm_Power *ThisData){
	u32* DataAddr = NULL;
	XStatus Status = XPmNode_RestoreNode(SavedData, &(ThisData->Node), &DataAddr);
	if (XST_SUCCESS != Status)
	{
		goto done;
	}
	u32 tmp = 0;
	RestoreStruct(DataAddr, tmp);
	ThisData->UseCount = (tmp >> 8) & 0xFF;
	ThisData->WfParentUseCnt = (tmp) & 0xFF;
	Status = XST_SUCCESS;
done:
	return Status;
}

XStatus XPmPower_DoSaveRestore(u32* SavedData, u32* ThisData, u32 Op){
	XStatus Status = XST_FAILURE;
	if (XPLMI_STORE_DATABASE  == Op){
		Status = SavePowerNode((XPm_Power*)ThisData);
		goto done;
	}

	if (XPLMI_RESTORE_DATABASE == Op){
		Status = RestorePowerNode(SavedData, (XPm_Power*)ThisData);
		goto done;
	}
	Status = XPM_UPDATE_UNKNOWN_OP;
done:
	return Status;
}
