/******************************************************************************
 * Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/

#include "xplmi_update.h"
#include "xpm_update.h"
#include "xpm_common.h"
#include "xpm_node.h"
#include "xpm_subsystem.h"
#include "xpm_pldevice.h"
#include "xplmi_generic.h"
#include "xpm_core.h"
#include "xpm_apucore.h"
#include "xpm_rpucore.h"
#include "xpm_psm.h"
#include "xpm_pmc.h"
#include "xpm_pldevice.h"
#include "xpm_periph.h"
#include "xpm_clock.h"
#include "xpm_power.h"
#include "xpm_domain_iso.h"
#include "xpm_powerdomain.h"
#include "xpm_pin.h"
#include "xpm_reset.h"
#include "xpm_pll.h"
#include "xpm_pmcdomain.h"
#include "xpm_pslpdomain.h"
#include "xpm_psfpdomain.h"
#include "xpm_npdomain.h"
#include "xpm_pldomain.h"
#include "xpm_cpmdomain.h"
#include "xpm_hnicxdomain.h"
#include "xpm_regulator.h"
#include "xpm_requirement.h"
#include "xpm_update_data.h"
#include "xpm_notifier_plat.h"

#include "xloader.h"
#include "xloader_plat.h"
#include "xloader_ddr.h"

#define XPM_UPDATE_PSM_RST_TIMEOUT 150000U /** < Timeout to wait for PSM firmware us */
#define DONTCAREVAL 0xCAFECAFEU
#define MAX_NUM_NODE 1000
extern int XPlm_RemoveKeepAliveTask(void);
static XPm_Node* AllNodes[MAX_NUM_NODE] = {NULL};
static XPm_SaveRegionInfo AllSaveRegionsInfo[INDEX(XPm_Max)];
static void Init_SRInfo(void);

#define X(T) AllSaveRegionsInfo[INDEX(T)]= (XPm_SaveRegionInfo){ .Offset = offsetof(T, save), .Size = member_size(T,save)};
void Init_SRInfo() {
	LIST_OF_XPM_TYPE
};
#undef X

/**
 * @brief This macro converts a pointer member of a given struct Type,
 * from new offset to old offset of the member within the given struct Type.
 * The return of this macro is not only the offset but also the pointer to the data of the old member
 *  Let's  X and X' are the offset of old and new  data member.
 *	   S and S' are offset of the save region.
 *
 *   +---------------+		   +---------------+
 *   |		     |		   |		   |
 *   |		     |		   |		   |
 *   +---------------+ ---> S	   |		   |
 *   |	   Save      |		   |		   |
 *   +---------------+		   |		   |
 *   |	 PtrMember1  |		   +---------------+ ----> S'
 *   +---------------+ ---> X	   |		   |
 *   |	 PtrMember2  |		   |	Save	   |
 *   +---------------+		   |		   |
 *   |		     |		   +---------------+
 *   +---------------+		   |  PtrMember1   |
 *				   +---------------+ ----> X'
 *				   |  PtrMember2   |
 *				   +---------------+
 *
 * It is important that there's no new data inserted between PtrMember1 and Save
 * With that condition, we have this following equation
 * (X - S) - (X' - S') = size(S) - size(S')
 *  To find X = X' + S - S' + Size(S) - Size(S')
 */
#define GET_SAVED_PTR_MEMBER_FROM_TYPE(Type, Ptr, PtrMember)\
	(typeof(PtrMember))XPm_ConvertToSavedAddress(Xil_In32(\
		RoundToWordAddr(\
			(u32)(Ptr) + (UINTPTR)(&(PtrMember))\
			+ AllSaveRegionsInfo[INDEX(Type)].Offset - (UINTPTR)(&(Ptr->save))\
			+ AllSaveRegionsInfo[INDEX(Type)].Size - sizeof(Ptr->save))));

static u32 PrevNumSaveRegionInfo = Index_XPm_Max;
static u32 NumNodes = 0;
static u32 SavedAllNodesAddr = 0;
static u32 PrevNumNodes = 0;

/* Handle to save AllNodes data. This is later use for iterating through the saved node*/
static int XPmNode_AllNodesOps(u32 Op, u64 Addr, void *Data);
/* Handle to save NumNodes. This is later use for iterating through the saved nodes */
static int XPmNode_NumNodesOps(u32 Op, u64 Addr, void *Data);


/* Handle to save AllSaveRegionsInfo data.
 * This is data to track previous save markers blocks/region.
 */
static int XPm_AllSRegionOps(u32 Op, u64 Addr, void *Data);

EXPORT_DS_W_HANDLER(AllNodes, \
	XPLMI_MODULE_XILPM_ID, XPM_ALLNODES_DS_ID, \
	XPM_DATA_STRUCT_VERSION, XPM_DATA_STRUCT_LCVERSION, \
	sizeof(AllNodes), (u32)(UINTPTR)AllNodes , XPmNode_AllNodesOps);

EXPORT_DS_W_HANDLER(PrevNumNodes, \
	XPLMI_MODULE_XILPM_ID, XPM_NUMNODES_DS_ID, \
	XPM_DATA_STRUCT_VERSION, XPM_DATA_STRUCT_LCVERSION, \
	sizeof(PrevNumNodes), (u32)(UINTPTR)(&PrevNumNodes) , XPmNode_NumNodesOps);

EXPORT_DS_W_HANDLER(AllSaveRegionsInfo, \
	XPLMI_MODULE_XILPM_ID, XPM_ALLSAVEREGIONSINFO_DS_ID, \
	XPM_DATA_STRUCT_VERSION, XPM_DATA_STRUCT_LCVERSION, \
	sizeof(AllSaveRegionsInfo), (u32)(UINTPTR)AllSaveRegionsInfo , XPm_AllSRegionOps);

EXPORT_DS(PrevNumSaveRegionInfo, \
	XPLMI_MODULE_XILPM_ID, XPM_PREVNUMSAVEREGIONINFO_DS_ID, \
	XPM_DATA_STRUCT_VERSION, XPM_DATA_STRUCT_LCVERSION, \
	sizeof(PrevNumSaveRegionInfo), (u32)(UINTPTR)(&PrevNumSaveRegionInfo));

static int XPmNode_AllNodesOps(u32 Op, u64 Addr, void *Data)
{
	XStatus Status = XST_FAILURE;
	if (XPLMI_STORE_DATABASE == Op) {
		/* Copy Data Structure to given address */
		Status = XPlmi_DsOps(Op, Addr, Data);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	} else if (XPLMI_RESTORE_DATABASE == Op) {
		/** Need to know the address of the storage of AllNodes
		*/
		SavedAllNodesAddr = (u32)(Addr + sizeof(XPlmi_DsHdr));
	}
	else {
		Status = XPLMI_ERR_PLM_UPDATE_INVALID_OP;
		goto done;
	}
	Status = XST_SUCCESS;

done:
	return Status;
}

static int XPm_AllSRegionOps(u32 Op, u64 Addr, void *Data)
{
	XStatus Status = XST_FAILURE;
	if (XPLMI_STORE_DATABASE == Op) {
		/* Capture the current Save Region Info */
		Init_SRInfo();
		/* Copy Data Structure to given address */
		Status = XPlmi_DsOps(Op, Addr, Data);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	} else if (XPLMI_RESTORE_DATABASE == Op) {
		/* Restore the current SaveRegionInfo */
		Status = XPlmi_DsOps(Op, Addr, Data);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	} else {
		Status = XPLMI_ERR_PLM_UPDATE_INVALID_OP;
		goto done;
	}

done:
	return Status;
}

static int XPmNode_NumNodesOps(u32 Op, u64 Addr, void *Data)
{
	XStatus Status = XST_FAILURE;
	if (XPLMI_STORE_DATABASE == Op) {
		/* Assigned current NumNodes */
		PrevNumNodes = NumNodes;
		/* Then store it. */
		Status = XPlmi_DsOps(Op, Addr, Data);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	} else if (XPLMI_RESTORE_DATABASE == Op) {
		/* Simply retstore it */
		Status = XPlmi_DsOps(Op, Addr, Data);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	} else {
		Status = XPLMI_ERR_PLM_UPDATE_INVALID_OP;
		goto done;
	}
	Status = XST_SUCCESS;

done:
	return Status;
}

/**
 * @brief Function to return a pointer to a Node within DDR region
 *
 * @param Index an index within the saved AllNodes array
 * @return NULL if Index out of bound; else pointer to XPm_Node within DDR region
 */
static XPm_Node* GetSavedNodeAt(u32 Index)
{
	if (0U == SavedAllNodesAddr) {
		return NULL;
	}
	if (PrevNumNodes < Index){
		return NULL;
	}
	u32 PrevNodeAddr = (u32)((XPm_Node**)SavedAllNodesAddr)[Index];
	u32 ByteBufferOffset =	XPm_GetByteBufferOffset();
	return (XPm_Node*)(PrevNodeAddr + ByteBufferOffset);
}

/**
 * @brief Function to return a pointer to a Node within runtime memory
 *
 * @param Index an index within the AllNodes array
 * @returnNULL if Index out of bound; else pointer to XPm_Node within runtime region
 */
static XPm_Node* GetNodeAt(u32 Index)
{
	if (Index > NumNodes){
		return NULL;
	}
	return AllNodes[Index];
}

/**
 * @brief Add a node to AllNodes array
 *
 * @param Node pointer to a Node to be added
 */
void XPmUpdate_AllNodes_Add(XPm_Node* Node)
{
	if(NULL == Node){
		PmErr("Error: Node can not be NULL !\n\r");
	} else if (NumNodes >= MAX_NUM_NODE){
		PmErr("Error: Too many Node !\n\r");
	} else {
		AllNodes[NumNodes++] = Node;
	}
}

/**
 * @brief Restore a mark region
 *
 * @param From a pointer to a saved region (source)
 * @param FromSize size of the saved region (source's size)
 * @param To a pointer to a runtime region (destination)
 * @param ToSize size of the runtime region (destination's size)
 * @return XST_SUCCESS if there's no error.
 */
static XStatus XPmUpdate_RegionRestore(u8 *From, u32 FromSize, u8 *To, u32 ToSize)
{
	XStatus Status = XST_FAILURE;
	u32 CpSize = FromSize >= ToSize? ToSize : FromSize;
	if (CpSize == 0) {
		/* Do nothing */
		Status = XST_SUCCESS;
		goto done;
	}
	Status = Xil_SMemCpy(To, ToSize, From, FromSize, CpSize);
done:
	return Status;
}

/**
 * @brief Node type base restore function.
 * It restored the region that is "marked" within the XPm_Node type.
 *
 * @param SavedNode the pointer to a Node that is in DDR
 * @param Node the pointer of a Node that is in current runtime memory.
 * @return XST_SUCCESS if there's no error.
 */
static XStatus XPm_Node_Restore(XPm_Node *SavedNode, XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;
	Status = RESTORE_REGION(SavedNode, AllSaveRegionsInfo[Index_XPm_Node], Node);
	if (XST_SUCCESS != Status){
		goto done;
	}
	Status = XST_SUCCESS;
done:
	return Status;
}
/**
 * @brief Subsystem type base restore function.
 * It restored the region that is "marked" within the XPm_Subsystem type.
 *
 * @param SavedNode the pointer to a Subsystem that is in DDR (source)
 * @param Node the pointer of a Node that is in current runtime memory (destination)
 * @return XST_SUCCESS
 */
static XStatus XPm_Subsystem_Restore(XPm_Subsystem *SavedNode, XPm_Subsystem *Node)
{
	XStatus Status = XST_FAILURE;
	Status = RESTORE_REGION(SavedNode, AllSaveRegionsInfo[Index_XPm_Subsystem], Node);
	if (XST_SUCCESS != Status){
		goto done;
	}
	Status = XST_SUCCESS;
done:
	return	Status;
}

/**
 * @brief Macro to make XPm_Device_Generic_Restore function
 *
 */
MAKE_GENERIC_RESTORE_FUNC(XPm_Device, XPm_Node, Node)

/**
 * @brief Find a requirement given device and subsystem
 *
 * @param Device pointer to a device
 * @param Subsystem pointer to a subsystem
 * @return NULL if there is no requirement between a given device and subsystem;
 *	else return a pointer to XPm_Requirement.
 */
static XPm_Requirement *FindReqm(const XPm_Device *Device, const XPm_Subsystem *Subsystem)
{
	XPm_Requirement *Reqm = NULL;
	Reqm = Device->Requirements;
	while (NULL != Reqm) {
		if (Reqm->Subsystem == Subsystem) {
			break;
		}
		Reqm = Reqm->NextSubsystem;
	}

	return Reqm;
}
/**
 * @brief Get a requirement given pointer to a "saved" requirement.
 * Saved requirement is the requirement within DDR.
 * This function is to find the corresponding requirement within the runtime memory
 *
 * @param SavedReq a requirement that was saved within the DDR
 * @return NULL if there's no corresponding requirement in runtime memory;
 * else return the pointer to XPm_Requirement.
 */
static XPm_Requirement* GetCurReqFromSavedReq(XPm_Requirement *SavedReq)
{
	XPm_Requirement* CurReq = NULL;
	XPm_Device *SavedDevice = GET_SAVED_PTR_MEMBER_FROM_TYPE(XPm_Requirement, SavedReq, SavedReq->Device);

	XPm_Subsystem *SavedSubsystem = GET_SAVED_PTR_MEMBER_FROM_TYPE(XPm_Requirement, SavedReq, SavedReq->Subsystem);

	XPm_Subsystem* Subsystem = XPmSubsystem_GetById(SavedSubsystem->Id);
	if (NULL == Subsystem)
	{
		PmWarn("Warning:Subsystem 0x%x Not Found!\n\r", SavedSubsystem->Id);
		goto done;
	}
	XPm_Device* Device = XPmDevice_GetById(SavedDevice->Node.Id);
	if (NULL == Device)
	{
		PmWarn("Warning:Device 0x%x Not Found!\n\r", SavedDevice->Node.Id);
		goto done;
	}
	CurReq = FindReqm(Device , Subsystem);
done:
	return CurReq;
}

static XStatus CreateReqFrom(XPm_Requirement *SavedReq, XPm_Requirement **OutReq)
{
	XStatus Status = XST_FAILURE;
	XPm_Device *SavedDevice = GET_SAVED_PTR_MEMBER_FROM_TYPE(XPm_Requirement, SavedReq, SavedReq->Device);

	XPm_Subsystem *SavedSubsystem = GET_SAVED_PTR_MEMBER_FROM_TYPE(XPm_Requirement, SavedReq, SavedReq->Subsystem);

	XPm_Subsystem* Subsystem = XPmSubsystem_GetById(SavedSubsystem->Id);
	if (NULL == Subsystem)
	{
		PmErr("Subsystem 0x%x Not Found!\n\r", SavedSubsystem->Id);
		goto done;
	}
	XPm_Device* Device = XPmDevice_GetById(SavedDevice->Node.Id);
	if (NULL == Device)
	{
		PmErr("Device 0x%x Not Found!\n\r", SavedDevice->Node.Id);
		goto done;
	}
	Status =  XPmRequirement_Add(Subsystem, Device, DONTCAREVAL ,DONTCAREVAL ,DONTCAREVAL);
	if (XST_SUCCESS != Status) {
		goto done;
	}
	*OutReq = FindReqm(Device, Subsystem);
done:
	return Status;
}

/**
 * @brief Restore requirements from a saved device to the running device node.
 *
 * @param SavedNode a pointer to a saved XPm_Device node that reside within DDR.
 * @param Node a pointer to a XPm_Device node that is reside in runtime memory.
 * @return XST_SUCCESS if there is no error.
 */
static XStatus RestoreRequirements(XPm_Device *SavedNode, XPm_Device *Node)
{
	XStatus Status = XST_FAILURE;
	XPm_Requirement* SavedReq = GET_SAVED_PTR_MEMBER_FROM_TYPE(XPm_Device, SavedNode, SavedNode->Requirements);
	XPm_Requirement* SavedPendingReq = GET_SAVED_PTR_MEMBER_FROM_TYPE(XPm_Device, SavedNode, SavedNode->PendingReqm);
	if (NULL == SavedReq) {
		/* Nothing to be done here*/
		Status = XST_SUCCESS;
		goto done;
	}
	while(NULL != SavedReq) {
		XPm_Requirement* CurReq = GetCurReqFromSavedReq(SavedReq);
		if (NULL == CurReq) {
			Status = CreateReqFrom(SavedReq, &CurReq);
			if (XST_SUCCESS != Status) {
				PmWarn("Warning: Requirement not added %x %x\n\r", SavedReq->Device->Node.Id, SavedReq->Subsystem->Id);
				goto done;
			}
		}
		Status = RESTORE_REGION(SavedReq, AllSaveRegionsInfo[Index_XPm_Requirement], CurReq);
		if (XST_SUCCESS != Status){
			goto done;
		}

		SavedReq = GET_SAVED_PTR_MEMBER_FROM_TYPE(XPm_Requirement, SavedReq, SavedReq->NextSubsystem);
	}
	/* Restore Pending Requirement*/
	if (NULL != SavedPendingReq){
		XPm_Requirement *Req = GetCurReqFromSavedReq(SavedPendingReq);
		Node->PendingReqm = Req;
	}

	Status = XST_SUCCESS;
done:
	return Status;
}

/**
 * @brief A restore function for XPm_Device node.
 *
 * @param SavedNode pointer to a saved XPm_Device (source)
 * @param Node pointer to a running XPm_Device (destination)
 * @return XST_SUCCESS if there is no errors
 */
static XStatus XPm_Device_Restore(XPm_Device *SavedNode, XPm_Device *Node)
{
	XStatus Status = XST_FAILURE;
	Status = XPm_Device_Generic_Restore(SavedNode, Node);
	if (XST_SUCCESS != Status) {
		goto done;
	}
	Status = RestoreRequirements(SavedNode, Node);
	if (XST_SUCCESS != Status) {
		goto done;
	}
	Status = XST_SUCCESS;
done:
	return Status;
}
/**
 * @brief These are macros that construct the restore function for given type of node
 *
 */
MAKE_RESTORE_FUNC(XPm_ClockNode, XPm_Node, Node)
MAKE_RESTORE_FUNC(XPm_Power, XPm_Node, Node)
MAKE_RESTORE_FUNC(XPm_PinNode, XPm_Node, Node)
MAKE_RESTORE_FUNC(XPm_Iso, XPm_Node, Node)
MAKE_RESTORE_FUNC(XPm_ResetNode, XPm_Node, Node)
MAKE_RESTORE_FUNC(XPm_Regulator, XPm_Node, Node)

MAKE_RESTORE_FUNC(XPm_PowerDomain, XPm_Power, Power)
MAKE_RESTORE_FUNC(XPm_Rail, XPm_Power, Power)

MAKE_RESTORE_FUNC(XPm_Core, XPm_Device, Device)
MAKE_RESTORE_FUNC(XPm_Periph, XPm_Device, Device)
MAKE_RESTORE_FUNC(XPm_MemDevice, XPm_Device, Device)

MAKE_RESTORE_FUNC(XPm_ApuCore, XPm_Core, Core)
MAKE_RESTORE_FUNC(XPm_RpuCore, XPm_Core, Core)
MAKE_RESTORE_FUNC(XPm_Psm, XPm_Core, Core)
MAKE_RESTORE_FUNC(XPm_Pmc, XPm_Core, Core)

MAKE_RESTORE_FUNC(XPm_OutClockNode, XPm_ClockNode, ClkNode)
MAKE_RESTORE_FUNC(XPm_PllClockNode, XPm_ClockNode, ClkNode)

MAKE_RESTORE_FUNC(XPm_PsLpDomain, XPm_PowerDomain, Domain)
MAKE_RESTORE_FUNC(XPm_PsFpDomain, XPm_PowerDomain, Domain)
MAKE_RESTORE_FUNC(XPm_PlDomain, XPm_PowerDomain, Domain)
MAKE_RESTORE_FUNC(XPm_NpDomain, XPm_PowerDomain, Domain)
MAKE_RESTORE_FUNC(XPm_HnicxDomain, XPm_PowerDomain, Domain)
MAKE_RESTORE_FUNC(XPm_CpmDomain, XPm_PowerDomain, Domain)
MAKE_RESTORE_FUNC(XPm_PmcDomain, XPm_PowerDomain, Domain)
MAKE_RESTORE_FUNC(XPm_MemRegnDevice, XPm_Device, Device)

/**
 * @brief Get a pointer to a node that is within the DDR region
 *
 * @param NodeId Id of the node to search for.
 * @return NULL if there's no Node with the given node ID found in DDR region;
 * else return pointer to the found Node.
 */
static void* XPmUpdate_GetSavedDataById(u32 NodeId)
{
	for (u32 i = 0; i< PrevNumNodes; i++){
		XPm_Node* Node= GetSavedNodeAt(i);
		if (Node && Node->Id == NodeId) return Node;
	}
	return NULL;
}

/**
 * @brief Special function to add missing PL devices
 * PL devices that are previously saved may not exist in the new runtime.
 * This function make sure they're present and later restore them propally.
 * @return XST_SUCCESS if there is no errors.
 */
static XStatus AddMissingPlDevices(void)
{
	XStatus Status = XST_FAILURE;
	for (u32 i = 0; i< PrevNumNodes; i++){
		XPm_Node* Node= GetSavedNodeAt(i);
		if (NULL == Node) {
			/** We keep going and try to restore the rest */
			continue;
		}
		u32 NodeId = Node->Id;
		if (NODECLASS(NodeId) == XPM_NODECLASS_DEVICE \
			&&  NODESUBCLASS(NodeId) == XPM_NODESUBCL_DEV_PL) {
			if (NULL == XPmDevice_GetById(NodeId)) {
				u32 Args[1] = {NodeId};
				Status = XPm_AddNode(Args, ARRAY_SIZE(Args));
				if (XST_SUCCESS != Status) {
					goto done;
				}
			}
		}
	}
	Status = XST_SUCCESS;
done:
	return Status;
}
static XStatus AddMissingMemRegnDevices(void)
{
	XStatus Status = XST_FAILURE;
	for (u32 i = 0; i< PrevNumNodes; i++){
		XPm_Node* Node= GetSavedNodeAt(i);
		if (NULL == Node) {
			/** We keep going and try to restore the rest */
			continue;
		}
		u32 NodeId = Node->Id;
		if (NODECLASS(NodeId) == XPM_NODECLASS_DEVICE \
			&&  NODESUBCLASS(NodeId) == XPM_NODESUBCL_DEV_MEM_REGN) {
			if (NULL == XPmDevice_GetById(NodeId)) {
				u32 Args[5] = {NodeId, DONTCAREVAL, DONTCAREVAL, DONTCAREVAL, DONTCAREVAL};
				Status = XPm_AddNode(Args,ARRAY_SIZE(Args));
				if (XST_SUCCESS != Status) {
					goto done;
				}
			}
		}
	}
	Status = XST_SUCCESS;
done:
	return Status;
}


static XStatus AddMissingxGGsDevices(void)
{
	XStatus Status = XST_FAILURE;
	for (u32 i = 0; i< PrevNumNodes; i++){
		XPm_Node* Node= GetSavedNodeAt(i);
		if (NULL == Node) {
			/** We keep going and try to restore the rest */
			continue;
		}
		u32 NodeId = Node->Id;
		if (NODECLASS(NodeId) == XPM_NODECLASS_DEVICE \
			&& NODESUBCLASS(NodeId) == XPM_NODESUBCL_DEV_PERIPH
			&& (NODETYPE(NodeId) == XPM_NODETYPE_DEV_PGGS
				|| NODETYPE(NodeId) == XPM_NODETYPE_DEV_GGS)) {
			if (NULL == XPmDevice_GetById(NodeId)) {
				u32 Args[5] = {NodeId, PM_POWER_PMC, DONTCAREVAL, DONTCAREVAL, DONTCAREVAL};
				Status = XPm_AddNode(Args, ARRAY_SIZE(Args));
				if (XST_SUCCESS != Status) {
					goto done;
				}
			}
		}
	}
	Status = XST_SUCCESS;
done:
	return Status;
}
/**
 * @brief A restore function for XPm_PlDevice node.
 *
 * @param SavedNode pointer to a saved XPm_PlDevice (source)
 * @param Node pointer to a running XPm_PlDevice (destination)
 * @return XST_SUCCESS if there is no errors
 */
static XStatus XPm_PlDevice_Restore(XPm_PlDevice *SavedNode, XPm_PlDevice *Node)
{
	XStatus Status = XST_FAILURE;
	Status = XPm_Device_Restore(&(SavedNode->Device), &(Node->Device));
	if (XST_SUCCESS != Status) {
		goto done;
	}
	Status = RESTORE_REGION(SavedNode, AllSaveRegionsInfo[Index_XPm_PlDevice], Node);
	if (XST_SUCCESS != Status) {
		goto done;
	}
	/* Restore MemCtrlDevice*/
	for(u32 i = 0 ; i< MAX_PLAT_DDRMC_COUNT; i++){
		XPm_MemCtrlrDevice* SavedMemCtrlr = GET_SAVED_PTR_MEMBER_FROM_TYPE(XPm_PlDevice, SavedNode, SavedNode->MemCtrlr[i]);
		if (NULL != SavedMemCtrlr){
			XPm_MemCtrlrDevice *CurMemCtrl = (XPm_MemCtrlrDevice*)XPmDevice_GetById(SavedMemCtrlr->Device.Node.Id);
			if (NULL == CurMemCtrl) {
				PmWarn("Wanring:MemCtrl Device not found. NodeId: %x\n\r", SavedMemCtrlr->Device.Node.Id);
			}
			Node->MemCtrlr[i] = CurMemCtrl;
		}
	}
	/* Restore Parents */
	XPm_PlDevice *PrevParent = GET_SAVED_PTR_MEMBER_FROM_TYPE(XPm_PlDevice, SavedNode, SavedNode->Parent);
	if (NULL != PrevParent) {
		XPm_PlDevice *CurParent =  (XPm_PlDevice*)XPmDevice_GetById(PrevParent->Device.Node.Id);
		if (NULL == CurParent) {
			PmWarn("Warning:PL device not found. NodeId: %x\n\r", PrevParent->Device.Node.Id);
		}
		Node->Parent = CurParent;
	}
	/* Restore Child */
	XPm_PlDevice *PrevChild = GET_SAVED_PTR_MEMBER_FROM_TYPE(XPm_PlDevice, SavedNode, SavedNode->Child);
	if (NULL != PrevChild){
		XPm_PlDevice *CurChild =  (XPm_PlDevice*)XPmDevice_GetById(PrevChild->Device.Node.Id);
		if (NULL == CurChild) {
			PmWarn("Warning:PL device not found. NodeId: %x\n\r", PrevChild->Device.Node.Id);
		}
		Node->Child = CurChild;
	}
	/* Restore Peer */
	XPm_PlDevice *PrevNextPeer = GET_SAVED_PTR_MEMBER_FROM_TYPE(XPm_PlDevice, SavedNode, SavedNode->NextPeer);
	if (NULL != PrevNextPeer){
		XPm_PlDevice *CurNextPeer =  (XPm_PlDevice*)XPmDevice_GetById(PrevNextPeer->Device.Node.Id);
		if (NULL == CurNextPeer) {
			PmWarn("Warning:PL device not found. NodeId: %x\n\r", PrevNextPeer->Device.Node.Id);
		}
		Node->NextPeer = CurNextPeer;
	}
	Status = XST_SUCCESS;
done:
	return Status;
}

/**
 * @brief A restore function for XPm_MemCtrlrDevice node.
 *
 * @param SavedNode pointer to a saved XPm_MemCtrlrDevice (source)
 * @param Node pointer to a running XPm_MemCtrlrDevice (destination)
 * @return XST_SUCCESS if there is no errors
 */
static XStatus XPm_MemCtrlrDevice_Restore(XPm_MemCtrlrDevice *SavedNode, XPm_MemCtrlrDevice *Node)
{
	XStatus Status = XST_FAILURE;
	Status = XPm_Device_Restore(&(SavedNode->Device), &(Node->Device));
	if (XST_SUCCESS != Status) {
		goto done;
	}
	Status = RESTORE_REGION(SavedNode, AllSaveRegionsInfo[Index_XPm_MemCtrlrDevice], Node);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	XPm_PlDevice *PrevPlDevice = GET_SAVED_PTR_MEMBER_FROM_TYPE(XPm_PlDevice, SavedNode, SavedNode->PlDevice);
	if (NULL != PrevPlDevice) {
		XPm_PlDevice *CurPlDevice = (XPm_PlDevice*)XPmDevice_GetById(PrevPlDevice->Device.Node.Id);
		if (NULL == CurPlDevice) {
			PmWarn("Warning:PL device not found. NodeId: %x\n\r",PrevPlDevice->Device.Node.Id);
			Status = XST_SUCCESS;
			goto done;
		}
		/** No need to restore Pl Device contents because it is done through the device restore routines */
		Node->PlDevice = CurPlDevice;
	}
	Status = XST_SUCCESS;
done:
	return Status;
}

static XStatus Class_Power_Restore(XPm_Power *SavedNode, XPm_Power *Node)
{
	XStatus Status = XST_FAILURE;
	u32 NodeId = Node->Node.Id;
	switch(NODETYPE(NodeId)){
	case XPM_NODETYPE_POWER_ISLAND:
		Status = RESTORE(XPm_Power, SavedNode, Node);
		break;
	case XPM_NODETYPE_POWER_DOMAIN_PMC:
		Status = RESTORE(XPm_PmcDomain, SavedNode, Node);
		break;
	case XPM_NODETYPE_POWER_DOMAIN_PS_FULL:
		Status = RESTORE(XPm_PsFpDomain, SavedNode, Node);
		break;
	case XPM_NODETYPE_POWER_DOMAIN_PS_LOW:
		Status = RESTORE(XPm_PsLpDomain, SavedNode, Node);
		break;
	case XPM_NODETYPE_POWER_DOMAIN_NOC:
		Status = RESTORE(XPm_NpDomain, SavedNode, Node);
		break;
	case XPM_NODETYPE_POWER_DOMAIN_CPM:
		Status = RESTORE(XPm_CpmDomain, SavedNode, Node);
		break;
	case XPM_NODETYPE_POWER_DOMAIN_PL:
		Status = RESTORE(XPm_PlDomain, SavedNode, Node);
		break;
	case XPM_NODETYPE_POWER_RAIL:
		Status = RESTORE(XPm_Rail, SavedNode, Node);
		break;
	case XPM_NODETYPE_POWER_REGULATOR:
		Status = RESTORE(XPm_Regulator, SavedNode, Node);
		break;
	case XPM_NODETYPE_POWER_DOMAIN_HNICX:
		Status = RESTORE(XPm_HnicxDomain, SavedNode, Node);
		break;
	case XPM_NODETYPE_POWER_DOMAIN_CTRL:
	case XPM_NODETYPE_POWER_ISLAND_XRAM:
	case XPM_NODETYPE_POWER_DOMAIN_ME:
	default:
		PmWarn("Warning: IPU unsupport power domain node. Id: %x.\n\r", NodeId);
		Status = XST_SUCCESS;
		break;
	}
	return Status;
}

/**
 * @brief Clock class restore function.
 * This function simply call to a correct function to
 * restore specific clock type based on the NodeId.
 *
 * @param SavedNode a pointer to a XPm_ClockNode that is in DDR region (source)
 * @param Node a pointer to a XPm_ClockNode that is in runtime region (destination)
 * @return XST_SUCCESS if there's no error.
 */
static XStatus Class_Clock_Restore(XPm_ClockNode *SavedNode, XPm_ClockNode *Node)
{
	XStatus Status = XST_FAILURE;
	u32 NodeId = Node->Node.Id;
	switch(NODETYPE(NodeId)){
	case XPM_NODETYPE_CLOCK_PLL:
		Status = RESTORE(XPm_PllClockNode, SavedNode, Node);
		break;
	case XPM_NODETYPE_CLOCK_OUT:
		Status = RESTORE(XPm_OutClockNode, SavedNode, Node);
		break;
	case XPM_NODETYPE_CLOCK_REF:
		Status = RESTORE(XPm_ClockNode, SavedNode, Node);
		break;
	case XPM_NODETYPE_CLOCK_SUBNODE:
	default:
		PmWarn("Warning: IPU unsupport clock node. Id: %x.\n\r", NodeId);
		Status = XST_SUCCESS;
		break;
	}
	return Status;
}

/**
 * @brief Device class restore function.
 * This function simply call to a correct function to
 * restore specific device type based on the NodeId.
 *
 * @param SavedNode a pointer to a XPm_Device that is in DDR region (source)
 * @param Node a pointer to a XPm_Device that is in runtime region (destination)
 * @return XST_SUCCESS if there's no error.
 */
static XStatus Class_Device_Restore(XPm_Device *SavedNode, XPm_Device *Node)
{
	XStatus Status = XST_FAILURE;
	u32 NodeId = Node->Node.Id;

	/** These nodes ignore NODETYPE, use NODESUBCLASS  */
	switch (NODESUBCLASS(NodeId)) {
	case XPM_NODESUBCL_DEV_PL:
		Status = RESTORE(XPm_PlDevice, SavedNode, Node);
		goto done;
	case XPM_NODESUBCL_DEV_MEM_CTRLR:
		Status = RESTORE(XPm_MemCtrlrDevice, SavedNode, Node);
		goto done;
	case XPM_NODESUBCL_DEV_MEM_REGN:
		Status = RESTORE(XPm_MemRegnDevice, SavedNode, Node);
		goto done;
	default:
		break;
	}

	switch(NODETYPE(NodeId)) {
	case XPM_NODETYPE_DEV_CORE_PMC:
		Status = RESTORE(XPm_Pmc, SavedNode, Node);
		break;
	case XPM_NODETYPE_DEV_CORE_PSM:
		Status = RESTORE(XPm_Psm, SavedNode, Node);
		break;
	case XPM_NODETYPE_DEV_CORE_APU:
		Status = RESTORE(XPm_ApuCore, SavedNode, Node);
		break;
	case XPM_NODETYPE_DEV_CORE_RPU:
		Status = RESTORE(XPm_RpuCore, SavedNode, Node);
		break;
	case XPM_NODETYPE_DEV_DDR:
	case XPM_NODETYPE_DEV_L2CACHE:
	case XPM_NODETYPE_DEV_EFUSE:
	case XPM_NODETYPE_DEV_OCM:
	case XPM_NODETYPE_DEV_TCM:
		Status = RESTORE(XPm_MemDevice, SavedNode, Node);
		break;
	case XPM_NODETYPE_DEV_PERIPH:
		Status = RESTORE(XPm_Periph, SavedNode, Node);
		break;
	case XPM_NODETYPE_DEV_GGS :
	case XPM_NODETYPE_DEV_PGGS :
		Status = RESTORE(XPm_Device, SavedNode, Node);
		break;
	case XPM_NODETYPE_DEV_SOC:
	case XPM_NODETYPE_DEV_GT:
	case XPM_NODETYPE_DEV_XRAM:
	case XPM_NODETYPE_DEV_RESERVED_0:
	case XPM_NODETYPE_DEV_RESERVED_1:
	case XPM_NODETYPE_DEV_HBM :
	case XPM_NODETYPE_DEV_VDU :
	case XPM_NODETYPE_DEV_HB_MON :
	case XPM_NODETYPE_DEV_BFRB :
	default:
		PmWarn("Warning: IPU unsupport device node. NodeId: %x.\n\r", NodeId);
		Status = XST_SUCCESS;
		break;
	}
done:
	return Status;
}

/**
 * @brief Restore all nodes. This is the top level function call for restoring ByteBuffer
 *
 * @return XST_SUCCESS if there's no error.
 *	   XST_NO_FEATURE if there's no implementation of restore function for given nodeID
 */
XStatus XPmUpdate_RestoreAllNodes(void)
{
	XStatus Status = XST_FAILURE;
	XPm_Node* FailedNode = NULL;
	Status = AddMissingPlDevices();
	if (XST_SUCCESS != Status) {
		goto done;
	}
	Status = AddMissingMemRegnDevices();
	if (XST_SUCCESS != Status) {
		goto done;
	}
	Status = AddMissingxGGsDevices();
	if (XST_SUCCESS != Status) {
		goto done;
	}
	/* Iterating through all Nodes */
	for (u32 i = 0; i < NumNodes; i++) {
		XPm_Node* node = GetNodeAt(i);
		if (NULL == node){
			/* Empty Node */
			continue;
		}
		u32 NodeId = node->Id;
		XPm_Node* SavedNode = (XPm_Node*) XPmUpdate_GetSavedDataById(node->Id);
		if (NULL == SavedNode) {
			/* Not found in Saved Data */
			continue;
		}
		switch(NODECLASS(NodeId)){
		case XPM_NODECLASS_SUBSYSTEM:
			Status = RESTORE(XPm_Subsystem, SavedNode, node);
			break;
		case XPM_NODECLASS_POWER:
			Status = Class_Power_Restore((XPm_Power*)SavedNode, (XPm_Power*)node);
			break;
		case XPM_NODECLASS_CLOCK:
			Status = Class_Clock_Restore((XPm_ClockNode*)SavedNode, (XPm_ClockNode*)node);
			break;
		case XPM_NODECLASS_RESET:
			Status = RESTORE(XPm_ResetNode, SavedNode, node);
			break;
		case XPM_NODECLASS_STMIC:
			Status = RESTORE(XPm_PinNode, SavedNode, node);
			Status = XST_SUCCESS;
			break;
		case XPM_NODECLASS_DEVICE:
			Status = Class_Device_Restore((XPm_Device*)SavedNode,(XPm_Device*)node);
			break;
		case XPM_NODECLASS_ISOLATION:
			Status = RESTORE(XPm_Iso, SavedNode, node);
			break;
		case XPM_NODECLASS_MEMIC:
		case XPM_NODECLASS_PROTECTION:
		case XPM_NODECLASS_EVENT:
		case XPM_NODECLASS_MONITOR:
		case XPM_NODECLASS_REGNODE:
		default:
			PmWarn("Warning: IPU unsupport node. NodeId: %x.\n\r", NodeId);
			Status = XST_SUCCESS;
		}
		if (XST_SUCCESS != Status) {
			FailedNode = node;
			goto done;
		}
	}
	/* Restore Error Event Nodes */
	Status = XPmNotifier_RestoreErrorEvents();
done:
	XPM_UPDATE_THROW_IF_ERROR(Status, FailedNode);
	return Status;
}

XStatus XPmUpdate_ResetPsm(void){
	XStatus Status = XST_FAILURE;

	XPmUpdate_PsmUpdateSetState(PSM_UPDATE_STATE_LOAD_ELF_DONE);
	/* Wakeup PSM */
	PmRmw32(CRL_PSM_RST_MODE_ADDR, CRL_PSM_RST_WAKEUP_MASK, CRL_PSM_RST_WAKEUP_MASK);
	/* Wait for PSMFW to initialize */
	Status = XPm_PollForMask(PSMX_GLOBAL_REG_GLOBAL_CNTRL,
				 PSMX_GLOBAL_REG_GLOBAL_CNTRL_FW_IS_PRESENT_MASK,
				 XPM_UPDATE_PSM_RST_TIMEOUT);
	if (XST_SUCCESS != Status) {
		PmErr("PSMFW is not present after reset\n\r");
		goto done;
	}

	/* Deassert Wakeup pin on PSM*/
	PmRmw32(CRL_PSM_RST_MODE_ADDR, CRL_PSM_RST_WAKEUP_MASK, 0);

done:
	return Status;
}

void XPmUpdate_PsmUpdateSetState(u32 State)
{
	Xil_Out32(PSM_UPDATE_REG_STATE, State);
}

u32 XPmUpdate_PsmUpdateGetState()
{
	return Xil_In32(PSM_UPDATE_REG_STATE);
}

XStatus XPmUpdate_ShutdownPsm() {
	XStatus Status = XST_FAILURE;
	u32 Payload[8] = {0U};

	if (1U != XPmPsm_FwIsPresent()) {
		Status = XST_NOT_ENABLED;
		PmErr("PSMFW is not present\n\r");
		goto done;
	}

	/**  Disable all PSM interrupts*/
	Payload[0U] = PSM_API_SHUTDOWN_PSM;
	Status = XPm_IpiSend(PSM_IPI_INT_MASK, Payload);
	if (XST_SUCCESS != Status) {
		PmErr("Failed to send IPI to PSM\n\r");
		goto done;
	}
	Status = XPm_IpiReadStatus(PSM_IPI_INT_MASK);
	if (XST_SUCCESS != Status) {
		PmErr("Failed to read IPI status from PSM\n\r");
		goto done;
	}

	Status = XPm_PollForMask(PSM_UPDATE_REG_STATE,
				 PSM_UPDATE_STATE_SHUTDOWN_DONE,
				 XPM_UPDATE_PSM_RST_TIMEOUT);
	if (XST_SUCCESS != Status) {
		PmErr("PSMFW is not shutting down\n\r");
		goto done;
	}

	/* Ecc initialize partial PSM RAM */
	Status = XPlmi_EccInit(XPM_PSM_RAM_BASE_ADDR, XPM_PSM_RAM_SIZE - XPM_PSM_FW_RESERVED_SIZE);
	if (XST_SUCCESS != Status) {
		PmErr("PSM failed to clear memory.\n\r");
		goto done;
	}
done:
	return Status;

}

/*****************************************************************************/
/**
 * @brief	This function provides update handler for xilpm
 *
 * @param	Op is the module operation variable
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPmUpdate_ShutdownHandler(XPlmi_ModuleOp Op)
{

	XStatus Status = XST_FAILURE;
	static u8 GenericHandlerState = XPLMI_MODULE_NORMAL_STATE;

	if (XPLMI_MODULE_SHUTDOWN_INITIATE == Op.Mode) {
		if (XPLMI_MODULE_NORMAL_STATE == GenericHandlerState ) {
#ifndef VERSAL_2VE_2VM
#ifdef XPLMI_IPI_DEVICE_ID
			/** Remove check PSM alive task */
			Status = XPlm_RemoveKeepAliveTask();
			if (XST_SUCCESS != Status) {
				goto done;

			}
#endif
			Status = XPmUpdate_ShutdownPsm();
			if (XST_SUCCESS != Status) {
				XPlmi_Printf(DEBUG_GENERAL, "PSM shutdown failed\n\r");
				goto done;
			}
#endif
			GenericHandlerState = XPLMI_MODULE_SHUTDOWN_INITIATED_STATE;
			Status = XST_SUCCESS;
		}
	}
	else if (XPLMI_MODULE_SHUTDOWN_COMPLETE == Op.Mode) {
		if (XPLMI_MODULE_SHUTDOWN_COMPLETED_STATE == GenericHandlerState) {
			Status = XST_SUCCESS;
			goto done;
		}
		if (XPLMI_MODULE_SHUTDOWN_INITIATED_STATE != GenericHandlerState) {
			goto done;
		}
		/* DO NOTHING */
		GenericHandlerState = XPLMI_MODULE_SHUTDOWN_COMPLETED_STATE;
		Status = XST_SUCCESS;
	}
	else if (XPLMI_MODULE_SHUTDOWN_ABORT == Op.Mode) {
		if (XPLMI_MODULE_SHUTDOWN_INITIATED_STATE == GenericHandlerState) {
			GenericHandlerState = XPLMI_MODULE_NORMAL_STATE;
			/* DO NOTHING */
			Status = XST_SUCCESS;
		}
	} else {
		/* Do Nothing */
	}

done:
	return Status;
}
