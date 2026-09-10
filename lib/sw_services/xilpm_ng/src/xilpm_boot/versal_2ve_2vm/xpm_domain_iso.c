/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_domain_iso.h"
#include "xpm_debug.h"
#include "xpm_powerdomain.h"
#include "xpm_device.h"
#include "xpm_regs.h"
#include "xpm_alloc.h"
#include "xpm_update.h"

static XPm_Iso* XPmDomainIso_List[XPM_NODEIDX_ISO_MAX] XPM_INIT_DATA(XPmDomainIso_List) = { NULL };

/****************************************************************************/
/**
 * @brief  Check whether an isolation node is a PSM single-word type.
 *
 * @param  ISO_NODE: Pointer to the isolation node
 *
 * @return 1U if the node format is PSM single-word (active low or high),
 *         0U otherwise
 *
 ****************************************************************************/
#define IS_PSM_ISO(ISO_NODE) (((((ISO_NODE)->Format) == (u32)PSM_SINGLE_WORD_ACTIVE_LOW) || \
			      (((ISO_NODE)->Format) == (u32)PSM_SINGLE_WORD_ACTIVE_HIGH)) ? 1U : 0U)
/****************************************************************************/
/**
 * @brief  Determine the active polarity of an isolation node from its format.
 *
 * @param  ISO_NODE: Pointer to the isolation node
 *
 * @return ACTIVE_HIGH if the node format is single-word active high or PSM
 *         single-word active high, ACTIVE_LOW otherwise
 *
 ****************************************************************************/
#define GET_POLARITY(ISO_NODE) (((((ISO_NODE)->Format) == (u32)SINGLE_WORD_ACTIVE_HIGH) || \
				(((ISO_NODE)->Format) == (u32)PSM_SINGLE_WORD_ACTIVE_HIGH)) ? ACTIVE_HIGH : ACTIVE_LOW)
/****************************************************************************/
/**
 * @brief  Get the mask value that enables (turns on) an isolation node.
 *
 * @param  ISO_NODE: Pointer to the isolation node
 *
 * @return The node's Mask when its polarity is ACTIVE_HIGH, 0U otherwise
 *
 ****************************************************************************/
#define ISO_ON(ISO_NODE) (((GET_POLARITY(ISO_NODE) == ACTIVE_HIGH)) ? (ISO_NODE)->Mask : 0U)
#define ISO_OFF(ISO_NODE) ~(ISO_ON(ISO_NODE))

/****************************************************************************/
/**
 * @brief	Write the isolation value to the AFI FM read and write control ports
 *
 * @param IsoNode	Pointer to the isolation node
 * @param Value		Value to write to the RD_CTRL and WR_CTRL ports
 *
 * @note	None
 *
 ****************************************************************************/
static void XPmDomainIso_AfifmControl(const XPm_Iso *IsoNode, u32 Value)
{
	/* Write RD_CTRL port */
	XPm_RMW32(IsoNode->Node.BaseAddress, IsoNode->Mask, Value);

	/* Write WR_CTRL port */
	XPm_RMW32(IsoNode->Node.BaseAddress + AFI_FM_WR_CTRL_OFFSET, IsoNode->Mask, Value);
}

/****************************************************************************/
/**
 * @brief	Write an isolation node's control value.
 *
 *		AFI FM nodes update masked fields in both read-control and
 *		write-control registers. All other non-PSM formats update the masked
 *		field at the node base address. PSM isolation formats are rejected.
 *
 * @param IsoNode	Pointer to the isolation node containing the register
 *			address, mask, and format.
 * @param Value		Value applied to the node's masked control field.
 *
 * @return	XST_SUCCESS after the register update, or XST_INVALID_PARAM
 *			for a PSM isolation format.
 *
 ****************************************************************************/
static XStatus XPmDomainIso_SetValue(const XPm_Iso* IsoNode, u32 Value){
	XStatus Status = XST_FAILURE;

	/* PSM-format isolation nodes are not controlled through this path */
	if (0U != IS_PSM_ISO(IsoNode)) {
		PmErr("Not expecting a PSM iso node!\r\n");
		Status = XST_INVALID_PARAM;
	} else if ((u32)PM_ISO_FORMAT_AFI_FM == IsoNode->Format) {
		/* AFI FM isolation uses paired read- and write-control registers */
		XPmDomainIso_AfifmControl(IsoNode, Value);
		Status = XST_SUCCESS;
	} else {
		/* Other formats update one masked field at the node base address */
		XPm_RMW32(IsoNode->Node.BaseAddress, IsoNode->Mask, Value);
		Status = XST_SUCCESS;
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief	Check whether all power domain and device dependencies of the
 *		isolation node are ready for isolation removal
 *
 * @param IsoNode	Pointer to the isolation node
 *
 * @return	XST_SUCCESS if all dependencies are satisfied, XST_FAILURE otherwise
 *
 ****************************************************************************/
static XStatus XPmDomainIso_CheckDependencies(const XPm_Iso* IsoNode)
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

/****************************************************************************/
/**
 * @brief	Allocate, initialize, and register an isolation node.
 *
 *		Initializes the node in the isolation-on state, stores its format and
 *		mask, copies the dependency IDs, and publishes it in the isolation
 *		node table at the index encoded by NodeId.
 *
 * @param NodeId	Unique isolation node ID.
 * @param BaseAddress	Base address of the isolation control register.
 * @param Mask		Bit mask controlling isolation.
 * @param Format	Isolation control format.
 * @param Dependencies	Array of dependency node IDs to copy into the node;
 *			must be non-NULL when NumDependencies is nonzero.
 * @param NumDependencies	Number of IDs in Dependencies; must not exceed
 *			PM_ISO_MAX_NUM_DEPENDENCIES.
 *
 * @return	XST_SUCCESS after the node is registered, or XST_FAILURE if
 *			the node index is out of range or allocation fails.
 *
 ****************************************************************************/

XStatus XPmDomainIso_NodeInit(u32 NodeId, u32 BaseAddress, u32 Mask, u32 Format,
			      const u32* Dependencies, u32 NumDependencies)
{
	XStatus Status = XST_FAILURE;
	u32 NodeIdx = NODEINDEX(NodeId);
	/* Validate the table slot encoded in the isolation node ID */
	if (NodeIdx >= (u32)XPM_NODEIDX_ISO_MAX) {
		goto done;
	}

	XPm_Iso *IsoNode = (XPm_Iso*)XPm_AllocBytes(sizeof(XPm_Iso));
	if (IsoNode == NULL) {
		goto done;
	}

	/* Initialize the software state with isolation asserted */
	XPmNode_Init(&IsoNode->Node, NodeId, (u8)PM_ISOLATION_ON, BaseAddress);
	IsoNode->Format = Format;
	IsoNode->Mask = Mask;
	IsoNode->NumDependencies = NumDependencies;

	/* Copy the dependency IDs supplied by the topology parser */
	for (u32 i =0; i < NumDependencies; i++) {
		IsoNode->Dependencies[i] = Dependencies[i];
	}

	/* Publish the initialized node in its indexed lookup slot */
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
 * 			0: isolation removal or disable
 * 			1: isolation installation or enable
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

	if ((XPM_ISO_DISABLE_IMMEDIATE == Enable) || (XPM_ISO_DISABLE == Enable)) {
		Output = ISO_OFF(XPmDomainIso_List[IsoIdx]);
	} else {
		Output = ISO_ON(XPmDomainIso_List[IsoIdx]);
	}

	/* Isolation node's states resolving to handle deferred isolation removal*/
	if ((XPM_ISO_ENABLE == Enable) || (XPM_ISO_ENABLE_PENDING == Enable)) {
		/* To turn on isolation */
		(void)XPmDomainIso_SetValue(IsoNode, Output);
		/* Mark node state appropriately, for later remove isolation in the case of XPM_ISO_ENABLE_PENDING */
		IsoNode->Node.State = (XPM_ISO_ENABLE == Enable) ?
			(u8)PM_ISOLATION_ON : (u8)PM_ISOLATION_REMOVE_PENDING;
	} else if(XPM_ISO_DISABLE_IMMEDIATE == Enable) {
		/* Immediately remove isolation*/
		(void)XPmDomainIso_SetValue(IsoNode, Output);
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
			(void)XPmDomainIso_SetValue(IsoNode, Output);
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
 * 		This function is used as "refreshing" of all isolation nodes state
 * 		to make sure all pending isolation got attention and being process
 *		further. This should be called at device state transition.
 * @param IsoIdx	Isolation node index that need to control
 * @param Enable	Desired isolation output state.
 * 			0: isolation removal or disable
 * 			1: isolation installation or enable
 *
 * @return	XST_SUCCESS or error code.
 *
 * @note	None
 *
 ****************************************************************************/
XStatus XPmDomainIso_ProcessPending(void)
{
	XStatus Status = XST_FAILURE;
	const XPm_Iso* IsoNode;
	u32 i;

	for(i = 0; i < ARRAY_SIZE(XPmDomainIso_List); i++) {
		IsoNode = XPmDomainIso_List[i];
		if (NULL != IsoNode){
			if (IsoNode->Node.State == (u8)PM_ISOLATION_REMOVE_PENDING) {
				Status = (XStatus)(XPmDomainIso_Control(i, XPM_ISO_DISABLE));
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
 * 			PM_ISOLATION_ON.
 * 			PM_ISOLATION_OFF
 *
 * @return	XST_SUCCESS or error code.
 *
 * @note	None
 *
 ****************************************************************************/
maybe_unused static XStatus XPmDomainIso_GetState(u32 IsoIdx, XPm_IsoStates *State)
{
	XStatus Status = XST_FAILURE;
	u32 Mask, BaseAddress, Polarity, RegVal;
	const XPm_Iso* IsoNode;

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
	Polarity = (u32)GET_POLARITY(IsoNode);

	if (0U != IS_PSM_ISO(IsoNode)) {
		PmErr("Not expecting a PSM iso node!\r\n");
		Status = XST_INVALID_PARAM;
		goto done;
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
