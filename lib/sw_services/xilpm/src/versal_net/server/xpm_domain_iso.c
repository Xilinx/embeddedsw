/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_domain_iso.h"
#include "xpm_debug.h"
#include "xpm_powerdomain.h"
#include "xpm_device.h"
#include "xpm_ipi.h"
#include "xpm_psm.h"

static XPm_Iso* XPmDomainIso_List[XPM_NODEIDX_ISO_MAX];

#define IS_PSM_ISO(ISO_NODE) ((((ISO_NODE)->Format) == (u32)PSM_SINGLE_WORD_ACTIVE_LOW) || \
			      (((ISO_NODE)->Format) == (u32)PSM_SINGLE_WORD_ACTIVE_HIGH)) ? 1U : 0U
#define GET_POLARITY(ISO_NODE) ((((ISO_NODE)->Format) == (u32)SINGLE_WORD_ACTIVE_HIGH) || \
				(((ISO_NODE)->Format) == (u32)PSM_SINGLE_WORD_ACTIVE_HIGH)) ? ACTIVE_HIGH : ACTIVE_LOW
#define ISO_ON(ISO_NODE) (GET_POLARITY(ISO_NODE) == ACTIVE_HIGH) ? (ISO_NODE)->Mask : 0U
#define ISO_OFF(ISO_NODE) ~(ISO_ON(ISO_NODE))

static XStatus XPmDomainIso_PsmSetValue(XPm_Iso* IsoNode, u32 Value)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 Payload[PAYLOAD_ARG_CNT] = {0U};

	if (1U != XPmPsm_FwIsPresent()) {
		DbgErr = XPM_INT_ERR_PSMFW_NOT_PRESENT;
		Status = XST_NOT_ENABLED;
		goto done;
	}

	Payload[0U] = PSM_API_DOMAIN_ISO;
	Payload[1U] = PSM_API_DOMAIN_ISO_SETTER_HEADER;
	Payload[2U] = IsoNode->Node.BaseAddress;
	Payload[3U] = IsoNode->Mask;
	Payload[4U] = Value;

	Status = XPm_IpiSend(PSM_IPI_INT_MASK, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPm_IpiReadStatus(PSM_IPI_INT_MASK);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static void XPmDomainIso_AfifmControl(XPm_Iso *IsoNode, u32 Value)
{
	/* Write RD_CTRL port */
	XPm_RMW32(IsoNode->Node.BaseAddress, IsoNode->Mask, Value);

	/* Write WR_CTRL port */
	XPm_RMW32(IsoNode->Node.BaseAddress + AFI_FM_WR_CTRL_OFFSET, IsoNode->Mask, Value);
}

static XStatus XPmDomainIso_SetValue(XPm_Iso* IsoNode, u32 Value){
	XStatus Status = XST_FAILURE;

	if (0U != IS_PSM_ISO(IsoNode)) {
		Status = XPmDomainIso_PsmSetValue(IsoNode, Value);
	} else if ((u32)PM_ISO_FORMAT_AFI_FM == IsoNode->Format) {
		XPmDomainIso_AfifmControl(IsoNode, Value);
		Status = XST_SUCCESS;
	} else {
		XPm_RMW32(IsoNode->Node.BaseAddress, IsoNode->Mask, Value);
		Status = XST_SUCCESS;
	}
	return Status;
}

static XStatus XPmDomainIso_CheckDependencies(XPm_Iso* IsoNode)
{
	XStatus Status = XST_FAILURE;
	u32 i=0, NodeId;
	const XPm_PowerDomain *PwrDomainNode;
	const XPm_Device *Device;

	for (i = 0; i < IsoNode->NumDependencies; i++) {
		NodeId = IsoNode->Dependencies[i];
		if (NODECLASS(NodeId) == (u32)XPM_NODECLASS_POWER) {
			PwrDomainNode = (XPm_PowerDomain *) XPmPower_GetById(NodeId);
			if ((NULL != PwrDomainNode) &&
			    (PwrDomainNode->Power.Node.State != (u8)XPM_POWER_STATE_ON)  &&
			    (PwrDomainNode->Power.Node.State != (u8)XPM_POWER_STATE_INITIALIZING)) {
				Status = XST_FAILURE;
				goto done;
			}
		} else if (PM_DEV_PLD_0 == NodeId) {
			Device = XPmDevice_GetById(NodeId);
			if ((NULL != Device) &&
				((u8)XPM_DEVSTATE_RUNNING != Device->Node.State) &&
				((u8)XPM_DEVSTATE_INITIALIZING != Device->Node.State)) {
				Status = XST_FAILURE;
				goto done;
			}
		} else {
			Status = XST_FAILURE;
			goto done;
		}
	}

	Status = XST_SUCCESS;
done:
	return Status;
}

static XStatus XPmDomainIso_PsmGetValue(XPm_Iso* IsoNode, u32 *OutValue)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 Payload[PAYLOAD_ARG_CNT] = {0U};

	if (1U != XPmPsm_FwIsPresent()) {
		DbgErr = XPM_INT_ERR_PSMFW_NOT_PRESENT;
		Status = XST_NOT_ENABLED;
		goto done;
	}

	Payload[0U] = PSM_API_DOMAIN_ISO;
	Payload[1U] = PSM_API_DOMAIN_ISO_GETTER_HEADER;
	Payload[2U] = IsoNode->Node.BaseAddress;
	Payload[3U] = IsoNode->Mask;

	Status = XPm_IpiSend(PSM_IPI_INT_MASK, Payload);
	if (XST_SUCCESS != Status){
		goto done;
	}

	Status = XPm_IpiRead(PSM_IPI_INT_MASK, &Payload);
	if (XST_SUCCESS != Status){
		goto done;
	}

	*OutValue = Payload[1];

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/****************************************************************************/
/**
 * @brief	Isolation Node initializing function
 * @param NodeId	Node unique identification number
 * @param Mask		Mask of bits field that control isolation
 * @param Format	Isolation control format
 * @param Dependencies	List of power domain node id that this isolation control
 *			depends on.
 * @param NumDependencies	Number of dependencies.
 *
 * @return	XST_SUCCESS or error code.
 *
 * @note	None
 *
 ****************************************************************************/

XStatus XPmDomainIso_NodeInit(u32 NodeId, u32 BaseAddress, u32 Mask, u32 Format,
			      const u32* Dependencies, u32 NumDependencies)
{
	XStatus Status = XST_FAILURE;
	u32 NodeIdx = NODEINDEX(NodeId);
	if (NodeIdx >= (u32)XPM_NODEIDX_ISO_MAX) {
		goto done;
	}

	XPm_Iso *IsoNode = (XPm_Iso*)XPm_AllocBytes(sizeof(XPm_Iso));
	if (IsoNode == NULL) {
		goto done;
	}

	XPmNode_Init(&IsoNode->Node, NodeId, (u8)PM_ISOLATION_ON, BaseAddress);
	IsoNode->Format = Format;
	IsoNode->Mask = Mask;
	IsoNode->NumDependencies = NumDependencies;

	for (u32 i =0; i < NumDependencies; i++) {
		IsoNode->Dependencies[i] = Dependencies[i];
	}

	XPmDomainIso_List[NodeIdx] = IsoNode;
	Status = XST_SUCCESS;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief	Isolation control handling
 * @param IsoIdx	Isolation node index that need to control
 * @param Enable	Desired isolation output state.
 *			0: isolation removal or disable
 *			1: isolation installation or enable
 *
 * @return	XST_SUCCESS or error code.
 *
 * @note	None
 *
 ****************************************************************************/
XStatus XPmDomainIso_Control(u32 IsoIdx, u32 Enable)
{
	XStatus Status = XST_FAILURE;
	u32 Output = 0;
	XPm_Iso* IsoNode;

	if (IsoIdx >= (u32)XPM_NODEIDX_ISO_MAX) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	IsoNode = XPmDomainIso_List[IsoIdx];
	if (NULL == IsoNode){
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	if ((FALSE_IMMEDIATE == Enable) || (FALSE_VALUE == Enable)) {
		Output = ISO_OFF(XPmDomainIso_List[IsoIdx]);
	} else {
		Output = ISO_ON(XPmDomainIso_List[IsoIdx]);
	}

	/* Isolation node's states resolving to handle deferred isolation removal*/
	if ((TRUE_VALUE == Enable) || (TRUE_PENDING_REMOVE == Enable)) {
		/* To turn on isolation */
		XPmDomainIso_SetValue(IsoNode, Output);
		/* Mark node state appropriately, for later remove isolation in the case of TRUE_PENDING_REMOVE */
		IsoNode->Node.State = (TRUE_VALUE == Enable) ?
			(u8)PM_ISOLATION_ON : (u8)PM_ISOLATION_REMOVE_PENDING;
	} else if(FALSE_IMMEDIATE == Enable) {
		/* Immediately remove isolation*/
		XPmDomainIso_SetValue(IsoNode, Output);
		IsoNode->Node.State = (u8)PM_ISOLATION_OFF;
	} else { /* Deferred isolation removal*/
		/* Check dependencies */
		Status = XPmDomainIso_CheckDependencies(IsoNode);
		if(XST_SUCCESS != Status) {
			/* Mark it pending if dependencies are not ready for isolation removal*/
			IsoNode->Node.State = (u8)PM_ISOLATION_REMOVE_PENDING;
			/*No output here. Keep isolation registers same state.*/
		} else {
			/* if all dependencies are ready, go ahead and remove isolation */
			XPmDomainIso_SetValue(IsoNode, Output);
			IsoNode->Node.State = (u8)PM_ISOLATION_OFF;
		}
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief	Isolation control process pending
 *		This function is used as "refreshing" of all isolation nodes state
 *		to make sure all pending isolation got attention and being process
 *		further. This should be called at device state transition.
 * @param IsoIdx	Isolation node index that need to control
 * @param Enable	Desired isolation output state.
 *			0: isolation removal or disable
 *			1: isolation installation or enable
 *
 * @return	XST_SUCCESS or error code.
 *
 * @note	None
 *
 ****************************************************************************/
XStatus XPmDomainIso_ProcessPending()
{
	XStatus Status = XST_FAILURE;
	XPm_Iso* IsoNode;
	u32 i;

	for(i = 0; i < ARRAY_SIZE(XPmDomainIso_List); i++) {
		IsoNode = XPmDomainIso_List[i];
		if (NULL != IsoNode){
			if (IsoNode->Node.State == (u8)PM_ISOLATION_REMOVE_PENDING) {
				Status = XPmDomainIso_Control(i, FALSE_VALUE);
				if ((XST_SUCCESS != Status) && (XST_DEVICE_NOT_FOUND != Status)){
					/* if device is not found, we still scan through the list of iso node*/
					goto done;
				}
			} else {
				Status = XST_SUCCESS;
			}
		}
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief	Get state of isolation node
 * @param IsoIdx	Isolation node index that need to get state
 * @param State		Output state.
 *			PM_ISOLATION_ON.
 *			PM_ISOLATION_OFF
 *
 * @return	XST_SUCCESS or error code.
 *
 * @note	None
 *
 ****************************************************************************/
XStatus XPmDomainIso_GetState(u32 IsoIdx, XPm_IsoStates *State)
{
	XStatus Status = XST_FAILURE;
	u32 Mask, BaseAddress, Polarity, RegVal;
	XPm_Iso* IsoNode;

	if ((IsoIdx >= (u32)XPM_NODEIDX_ISO_MAX) || (NULL == State)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	IsoNode = XPmDomainIso_List[IsoIdx];
	if (NULL == IsoNode){
		goto done;
	}

	Mask = IsoNode->Mask;
	BaseAddress = IsoNode->Node.BaseAddress;
	Polarity = GET_POLARITY(IsoNode);

	if (0U != IS_PSM_ISO(IsoNode)) {
		Status = XPmDomainIso_PsmGetValue(IsoNode, &RegVal);
		if (XST_SUCCESS != Status){
			goto done;
		}
	} else {
		RegVal = XPm_In32(BaseAddress);
	}

	if (Mask == (RegVal & Mask)) {
		*State = (Polarity == (u32)ACTIVE_HIGH)?
			PM_ISOLATION_ON : PM_ISOLATION_OFF;
	} else {
		*State = (Polarity == (u32)ACTIVE_HIGH)?
			PM_ISOLATION_OFF : PM_ISOLATION_ON;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}
